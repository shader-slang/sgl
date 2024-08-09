"""
Script for running CI tasks.
"""

import os
import sys
import platform
import argparse
import subprocess
import json


def get_os():
    """
    Return the OS name (windows, linux, macos).
    """
    platform = sys.platform
    if platform == "win32":
        return "windows"
    elif platform == "linux" or platform == "linux2":
        return "linux"
    elif platform == "darwin":
        return "macos"
    else:
        raise NameError(f"Unsupported OS: {sys.platform}")


def get_platform():
    """
    Return the platform name (x86_64, aarch64).
    """
    machine = platform.machine()
    if machine == "x86_64" or machine == "AMD64":
        return "x86_64"
    elif machine == "aarch64":
        return "aarch64"
    else:
        raise NameError(f"Unsupported platform: {machine}")


def get_default_compiler():
    """
    Return the default compiler name for the current OS (msvc, gcc, clang).
    """
    if get_os() == "windows":
        return "msvc"
    elif get_os() == "linux":
        return "gcc"
    elif get_os() == "macos":
        return "clang"
    else:
        raise NameError(f"Unsupported OS: {get_os()}")


def run_command(command, shell=True, env=None):
    if get_os() == "windows":
        command = command.replace("/", "\\")
    if env != None:
        new_env = os.environ.copy()
        new_env.update(env)
        env = new_env
    print(f'Running "{command}" ...')
    sys.stdout.flush()

    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        universal_newlines=True,
        shell=shell,
        env=env,
    )

    out = ""
    while True:
        nextline = process.stdout.readline()
        if nextline == "" and process.poll() is not None:
            break
        sys.stdout.write(nextline)
        sys.stdout.flush()
        out += nextline

    process.communicate()
    if process.returncode != 0:
        raise RuntimeError(f'Error running "{command}"')

    return out


def get_changed_env(command):
    if get_os() == "windows":
        command = command.replace("/", "\\") + " && set"
    else:
        command = command + " && env"
    result = subprocess.run(
        command,
        capture_output=True,
        text=True,
        shell=True,
        executable="/bin/bash" if get_os() != "windows" else None,
    )
    if result.returncode != 0:
        raise NameError(f'Error running "{command}"')
    env_vars = {}
    for line in result.stdout.splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            if key in ["_"]:
                continue
            curr = os.getenv(key)
            if curr is None or curr != value:
                env_vars[key] = value
    return env_vars


def get_python_env(args):
    if args.os == "windows":
        return get_changed_env(f"{args.bin_dir}/setpath.bat")
    else:
        return get_changed_env(f"source {args.bin_dir}/setpath.sh")


def setup(args):
    if args.os == "windows":
        run_command("./setup.bat")
    else:
        run_command("./setup.sh")


def configure(args):
    run_command(f"{args.cmake} --preset {args.preset}")


def build(args):
    run_command(f"{args.cmake} --build build/{args.preset} --config {args.config}")


def unit_test_cpp(args):
    out = run_command(f"{args.bin_dir}/sgl_tests -r=console,junit")
    # doctest outputs both regular output and junit xml report on stdout
    # filter out regular output and write remaining to junit xml file
    report = "\n".join(
        filter(lambda line: line.strip().startswith("<"), out.splitlines())
    )
    os.makedirs("reports", exist_ok=True)
    with open("reports/doctest-junit.xml", "w") as f:
        f.write(report)


def unit_test_python(args):
    env = get_python_env(args)
    os.makedirs("reports", exist_ok=True)
    run_command(f"pytest src -r a --junit-xml=reports/pytest-junit.xml", env=env)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--os", type=str, action="store", help="OS (windows, linux, macos)"
    )
    parser.add_argument(
        "--platform", type=str, action="store", help="Platform (x86_64, aarch64)"
    )
    parser.add_argument(
        "--compiler", type=str, action="store", help="Compiler (msvc, gcc, clang)"
    )
    parser.add_argument(
        "--config", type=str, action="store", help="Config (Release, Debug)"
    )
    parser.add_argument("--python", type=str, action="store", help="Python version")
    commands = parser.add_subparsers(
        dest="command", required=True, help="sub-command help"
    )

    parser_setup = commands.add_parser("setup", help="run setup.bat or setup.sh")

    parser_configure = commands.add_parser("configure", help="run cmake configure")

    parser_build = commands.add_parser("build", help="run cmake build")

    parser_test_cpp = commands.add_parser("unit-test-cpp", help="run unit tests (c++)")

    parser_test_python = commands.add_parser(
        "unit-test-python", help="run unit tests (python)"
    )

    args = parser.parse_args()
    args = vars(args)

    VARS = [
        ("os", "CI_OS", get_os()),
        ("platform", "CI_PLATFORM", get_platform()),
        ("compiler", "CI_COMPILER", get_default_compiler()),
        ("config", "CI_CONFIG", "Debug"),
        ("python", "CI_PYTHON", "3.9"),
    ]

    for var, env_var, default_value in VARS:
        if not var in args or args[var] == None:
            args[var] = os.environ[env_var] if env_var in os.environ else default_value

    # Determine cmake executable path.
    args["cmake"] = {
        "windows": "./tools/host/cmake/bin/cmake.exe",
        "linux": "./tools/host/cmake/bin/cmake",
        "macos": "./tools/host/cmake/CMake.app/Contents/bin/cmake",
    }[args["os"]]

    # Determine cmake preset.
    preset = args["os"] + "-" + args["compiler"]
    if args["os"] == "macos":
        if args["platform"] == "x86_64":
            preset += "-x64"
        elif args["platform"] == "aarch64":
            preset += "-arm64"
    args["preset"] = preset

    # Determine binary directory.
    bin_dir = f"./build/{args['preset']}/bin/{args['config']}"
    args["bin_dir"] = bin_dir

    print("CI configuration:")
    print(json.dumps(args, indent=4))

    args = argparse.Namespace(**args)

    {
        "setup": setup,
        "configure": configure,
        "build": build,
        "unit-test-cpp": unit_test_cpp,
        "unit-test-python": unit_test_python,
    }[args.command](args)

    return 0


if __name__ == "__main__":
    main()
