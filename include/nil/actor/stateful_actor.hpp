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

#include <new>
#include <type_traits>

#include <nil/actor/fwd.hpp>
#include <nil/actor/sec.hpp>

#include <nil/actor/detail/type_traits.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            /// Conditional base type for `stateful_actor` that overrides `make_behavior` if
            /// `State::make_behavior()` exists.
            template<class State, class Base>
            class stateful_actor_base : public Base {
            public:
                using Base::Base;

                typename Base::behavior_type make_behavior() override;
            };

            /// Evaluates to either `stateful_actor_base<State, Base> `or `Base`, depending
            /// on whether `State::make_behavior()` exists.
            template<class State, class Base>
            using stateful_actor_base_t =
                std::conditional_t<has_make_behavior_member<State>::value, stateful_actor_base<State, Base>, Base>;

        }    // namespace detail
    }        // namespace actor
}    // namespace nil

namespace nil {
    namespace actor {

        /// An event-based actor with managed state. The state is constructed with the
        /// actor, but destroyed when the actor calls `quit`. This state management
        /// brakes cycles and allows actors to automatically release resources as soon
        /// as possible.
        template<class State, class Base /* = event_based_actor (see fwd.hpp) */>
        class stateful_actor : public detail::stateful_actor_base_t<State, Base> {
        public:
            using super = detail::stateful_actor_base_t<State, Base>;

            template<class... Ts>
            explicit stateful_actor(actor_config &cfg, Ts &&... xs) : super(cfg) {
                if constexpr (std::is_constructible<State, Ts &&...>::value)
                    new (&state) State(std::forward<Ts>(xs)...);
                else
                    new (&state) State(this, std::forward<Ts>(xs)...);
            }

            ~stateful_actor() override {
                // nop
            }

            /// @copydoc local_actor::on_exit
            /// @note when overriding this member function, make sure to call
            ///       `super::on_exit()` in order to clean up the state.
            void on_exit() override {
                state.~State();
            }

            const char *name() const override {
                if constexpr (detail::has_name<State>::value) {
                    if constexpr (std::is_convertible<decltype(state.name), const char *>::value)
                        return state.name;
                    else
                        return state.name.c_str();
                } else {
                    return Base::name();
                }
            }

            union {
                /// The actor's state. This member lives inside a union since its lifetime
                /// ends when the actor terminates while the actual actor object lives until
                /// its reference count drops to zero.
                State state;
            };
        };
    }    // namespace actor
}    // namespace nil

namespace nil {
    namespace actor {
        namespace detail {

            template<class State, class Base>
            typename Base::behavior_type stateful_actor_base<State, Base>::make_behavior() {
                auto dptr = static_cast<stateful_actor<State, Base> *>(this);
                return dptr->state.make_behavior();
            }
        }    // namespace detail
    }        // namespace actor
}    // namespace nil