//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2017-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <nil/actor/fwd.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            /// A central place where workers return to after finishing a task. A hub
            /// supports any number of workers that call `push`, but only a single master
            /// that calls `pop`. The hub takes ownership of all workers. Workers register
            /// at the hub during construction and get destroyed when the hub gets
            /// destroyed.
            class BOOST_SYMBOL_VISIBLE abstract_worker_hub {
            public:
                // -- constructors, destructors, and assignment operators --------------------

                abstract_worker_hub();

                virtual ~abstract_worker_hub();

                // -- synchronization --------------------------------------------------------

                /// Waits until all workers are back at the hub.
                void await_workers();

            protected:
                // -- worker management ------------------------------------------------------

                /// Adds a new worker to the hub.
                void push_new(abstract_worker *ptr);

                /// Returns a worker to the hub.
                void push_returning(abstract_worker *ptr);

                /// Tries to retrieve a worker from the hub.
                /// @returns the next available worker (in LIFO order) or `nullptr` if the
                ///          hub is currently empty.
                abstract_worker *pop_impl();

                /// Checks which worker would `pop` currently return.
                /// @returns the next available worker (in LIFO order) or `nullptr` if the
                ///          hub is currently empty.
                abstract_worker *peek_impl();

                // -- member variables -------------------------------------------------------

                std::atomic<abstract_worker *> head_;

                std::atomic<size_t> running_;

                std::mutex mtx_;

                std::condition_variable cv_;
            };

        }    // namespace detail
    }        // namespace actor
}    // namespace nil