//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2017-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/config.hpp>

// exclude this suite; seems to be too much to swallow for MSVC
#ifndef BOOST_OS_WINDOWS_AVAILABLE

#define BOOST_TEST_MODULE typed_spawn

#include <nil/actor/test/dsl.hpp>

#include <nil/actor/string_algorithms.hpp>

#include <nil/actor/all.hpp>

#define ERROR_HANDLER [&](error &err) { BOOST_FAIL(sys.render(err)); }

using std::string;

using namespace nil::actor;
using namespace std::string_literals;

namespace boost {
    namespace test_tools {
        namespace tt_detail {
            template<template<typename...> class P, typename... T>
            struct print_log_value<P<T...>> {
                void operator()(std::ostream &, P<T...> const &) {
                }
            };
            template<>
            struct print_log_value<error> {
                void operator()(std::ostream &, error const &) {
                }
            };
            template<>
            struct print_log_value<exit_reason> {
                void operator()(std::ostream &, exit_reason const &) {
                }
            };
        }    // namespace tt_detail
    }        // namespace test_tools
}    // namespace boost

namespace {

    // check invariants of type system
    using dummy1 = typed_actor<reacts_to<int, int>, replies_to<double>::with<double>>;

    using dummy2 = dummy1::extend<reacts_to<ok_atom>>;

    static_assert(std::is_convertible<dummy2, dummy1>::value, "handle not assignable to narrower definition");

    using dummy3 = typed_actor<reacts_to<float, int>>;
    using dummy4 = typed_actor<replies_to<int>::with<double>>;
    using dummy5 = dummy4::extend_with<dummy3>;

    static_assert(std::is_convertible<dummy5, dummy3>::value, "handle not assignable to narrower definition");

    static_assert(std::is_convertible<dummy5, dummy4>::value, "handle not assignable to narrower definition");

    /******************************************************************************
     *                        simple request/response test                        *
     ******************************************************************************/

    using server_type = typed_actor<replies_to<my_request>::with<bool>>;

    server_type::behavior_type typed_server1() {
        return {
            [](const my_request &req) { return req.a == req.b; },
        };
    }

    server_type::behavior_type typed_server2(server_type::pointer) {
        return typed_server1();
    }

    class typed_server3 : public server_type::base {
    public:
        typed_server3(actor_config &cfg, const string &line, actor buddy) : server_type::base(cfg) {
            anon_send(buddy, line);
        }

        behavior_type make_behavior() override {
            return typed_server2(this);
        }
    };

    void client(event_based_actor *self, const actor &parent, const server_type &serv) {
        self->request(serv, infinite, my_request {0, 0}).then([=](bool val1) {
            BOOST_CHECK_EQUAL(val1, true);
            self->request(serv, infinite, my_request {10, 20}).then([=](bool val2) {
                BOOST_CHECK_EQUAL(val2, false);
                self->send(parent, ok_atom_v);
            });
        });
    }

    /******************************************************************************
     *          test skipping of messages intentionally + using become()          *
     ******************************************************************************/

    using event_testee_type = typed_actor<replies_to<get_state_atom>::with<string>,
                                          replies_to<string>::with<void>,
                                          replies_to<float>::with<void>,
                                          replies_to<int>::with<int>>;

    class event_testee : public event_testee_type::base {
    public:
        event_testee(actor_config &cfg) : event_testee_type::base(cfg) {
            // nop
        }

        behavior_type wait4string() {
            return {
                [=](get_state_atom) { return "wait4string"; },
                [=](const string &) { become(wait4int()); },
                [=](float) { return skip(); },
                [=](int) { return skip(); },
            };
        }

        behavior_type wait4int() {
            return {
                [=](get_state_atom) { return "wait4int"; },
                [=](int) -> int {
                    become(wait4float());
                    return 42;
                },
                [=](float) { return skip(); },
                [=](const string &) { return skip(); },
            };
        }

        behavior_type wait4float() {
            return {
                [=](get_state_atom) { return "wait4float"; },
                [=](float) { become(wait4string()); },
                [=](const string &) { return skip(); },
                [=](int) { return skip(); },
            };
        }

        behavior_type make_behavior() override {
            return wait4int();
        }
    };

    /******************************************************************************
     *                         simple 'forwarding' chain                          *
     ******************************************************************************/

    using string_actor = typed_actor<replies_to<string>::with<string>>;

    string_actor::behavior_type string_reverter() {
        return {
            [](string &str) -> string {
                std::reverse(str.begin(), str.end());
                return std::move(str);
            },
        };
    }

    // uses `return delegate(...)`
    string_actor::behavior_type string_delegator(string_actor::pointer self, string_actor master, bool leaf) {
        auto next = leaf ? self->spawn(string_delegator, master, false) : master;
        self->link_to(next);
        return {
            [=](string &str) -> delegated<string> { return self->delegate(next, std::move(str)); },
        };
    }

    using maybe_string_actor = typed_actor<replies_to<string>::with<ok_atom, string>>;

    maybe_string_actor::behavior_type maybe_string_reverter() {
        return {
            [](string &str) -> result<ok_atom, string> {
                if (str.empty())
                    return sec::invalid_argument;
                std::reverse(str.begin(), str.end());
                return {ok_atom_v, std::move(str)};
            },
        };
    }

    maybe_string_actor::behavior_type maybe_string_delegator(maybe_string_actor::pointer self,
                                                             const maybe_string_actor &x) {
        self->link_to(x);
        return {
            [=](string &s) -> delegated<ok_atom, string> { return self->delegate(x, std::move(s)); },
        };
    }

    /******************************************************************************
     *                        sending typed actor handles                         *
     ******************************************************************************/

    int_actor::behavior_type int_fun() {
        return {
            [](int i) { return i * i; },
        };
    }

    behavior foo(event_based_actor *self) {
        return {
            [=](int i, int_actor server) {
                self->delegate(server, i);
                self->quit();
            },
        };
    }

    int_actor::behavior_type int_fun2(int_actor::pointer self) {
        self->set_down_handler([=](down_msg &dm) {
            BOOST_CHECK_EQUAL(dm.reason, exit_reason::normal);
            self->quit();
        });
        return {
            [=](int i) {
                self->monitor(self->current_sender());
                return i * i;
            },
        };
    }

    behavior foo2(event_based_actor *self) {
        return {
            [=](int i, int_actor server) {
                self->delegate(server, i);
                self->quit();
            },
        };
    }

    float_actor::behavior_type float_fun(float_actor::pointer self) {
        return {
            [=](float a) {
                BOOST_CHECK_EQUAL(a, 1.0f);
                self->quit(exit_reason::user_shutdown);
            },
        };
    }

    int_actor::behavior_type foo3(int_actor::pointer self) {
        auto b = self->spawn<linked>(float_fun);
        self->send(b, 1.0f);
        return {
            [=](int) { return 0; },
        };
    }

    struct fixture : test_coordinator_fixture<> {
        void test_typed_spawn(server_type ts) {
            BOOST_TEST_MESSAGE("the server returns false for inequal numbers");
            inject((my_request), from(self).to(ts).with(my_request {1, 2}));
            expect((bool), from(ts).to(self).with(false));
            BOOST_TEST_MESSAGE("the server returns true for equal numbers");
            inject((my_request), from(self).to(ts).with(my_request {42, 42}));
            expect((bool), from(ts).to(self).with(true));
            BOOST_CHECK_EQUAL(sys.registry().running(), 2u);
            auto c1 = self->spawn(client, self, ts);
            run();
            expect((ok_atom), from(c1).to(self).with(ok_atom_v));
            BOOST_CHECK_EQUAL(sys.registry().running(), 2u);
        }
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(typed_spawn_tests, fixture)

/******************************************************************************
 *                             put it all together                            *
 ******************************************************************************/

BOOST_AUTO_TEST_CASE(typed_spawns) {
    BOOST_TEST_MESSAGE("run test series with typed_server1");
    test_typed_spawn(sys.spawn(typed_server1));
    self->await_all_other_actors_done();
    BOOST_TEST_MESSAGE("finished test series with `typed_server1`");
    BOOST_TEST_MESSAGE("run test series with typed_server2");
    test_typed_spawn(sys.spawn(typed_server2));
    self->await_all_other_actors_done();
    BOOST_TEST_MESSAGE("finished test series with `typed_server2`");
    auto serv3 = self->spawn<typed_server3>("hi there", self);
    run();
    expect((string), from(serv3).to(self).with("hi there"s));
    test_typed_spawn(serv3);
}

BOOST_AUTO_TEST_CASE(event_testee_series) {
    auto et = self->spawn<event_testee>();
    BOOST_TEST_MESSAGE("et->message_types() returns an interface description");
    typed_actor<replies_to<get_state_atom>::with<string>> sub_et = et;
    std::set<string> iface {
        "nil::actor::replies_to<get_state_atom>::with<std::string>", "nil::actor::replies_to<std::string>::with<void>",
        "nil::actor::replies_to<float>::with<void>", "nil::actor::replies_to<int32_t>::with<int32_t>"};
    BOOST_CHECK_EQUAL(join(sub_et->message_types(), ","), join(iface, ","));
    BOOST_TEST_MESSAGE("the testee skips messages to drive its internal state machine");
    self->send(et, 1);
    self->send(et, 2);
    self->send(et, 3);
    self->send(et, .1f);
    self->send(et, "hello event testee!"s);
    self->send(et, .2f);
    self->send(et, .3f);
    self->send(et, "hello again event testee!"s);
    self->send(et, "goodbye event testee!"s);
    run();
    expect((int), from(et).to(self).with(42));
    expect((int), from(et).to(self).with(42));
    expect((int), from(et).to(self).with(42));
    inject((get_state_atom), from(self).to(sub_et).with(get_state_atom_v));
    expect((string), from(et).to(self).with("wait4int"s));
}

BOOST_AUTO_TEST_CASE(string_delegator_chain) {
    // run test series with string reverter
    auto aut = self->spawn<monitored>(string_delegator, sys.spawn(string_reverter), true);
    std::set<string> iface {"nil::actor::replies_to<std::string>::with<std::string>"};
    BOOST_CHECK_EQUAL(aut->message_types(), iface);
    inject((string), from(self).to(aut).with("Hello World!"s));
    run();
    expect((string), to(self).with("!dlroW olleH"s));
}

BOOST_AUTO_TEST_CASE(maybe_string_delegator_chain) {
    ACTOR_LOG_TRACE(ACTOR_ARG(self));
    auto aut = sys.spawn(maybe_string_delegator, sys.spawn(maybe_string_reverter));
    BOOST_TEST_MESSAGE("send empty string, expect error");
    inject((string), from(self).to(aut).with(""s));
    run();
    expect((error), to(self).with(sec::invalid_argument));
    BOOST_TEST_MESSAGE("send abcd string, expect dcba");
    inject((string), from(self).to(aut).with("abcd"s));
    run();
    expect((ok_atom, string), to(self).with(ok_atom_v, "dcba"s));
}

BOOST_AUTO_TEST_CASE(sending_typed_actors) {
    auto aut = sys.spawn(int_fun);
    self->send(self->spawn(foo), 10, aut);
    run();
    expect((int), to(self).with(100));
    self->spawn(foo3);
    run();
}

BOOST_AUTO_TEST_CASE(sending_typed_actors_and_down_msg) {
    auto aut = sys.spawn(int_fun2);
    self->send(self->spawn(foo2), 10, aut);
    run();
    expect((int), to(self).with(100));
}

BOOST_AUTO_TEST_CASE(check_signature) {
    using foo_type = typed_actor<replies_to<put_atom>::with<ok_atom>>;
    using foo_result_type = optional<ok_atom>;
    using bar_type = typed_actor<reacts_to<ok_atom>>;
    auto foo_action = [](foo_type::pointer ptr) -> foo_type::behavior_type {
        return {
            [=](put_atom) -> foo_result_type {
                ptr->quit();
                return {ok_atom_v};
            },
        };
    };
    auto bar_action = [=](bar_type::pointer ptr) -> bar_type::behavior_type {
        auto foo = ptr->spawn<linked>(foo_action);
        ptr->send(foo, put_atom_v);
        return {
            [=](ok_atom) { ptr->quit(); },
        };
    };
    auto x = self->spawn(bar_action);
    run();
}

BOOST_AUTO_TEST_SUITE_END()

#endif    // BOOST_OS_WINDOWS_AVAILABLE
