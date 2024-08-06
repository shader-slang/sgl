// SPDX-License-Identifier: Apache-2.0

/**
 * This is a modified version of the struct.h file from Mitsuba 3.0 found at
 * https://github.com/mitsuba-renderer/mitsuba3/blob/master/include/mitsuba/core/struct.h
 *
 * Original license below:
 *
 * Copyright (c) 2017 Wenzel Jakob <wenzel.jakob\epfl.ch>, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You are under no obligation whatsoever to provide any bug fixes, patches, or
 * upgrades to the features, functionality or performance of the source code
 * ("Enhancements") to anyone; however, if you choose to make your Enhancements
 * available either publicly, or directly to the author of this software, without
 * imposing a separate written license agreement for such Enhancements, then you
 * hereby grant the following license: a non-exclusive, royalty-free perpetual
 * license to install, use, modify, prepare derivative works, incorporate into
 * other computer software, distribute, and sublicense such enhancements or
 * derivative works thereof, in binary and source code form.
 */

#pragma once

#include "sgl/core/macros.h"
#include "sgl/core/object.h"

#include <utility>
#include <set>

namespace sgl {

class BindingTestsRoot;

class SGL_API BindingTestsChild {
public:
    BindingTestsChild(){};
    BindingTestsChild(BindingTestsRoot* root)
        : m_root(root){};

    ~BindingTestsChild();

    BindingTestsRoot* m_root{nullptr};
};

class SGL_API BindingTestsChildRefCounted : Object {
    SGL_OBJECT(BindingTestChildRefCounted)
public:
    BindingTestsChildRefCounted(){};
    BindingTestsChildRefCounted(BindingTestsRoot* root)
        : Object()
        , m_root(root){};

    ~BindingTestsChildRefCounted();

    BindingTestsRoot* m_root{nullptr};
};

class SGL_API BindingTestsRoot : Object {
    SGL_OBJECT(BindingTestsRoot)
public:
    static ref<BindingTestsRoot> init();

    ~BindingTestsRoot();

    const BindingTestsChild* get_child_const()
    {
        auto res = new BindingTestsChild(this);
        m_allocated_children.insert(res);
        return res;
    }
    BindingTestsChild* get_child_none_const()
    {
        auto res = new BindingTestsChild(this);
        m_allocated_children.insert(res);
        return res;
    }
    std::vector<BindingTestsChild*> get_ten_children()
    {
        std::vector<BindingTestsChild*> res;
        for (int i = 0; i < 10; i++) {
            res.push_back(get_child_none_const());
        }
        return res;
    }

    ref<const BindingTestsChildRefCounted> get_rc_child_const()
    {
        auto res = make_ref<const BindingTestsChildRefCounted>(this);
        m_allocated_ref_children.insert(res.get());
        return res;
    }
    ref<BindingTestsChildRefCounted> get_rc_child_none_const()
    {
        auto res = make_ref<BindingTestsChildRefCounted>(this);
        m_allocated_ref_children.insert(res.get());
        return res;
    }
    std::vector<ref<BindingTestsChildRefCounted>> get_ten_rc_children()
    {
        std::vector<ref<BindingTestsChildRefCounted>> res;
        for (int i = 0; i < 10; i++) {
            res.push_back(get_rc_child_none_const());
        }
        return res;
    }

    static void on_child_deleted(BindingTestsChild* child) { m_allocated_children.erase(child); }
    static void on_child_deleted(BindingTestsChildRefCounted* child) { m_allocated_ref_children.erase(child); }

    static std::set<const BindingTestsRoot*> m_allocated_roots;
    static std::set<const BindingTestsChild*> m_allocated_children;
    static std::set<const BindingTestsChildRefCounted*> m_allocated_ref_children;
};


} // namespace sgl
