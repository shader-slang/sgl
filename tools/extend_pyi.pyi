from typing import Optional
import libcst as cst
from pathlib import Path

code = open(
    Path(__file__).parent / "../build/windows-vs2022/bin/Debug/python/sgl/__init__.pyi"
).read()

stub_tree = cst.parse_module(code)

class FindConvertableToDictionaryTypes(cst.CSTVisitor):
    def __init__(self):
        super().__init__()
        self.stack: list[cst.ClassDef] = []

    def visit_ClassDef(self, node: cst.ClassDef) -> Optional[bool]:
        self.stack.append(node)

    def leave_ClassDef(self, original_node: cst.ClassDef) -> None:  # type: ignore
        self.stack.pop()

    def visit_FunctionDef(self, node: cst.FunctionDef) -> Optional[bool]:
        # Only looking at functions inside classes 1 level deep
        if len(self.stack) != 1:
            return False
        # Only looking at __init__ functions
        if node.name.value != "__init__":
            return False
        func_params = node.params.params
        # if len(func_params) != 2:
        #    return False
        # if func_params[0].name.value != "self":
        #    return False

        if self.stack[0].name.value != "ComputePipelineDesc":
            return False

        # annotation = func_params[1].annotation
        # if annotation is None:
        #    return False
        # if not isinstance(annotation, cst.Annotation):
        #    return False
        # if not isinstance(annotation.annotation, cst.Name):
        #    return False
        # print(annotation.annotation.value)

        # if annotation.annotation.name.value == "Dict":
        #    print("x")

        pass

    def leave_FunctionDef(self, original_node: cst.FunctionDef) -> None:  # type: ignore
        pass

class TypingTransformer(cst.CSTTransformer):
    def __init__(self):
        super().__init__()
        self.stack: list[cst.ClassDef] = []

    def visit_ClassDef(self, node: cst.ClassDef) -> Optional[bool]:
        self.stack.append(node)

    def leave_ClassDef(  # type: ignore
        self, original_node: cst.ClassDef, updated_node: cst.ClassDef
    ) -> cst.CSTNode:
        self.stack.pop()
        return updated_node

findconv = FindConvertableToDictionaryTypes()
stub_tree.visit(findconv)

transformer = TypingTransformer()

stub_tree.visit(transformer)

print("x")
