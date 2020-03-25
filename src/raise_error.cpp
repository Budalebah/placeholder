//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
//
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt for Boost License or
// http://opensource.org/licenses/BSD-3-Clause for BSD 3-Clause License
//---------------------------------------------------------------------------//

#include <nil/actor/raise_error.hpp>

#include <nil/actor/logger.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            void log_cstring_error(const char *cstring) {
                ACTOR_IGNORE_UNUSED(cstring);
                ACTOR_LOG_ERROR(cstring);
            }

        }    // namespace detail
    }        // namespace actor
}    // namespace nil
