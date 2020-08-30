//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2017-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/detail/invoke_result_visitor.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            invoke_result_visitor::~invoke_result_visitor() {
                // nop
            }

        }    // namespace detail
    }        // namespace actor
}    // namespace nil