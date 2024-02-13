import re
import importlib
from inspect import isclass, ismodule, ismethod, isfunction
from pathlib import Path

api_list = []

INDENT = "    "


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

    docs = [doc.strip() for doc in docs]
    return list(zip(signatures, docs))


class Context:
    def __init__(self):
        self.level = 0
        self.prefix = ""
        self.stack = []
        self.enable_write = True
        self.output = ""

    def write(self, text: str):
        lines = text.split("\n")
        for line in lines:
            self.output += f"{INDENT * self.level}{line}\n"

    def push(self, name, indent=True):
        self.stack.append((self.level, self.prefix))
        if indent:
            self.level += 1
        self.prefix = name if self.prefix == "" else f"{self.prefix}.{name}"

    def pop(self):
        self.level, self.prefix = self.stack.pop()


def process_method(obj, name, ctx: Context):
    for signature, doc in split_signature_doc(obj.__doc__):
        ctx.write(f".. py:method:: {signature}\n")
        if doc:
            ctx.write(doc + "\n")


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
        if obj.__doc__:
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

    ctx.pop()


def process_function(obj, name, ctx: Context):
    for signature, doc in split_signature_doc(obj.__doc__):
        ctx.write(f".. py:function:: {ctx.prefix}.{signature}\n")
        if doc:
            ctx.write(doc + "\n")


def process_module(obj, name, ctx: Context):
    ctx.push(name, indent=False)
    ctx.write(ctx.prefix + "\n\n")

    for cn in obj.__dict__:
        co = getattr(obj, cn)
        if ismodule(co):
            process_module(co, cn, ctx)
        elif isclass(co):
            process_class(co, cn, ctx)
        # elif isfunction(co):
        #     process_function(co, cn, ctx)
        elif "nanobind.nb_func" in str(co):
            process_function(co, cn, ctx)

    ctx.pop()


def generate_api():
    ctx = Context()
    module = importlib.import_module("kali")
    process_module(module, "kali", ctx)
    # print(ctx.output)

    # Write file if it changed.
    api_path = Path(__file__).parent / "generated" / "api.rst"
    api_path.parent.mkdir(exist_ok=True)
    if not api_path.exists() or api_path.read_text() != ctx.output:
        api_path.write_text(ctx.output)


if __name__ == "__main__":
    generate_api()
