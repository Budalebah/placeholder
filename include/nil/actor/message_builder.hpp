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

#include <vector>


#include <nil/actor/detail/implicit_conversions.hpp>
#include <nil/actor/detail/message_builder_element.hpp>
#include <nil/actor/detail/type_id_list_builder.hpp>
#include <nil/actor/fwd.hpp>
#include <nil/actor/message.hpp>

namespace nil {
    namespace actor {

        /// Provides a convenient interface for creating `message` objects
        /// from a series of values using the member function `append`.
        class BOOST_SYMBOL_VISIBLE message_builder {
        public:
            friend class message;

            message_builder() = default;

            message_builder(const message_builder &) = delete;

            message_builder &operator=(const message_builder &) = delete;

            /// Creates a new instance and immediately calls `append(first, last)`.
            template<class Iter>
            message_builder(Iter first, Iter last) {
                append(first, last);
            }

            /// Appends all values in range [first, last).
            template<class Iter>
            message_builder &append(Iter first, Iter last) {
                for (; first != last; ++first)
                    append(*first);
                return *this;
            }

            /// Adds `x` to the elements of the buffer.
            template<class T>
            message_builder &append(T &&x) {
                using namespace detail;
                using value_type = strip_and_convert_t<T>;
                static_assert(sendable<value_type>);
                types_.push_back(type_id_v<value_type>);
                elements_.emplace_back(make_message_builder_element(std::forward<T>(x)));
                return *this;
            }

            template<class... Ts>
            message_builder &append_all(Ts &&... xs) {
                (append(std::forward<Ts>(xs)), ...);
                return *this;
            }

            template<class Tuple, size_t... Is>
            message_builder &append_tuple(Tuple &xs, std::index_sequence<Is...>) {
                (append(std::get<Is>(xs)), ...);
                return *this;
            }

            template<class... Ts>
            message_builder &append_tuple(std::tuple<Ts...> xs) {
                return append_tuple(std::integral_constant<size_t, 0> {},
                                    std::integral_constant<size_t, sizeof...(Ts)> {}, xs);
            }

            /// Converts the buffer to an actual message object without
            /// invalidating this message builder (nor clearing it).
            message to_message() const;

            /// Converts the buffer to an actual message object and transfers
            /// ownership of the data to it, leaving this object in an invalid state.
            /// @warning Calling *any*  member function on this object afterwards
            ///          is undefined behavior (dereferencing a `nullptr`)
            message move_to_message();

            /// Removes all elements from the buffer.
            void clear() noexcept;

            /// Returns whether the buffer is empty.
            bool empty() const noexcept {
                return elements_.empty();
            }

            /// Returns the number of elements in the buffer.
            size_t size() const noexcept {
                return elements_.size();
            }

        private:
            detail::type_id_list_builder types_;
            std::vector<detail::message_builder_element_ptr> elements_;
        };

    }    // namespace actor
}    // namespace nil
