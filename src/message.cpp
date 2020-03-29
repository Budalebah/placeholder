//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2017-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/message.hpp>

#include <utility>

#include <nil/actor/spawner.hpp>
#include <nil/actor/binary_deserializer.hpp>
#include <nil/actor/binary_serializer.hpp>
#include <nil/actor/deserializer.hpp>
#include <nil/actor/detail/meta_object.hpp>
#include <nil/actor/detail/type_id_list_builder.hpp>
#include <nil/actor/message_builder.hpp>
#include <nil/actor/message_handler.hpp>
#include <nil/actor/serializer.hpp>
#include <nil/actor/string_algorithms.hpp>

namespace nil {
    namespace actor {

        namespace {

            template<class Deserializer>
            typename Deserializer::result_type load_data(Deserializer &source, message::data_ptr &data) {
                uint16_t ids_size = 0;
                if (auto err = source.apply(ids_size))
                    return err;
                if (ids_size == 0) {
                    data.reset();
                    return nil::actor::none;
                }
                detail::type_id_list_builder ids;
                ids.reserve(ids_size + 1);    // +1 for the prefixed size
                for (size_t i = 0; i < ids_size; ++i) {
                    type_id_t id = 0;
                    if (auto err = source.apply(id))
                        return err;
                    ids.push_back(id);
                }
                ACTOR_ASSERT(ids.size() == ids_size);
                auto gmos = detail::global_meta_objects();
                size_t data_size = 0;
                for (size_t i = 0; i < ids_size; ++i) {
                    auto id = ids[i];
                    if (id >= gmos.size())
                        return sec::unknown_type;
                    auto &mo = gmos[id];
                    if (mo.type_name == nullptr)
                        return sec::unknown_type;
                    data_size += mo.padded_size;
                }
                auto vptr = malloc(sizeof(detail::message_data) + data_size);
                auto ptr = new (vptr) detail::message_data(ids.move_to_list());
                auto pos = ptr->storage();
                auto types = ptr->types();
                for (auto i = 0; i < ids_size; ++i) {
                    auto &meta = gmos[types[i]];
                    meta.default_construct(pos);
                    if (auto err = load(meta, source, pos)) {
                        auto rpos = pos;
                        for (auto j = i; j > 0; --j) {
                            auto &jmeta = gmos[types[j]];
                            jmeta.destroy(rpos);
                            rpos -= jmeta.padded_size;
                        }
                        ptr->~message_data();
                        free(vptr);
                        return err;
                    }
                    pos += meta.padded_size;
                }
                data.reset(ptr, false);
                return nil::actor::none;
            }

        }    // namespace

        error message::load(deserializer &source) {
            return load_data(source, data_);
        }

        error_code<sec> message::load(binary_deserializer &source) {
            return load_data(source, data_);
        }

        namespace {

            template<class Serializer>
            typename Serializer::result_type save_data(Serializer &sink, const message::data_ptr &data) {
                // Short-circuit empty tuples.
                if (data == nullptr) {
                    uint16_t zero = 0;
                    return sink(zero);
                }
                // Write type information.
                auto type_ids = data->types();
                auto type_ids_size = static_cast<uint16_t>(type_ids.size());
                if (auto err = sink(type_ids_size))
                    return err;
                for (auto id : type_ids)
                    if (auto err = sink(id))
                        return err;
                // Write elements.
                auto gmos = detail::global_meta_objects();
                auto storage = data->storage();
                for (auto id : type_ids) {
                    auto &meta = gmos[id];
                    if (auto err = save(meta, sink, storage))
                        return err;
                    storage += meta.padded_size;
                }
                return {};
            }

        }    // namespace

        error message::save(serializer &sink) const {
            return save_data(sink, data_);
        }

        error_code<sec> message::save(binary_serializer &sink) const {
            return save_data(sink, data_);
        }

        // -- related non-members ------------------------------------------------------

        error inspect(serializer &sink, const message &msg) {
            return msg.save(sink);
        }

        error_code<sec> inspect(binary_serializer &sink, const message &msg) {
            return msg.save(sink);
        }

        error inspect(deserializer &source, message &msg) {
            return msg.load(source);
        }

        error_code<sec> inspect(binary_deserializer &source, message &msg) {
            return msg.load(source);
        }

        std::string to_string(const message &msg) {
            if (msg.empty())
                return "<empty-message>";
            std::string result;
            result += '(';
            auto types = msg.types();
            if (!types.empty()) {
                auto ptr = msg.cdata().storage();
                auto meta = detail::global_meta_object(types[0]);
                ACTOR_ASSERT(meta != nullptr);
                meta->stringify(result, ptr);
                ptr += meta->padded_size;
                for (size_t index = 1; index < types.size(); ++index) {
                    result += ", ";
                    meta = detail::global_meta_object(types[index]);
                    ACTOR_ASSERT(meta != nullptr);
                    meta->stringify(result, ptr);
                    ptr += meta->padded_size;
                }
            }
            result += ')';
            return result;
        }

    }    // namespace actor
}    // namespace nil
