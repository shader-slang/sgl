from typing import Optional, Union
import libcst as cst
from pathlib import Path
from libcst import parse_expression

code = open(
    Path(__file__).parent / "../build/windows-vs2022/bin/Debug/python/sgl/__init__.pyi"
).read()

stub_tree = cst.parse_module(code)

def name_equals(expression: Optional[cst.BaseExpression], name: str):
    if isinstance(expression, cst.Name):
        return expression.value == name  # type: ignore
    return False

def annotation_name_equals(annotation: Optional[cst.Annotation], name: str):
    if isinstance(annotation, cst.Annotation):
        return name_equals(annotation.annotation, name)  # type: ignore
    return False

def is_setter(node: cst.FunctionDef):
    for decorator in node.decorators:
        if (
            isinstance(decorator.decorator, cst.Attribute)
            and decorator.decorator.attr.value == "setter"
        ):
            return True
    return False

class FCDFieldInfo:
    def __init__(self, name: cst.Name, annotation: cst.BaseExpression):
        self.name = name
        self.annotation = annotation

class FCDStackInfo:
    def __init__(
        self, porent_class_type: cst.ClassDef | None, class_type: cst.ClassDef
    ):
        self.parent_class_type = porent_class_type
        self.class_type = class_type
        self.valid = False
        self.fields: list[FCDFieldInfo] = []

def build_types_for_descriptor(descriptor: FCDStackInfo):

    # Generate the elements for the dictionary that defines the
    # new TypedDict. This is really just an element with name:annotation
    # for each entry, but looks more complex due to insertion of correct
    # whitespace (each element other than the last adds a newline and
    # 4 spaces after the comma)
    dict_elements: list[cst.DictElement] = []
    for idx in range(len(result.fields)):
        field = result.fields[idx]
        if idx < len(result.fields) - 1:
            dict_elements.append(
                cst.DictElement(
                    key=cst.SimpleString(f'"{field.name.value}"'),
                    value=field.annotation,
                    comma=cst.Comma(
                        whitespace_after=cst.ParenthesizedWhitespace(
                            last_line=cst.SimpleWhitespace("    ")
                        )
                    ),
                )
            )
        else:
            dict_elements.append(
                cst.DictElement(
                    key=cst.SimpleString(f'"{field.name.value}"'),
                    value=field.annotation,
                )
            )

    # Generate the dictionary definition. Formatting adds a new line
    # and 4 spaces after the opening brace and a new line before the closing
    # brace.
    dict_def = cst.Dict(
        elements=dict_elements,
        lbrace=cst.LeftCurlyBrace(
            whitespace_after=cst.ParenthesizedWhitespace(
                last_line=cst.SimpleWhitespace("    ")
            )
        ),
        rbrace=cst.RightCurlyBrace(whitespace_before=cst.ParenthesizedWhitespace()),
    )

    # Wrap the dictionary in a call to 'TypedDict' to create the new type
    typed_dict_annotation = cst.Call(
        func=cst.Name("TypedDict"),
        args=[
            cst.Arg(
                value=cst.SimpleString(f'"{result.class_type.name.value}Dict"'),
            ),
            cst.Arg(value=dict_def),
        ],
    )

    # Store the new TypedDict in a type alias
    type_alias = cst.Assign(
        targets=[
            cst.AssignTarget(target=cst.Name(f"{result.class_type.name.value}Dict"))
        ],
        value=typed_dict_annotation,
    )

    # Also generate a corresponding union that combines the source type with the new TypedDict
    union_type_alias = cst.Assign(
        targets=[
            cst.AssignTarget(target=cst.Name(f"{result.class_type.name.value}Type"))
        ],
        value=cst.Subscript(
            value=cst.Name("Union"),
            slice=[
                cst.SubscriptElement(
                    slice=cst.Index(value=cst.Name(result.class_type.name.value))
                ),
                cst.SubscriptElement(
                    slice=cst.Index(
                        value=cst.Name(f"{result.class_type.name.value}Dict")
                    )
                ),
            ],
        ),
    )

    return type_alias, union_type_alias

class FindConvertableToDictionaryTypes(cst.CSTVisitor):
    def __init__(self):
        super().__init__()
        self.stack: list[FCDStackInfo] = []
        self.results: list[FCDStackInfo] = []
        self.read_class = False

    def visit_ClassDef(self, node: cst.ClassDef) -> Optional[bool]:
        parent = self.stack[-1].class_type if len(self.stack) > 0 else None
        self.stack.append(FCDStackInfo(parent, node))

    def leave_ClassDef(self, original_node: cst.ClassDef) -> None:  # type: ignore
        self.stack.pop()

    def visit_FunctionDef(self, node: cst.FunctionDef) -> Optional[bool]:
        if node.name.value == "__init__":
            # If this is an init function, use to check if this is a type we're interested in.
            # Function shuld have 2 positional parameters
            func_params = node.params
            if (
                len(func_params.params) != 0
                or len(func_params.kwonly_params) != 0
                or len(func_params.posonly_params) != 2
            ):
                return False
            if func_params.posonly_params[0].name.value != "self":
                return False
            if func_params.posonly_params[1].name.value != "arg":
                return False

            # Check it's a dictionary
            if not annotation_name_equals(
                func_params.posonly_params[1].annotation, "dict"
            ):
                return False

            # Store the type
            # print(self.stack[-1].class_type.name.value)
            self.stack[-1].valid = True
            self.results.append(self.stack[-1])
        elif len(self.stack) > 0 and self.stack[-1].valid:
            # Not an init function but this type is a descriptor so want to record the setter types
            if is_setter(node):
                # print("  "+node.name.value+":"+stub_tree.code_for_node(node.params.posonly_params[1]))
                self.stack[-1].fields.append(
                    FCDFieldInfo(
                        node.name, node.params.posonly_params[1].annotation.annotation
                    )
                )

class TypingTransformer(cst.CSTTransformer):
    def __init__(self, discovered_descriptor_types: list[FCDStackInfo]):
        super().__init__()
        self.stack: list[cst.ClassDef] = []
        self.discovered_descriptor_types = discovered_descriptor_types

    def visit_Module(self, node: cst.Module) -> Optional[bool]:
        return True

    def leave_Module(
        self, original_node: cst.Module, updated_node: cst.Module
    ) -> cst.Module:
        new_types: list[cst.CSTNode] = []
        for desc_type in self.discovered_descriptor_types:
            if desc_type.parent_class_type is None:
                dict_type, union_type = build_types_for_descriptor(desc_type)
                new_types.append(dict_type)
                new_types.append(cst.Newline())
                new_types.append(union_type)
                new_types.append(cst.Newline())

        if len(new_types) > 0:
            insert_idx = 0
            while insert_idx < len(original_node.body):
                if isinstance(original_node.body[insert_idx], cst.ClassDef):
                    break
                insert_idx += 1
            new_body = (
                list(original_node.body[:insert_idx])
                + new_types
                + list(original_node.body[insert_idx:])
            )
            return updated_node.with_changes(body=new_types)
        return updated_node

    def visit_ClassDef(self, node: cst.ClassDef) -> Optional[bool]:
        self.stack.append(node)

    def leave_ClassDef(  # type: ignore
        self, original_node: cst.ClassDef, updated_node: cst.ClassDef
    ) -> cst.CSTNode:
        self.stack.pop()
        new_types: list[cst.CSTNode] = []
        for desc_type in self.discovered_descriptor_types:
            if desc_type.parent_class_type == original_node:
                pass
        if len(new_types) > 0:
            class_body = original_node.body
            class_body = class_body.with_changes(body=new_types + list(class_body.body))
            return updated_node.with_changes(body=class_body)

        return updated_node

findconv = FindConvertableToDictionaryTypes()
stub_tree.visit(findconv)

transformer = TypingTransformer(findconv.results)

stub_tree.visit(transformer)

new_types: list[cst.CSTNode] = []

for result in findconv.results:

    dict_elements: list[cst.DictElement] = []
    for idx in range(len(result.fields)):
        field = result.fields[idx]
        if idx < len(result.fields) - 1:
            dict_elements.append(
                cst.DictElement(
                    key=cst.SimpleString(f'"{field.name.value}"'),
                    value=field.annotation,
                    comma=cst.Comma(
                        whitespace_after=cst.ParenthesizedWhitespace(
                            last_line=cst.SimpleWhitespace("    ")
                        )
                    ),
                )
            )
        else:
            dict_elements.append(
                cst.DictElement(
                    key=cst.SimpleString(f'"{field.name.value}"'),
                    value=field.annotation,
                )
            )

    dict_def = cst.Dict(
        elements=dict_elements,
        lbrace=cst.LeftCurlyBrace(
            whitespace_after=cst.ParenthesizedWhitespace(
                last_line=cst.SimpleWhitespace("    ")
            )
        ),
        rbrace=cst.RightCurlyBrace(whitespace_before=cst.ParenthesizedWhitespace()),
    )

    typed_dict_annotation = cst.Call(
        func=cst.Name("TypedDict"),
        args=[
            cst.Arg(
                value=cst.SimpleString(f'"{result.class_type.name.value}Dict"'),
            ),
            cst.Arg(value=dict_def),
        ],
    )

    type_alias = cst.Assign(
        targets=[
            cst.AssignTarget(target=cst.Name(f"{result.class_type.name.value}Dict"))
        ],
        value=typed_dict_annotation,
    )
    new_types.append(type_alias)
    new_types.append(cst.Newline())

    print(stub_tree.code_for_node(type_alias))

    x = Union[int, float]

    union_type_alias = cst.Assign(
        targets=[
            cst.AssignTarget(target=cst.Name(f"{result.class_type.name.value}Type"))
        ],
        value=cst.Subscript(
            value=cst.Name("Union"),
            slice=[
                cst.SubscriptElement(
                    slice=cst.Index(value=cst.Name(result.class_type.name.value))
                ),
                cst.SubscriptElement(
                    slice=cst.Index(
                        value=cst.Name(f"{result.class_type.name.value}Dict")
                    )
                ),
            ],
        ),
    )
    new_types.append(union_type_alias)
    new_types.append(cst.Newline())

    print(stub_tree.code_for_node(union_type_alias))

    pass

insert_idx = 0
while insert_idx < len(stub_tree.body):
    if isinstance(stub_tree.body[insert_idx], cst.ClassDef):
        break
    insert_idx += 1

new_imports = [
    cst.SimpleStatementLine(
        body=[
            cst.ImportFrom(
                module=cst.Name("typing"),
                names=[
                    cst.ImportAlias(name=cst.Name("TypedDict"), asname=None),
                    cst.ImportAlias(name=cst.Name("Union"), asname=None),
                ],
                whitespace_after_import=cst.SimpleWhitespace("    "),
            )
        ]
    )
]

new_body = (
    list(stub_tree.body[:insert_idx])
    + new_imports
    + new_types
    + list(stub_tree.body[insert_idx:])
)

new_tree = stub_tree.with_changes(body=new_body)

open(
    Path(__file__).parent
    / "../build/windows-vs2022/bin/Debug/python/sgl/__init__2.pyi",
    "w",
).write(stub_tree.code_for_node(stub_tree.with_changes(body=new_body)))

pass
