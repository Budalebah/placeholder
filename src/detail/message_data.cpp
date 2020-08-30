//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2017-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/detail/message_data.hpp>

#include <cstring>
#include <numeric>

#include <nil/actor/detail/meta_object.hpp>
#include <nil/actor/error.hpp>
#include <nil/actor/error_code.hpp>
#include <nil/actor/sec.hpp>
#include <nil/actor/span.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            message_data::message_data(type_id_list types) : rc_(1), types_(std::move(types)) {
                // nop
            }

            message_data::~message_data() noexcept {
                // TODO: we unconditionally destroy all objects without some way of telling
                //       whether these objects were constructed in the first place.
                auto gmos = global_meta_objects();
                auto ptr = storage();
                // TODO: C++ usually destroys members in reverse order.
                for (auto id : types_) {
                    auto &meta = gmos[id];
                    meta.destroy(ptr);
                    ptr += meta.padded_size;
                }
            }

            message_data *message_data::copy() const {
                auto gmos = global_meta_objects();
                size_t storage_size = 0;
                for (auto id : types_)
                    storage_size += gmos[id].padded_size;
                auto total_size = sizeof(message_data) + storage_size;
                auto vptr = malloc(total_size);
                if (vptr == nullptr)
                    throw std::bad_alloc();
                auto ptr = new (vptr) message_data(types_);
                auto src = storage();
                auto dst = ptr->storage();
                for (auto id : types_) {
                    auto &meta = gmos[id];
                    // TODO: exception handling.
                    meta.copy_construct(dst, src);
                    src += meta.padded_size;
                    dst += meta.padded_size;
                }
                return ptr;
            }

            byte *message_data::at(size_t index) noexcept {
                if (index == 0)
                    return storage();
                auto gmos = global_meta_objects();
                auto ptr = storage();
                for (size_t i = 0; i < index; ++i)
                    ptr += gmos[types_[i]].padded_size;
                return ptr;
            }

            const byte *message_data::at(size_t index) const noexcept {
                if (index == 0)
                    return storage();
                auto gmos = global_meta_objects();
                auto ptr = storage();
                for (size_t i = 0; i < index; ++i)
                    ptr += gmos[types_[i]].padded_size;
                return ptr;
            }

            error message_data::save(serializer &sink) const {
                auto gmos = global_meta_objects();
                auto ptr = storage();
                for (auto id : types_) {
                    auto &meta = gmos[id];
                    if (auto err = detail::save(meta, sink, ptr))
                        return err;
                    ptr += meta.padded_size;
                }
                return none;
            }

            error message_data::save(binary_serializer &sink) const {
                auto gmos = global_meta_objects();
                auto ptr = storage();
                for (auto id : types_) {
                    auto &meta = gmos[id];
                    if (auto err = detail::save(meta, sink, ptr))
                        return err;
                    ptr += meta.padded_size;
                }
                return none;
            }

            error message_data::load(deserializer &source) {
                auto gmos = global_meta_objects();
                auto ptr = storage();
                for (auto id : types_) {
                    auto &meta = gmos[id];
                    if (auto err = detail::load(meta, source, ptr))
                        return err;
                    ptr += meta.padded_size;
                }
                return none;
            }

            error message_data::load(binary_deserializer &source) {
                auto gmos = global_meta_objects();
                auto ptr = storage();
                for (auto id : types_) {
                    auto &meta = gmos[id];
                    if (auto err = detail::load(meta, source, ptr))
                        return err;
                    ptr += meta.padded_size;
                }
                return none;
            }

        }    // namespace detail
    }        // namespace actor
}    // namespace nil