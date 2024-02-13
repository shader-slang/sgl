import re
import importlib
from inspect import isclass, ismodule, ismethod, isfunction
from pathlib import Path

INDENT = "    "

SECTIONS = {
    "Core": [
        "kali.Object",
        "kali.Bitmap",
        "kali.Struct",
        "kali.StructConverter",
        "kali.Timer",
    ],
    "Logging": [
        "kali.LogLevel",
        "kali.LogFrequency",
        "kali.Logger",
        "kali.LoggerOutput",
        "kali.ConsoleLoggerOutput",
        "kali.FileLoggerOutput",
        "kali.DebugConsoleLoggerOutput",
        r"kali\.log[\w]*",
    ],
    "Windowing": [
        "kali.WindowMode",
        "kali.Window",
        "kali.MouseButton",
        "kali.MouseButton",
        "kali.KeyModifierFlags",
        "kali.KeyModifier",
        "kali.KeyCode",
        "kali.KeyboardEventType",
        "kali.KeyboardEvent",
        "kali.MouseEventType",
        "kali.MouseEvent",
        "kali.GamepadEventType",
        "kali.GamepadButton",
        "kali.GamepadEvent",
        "kali.GamepadState",
    ],
    "Platform": [r"kali\.platform\.[\w]+"],
    "Threading": [r"kali\.thread\.[\w]+"],
    "Device": [],
    "Math": [
        r"kali\.math\.float[\d]",
        r"kali\.math\.int[\d]",
        r"kali\.math\.uint[\d]",
        r"kali\.math\.bool[\d]",
        r"kali\.math\.float16_t[\d]",
        r"kali\.math\.float[\d]x[\d]",
        r"kali\.math\.[\w]+",
    ],
    "UI": [r"kali\.ui\.[\w]+"],
    "Utilities": [r"kali\.utils\.[\w]+"],
}


def parse_signature(signature: str):
    # print(f"old signature: {signature}")

    name, rest = signature.split("(", maxsplit=1)
    args, return_type = rest.split(") -> ")

    slash_arg = False
    if len(args) > 3 and args[-1] == "/":
        slash_arg = True
        args = args[:-3]

    new_signature = f"{name}("

    # split items by "arg: ""
    items = re.split(r"([a-zA-Z\_0-9]+): ", args)

    # handle first argument which is either "" or "self"
    first_arg = True
    if len(items) > 0:
        if "self" in items[0]:
            new_signature += "self"
            first_arg = False
        items.pop(0)

    args = []

    for i in range(len(items) // 2):
        arg_name = items[i * 2].strip()
        arg_type = items[i * 2 + 1].strip()
        arg_default = None
        if "=" in arg_type:
            arg_type, arg_default = [
                s.strip() for s in arg_type.rsplit("=", maxsplit=1)
            ]
            if arg_default[-1] == ",":
                arg_default = arg_default[:-1]

        args.append((arg_name, arg_type, arg_default))

        if not first_arg:
            new_signature += ", "
        new_signature += arg_name
        if arg_default:
            new_signature += f"={arg_default}"
        first_arg = False

    if slash_arg:
        new_signature += ", /"

    new_signature += f") -> {return_type}"

    # print(args)
    # print(f"new signature: {new_signature}")

    return new_signature


def split_signature_doc(doc: str):
    lines = doc.split("\n")
    signatures = []
    docs = []
    signature_index = 0
    state = 0
    for line in lines:
        # Read signatures
        if state == 0:
            if line == "":
                state = 1
            else:
                signatures.append(line)
                docs.append("")
        # Parse docs
        elif state == 1:
            if line == "Overloaded function.":
                continue
            if any(s in line for s in signatures):
                for i in range(len(signatures)):
                    if signatures[i] in line:
                        signature_index = i
                        break
                continue
            docs[signature_index] += line + "\n"

    signatures = [parse_signature(signature) for signature in signatures]
    docs = [doc.strip() for doc in docs]
    return list(zip(signatures, docs))


class Context:
    def __init__(self):
        self.level = 0
        self.prefix = ""
        self.stack = []
        self.enable_write = True
        self.output = ""
        self.entries = {"*": ""}
        self.current_entry = "*"

    def write(self, text: str):
        lines = text.split("\n")
        for line in lines:
            self.output += f"{INDENT * self.level}{line}\n"
            self.entries[self.current_entry] += f"{INDENT * self.level}{line}\n"

    def push(self, name, indent=True):
        self.stack.append((self.level, self.prefix))
        if indent:
            self.level += 1
        self.prefix = name if self.prefix == "" else f"{self.prefix}.{name}"

    def pop(self):
        self.level, self.prefix = self.stack.pop()

    def new_entry(self, name):
        self.current_entry = f"{self.prefix}.{name}"
        if self.current_entry in self.entries:
            raise "duplicate entry"
        self.entries[self.current_entry] = ""


def process_method(obj, name, ctx: Context):
    first = True
    for signature, doc in split_signature_doc(obj.__doc__):
        ctx.write(f".. py:method:: {signature}")
        if not first:
            ctx.write(f"{INDENT}:no-index:")
        ctx.write("")
        if doc:
            ctx.write(doc + "\n")
        first = False


def process_property(obj, name, ctx: Context):
    ctx.write(f".. py:property:: {name}")
    read_write = obj.fset != None
    doc = obj.fget.__doc__.split("\n")
    type = doc[0].split("->")[1].strip()
    doc = "\n".join(doc[1:]).strip()
    ctx.write(f"{INDENT}:type: {type}\n")
    if doc != "":
        ctx.write(doc + "\n")


def process_class(obj, name, ctx: Context):
    ctx.write(f".. py:class:: {ctx.prefix}.{name}\n")
    ctx.push(name)
    base = obj.__base__
    if base.__name__ != "object":
        ctx.write(f"Base class: :py:class:`{base.__module__}.{base.__name__}`\n")

    if isinstance(obj.__doc__, str):
        ctx.write(obj.__doc__ + "\n")

    for cn in obj.__dict__:
        # Skip properties
        if re.match(r"__[a-zA-Z\_0-9]+__", cn) and cn != "__init__":
            continue
        # Skip private attributes
        if re.match(r"_[a-zA-Z\_0-9]+", cn) and cn != "__init__":
            continue
        co = getattr(obj, cn)
        if "nanobind.nb_method" in str(co):
            process_method(co, cn, ctx)
        elif isinstance(co, property):
            process_property(co, cn, ctx)

    # Handle enum values
    if hasattr(obj, "@entries"):
        entries = getattr(obj, "@entries")
        for value, info in entries.items():
            ctx.write(f".. py:attribute:: {info[0]}")
            ctx.write(f"{INDENT}:value: {value}")
            ctx.write("")
            if info[1]:
                ctx.write(info[1] + "\n")

    ctx.pop()


def process_function(obj, name, ctx: Context):
    first = True
    for signature, doc in split_signature_doc(obj.__doc__):
        ctx.write(f".. py:function:: {ctx.prefix}.{signature}")
        if not first:
            ctx.write(f"{INDENT}:no-index:")
        ctx.write("")
        if doc:
            ctx.write(doc + "\n")
        first = False


def process_module(obj, name, ctx: Context):
    ctx.push(name, indent=False)
    ctx.write(ctx.prefix + "\n\n")

    for cn in obj.__dict__:
        co = getattr(obj, cn)
        if ismodule(co):
            process_module(co, cn, ctx)
        elif isclass(co):
            ctx.new_entry(cn)
            process_class(co, cn, ctx)
        # elif isfunction(co):
        #     process_function(co, cn, ctx)
        elif "nanobind.nb_func" in str(co):
            ctx.new_entry(cn)
            process_function(co, cn, ctx)

    ctx.pop()


def generate_api():
    ctx = Context()
    module = importlib.import_module("kali")
    process_module(module, "kali", ctx)
    # print(ctx.output)

    out = ""
    entries = ctx.entries
    added_entries = set()
    for section_name, patterns in SECTIONS.items():
        out += f"{section_name}\n"
        out += "-" * len(section_name) + "\n\n"

        for pattern in patterns:
            for entry in entries:
                if entry in added_entries:
                    continue
                if re.fullmatch(pattern, entry):
                    out += entries[entry] + "\n"
                    out += "\n----\n\n"
                    added_entries.add(entry)

    out += "Miscellaneous\n"
    out += "-------------\n\n"
    for entry in entries:
        if entry in added_entries:
            continue
        out += entries[entry] + "\n"
        out += "\n----\n\n"
        print(f"Unassigned entry: {entry}")

    # Write file if it changed.
    api_path = Path(__file__).parent / "generated" / "api.rst"
    api_path.parent.mkdir(exist_ok=True)
    if not api_path.exists() or api_path.read_text() != out:
        api_path.write_text(out)


if __name__ == "__main__":
    generate_api()
