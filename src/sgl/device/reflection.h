// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/types.h"
#include "sgl/device/fwd.h"

#include "sgl/core/macros.h"
#include "sgl/core/enum.h"
#include "sgl/core/error.h"
#include "sgl/core/type_utils.h"

#include "sgl/math/vector_types.h"

#include <slang.h>

#include <map>
#include <string>
#include <vector>

namespace sgl {

namespace detail {

    ref<const DeclReflection> from_slang(ref<const Object> owner, slang::DeclReflection* decl_reflection);
    ref<const TypeReflection> from_slang(ref<const Object> owner, slang::TypeReflection* type_reflection);
    ref<const TypeLayoutReflection>
    from_slang(ref<const Object> owner, slang::TypeLayoutReflection* type_layout_reflection);
    ref<const FunctionReflection> from_slang(ref<const Object> owner, slang::FunctionReflection* variable_reflection);
    ref<const VariableReflection> from_slang(ref<const Object> owner, slang::VariableReflection* variable_reflection);
    ref<const VariableLayoutReflection>
    from_slang(ref<const Object> owner, slang::VariableLayoutReflection* variable_layout_reflection);
    ref<const EntryPointLayout> from_slang(ref<const Object> owner, slang::EntryPointLayout* entry_point_reflection);
    ref<const ProgramLayout> from_slang(ref<const Object> owner, slang::ProgramLayout* program_layout);

} // namespace detail


enum class ModifierID {
    shared = slang::Modifier::ID::Shared,
    nodiff = slang::Modifier::ID::NoDiff,
    static_ = slang::Modifier::ID::Static,
    const_ = slang::Modifier::ID::Const,
    export_ = slang::Modifier::ID::Export,
    extern_ = slang::Modifier::ID::Extern,
    differentiable = slang::Modifier::ID::Differentiable,
    mutating = slang::Modifier::ID::Mutating,
    in = slang::Modifier::ID::In,
    out = slang::Modifier::ID::Out,
    inout = slang::Modifier::ID::InOut,
};

SGL_ENUM_INFO(
    ModifierID,
    {
        {ModifierID::shared, "shared"},
        {ModifierID::nodiff, "nodiff"},
        {ModifierID::static_, "static"},
        {ModifierID::const_, "const"},
        {ModifierID::export_, "export"},
        {ModifierID::extern_, "extern"},
        {ModifierID::differentiable, "differentiable"},
        {ModifierID::mutating, "mutating"},
        {ModifierID::in, "inn"},
        {ModifierID::out, "out"},
        {ModifierID::inout, "inout"},
    }
);
SGL_ENUM_REGISTER(ModifierID);

class SGL_API BaseReflectionObject : public Object {
public:
    BaseReflectionObject(ref<const Object> owner)
        : m_owner(owner){};

protected:
    ref<const Object> m_owner;
};

class SGL_API DeclReflection : public BaseReflectionObject {

public:
    DeclReflection(ref<const Object> owner, slang::DeclReflection* target)
        : BaseReflectionObject(owner)
        , m_target(target){};


    /// Cast to non-const base pointer.
    /// The underlying slang API is not const-correct.
    slang::DeclReflection* base() const { return (slang::DeclReflection*)(this); }

    /// Different kinds of decl slang can return.
    enum class Kind {
        unsupported = SLANG_DECL_KIND_UNSUPPORTED_FOR_REFLECTION,
        struct_ = SLANG_DECL_KIND_STRUCT,
        func = SLANG_DECL_KIND_FUNC,
        module = SLANG_DECL_KIND_MODULE,
        generic = SLANG_DECL_KIND_GENERIC,
        variable = SLANG_DECL_KIND_VARIABLE,
    };
    SGL_ENUM_INFO(
        Kind,
        {
            {Kind::unsupported, "unsupported"},
            {Kind::struct_, "struct"},
            {Kind::func, "func"},
            {Kind::module, "module"},
            {Kind::generic, "generic"},
            {Kind::variable, "variable"},
        }
    );

    /// Decl kind (struct/function/module/generic/variable).
    Kind kind() const { return static_cast<Kind>(m_target->getKind()); }

    /// List of children of this cursor.
    std::vector<ref<const DeclReflection>> children() const;

    /// Get number of children.
    uint32_t child_count() const { return m_target->getChildrenCount(); }

    /// Get the name of this decl (if it is of a kind that has a name).
    /// Note: Only supported for types, functions and variables.
    std::string name() const;

    /// List of children of this cursor of a specific kind.
    std::vector<ref<const DeclReflection>> children_of_kind(Kind kind) const;

    /// Index operator to get nth child.
    ref<const DeclReflection> operator[](uint32_t index) const
    {
        return detail::from_slang(m_owner, m_target->getChild(index));
    }

    /// Description as string.
    std::string to_string() const;

    /// Get type corresponding to this decl ref.
    ref<const TypeReflection> as_type() const;

    /// Get variable corresponding to this decl ref.
    ref<const VariableReflection> as_variable() const { return detail::from_slang(m_owner, m_target->asVariable()); }

    /// Get function corresponding to this decl ref.
    ref<const FunctionReflection> as_function() const { return detail::from_slang(m_owner, m_target->asFunction()); }

    /// Finds all children of a specific kind with a given name.
    /// Note: Only supported for types, functions and variables.
    std::vector<ref<const DeclReflection>> find_children_of_kind(Kind kind, std::string_view child_name) const;

    /// Finds the first child of a specific kind with a given name.
    /// Note: Only supported for types, functions and variables.
    ref<const DeclReflection> find_first_child_of_kind(Kind kind, std::string_view child_name) const;

private:
    slang::DeclReflection* m_target;
};
SGL_ENUM_REGISTER(DeclReflection::Kind);

class SGL_API TypeReflection : public BaseReflectionObject {
public:
    enum class Kind {
        none = SLANG_TYPE_KIND_NONE,
        struct_ = SLANG_TYPE_KIND_STRUCT,
        array = SLANG_TYPE_KIND_ARRAY,
        matrix = SLANG_TYPE_KIND_MATRIX,
        vector = SLANG_TYPE_KIND_VECTOR,
        scalar = SLANG_TYPE_KIND_SCALAR,
        constant_buffer = SLANG_TYPE_KIND_CONSTANT_BUFFER,
        resource = SLANG_TYPE_KIND_RESOURCE,
        sampler_state = SLANG_TYPE_KIND_SAMPLER_STATE,
        texture_buffer = SLANG_TYPE_KIND_TEXTURE_BUFFER,
        shader_storage_buffer = SLANG_TYPE_KIND_SHADER_STORAGE_BUFFER,
        parameter_block = SLANG_TYPE_KIND_PARAMETER_BLOCK,
        generic_type_parameter = SLANG_TYPE_KIND_GENERIC_TYPE_PARAMETER,
        interface = SLANG_TYPE_KIND_INTERFACE,
        output_stream = SLANG_TYPE_KIND_OUTPUT_STREAM,
        specialized = SLANG_TYPE_KIND_SPECIALIZED,
        feedback = SLANG_TYPE_KIND_FEEDBACK,
        pointer = SLANG_TYPE_KIND_POINTER,
    };

    SGL_ENUM_INFO(
        Kind,
        {
            {Kind::none, "none"},
            {Kind::struct_, "struct"},
            {Kind::array, "array"},
            {Kind::matrix, "matrix"},
            {Kind::vector, "vector"},
            {Kind::scalar, "scalar"},
            {Kind::constant_buffer, "constant_buffer"},
            {Kind::resource, "resource"},
            {Kind::sampler_state, "sampler_state"},
            {Kind::texture_buffer, "texture_buffer"},
            {Kind::shader_storage_buffer, "shader_storage_buffer"},
            {Kind::parameter_block, "parameter_block"},
            {Kind::generic_type_parameter, "generic_type_parameter"},
            {Kind::interface, "interface"},
            {Kind::output_stream, "output_stream"},
            {Kind::specialized, "specialized"},
            {Kind::feedback, "feedback"},
            {Kind::pointer, "pointer"},
        }
    );

    enum class ScalarType {
        none_ = SLANG_SCALAR_TYPE_NONE,
        void_ = SLANG_SCALAR_TYPE_VOID,
        bool_ = SLANG_SCALAR_TYPE_BOOL,
        int32 = SLANG_SCALAR_TYPE_INT32,
        uint32 = SLANG_SCALAR_TYPE_UINT32,
        int64 = SLANG_SCALAR_TYPE_INT64,
        uint64 = SLANG_SCALAR_TYPE_UINT64,
        float16 = SLANG_SCALAR_TYPE_FLOAT16,
        float32 = SLANG_SCALAR_TYPE_FLOAT32,
        float64 = SLANG_SCALAR_TYPE_FLOAT64,
        int8 = SLANG_SCALAR_TYPE_INT8,
        uint8 = SLANG_SCALAR_TYPE_UINT8,
        int16 = SLANG_SCALAR_TYPE_INT16,
        uint16 = SLANG_SCALAR_TYPE_UINT16,
        intptr = SLANG_SCALAR_TYPE_INTPTR,
        uintptr = SLANG_SCALAR_TYPE_UINTPTR,
        COUNT,
    };

    SGL_ENUM_INFO(
        ScalarType,
        {
            {ScalarType::none_, "none"},
            {ScalarType::void_, "void"},
            {ScalarType::bool_, "bool"},
            {ScalarType::int32, "int32"},
            {ScalarType::uint32, "uint32"},
            {ScalarType::int64, "int64"},
            {ScalarType::uint64, "uint64"},
            {ScalarType::float16, "float16"},
            {ScalarType::float32, "float32"},
            {ScalarType::float64, "float64"},
            {ScalarType::int8, "int8"},
            {ScalarType::uint8, "uint8"},
            {ScalarType::int16, "int16"},
            {ScalarType::uint16, "uint16"},
        }
    );

    enum class ResourceShape {
        base_shape_mask = SLANG_RESOURCE_BASE_SHAPE_MASK,

        none = SLANG_RESOURCE_NONE,
        texture_1d = SLANG_TEXTURE_1D,
        texture_2d = SLANG_TEXTURE_2D,
        texture_3d = SLANG_TEXTURE_3D,
        texture_cube = SLANG_TEXTURE_CUBE,
        texture_buffer = SLANG_TEXTURE_BUFFER,

        structured_buffer = SLANG_STRUCTURED_BUFFER,
        byte_address_buffer = SLANG_BYTE_ADDRESS_BUFFER,
        unknown = SLANG_RESOURCE_UNKNOWN,
        acceleration_structure = SLANG_ACCELERATION_STRUCTURE,

        ext_shape_mask = SLANG_RESOURCE_EXT_SHAPE_MASK,

        texture_feedback_flag = SLANG_TEXTURE_FEEDBACK_FLAG,
        texture_array_flag = SLANG_TEXTURE_ARRAY_FLAG,
        texture_multisample_flag = SLANG_TEXTURE_MULTISAMPLE_FLAG,

        texture_1d_array = texture_1d | texture_array_flag,
        texture_2d_array = texture_2d | texture_array_flag,
        texture_cube_array = texture_cube | texture_array_flag,

        texture_2d_multisample = texture_2d | texture_multisample_flag,
        texture_2d_multisample_array = texture_2d | texture_multisample_flag | texture_array_flag,
    };

    SGL_ENUM_INFO(
        ResourceShape,
        {
            {ResourceShape::none, "none"},
            {ResourceShape::texture_1d, "texture_1d"},
            {ResourceShape::texture_2d, "texture_2d"},
            {ResourceShape::texture_3d, "texture_3d"},
            {ResourceShape::texture_cube, "texture_cube"},
            {ResourceShape::texture_buffer, "texture_buffer"},
            {ResourceShape::structured_buffer, "structured_buffer"},
            {ResourceShape::byte_address_buffer, "byte_address_buffer"},
            {ResourceShape::unknown, "unknown"},
            {ResourceShape::acceleration_structure, "acceleration_structure"},
            {ResourceShape::texture_feedback_flag, "texture_feedback_flag"},
            {ResourceShape::texture_array_flag, "texture_array_flag"},
            {ResourceShape::texture_multisample_flag, "texture_multisample_flag"},
            {ResourceShape::texture_1d_array, "texture_1d_array"},
            {ResourceShape::texture_2d_array, "texture_2d_array"},
            {ResourceShape::texture_cube_array, "texture_cube_array"},
            {ResourceShape::texture_2d_multisample, "texture_2d_multisample"},
            {ResourceShape::texture_2d_multisample_array, "texture_2d_multisample_array"},
        }
    );

    enum class ResourceAccess {
        none = SLANG_RESOURCE_ACCESS_NONE,
        read = SLANG_RESOURCE_ACCESS_READ,
        read_write = SLANG_RESOURCE_ACCESS_READ_WRITE,
        raster_ordered = SLANG_RESOURCE_ACCESS_RASTER_ORDERED,
        access_append = SLANG_RESOURCE_ACCESS_APPEND,
        access_consume = SLANG_RESOURCE_ACCESS_CONSUME,
        access_write = SLANG_RESOURCE_ACCESS_WRITE,
    };

    SGL_ENUM_INFO(
        ResourceAccess,
        {
            {ResourceAccess::none, "none"},
            {ResourceAccess::read, "read"},
            {ResourceAccess::read_write, "read_write"},
            {ResourceAccess::raster_ordered, "raster_ordered"},
            {ResourceAccess::access_append, "access_append"},
            {ResourceAccess::access_consume, "access_consume"},
            {ResourceAccess::access_write, "access_write"},
        }
    );

    enum class ParameterCategory {
        none = SLANG_PARAMETER_CATEGORY_NONE,
        mixed = SLANG_PARAMETER_CATEGORY_MIXED,
        constant_buffer = SLANG_PARAMETER_CATEGORY_CONSTANT_BUFFER,
        shader_resource = SLANG_PARAMETER_CATEGORY_SHADER_RESOURCE,
        unordered_access = SLANG_PARAMETER_CATEGORY_UNORDERED_ACCESS,
        varying_input = SLANG_PARAMETER_CATEGORY_VARYING_INPUT,
        varying_output = SLANG_PARAMETER_CATEGORY_VARYING_OUTPUT,
        sampler_state = SLANG_PARAMETER_CATEGORY_SAMPLER_STATE,
        uniform = SLANG_PARAMETER_CATEGORY_UNIFORM,
        descriptor_table_slot = SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT,
        specialization_constant = SLANG_PARAMETER_CATEGORY_SPECIALIZATION_CONSTANT,
        push_constant_buffer = SLANG_PARAMETER_CATEGORY_PUSH_CONSTANT_BUFFER,
        register_space = SLANG_PARAMETER_CATEGORY_REGISTER_SPACE,
        generic = SLANG_PARAMETER_CATEGORY_GENERIC,
        ray_payload = SLANG_PARAMETER_CATEGORY_RAY_PAYLOAD,
        hit_attributes = SLANG_PARAMETER_CATEGORY_HIT_ATTRIBUTES,
        callable_payload = SLANG_PARAMETER_CATEGORY_CALLABLE_PAYLOAD,
        shader_record = SLANG_PARAMETER_CATEGORY_SHADER_RECORD,
        existential_type_param = SLANG_PARAMETER_CATEGORY_EXISTENTIAL_TYPE_PARAM,
        existential_object_param = SLANG_PARAMETER_CATEGORY_EXISTENTIAL_OBJECT_PARAM,
    };

    SGL_ENUM_INFO(
        ParameterCategory,
        {
            {ParameterCategory::none, "none"},
            {ParameterCategory::mixed, "mixed"},
            {ParameterCategory::constant_buffer, "constant_buffer"},
            {ParameterCategory::shader_resource, "shader_resource"},
            {ParameterCategory::unordered_access, "unordered_access"},
            {ParameterCategory::varying_input, "varying_input"},
            {ParameterCategory::varying_output, "varying_output"},
            {ParameterCategory::sampler_state, "sampler_state"},
            {ParameterCategory::uniform, "uniform"},
            {ParameterCategory::descriptor_table_slot, "descriptor_table_slot"},
            {ParameterCategory::specialization_constant, "specialization_constant"},
            {ParameterCategory::push_constant_buffer, "push_constant_buffer"},
            {ParameterCategory::register_space, "register_space"},
            {ParameterCategory::generic, "generic"},
            {ParameterCategory::ray_payload, "ray_payload"},
            {ParameterCategory::hit_attributes, "hit_attributes"},
            {ParameterCategory::callable_payload, "callable_payload"},
            {ParameterCategory::shader_record, "shader_record"},
            {ParameterCategory::existential_type_param, "existential_type_param"},
            {ParameterCategory::existential_object_param, "existential_object_param"},
        }
    );

    TypeReflection(ref<const Object> owner, slang::TypeReflection* target)
        : BaseReflectionObject(owner)
        , m_target(target){};

    Kind kind() const { return static_cast<Kind>(m_target->getKind()); }

    char const* name() const { return m_target->getName(); }

    bool is_struct() const { return kind() == Kind::struct_; }

    uint32_t field_count() const
    {
        SGL_CHECK(is_struct(), "Type is not a struct");
        return m_target->getFieldCount();
    }

    ref<const VariableReflection> get_field_by_index(uint32_t index) const
    {
        SGL_CHECK(is_struct(), "Type is not a struct");
        return detail::from_slang(m_owner, m_target->getFieldByIndex(index));
    }

    std::vector<ref<const VariableReflection>> fields() const
    {
        std::vector<ref<const VariableReflection>> result;
        for (uint32_t i = 0; i < m_target->getFieldCount(); ++i) {
            result.push_back(detail::from_slang(m_owner, m_target->getFieldByIndex(i)));
        }
        return result;
    }

    bool is_array() const { return kind() == Kind::array; }

    size_t element_count() const
    {
        SGL_CHECK(is_array(), "Type is not an array");
        return m_target->getElementCount();
    }

    size_t total_element_count() const
    {
        SGL_CHECK(is_array(), "Type is not an array");
        size_t result = 1;
        ref<const TypeReflection> type = ref(this);
        while (type->is_array()) {
            result *= type->element_count();
            type = type->element_type();
        }
        return result;
    }

    ref<const TypeReflection> element_type() const
    {
        SGL_CHECK(is_array(), "Type is not an array");
        return detail::from_slang(m_owner, m_target->getElementType());
    }

    ref<const TypeReflection> unwrap_array() const
    {
        ref<const TypeReflection> type = ref(this);
        while (type->is_array()) {
            type = type->element_type();
        }
        return type;
    }

    uint32_t row_count() const { return m_target->getRowCount(); }

    uint32_t col_count() const { return m_target->getColumnCount(); }

    ScalarType scalar_type() const { return static_cast<ScalarType>(m_target->getScalarType()); }

    ref<const TypeReflection> resource_result_type() const
    {
        return detail::from_slang(m_owner, m_target->getResourceResultType());
    }

    ResourceShape resource_shape() const { return static_cast<ResourceShape>(m_target->getResourceShape()); }

    ResourceAccess resource_access() const { return static_cast<ResourceAccess>(m_target->getResourceAccess()); }

#if 0
    unsigned int getUserAttributeCount() { return spReflectionType_GetUserAttributeCount((SlangReflectionType*)this); }
    UserAttribute* getUserAttributeByIndex(unsigned int index)
    {
        return (UserAttribute*)spReflectionType_GetUserAttribute((SlangReflectionType*)this, index);
    }
    UserAttribute* findUserAttributeByName(char const* name)
    {
        return (UserAttribute*)spReflectionType_FindUserAttributeByName((SlangReflectionType*)this, name);
    }
#endif

    std::string to_string() const;

private:
    /// Cast to non-const base pointer.
    /// The underlying slang API is not const-correct.
    slang::TypeReflection* base() const { return (slang::TypeReflection*)(this); }

    slang::TypeReflection* m_target;
};

SGL_ENUM_CLASS_OPERATORS(TypeReflection::ResourceShape);

SGL_ENUM_REGISTER(TypeReflection::Kind);
SGL_ENUM_REGISTER(TypeReflection::ScalarType);
SGL_ENUM_REGISTER(TypeReflection::ResourceShape);
SGL_ENUM_REGISTER(TypeReflection::ResourceAccess);
SGL_ENUM_REGISTER(TypeReflection::ParameterCategory);

class SGL_API TypeLayoutReflection : public BaseReflectionObject {
public:
    static ref<const TypeLayoutReflection>
    from_slang(ref<const Object> owner, slang::TypeLayoutReflection* type_layout_reflection)
    {
        return detail::from_slang(owner, type_layout_reflection);
    }

    TypeLayoutReflection(ref<const Object> owner, slang::TypeLayoutReflection* target)
        : BaseReflectionObject(owner)
        , m_target(target){};

    /// Cast to non-const base pointer.
    /// The underlying slang API is not const-correct.
    slang::TypeLayoutReflection* base() const { return (slang::TypeLayoutReflection*)(this); }

    ref<const TypeReflection> type() const { return detail::from_slang(m_owner, m_target->getType()); }

    TypeReflection::Kind kind() const { return static_cast<TypeReflection::Kind>(m_target->getKind()); }

    const char* name() const { return m_target->getName(); }

    size_t size() const { return m_target->getSize(SlangParameterCategory::SLANG_PARAMETER_CATEGORY_UNIFORM); }
    size_t stride() const { return m_target->getStride(SlangParameterCategory::SLANG_PARAMETER_CATEGORY_UNIFORM); }
    int32_t alignment() const
    {
        return m_target->getAlignment(SlangParameterCategory::SLANG_PARAMETER_CATEGORY_UNIFORM);
    }

#if 0
    size_t getSize(SlangParameterCategory category = SLANG_PARAMETER_CATEGORY_UNIFORM)
    {
        return spReflectionTypeLayout_GetSize((SlangReflectionTypeLayout*)this, category);
    }

    size_t getStride(SlangParameterCategory category = SLANG_PARAMETER_CATEGORY_UNIFORM)
    {
        return spReflectionTypeLayout_GetStride((SlangReflectionTypeLayout*)this, category);
    }

    int32_t getAlignment(SlangParameterCategory category = SLANG_PARAMETER_CATEGORY_UNIFORM)
    {
        return spReflectionTypeLayout_getAlignment((SlangReflectionTypeLayout*)this, category);
    }
#endif

    uint32_t field_count() const { return m_target->getFieldCount(); }

    ref<const VariableLayoutReflection> get_field_by_index(uint32_t index) const
    {
        // TODO field_count() does not always return the right number of fields
        // SGL_CHECK(index < field_count(), "Field index out of range");
        return detail::from_slang(m_owner, m_target->getFieldByIndex(index));
    }

    int32_t find_field_index_by_name(const char* name_begin, const char* name_end = nullptr) const
    {
        return narrow_cast<int32_t>(m_target->findFieldIndexByName(name_begin, name_end));
    }

    ref<const VariableLayoutReflection> find_field_by_name(const char* name_begin, const char* name_end = nullptr) const
    {
        if (auto index = m_target->findFieldIndexByName(name_begin, name_end); index >= 0)
            return detail::from_slang(m_owner, m_target->getFieldByIndex(narrow_cast<unsigned int>(index)));
        return nullptr;
    }

    std::vector<ref<const VariableLayoutReflection>> fields() const
    {
        std::vector<ref<const VariableLayoutReflection>> result;
        for (uint32_t i = 0; i < m_target->getFieldCount(); ++i) {
            result.push_back(detail::from_slang(m_owner, m_target->getFieldByIndex(i)));
        }
        return result;
    }

#if 0
    VariableLayoutReflection* getExplicitCounter()
    {
        return (VariableLayoutReflection*)spReflectionTypeLayout_GetExplicitCounter((SlangReflectionTypeLayout*)this);
    }
#endif

    bool is_array() const { return type()->is_array(); }

    ref<const TypeLayoutReflection> unwrap_array() const
    {
        ref<const TypeLayoutReflection> type_layout = ref(this);
        while (type_layout->is_array()) {
            type_layout = type_layout->element_type_layout();
        }
        return ref<const TypeLayoutReflection>(type_layout);
    }

    size_t element_count() const { return type()->element_count(); }

#if 0
    size_t getTotalArrayElementCount() { return getType()->getTotalArrayElementCount(); }
#endif

    size_t element_stride(TypeReflection::ParameterCategory category = TypeReflection::ParameterCategory::uniform) const
    {
        return m_target->getElementStride(static_cast<SlangParameterCategory>(category));
    }

    ref<const TypeLayoutReflection> element_type_layout() const
    {
        return detail::from_slang(m_owner, m_target->getElementTypeLayout());
    }

    ref<const VariableLayoutReflection> element_var_layout() const
    {
        return detail::from_slang(m_owner, m_target->getElementVarLayout());
    }

    ref<const VariableLayoutReflection> container_var_layout() const
    {
        return detail::from_slang(m_owner, m_target->getContainerVarLayout());
    }

    // How is this type supposed to be bound?
    TypeReflection::ParameterCategory parameter_category() const
    {
        return static_cast<TypeReflection::ParameterCategory>(m_target->getParameterCategory());
    }

#if 0
    unsigned int getCategoryCount()
    {
        return spReflectionTypeLayout_GetCategoryCount((SlangReflectionTypeLayout*)this);
    }

    ParameterCategory getCategoryByIndex(unsigned int index)
    {
        return (ParameterCategory)spReflectionTypeLayout_GetCategoryByIndex((SlangReflectionTypeLayout*)this, index);
    }

    unsigned getRowCount() { return getType()->getRowCount(); }

    unsigned getColumnCount() { return getType()->getColumnCount(); }

    TypeReflection::ScalarType getScalarType() { return getType()->getScalarType(); }

    TypeReflection* getResourceResultType() { return getType()->getResourceResultType(); }

    SlangResourceShape getResourceShape() { return getType()->getResourceShape(); }

    SlangResourceAccess getResourceAccess() { return getType()->getResourceAccess(); }

    SlangMatrixLayoutMode getMatrixLayoutMode()
    {
        return spReflectionTypeLayout_GetMatrixLayoutMode((SlangReflectionTypeLayout*)this);
    }

    int getGenericParamIndex() { return spReflectionTypeLayout_getGenericParamIndex((SlangReflectionTypeLayout*)this); }

    TypeLayoutReflection* getPendingDataTypeLayout()
    {
        return (TypeLayoutReflection*)spReflectionTypeLayout_getPendingDataTypeLayout((SlangReflectionTypeLayout*)this);
    }

    VariableLayoutReflection* getSpecializedTypePendingDataVarLayout()
    {
        return (VariableLayoutReflection*)spReflectionTypeLayout_getSpecializedTypePendingDataVarLayout(
            (SlangReflectionTypeLayout*)this
        );
    }

    SlangInt getBindingRangeCount()
    {
        return spReflectionTypeLayout_getBindingRangeCount((SlangReflectionTypeLayout*)this);
    }

    BindingType getBindingRangeType(SlangInt index)
    {
        return (BindingType)spReflectionTypeLayout_getBindingRangeType((SlangReflectionTypeLayout*)this, index);
    }

    SlangInt getBindingRangeBindingCount(SlangInt index)
    {
        return spReflectionTypeLayout_getBindingRangeBindingCount((SlangReflectionTypeLayout*)this, index);
    }

    /*
    SlangInt getBindingRangeIndexOffset(SlangInt index)
    {
        return spReflectionTypeLayout_getBindingRangeIndexOffset(
            (SlangReflectionTypeLayout*) this,
            index);
    }

    SlangInt getBindingRangeSpaceOffset(SlangInt index)
    {
        return spReflectionTypeLayout_getBindingRangeSpaceOffset(
            (SlangReflectionTypeLayout*) this,
            index);
    }
    */
#endif

    uint32_t get_field_binding_range_offset(uint32_t field_index) const
    {
        return narrow_cast<uint32_t>(m_target->getFieldBindingRangeOffset(field_index));
    }

#if 0
    SlangInt getExplicitCounterBindingRangeOffset()
    {
        return spReflectionTypeLayout_getExplicitCounterBindingRangeOffset((SlangReflectionTypeLayout*)this);
    }

    TypeLayoutReflection* getBindingRangeLeafTypeLayout(SlangInt index)
    {
        return (TypeLayoutReflection*)
            spReflectionTypeLayout_getBindingRangeLeafTypeLayout((SlangReflectionTypeLayout*)this, index);
    }

    VariableReflection* getBindingRangeLeafVariable(SlangInt index)
    {
        return (VariableReflection*)
            spReflectionTypeLayout_getBindingRangeLeafVariable((SlangReflectionTypeLayout*)this, index);
    }

    SlangInt getBindingRangeDescriptorSetIndex(SlangInt index)
    {
        return spReflectionTypeLayout_getBindingRangeDescriptorSetIndex((SlangReflectionTypeLayout*)this, index);
    }

    SlangInt getBindingRangeFirstDescriptorRangeIndex(SlangInt index)
    {
        return spReflectionTypeLayout_getBindingRangeFirstDescriptorRangeIndex((SlangReflectionTypeLayout*)this, index);
    }

    SlangInt getBindingRangeDescriptorRangeCount(SlangInt index)
    {
        return spReflectionTypeLayout_getBindingRangeDescriptorRangeCount((SlangReflectionTypeLayout*)this, index);
    }

    SlangInt getDescriptorSetCount()
    {
        return spReflectionTypeLayout_getDescriptorSetCount((SlangReflectionTypeLayout*)this);
    }

    SlangInt getDescriptorSetSpaceOffset(SlangInt setIndex)
    {
        return spReflectionTypeLayout_getDescriptorSetSpaceOffset((SlangReflectionTypeLayout*)this, setIndex);
    }

    SlangInt getDescriptorSetDescriptorRangeCount(SlangInt setIndex)
    {
        return spReflectionTypeLayout_getDescriptorSetDescriptorRangeCount((SlangReflectionTypeLayout*)this, setIndex);
    }

    SlangInt getDescriptorSetDescriptorRangeIndexOffset(SlangInt setIndex, SlangInt rangeIndex)
    {
        return spReflectionTypeLayout_getDescriptorSetDescriptorRangeIndexOffset(
            (SlangReflectionTypeLayout*)this,
            setIndex,
            rangeIndex
        );
    }

    SlangInt getDescriptorSetDescriptorRangeDescriptorCount(SlangInt setIndex, SlangInt rangeIndex)
    {
        return spReflectionTypeLayout_getDescriptorSetDescriptorRangeDescriptorCount(
            (SlangReflectionTypeLayout*)this,
            setIndex,
            rangeIndex
        );
    }

    BindingType getDescriptorSetDescriptorRangeType(SlangInt setIndex, SlangInt rangeIndex)
    {
        return (BindingType)spReflectionTypeLayout_getDescriptorSetDescriptorRangeType(
            (SlangReflectionTypeLayout*)this,
            setIndex,
            rangeIndex
        );
    }

    ParameterCategory getDescriptorSetDescriptorRangeCategory(SlangInt setIndex, SlangInt rangeIndex)
    {
        return (ParameterCategory)spReflectionTypeLayout_getDescriptorSetDescriptorRangeCategory(
            (SlangReflectionTypeLayout*)this,
            setIndex,
            rangeIndex
        );
    }

    SlangInt getSubObjectRangeCount()
    {
        return spReflectionTypeLayout_getSubObjectRangeCount((SlangReflectionTypeLayout*)this);
    }

    SlangInt getSubObjectRangeBindingRangeIndex(SlangInt subObjectRangeIndex)
    {
        return spReflectionTypeLayout_getSubObjectRangeBindingRangeIndex(
            (SlangReflectionTypeLayout*)this,
            subObjectRangeIndex
        );
    }

    SlangInt getSubObjectRangeSpaceOffset(SlangInt subObjectRangeIndex)
    {
        return spReflectionTypeLayout_getSubObjectRangeSpaceOffset(
            (SlangReflectionTypeLayout*)this,
            subObjectRangeIndex
        );
    }

    VariableLayoutReflection* getSubObjectRangeOffset(SlangInt subObjectRangeIndex)
    {
        return (VariableLayoutReflection*)
            spReflectionTypeLayout_getSubObjectRangeOffset((SlangReflectionTypeLayout*)this, subObjectRangeIndex);
    }
#endif

    std::string to_string() const;

private:
    slang::TypeLayoutReflection* m_target;
};

class SGL_API FunctionReflection : public BaseReflectionObject {
public:
    FunctionReflection(ref<const Object> owner, slang::FunctionReflection* target)
        : BaseReflectionObject(owner)
        , m_target(target){};

    /// Cast to non-const base pointer.
    /// The underlying slang API is not const-correct.
    slang::FunctionReflection* base() const { return (slang::FunctionReflection*)(this); }

    char const* name() const { return m_target->getName(); }

    ref<const TypeReflection> return_type() { return detail::from_slang(m_owner, m_target->getReturnType()); }

    uint32_t parameter_count() const { return m_target->getParameterCount(); }

    ref<const VariableReflection> get_parameter_by_index(uint32_t index) const
    {
        SGL_CHECK(index < parameter_count(), "Parameter index out of range");
        return detail::from_slang(m_owner, m_target->getParameterByIndex(index));
    }

    std::vector<ref<const VariableReflection>> parameters() const
    {
        std::vector<ref<const VariableReflection>> result;
        for (uint32_t i = 0; i < m_target->getParameterCount(); ++i) {
            result.push_back(detail::from_slang(m_owner, m_target->getParameterByIndex(i)));
        }
        return result;
    }

    /// Check if variable has a given modifier (e.g. 'inout').
    bool has_modifier(ModifierID modifier) const
    {
        return m_target->findModifier(static_cast<slang::Modifier::ID>(modifier)) != nullptr;
    }

private:
    slang::FunctionReflection* m_target;
};

class SGL_API VariableReflection : public BaseReflectionObject {
public:
    static ref<const VariableReflection>
    from_slang(ref<const Object> owner, slang::VariableReflection* variable_reflection)
    {
        return detail::from_slang(owner, variable_reflection);
    }

    VariableReflection(ref<const Object> owner, slang::VariableReflection* target)
        : BaseReflectionObject(owner)
        , m_target(target){};

    /// Cast to non-const base pointer.
    /// The underlying slang API is not const-correct.
    slang::VariableReflection* base() const { return (slang::VariableReflection*)(this); }

    /// Variable name.
    const char* name() const { return m_target->getName(); }

    /// Variable type reflection.
    ref<const TypeReflection> type() const { return detail::from_slang(m_owner, m_target->getType()); }

    /// Check if variable has a given modifier (e.g. 'inout').
    bool has_modifier(ModifierID modifier) const
    {
        return m_target->findModifier(static_cast<slang::Modifier::ID>(modifier)) != nullptr;
    }

#if 0
    Modifier* findModifier(Modifier::ID id)
    {
        return (Modifier*)spReflectionVariable_FindModifier((SlangReflectionVariable*)this, (SlangModifierID)id);
    }

    unsigned int getUserAttributeCount()
    {
        return spReflectionVariable_GetUserAttributeCount((SlangReflectionVariable*)this);
    }
    UserAttribute* getUserAttributeByIndex(unsigned int index)
    {
        return (UserAttribute*)spReflectionVariable_GetUserAttribute((SlangReflectionVariable*)this, index);
    }
    UserAttribute* findUserAttributeByName(SlangSession* session, char const* name)
    {
        return (UserAttribute*)
            spReflectionVariable_FindUserAttributeByName((SlangReflectionVariable*)this, session, name);
    }
#endif

private:
    slang::VariableReflection* m_target;
};

class SGL_API VariableLayoutReflection : public BaseReflectionObject {
public:
    VariableLayoutReflection(ref<const Object> owner, slang::VariableLayoutReflection* target)
        : BaseReflectionObject(owner)
        , m_target(target){};

    /// Cast to non-const base pointer.
    /// The underlying slang API is not const-correct.
    slang::VariableLayoutReflection* base() const { return (slang::VariableLayoutReflection*)(this); }

    ref<const VariableReflection> variable() const { return detail::from_slang(m_owner, m_target->getVariable()); }

    const char* name() const { return m_target->getName(); }

#if 0
    Modifier* findModifier(Modifier::ID id) { return getVariable()->findModifier(id); }
#endif

    ref<const TypeLayoutReflection> type_layout() const
    {
        return detail::from_slang(m_owner, m_target->getTypeLayout());
    }

#if 0
    ParameterCategory getCategory() { return getTypeLayout()->getParameterCategory(); }

    unsigned int getCategoryCount() { return getTypeLayout()->getCategoryCount(); }

    ParameterCategory getCategoryByIndex(unsigned int index) { return getTypeLayout()->getCategoryByIndex(index); }
#endif

    size_t offset() const { return m_target->getOffset(SlangParameterCategory::SLANG_PARAMETER_CATEGORY_UNIFORM); }

#if 0
    TypeReflection* getType() { return getVariable()->getType(); }

    unsigned getBindingIndex() { return spReflectionParameter_GetBindingIndex((SlangReflectionVariableLayout*)this); }

    unsigned getBindingSpace() { return spReflectionParameter_GetBindingSpace((SlangReflectionVariableLayout*)this); }

    size_t getBindingSpace(SlangParameterCategory category)
    {
        return spReflectionVariableLayout_GetSpace((SlangReflectionVariableLayout*)this, category);
    }

    char const* getSemanticName()
    {
        return spReflectionVariableLayout_GetSemanticName((SlangReflectionVariableLayout*)this);
    }

    size_t getSemanticIndex()
    {
        return spReflectionVariableLayout_GetSemanticIndex((SlangReflectionVariableLayout*)this);
    }

    SlangStage getStage() { return spReflectionVariableLayout_getStage((SlangReflectionVariableLayout*)this); }

    VariableLayoutReflection* getPendingDataLayout()
    {
        return (VariableLayoutReflection*)spReflectionVariableLayout_getPendingDataLayout(
            (SlangReflectionVariableLayout*)this
        );
    }
#endif

    std::string to_string() const;

private:
    slang::VariableLayoutReflection* m_target;
};

class SGL_API EntryPointLayout : public BaseReflectionObject {
public:
    static const EntryPointLayout* from_slang(ref<const Object> owner, slang::EntryPointLayout* entry_point_reflection)
    {
        return detail::from_slang(owner, entry_point_reflection);
    }

    EntryPointLayout(ref<const Object> owner, slang::EntryPointLayout* target)
        : BaseReflectionObject(owner)
        , m_target(target){};

    /// Cast to non-const base pointer.
    /// The underlying slang API is not const-correct.
    slang::EntryPointLayout* base() const { return (slang::EntryPointLayout*)(this); }

    const char* name() const { return m_target->getName(); }

    const char* name_override() const { return m_target->getNameOverride(); }

    ShaderStage stage() const { return static_cast<ShaderStage>(m_target->getStage()); }

    uint32_t parameter_count() const { return m_target->getParameterCount(); }

    ref<const VariableLayoutReflection> get_parameter_by_index(uint32_t index) const
    {
        SGL_CHECK(index < parameter_count(), "Parameter index out of range");
        return detail::from_slang(m_owner, m_target->getParameterByIndex(index));
    }

    std::vector<ref<const VariableLayoutReflection>> parameters() const
    {
        std::vector<ref<const VariableLayoutReflection>> result;
        for (uint32_t i = 0; i < m_target->getParameterCount(); ++i) {
            result.push_back(detail::from_slang(m_owner, m_target->getParameterByIndex(i)));
        }
        return result;
    }

    uint3 compute_thread_group_size() const
    {
        SlangUInt size[3];
        m_target->getComputeThreadGroupSize(3, size);
        return uint3(narrow_cast<uint32_t>(size[0]), narrow_cast<uint32_t>(size[1]), narrow_cast<uint32_t>(size[2]));
    }

    bool uses_any_sample_rate_input() const { return m_target->usesAnySampleRateInput(); }

#if 0
    VariableLayoutReflection* getVarLayout()
    {
        return (VariableLayoutReflection*)spReflectionEntryPoint_getVarLayout((SlangReflectionEntryPoint*)this);
    }

    TypeLayoutReflection* getTypeLayout() { return getVarLayout()->getTypeLayout(); }

    VariableLayoutReflection* getResultVarLayout()
    {
        return (VariableLayoutReflection*)spReflectionEntryPoint_getResultVarLayout((SlangReflectionEntryPoint*)this);
    }

    bool hasDefaultConstantBuffer()
    {
        return spReflectionEntryPoint_hasDefaultConstantBuffer((SlangReflectionEntryPoint*)this) != 0;
    }
#endif

    std::string to_string() const;

private:
    slang::EntryPointLayout* m_target;
};

class SGL_API ProgramLayout : public BaseReflectionObject {
public:
    static ref<const ProgramLayout> from_slang(ref<const Object> owner, slang::ProgramLayout* program_layout)
    {
        return detail::from_slang(owner, program_layout);
    }

    ProgramLayout(ref<const Object> owner, slang::ProgramLayout* target)
        : BaseReflectionObject(owner)
        , m_target(target){};

    /// Cast to non-const base pointer.
    /// The underlying slang API is not const-correct.
    slang::ProgramLayout* base() const { return (slang::ProgramLayout*)(this); }

    ref<const TypeLayoutReflection> globals_type_layout() const
    {
        return detail::from_slang(m_owner, m_target->getGlobalParamsTypeLayout());
    }

    ref<const VariableLayoutReflection> globals_variable_layout() const
    {
        return detail::from_slang(m_owner, m_target->getGlobalParamsVarLayout());
    }

    uint32_t parameter_count() const { return m_target->getParameterCount(); }

    ref<const VariableLayoutReflection> get_parameter_by_index(uint32_t index) const
    {
        SGL_CHECK(index < parameter_count(), "Parameter index out of range");
        return detail::from_slang(m_owner, m_target->getParameterByIndex(index));
    }

    std::vector<ref<const VariableLayoutReflection>> parameters() const
    {
        std::vector<ref<const VariableLayoutReflection>> result;
        for (uint32_t i = 0; i < m_target->getParameterCount(); ++i) {
            result.push_back(detail::from_slang(m_owner, m_target->getParameterByIndex(i)));
        }
        return result;
    }

    uint32_t type_parameter_count() const { return m_target->getTypeParameterCount(); }

#if 0
    TypeParameterReflection get_type_parameter_by_index(uint32_t index) const
    {
        return (TypeParameterReflection*)spReflection_GetTypeParameterByIndex((SlangReflection*)this, index);
    }

    TypeParameterReflection* findTypeParameter(char const* name)
    {
        return (TypeParameterReflection*)spReflection_FindTypeParameter((SlangReflection*)this, name);
    }
#endif

    uint32_t entry_point_count() const { return narrow_cast<uint32_t>(m_target->getEntryPointCount()); }

    ref<const EntryPointLayout> get_entry_point_by_index(uint32_t index) const
    {
        SGL_CHECK(index < entry_point_count(), "Entry point index out of range");
        return detail::from_slang(m_owner, m_target->getEntryPointByIndex(index));
    }

    ref<const EntryPointLayout> get_entry_point_by_name(const char* name) const
    {
        return detail::from_slang(m_owner, m_target->findEntryPointByName(name));
    }

    ref<const EntryPointLayout> get_entry_point_by_name(std::string_view name) const
    {
        std::string name_str(name);
        return get_entry_point_by_name(name_str.c_str());
    }

    ref<const EntryPointLayout> find_entry_point_by_name(const char* name) const
    {
        if (auto entry_point = m_target->findEntryPointByName(name)) {
            return detail::from_slang(m_owner, entry_point);
        }
        return nullptr;
    }

    ref<const EntryPointLayout> find_entry_point_by_name(std::string_view name) const
    {
        std::string name_str(name);
        return find_entry_point_by_name(name_str.c_str());
    }

    std::vector<ref<const EntryPointLayout>> entry_points() const
    {
        std::vector<ref<const EntryPointLayout>> result;
        for (uint32_t i = 0; i < m_target->getEntryPointCount(); ++i) {
            result.push_back(detail::from_slang(m_owner, m_target->getEntryPointByIndex(i)));
        }
        return result;
    }

#if 0
    SlangUInt getGlobalConstantBufferBinding()
    {
        return spReflection_getGlobalConstantBufferBinding((SlangReflection*)this);
    }

    size_t getGlobalConstantBufferSize() { return spReflection_getGlobalConstantBufferSize((SlangReflection*)this); }

    TypeReflection* findTypeByName(const char* name)
    {
        return (TypeReflection*)spReflection_FindTypeByName((SlangReflection*)this, name);
    }

    TypeLayoutReflection* getTypeLayout(TypeReflection* type, LayoutRules rules = LayoutRules::Default)
    {
        return (TypeLayoutReflection*)
            spReflection_GetTypeLayout((SlangReflection*)this, (SlangReflectionType*)type, SlangLayoutRules(rules));
    }

    TypeReflection* specializeType(
        TypeReflection* type,
        SlangInt specializationArgCount,
        TypeReflection* const* specializationArgs,
        ISlangBlob** outDiagnostics
    )
    {
        return (TypeReflection*)spReflection_specializeType(
            (SlangReflection*)this,
            (SlangReflectionType*)type,
            specializationArgCount,
            (SlangReflectionType* const*)specializationArgs,
            outDiagnostics
        );
    }
#endif

    uint32_t hashed_string_count() const { return narrow_cast<uint32_t>(m_target->getHashedStringCount()); }

    struct HashedString {
        std::string string;
        uint32_t hash;
    };

    HashedString get_hashed_string(uint32_t index) const
    {
        SGL_CHECK(index < hashed_string_count(), "Hashed string index out of range");
        size_t size;
        const char* str = m_target->getHashedString(index, &size);
        return {std::string(str, str + size), spComputeStringHash(str, size)};
    }

    std::vector<HashedString> hashed_strings() const
    {
        std::vector<HashedString> result;
        for (uint32_t i = 0; i < m_target->getHashedStringCount(); ++i) {
            size_t size;
            const char* str = m_target->getHashedString(i, &size);
            result.push_back({std::string(str, str + size), spComputeStringHash(str, size)});
        }
        return result;
    }

    std::map<uint32_t, std::string> hashed_strings_map() const
    {
        std::map<uint32_t, std::string> result;
        for (uint32_t i = 0; i < m_target->getHashedStringCount(); ++i) {
            size_t size;
            const char* str = m_target->getHashedString(i, &size);
            uint32_t hash = spComputeStringHash(str, size);
            result.emplace(hash, std::string(str, str + size));
        }
        return result;
    }

    std::string to_string() const;

private:
    slang::ProgramLayout* m_target;
};

class ShaderProgram;

class SGL_API ReflectionCursor {
public:
    ReflectionCursor() = default;

    ReflectionCursor(const ShaderProgram* shader_program);
    ReflectionCursor(ref<const EntryPointLayout> entry_point_layout);
    ReflectionCursor(ref<const TypeLayoutReflection> type_layout);

    ref<const TypeLayoutReflection> type_layout() const { return m_type_layout; }
    ref<const TypeReflection> type() const { return m_type_layout->type(); }

    bool is_valid() const { return m_valid; }

    // operator bool() const { return is_valid(); }

    operator ref<const TypeLayoutReflection>() const { return type_layout(); }

    ReflectionCursor operator[](std::string_view name) const;
    ReflectionCursor operator[](uint32_t index) const;

    ReflectionCursor find_field(std::string_view name) const;
    ReflectionCursor find_element(uint32_t index) const;

    bool has_field(std::string_view name) const { return find_field(name).is_valid(); }
    bool has_element(uint32_t index) const { return find_element(index).is_valid(); }

    std::string to_string() const;

private:
    const ShaderProgram* m_shader_program{nullptr};
    ref<const EntryPointLayout> m_entry_point_layout{nullptr};
    ref<const TypeLayoutReflection> m_type_layout{nullptr};
    bool m_valid{false};
};

} // namespace sgl
