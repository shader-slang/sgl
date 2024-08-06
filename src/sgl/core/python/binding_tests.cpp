// SPDX-License-Identifier: Apache-2.0

#include "nanobind.h"

#include "sgl/core/binding_tests.h"

SGL_PY_EXPORT(core_binding_tests)
{
    using namespace sgl;

    nb::class_<BindingTestsRoot, Object>(m, "BindingTestsRoot")
        .def_prop_ro("child_const", &BindingTestsRoot::get_child_const, D_NA())
        .def_prop_ro("child_none_const", &BindingTestsRoot::get_child_none_const, D_NA())
        .def("get_child_const", &BindingTestsRoot::get_child_const, D_NA())
        .def("get_child_none_const", &BindingTestsRoot::get_child_none_const, D_NA())

        .def_prop_ro("rc_child_const", &BindingTestsRoot::get_rc_child_const, D_NA())
        .def_prop_ro("rc_child_none_const", &BindingTestsRoot::get_rc_child_none_const, D_NA())
        .def("get_rc_child_const", &BindingTestsRoot::get_rc_child_const, D_NA())
        .def("get_rc_child_none_const", &BindingTestsRoot::get_rc_child_none_const, D_NA())

        .def_prop_ro(
            "child_const_rvrefinternal",
            &BindingTestsRoot::get_child_const,
            nb::rv_policy::reference_internal,
            D_NA()
        )
        .def_prop_ro(
            "child_none_const_rvrefinternal",
            &BindingTestsRoot::get_child_none_const,
            nb::rv_policy::reference_internal,
            D_NA()
        )
        .def(
            "get_child_const_rvrefinternal",
            &BindingTestsRoot::get_child_const,
            nb::rv_policy::reference_internal,
            D_NA()
        )
        .def(
            "get_child_none_const_rvrefinternal",
            &BindingTestsRoot::get_child_none_const,
            nb::rv_policy::reference_internal,
            D_NA()
        )

        .def_prop_ro(
            "child_const_rvtakeownership",
            &BindingTestsRoot::get_child_const,
            nb::rv_policy::take_ownership,
            D_NA()
        )
        .def_prop_ro(
            "child_none_const_rvtakeownership",
            &BindingTestsRoot::get_child_none_const,
            nb::rv_policy::take_ownership,
            D_NA()
        )
        .def(
            "get_child_const_rvtakeownership",
            &BindingTestsRoot::get_child_const,
            nb::rv_policy::take_ownership,
            D_NA()
        )
        .def(
            "get_child_none_const_rvtakeownership",
            &BindingTestsRoot::get_child_none_const,
            nb::rv_policy::take_ownership,
            D_NA()
        )

        .def_prop_ro("child_const_rvref", &BindingTestsRoot::get_child_const, nb::rv_policy::reference, D_NA())
        .def_prop_ro(
            "child_none_const_rvref",
            &BindingTestsRoot::get_child_none_const,
            nb::rv_policy::reference,
            D_NA()
        )
        .def("get_child_const_rvref", &BindingTestsRoot::get_child_const, nb::rv_policy::reference, D_NA())
        .def("get_child_none_const_rvref", &BindingTestsRoot::get_child_none_const, nb::rv_policy::reference, D_NA())

        .def_prop_ro("ten_children", &BindingTestsRoot::get_ten_children, D_NA())
        .def("get_ten_children", &BindingTestsRoot::get_ten_children, D_NA())
        .def_prop_ro("ten_rc_children", &BindingTestsRoot::get_ten_rc_children, D_NA())
        .def("get_ten_rc_children", &BindingTestsRoot::get_ten_rc_children, D_NA())

        .def_prop_ro(
            "ten_children_rvrefinternal",
            &BindingTestsRoot::get_ten_children,
            nb::rv_policy::reference_internal,
            D_NA()
        )
        .def(
            "get_ten_children_rvrefinternal",
            &BindingTestsRoot::get_ten_children,
            nb::rv_policy::reference_internal,
            D_NA()
        )
        .def_prop_ro("ten_children_rvref", &BindingTestsRoot::get_ten_children, nb::rv_policy::reference, D_NA())
        .def("get_ten_children_rvref", &BindingTestsRoot::get_ten_children, nb::rv_policy::reference, D_NA())
        .def_prop_ro(
            "ten_children_rvtakeownership",
            &BindingTestsRoot::get_ten_children,
            nb::rv_policy::take_ownership,
            D_NA()
        )
        .def(
            "get_ten_children_rvtakeownership",
            &BindingTestsRoot::get_ten_children,
            nb::rv_policy::take_ownership,
            D_NA()
        )

        .def_static(
            "create",
            []() { return BindingTestsRoot::init(); },
            D_NA()
        )
        .def_static(
            "get_root_count",
            []() { return BindingTestsRoot::m_allocated_roots.size(); },
            D_NA()
        )
        .def_static(
            "get_child_count",
            []() {
                return BindingTestsRoot::m_allocated_children.size()
                    + BindingTestsRoot::m_allocated_ref_children.size();
            },
            D_NA()
        );


    nb::class_<BindingTestsChild>(m, "BindingTestsChild");
    nb::class_<BindingTestsChildRefCounted, Object>(m, "BindingTestsChildRefCounted");
}
