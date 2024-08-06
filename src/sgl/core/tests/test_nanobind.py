# SPDX-License-Identifier: Apache-2.0

import pytest
import sgl


def test_create_root():
    root = sgl.BindingTestsRoot.create()
    assert sgl.BindingTestsRoot.get_child_count() == 0
    assert sgl.BindingTestsRoot.get_root_count() == 1
    root = None
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Tests creation of child by calling property that returns raw pointers.
# Expecting to default to ref_internal, so child should not
# be freed even once set to None. Even after the root is freed, as
# nanobind expects the child to be automatically freed when the root
# is freed, it is never explicitly destructed
def test_prop_create_raw_child():
    root = sgl.BindingTestsRoot.create()
    child = root.child_const
    assert sgl.BindingTestsRoot.get_child_count() == 1
    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Same using the explicit ref internal version, so should behave
# exactly the same
def test_prop_create_raw_child_rvrefinternal():
    root = sgl.BindingTestsRoot.create()
    child = root.child_const_rvrefinternal
    assert sgl.BindingTestsRoot.get_child_count() == 1
    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Same again using ref
def test_prop_create_raw_child_rvref():
    root = sgl.BindingTestsRoot.create()
    child = root.child_const_rvref
    assert sgl.BindingTestsRoot.get_child_count() == 1
    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Now using take ownership, child should get cleaned up
# when set to None.
def test_prop_create_raw_child_rvtakeownership():
    root = sgl.BindingTestsRoot.create()
    child = root.child_const_rvtakeownership
    assert sgl.BindingTestsRoot.get_child_count() == 1
    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Tests creation of child by calling function that returns raw pointers.
# Functions default to take ownership, so child should be freed once set to None.
def test_func_create_raw_child():
    root = sgl.BindingTestsRoot.create()
    child = root.get_child_const()
    assert sgl.BindingTestsRoot.get_child_count() == 1
    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Similar test with root freed before child. As ownership of child
# is managed by nanobind, freeing the root should be independent
# of freeing the child
def test_func_create_raw_child_reverse_destruction():
    root = sgl.BindingTestsRoot.create()
    child = root.get_child_const()
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 1

    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 0

    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Tests creation of child by calling function that returns raw pointers.
# Now explicitly using rv ref internal, so should do the same as
# the property version
def test_func_create_raw_child_rvrefinternal():
    root = sgl.BindingTestsRoot.create()
    child = root.get_child_const_rvrefinternal()
    assert sgl.BindingTestsRoot.get_child_count() == 1
    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 0


# As with prior test, reverse destruction. As we're using ref internal,
# the child should now keep the parent alive until its python side
# wrapper is set to None. Once done, the child should stil remain
# as its not been explicitly destroyed, but the managed root object should
# be freed.
def test_func_create_raw_child_rvrefinternal_reverse_destruction():
    root = sgl.BindingTestsRoot.create()
    child = root.get_child_const_rvrefinternal()
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 1

    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 1

    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Test reverse destruction again using only ref. Liftime of root
# is now independent of child.
def test_func_create_raw_child_rvref_reverse_destruction():
    root = sgl.BindingTestsRoot.create()
    child = root.get_child_const_rvref()
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 1

    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 0

    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 0


# And one more time with explicit ownership. Child should now
# be automatically freed as well.
def test_func_create_raw_child_rvtakeownership_reverse_destruction():
    root = sgl.BindingTestsRoot.create()
    child = root.get_child_const_rvtakeownership()
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 1

    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 1
    assert sgl.BindingTestsRoot.get_root_count() == 0

    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Creation of ref counted child should free as soon as
# child is no longer referenced
def test_prop_create_refcounted_child():
    root = sgl.BindingTestsRoot.create()
    child = root.rc_child_const
    assert sgl.BindingTestsRoot.get_child_count() == 1
    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Function version should be identical
def test_func_create_refcounted_child():
    root = sgl.BindingTestsRoot.create()
    child = root.get_rc_child_const()
    assert sgl.BindingTestsRoot.get_child_count() == 1
    child = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    root = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    assert sgl.BindingTestsRoot.get_root_count() == 0


# Creation of ten children via property which defaults to
# ref internal, so children don't get freed.
def test_prop_create_ten_raw_children():
    root = sgl.BindingTestsRoot.create()
    children = root.ten_children
    assert sgl.BindingTestsRoot.get_child_count() == 10
    children = None
    assert sgl.BindingTestsRoot.get_child_count() == 10
    root = None
    assert sgl.BindingTestsRoot.get_root_count() == 0
    assert sgl.BindingTestsRoot.get_child_count() == 10


# Same result with explicit ref internal
def test_prop_create_ten_raw_children_rvrefinternal():
    root = sgl.BindingTestsRoot.create()
    children = root.ten_children_rvrefinternal
    assert sgl.BindingTestsRoot.get_child_count() == 10
    children = None
    assert sgl.BindingTestsRoot.get_child_count() == 10
    root = None
    assert sgl.BindingTestsRoot.get_root_count() == 0
    assert sgl.BindingTestsRoot.get_child_count() == 10


# And again with ref
def test_prop_create_ten_raw_children_rvref():
    root = sgl.BindingTestsRoot.create()
    children = root.ten_children_rvref
    assert sgl.BindingTestsRoot.get_child_count() == 10
    children = None
    assert sgl.BindingTestsRoot.get_child_count() == 10
    root = None
    assert sgl.BindingTestsRoot.get_root_count() == 0
    assert sgl.BindingTestsRoot.get_child_count() == 10


# But not with take ownership, in which children are freed
# when the vector is freed.
def test_prop_create_ten_raw_children_rvtakeownership():
    root = sgl.BindingTestsRoot.create()
    children = root.ten_children_rvtakeownership
    assert sgl.BindingTestsRoot.get_child_count() == 10
    children = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    root = None
    assert sgl.BindingTestsRoot.get_root_count() == 0
    assert sgl.BindingTestsRoot.get_child_count() == 0


# Creation of ten children via function, which defaults
# to take ownership.
def test_func_create_ten_raw_children():
    root = sgl.BindingTestsRoot.create()
    children = root.get_ten_children()
    assert sgl.BindingTestsRoot.get_child_count() == 10
    children = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    root = None
    assert sgl.BindingTestsRoot.get_root_count() == 0
    assert sgl.BindingTestsRoot.get_child_count() == 0


# Ref internal doesn't free them
def test_func_create_ten_raw_children_rvrefinternal():
    root = sgl.BindingTestsRoot.create()
    children = root.get_ten_children_rvrefinternal()
    assert sgl.BindingTestsRoot.get_child_count() == 10
    children = None
    assert sgl.BindingTestsRoot.get_child_count() == 10
    root = None
    assert sgl.BindingTestsRoot.get_root_count() == 0
    assert sgl.BindingTestsRoot.get_child_count() == 10


# And again with ref
def test_func_create_ten_raw_children_rvref():
    root = sgl.BindingTestsRoot.create()
    children = root.get_ten_children_rvref()
    assert sgl.BindingTestsRoot.get_child_count() == 10
    children = None
    assert sgl.BindingTestsRoot.get_child_count() == 10
    root = None
    assert sgl.BindingTestsRoot.get_root_count() == 0
    assert sgl.BindingTestsRoot.get_child_count() == 10


# But not with take ownership, in which children are freed
# when the vector is freed.
def test_func_create_ten_raw_children_rvtakeownership():
    root = sgl.BindingTestsRoot.create()
    children = root.get_ten_children_rvtakeownership()
    assert sgl.BindingTestsRoot.get_child_count() == 10
    children = None
    assert sgl.BindingTestsRoot.get_child_count() == 0
    root = None
    assert sgl.BindingTestsRoot.get_root_count() == 0
    assert sgl.BindingTestsRoot.get_child_count() == 0


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
