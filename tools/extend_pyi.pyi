from typing import Optional, Union
import libcst as cst
from pathlib import Path
from libcst import parse_expression

# List of types we expect to discover that can be constructed from
# a dictionary. If 'True', a corresponding TypedDict and Union type
# will be generated. If 'False', it will be ignored. If a type
# is discovered but not in this list it will be treated as an error.
DESCRIPTOR_CONVERT_TYPES = {
    "AppDesc": True,
    "AppWindowDesc": True,
    "DeviceDesc": True,
    "FenceDesc": True,
    "FramebufferLayoutTargetDesc": True,
    "FramebufferLayoutDesc": True,
    "FramebufferDesc": True,
    "InputElementDesc": True,
    "VertexStreamDesc": True,
    "InputLayoutDesc": True,
    "ComputePipelineDesc": True,
    "GraphicsPipelineDesc": True,
    "HitGroupDesc": True,
    "RayTracingPipelineDesc": True,
    "QueryPoolDesc": True,
    "ShaderTableDesc": True,
    "BufferDesc": True,
    "TextureDesc": True,
    "SamplerDesc": True,
    "SlangCompilerOptions": True,
    "SlangLinkOptions": True,
    "SlangSessionDesc": True,
    "SwapchainDesc": True,
    "Viewport": True,
    "ScissorRect": True,
    "DepthStencilDesc": True,
    "RasterizerDesc": True,
    "AspectBlendDesc": True,
    "TargetBlendDesc": True,
    "BlendDesc": True,
    "TextureLoader.Options": True,
}

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

def insert_nodes_after_class_by_name(
    body_nodes: list[cst.CSTNode], class_name: str, nodes_to_insert: list[cst.CSTNode]
):
    insert_idx = 0
    while insert_idx < len(body_nodes):
        bn = body_nodes[insert_idx]
        if isinstance(bn, cst.ClassDef) and bn.name.value == class_name:
            break
        insert_idx += 1
    insert_idx += 1
    for x in nodes_to_insert:
        body_nodes.insert(insert_idx, x)
        insert_idx += 1

class FCDFieldInfo:
    def __init__(self, name: cst.Name, annotation: cst.BaseExpression):
        self.name = name
        self.annotation = annotation

class FCDStackInfo:
    def __init__(
        self,
        parent_classs_type: cst.ClassDef | None,
        class_type: cst.ClassDef,
        full_name: str,
    ):
        self.parent_class_type = parent_classs_type
        self.class_type = class_type
        self.valid = False
        self.fields: list[FCDFieldInfo] = []
        self.full_name = full_name

class FindConvertableToDictionaryTypes(cst.CSTVisitor):
    def __init__(self):
        super().__init__()
        self.stack: list[FCDStackInfo] = []
        self.results: list[FCDStackInfo] = []
        self.read_class = False
        self.name_stack: list[str] = []

    def visit_ClassDef(self, node: cst.ClassDef) -> Optional[bool]:
        parent = self.stack[-1].class_type if len(self.stack) > 0 else None
        self.name_stack.append(node.name.value)
        self.stack.append(FCDStackInfo(parent, node, ".".join(self.name_stack)))

    def leave_ClassDef(self, original_node: cst.ClassDef) -> None:  # type: ignore
        self.stack.pop()
        self.name_stack.pop()

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

class InsertTypesTransformer(cst.CSTTransformer):
    def __init__(self, discovered_descriptor_types: list[FCDStackInfo]):
        super().__init__()
        self.discovered_descriptor_types = discovered_descriptor_types

    def leave_Module(
        self, original_node: cst.Module, updated_node: cst.Module
    ) -> cst.Module:
        # On leaving a module, insert the dictionary and union types for any global descriptor types
        new_body: list[cst.CSTNode] = list(updated_node.body)
        changed = self._insert_descriptor_nodes(None, new_body)
        if changed:
            return updated_node.with_changes(body=new_body)
        return updated_node

    def leave_ClassDef(  # type: ignore
        self, original_node: cst.ClassDef, updated_node: cst.ClassDef
    ) -> cst.CSTNode:
        # On leaving a class, insert the dictionary and union types for any child descriptor types
        new_body: list[cst.CSTNode] = list(updated_node.body.body)
        changed = self._insert_descriptor_nodes(original_node, new_body)
        if changed:
            class_body = updated_node.body
            class_body = class_body.with_changes(body=new_body)
            return updated_node.with_changes(body=class_body)

        return updated_node

    def _insert_descriptor_nodes(
        self, original_parent: cst.ClassDef | None, updated_body: list[cst.CSTNode]
    ) -> bool:
        # Finds any descriptor classes that are children of the specified parent (or None for module)
        # and inserts the dictionary and union types after the corresponding class definition.
        changed = False
        for desc_type in self.discovered_descriptor_types:
            if desc_type.parent_class_type == original_parent:
                dict_type, union_type = self._build_types_for_descriptor(desc_type)
                insert_nodes_after_class_by_name(
                    updated_body,
                    desc_type.class_type.name.value,
                    [cst.Newline(), dict_type, cst.Newline(), union_type],
                )
                changed = True
        return changed

    def _build_types_for_descriptor(self, result: FCDStackInfo):
        # Generates the dictionary and union types for a descriptor class.

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
                                last_line=cst.SimpleWhitespace("    "), indent=True
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
                    last_line=cst.SimpleWhitespace("    "), indent=True
                )
            ),
            rbrace=cst.RightCurlyBrace(
                whitespace_before=cst.ParenthesizedWhitespace(indent=True)
            ),
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
        type_alias = cst.SimpleStatementLine(
            body=[
                cst.Assign(
                    targets=[
                        cst.AssignTarget(
                            target=cst.Name(f"{result.class_type.name.value}Dict")
                        )
                    ],
                    value=typed_dict_annotation,
                )
            ]
        )

        # Also generate a corresponding union that combines the source type with the new TypedDict
        union_type_alias = cst.SimpleStatementLine(
            body=[
                cst.Assign(
                    targets=[
                        cst.AssignTarget(
                            target=cst.Name(f"{result.class_type.name.value}Type")
                        )
                    ],
                    value=cst.Subscript(
                        value=cst.Name("Union"),
                        slice=[
                            cst.SubscriptElement(
                                slice=cst.Index(
                                    value=cst.Name(result.class_type.name.value)
                                )
                            ),
                            cst.SubscriptElement(
                                slice=cst.Index(
                                    value=cst.Name(
                                        f"{result.class_type.name.value}Dict"
                                    )
                                )
                            ),
                        ],
                    ),
                )
            ]
        )

        return type_alias, union_type_alias

findconv = FindConvertableToDictionaryTypes()
stub_tree.visit(findconv)

for result in findconv.results:
    if not result.full_name in DESCRIPTOR_CONVERT_TYPES:
        raise Exception(
            f"Discovered descriptor class {result.full_name}, but not in list of types to convert."
        )
for conv_type in DESCRIPTOR_CONVERT_TYPES:
    if not any(result.full_name == conv_type for result in findconv.results):
        raise Exception(
            f"Type {conv_type} is in list of types to convert but no corresponding descriptor class was discovered."
        )

transformer = InsertTypesTransformer(findconv.results)

new_tree = stub_tree.visit(transformer)

open(
    Path(__file__).parent
    / "../build/windows-vs2022/bin/Debug/python/sgl/__init__2.pyi",
    "w",
).write(new_tree.code)

pass
