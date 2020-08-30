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

#include <nil/actor/fwd.hpp>

namespace nil {
    namespace actor {

        /**
         * Evaluates to true if `T` is a `typed_actor<...>`.
         */
        template<class T>
        struct is_typed_actor : std::false_type { };

        template<class... Ts>
        struct is_typed_actor<typed_actor<Ts...>> : std::true_type { };

    }    // namespace actor
}    // namespace nil
