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
    machine = platform.machine()
    if machine == "x86_64" or machine == "AMD64":
        return "x64"
    elif machine == "aarch64":
        return "arm64"
    else:
        raise NameError(f"Unsupported platform: {machine}")


def get_default_compiler():
    if get_os() == "windows":
        return "msvc"
    elif get_os() == "linux":
        return "gcc"
    elif get_os() == "macos":
        return "clang"
    else:
        raise NameError(f"Unsupported OS: {get_os()}")


def run_command(command, shell=True):
    if get_os() == "windows":
        command = command.replace("/", "\\")
    print(f'Running "{command}" ...')
    sys.stdout.flush()
    result = subprocess.run(command, shell=shell)
    if result.returncode != 0:
        raise RuntimeError(f'Error running "{command}"')
    return result


def get_changed_env(command):
    if get_os() == "windows":
        command = command.replace("/", "\\") + " && set"
    else:
        command += " && env"
    result = subprocess.run(command, capture_output=True, text=True, shell=True)
    if result.returncode != 0:
        raise NameError(f'Error running "{command}"')
    env_vars = {}
    for line in result.stdout.splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            curr = os.getenv(key)
            if curr is None or curr != value:
                env_vars[key] = value
    return env_vars


def set_python_env_vars(args):
    if args.os == "windows":
        env_vars = get_changed_env(f"{args.bin_dir}/setpath.bat")
    else:
        env_vars = get_changed_env(f". {args.bin_dir}/setpath.sh")
    os.environ.update(env_vars)


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
    run_command(f"{args.bin_dir}/sgl_tests -r=console")


def unit_test_python(args):
    set_python_env_vars(args)
    run_command(f"pytest src -r a --junit-xml=pytest-junit.xml")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--os", type=str, action="store", help="OS (windows, linux, macos)"
    )
    parser.add_argument(
        "--platform", type=str, action="store", help="Platform (x64, arm64)"
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
        preset += "-" + args["platform"]
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
