//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2017-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace nil {
    namespace actor {
        namespace detail {

            template<class To, bool LargeUnsigned = sizeof(To) >= sizeof(int64_t) && std::is_unsigned<To>::value>
            struct bounds_checker {
                static inline bool check(int64_t x) {
                    return x >= std::numeric_limits<To>::min() && x <= std::numeric_limits<To>::max();
                }
            };

            template<class To>
            struct bounds_checker<To, true> {
                static inline bool check(int64_t x) {
                    return x >= 0;
                }
            };
        }    // namespace detail
    }        // namespace actor
}    // namespace nil