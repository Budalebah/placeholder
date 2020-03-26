//---------------------------------------------------------------------------//
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2019 Aleksey Moskvin <zerg1996@yandex.ru>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_INJECT_HASH_HPP
#define CRYPTO3_INJECT_HASH_HPP

#include <boost/parameter/value_type.hpp>

#include <boost/accumulators/framework/accumulator_base.hpp>
#include <boost/accumulators/framework/extractor.hpp>
#include <boost/accumulators/framework/depends_on.hpp>
#include <boost/accumulators/framework/parameters/sample.hpp>

#include <boost/container/static_vector.hpp>

#include <nil/crypto3/detail/make_array.hpp>
#include <nil/crypto3/detail/static_digest.hpp>
#include <nil/crypto3/detail/stream_endian.hpp>
#include <nil/crypto3/detail/basic_functions.hpp>
#include <nil/crypto3/detail/unbounded_shift.hpp>


#include <nil/crypto3/hash/accumulators/bits_count.hpp>

#include <nil/crypto3/hash/accumulators/parameters/bits.hpp>
#include <nil/crypto3/hash/accumulators/parameters/salt.hpp>

#include <boost/accumulators/statistics/count.hpp>

namespace nil {
    namespace crypto3 {
        namespace detail {
            /*using 


                template<typename Endianness, std::size_t WordBits>
                struct injector: public ::nil::crypto3::detail::basic_functions<WordBits>;

                template<int UnitBits, std::size_t WordBits>
                struct injector<stream_endian::big_unit_big_bit<UnitBits>> {
                    static void inject(word_type w, length_type word_seen, block_type &b, length_type &block_seen) {
                        //Insert word_seen-bit part of word into the block b according to endianness

                        // Check whether we fall out of the block
                        if (block_seen + word_seen <= block_bits) {
                            length_type last_word_ind = block_seen / word_bits;
                            length_type last_word_seen = block_seen % word_bits;
                            // Remove garbage
                            w &= left_bits<word_type>(word_seen);
                            b[last_word_ind] &= left_bits<word_type>(last_word_seen);

                            // Add significant word bits to block word
                            b[last_word_ind] |= (w >> last_word_seen);
                            // If we fall out of the block word, push the remainder of element to the next block word
                            if (last_word_seen + word_seen > word_bits)
                                b[last_word_ind + 1] = w << (word_bits - last_word_seen);

                            block_seen += word_seen;
                        }
                    }

                    static void inject(const block_type &b_src, length_type b_src_seen, block_type &b_dst, length_type &b_dst_seen) {
                        //Insert word_seen-bit part of word into the block b according to endianness

                        // Check whether we fall out of the block
                        if (b_src_seen + b_dst_seen <= block_bits) {
                            
                            for (length_type i = 0; i< (b_src_seen / word_bits); i++){
                                inject(b_src[i], word_bits, b_dst, b_dst_seen);
                            }

                            if(b_src_seen%word_bits){
                                inject(b_src[b_src_seen / word_bits], b_src_seen%word_bits, b_dst, b_dst_seen);
                            }

                        }
                    }
                };

                template<int UnitBits>
                struct injector<stream_endian::little_unit_big_bit<UnitBits>> {
                    static void inject(word_type w, length_type word_seen, block_type &b, length_type block_seen) {
                        //Insert word_seen-bit part of word into the block b according to endianness

                        // Check whether we fall out of the block
                        if (block_seen + word_seen <= block_bits) {
                            length_type last_word_ind = block_seen / word_bits;
                            length_type last_word_seen = block_seen % word_bits;
                            // Remove garbage
                            length_type w_rem = word_seen % UnitBits;
                            length_type w_unit_bits = word_seen - w_rem;
                            word_type mask = right_bits<word_type>(w_unit_bits) | (right_bits<word_type>(w_rem) << (w_unit_bits + UnitBits - w_rem)); 
                            w &= mask; 
                            length_type b_rem = last_word_seen % UnitBits;
                            length_type b_unit_bits = last_word_seen - b_rem;
                            mask = right_bits<word_type>(b_unit_bits) | (right_bits<word_type>(b_rem) << (b_unit_bits + UnitBits - b_rem));
                            b[last_word_ind] &= mask;   
                            // Split and combine parts of unit values
                            length_type sz[2] = {UnitBits - b_rem, b_rem};
                            word_type masks[2] = {right_bits<word_type>(UnitBits - b_rem) << b_rem, right_bits<word_type>(b_rem)};
                            length_type bw_space = word_bits - last_word_seen;
                            word_type w_split = 0;
                            std::size_t sz_ind = 0;
                              
                            while (bw_space && w) {
                                w_split |= (!sz_ind ? ((w & masks[0]) >> b_rem) : ((w & masks[1]) << (UnitBits + sz[0]))); 
                                bw_space -= sz[sz_ind];
                                w &= ~masks[sz_ind];
                                masks[sz_ind] <<= UnitBits;
                                sz_ind = 1 - sz_ind;
                            }
                            // Add significant word bits to block word
                            b[last_word_ind] |= w_split << b_unit_bits;
                            // If we fall out of the block word, push the remainder of element to the next block word
                            if (w) {
                                w >>= (word_bits - b_unit_bits - UnitBits);
                                w_split = 0;
                                masks[0] = right_bits<word_type>(UnitBits - b_rem) << b_rem; 
                                masks[1] = right_bits<word_type>(b_rem);

                                while (w) {
                                    w_split |= (!sz_ind ? ((w & masks[0]) >> b_rem) : ((w & masks[1]) << (UnitBits + sz[0]))); 
                                    w &= ~masks[sz_ind];
                                    masks[sz_ind] <<= UnitBits;
                                    sz_ind = 1 - sz_ind;
                                }

                                b[last_word_ind + 1] = w_split >> UnitBits;
                            }

                            block_seen += word_seen;
                        }
                    }

                    static void inject(const block_type &b_src, length_type b_src_seen, block_type &b_dst, length_type &b_dst_seen) {
                        //Insert word_seen-bit part of word into the block b according to endianness

                        // Check whether we fall out of the block
                        if (b_src_seen + b_dst_seen <= block_bits) {
                            
                            for (length_type i = 0; i< (b_src_seen / word_bits); i++){
                                inject(b_src[i], word_bits, b_dst, b_dst_seen);
                            }

                            inject(b_src[b_src_seen / word_bits + (b_src_seen%word_bits? 1 : 0)], b_src_seen%word_bits, b_dst, b_dst_seen);

                        }
                    }
                };

                template<int UnitBits>
                struct injector<stream_endian::big_unit_little_bit<UnitBits>> {
                    static void inject(word_type w, length_type word_seen, block_type &b, length_type block_seen) {
                        //Insert word_seen-bit part of word into the block b according to endianness

                        // Check whether we fall out of the block
                        if (block_seen + word_seen <= block_bits) {
                            length_type last_word_ind = block_seen / word_bits;
                            length_type last_word_seen = block_seen % word_bits;
                            // Remove garbage
                            length_type w_rem = word_seen % UnitBits;
                            length_type w_unit_bits = word_seen - w_rem;
                            word_type mask = left_bits<word_type>(w_unit_bits) | (left_bits<word_type>(w_rem) >> (w_unit_bits + UnitBits - w_rem)); 
                            w &= mask; 
                            length_type b_rem = last_word_seen % UnitBits;
                            length_type b_unit_bits = last_word_seen - b_rem;
                            mask = left_bits<word_type>(b_unit_bits) | (left_bits<word_type>(b_rem) >> (b_unit_bits + UnitBits - b_rem));
                            b[last_word_ind] &= mask; 
                            // Split and combine parts of unit values 
                            length_type sz[2] = {UnitBits - b_rem, b_rem};
                            word_type masks[2] = {left_bits<word_type>(UnitBits - b_rem) >> b_rem, left_bits<word_type>(b_rem)};
                            length_type bw_space = word_bits - last_word_seen;
                            word_type w_split = 0;
                            std::size_t sz_ind = 0;
                             
                            while (bw_space && w) {
                                w_split |= (!sz_ind ? ((w & masks[0]) << b_rem) : ((w & masks[1]) >> (UnitBits + sz[0]))); 
                                bw_space -= sz[sz_ind];
                                w &= ~masks[sz_ind];
                                masks[sz_ind] >>= UnitBits;
                                sz_ind = 1 - sz_ind;
                            }
                            // Add significant word bits to block word
                            b[last_word_ind] |= w_split >> b_unit_bits;
                            // If we fall out of the block word, push the remainder of element to the next block word
                            if (w) {
                                w <<= (word_bits - b_unit_bits - UnitBits);
                                w_split = 0;
                                masks[0] = left_bits<word_type>(UnitBits - b_rem) >> b_rem; 
                                masks[1] = left_bits<word_type>(b_rem);

                                while (w) {
                                    w_split |= (!sz_ind ? ((w & masks[0]) << b_rem) : ((w & masks[1]) >> (UnitBits + sz[0]))); 
                                    w &= ~masks[sz_ind];
                                    masks[sz_ind] >>= UnitBits;
                                    sz_ind = 1 - sz_ind;
                                }

                                b[last_word_ind + 1] = w_split << UnitBits;
                            }
                            block_seen += word_seen;
                        }
                    }

                    static void inject(const block_type &b_src, length_type b_src_seen, block_type &b_dst, length_type &b_dst_seen) {
                        //Insert word_seen-bit part of word into the block b according to endianness

                        // Check whether we fall out of the block
                        if (b_src_seen + b_dst_seen <= block_bits) {
                            
                            for (length_type i = 0; i< (b_src_seen / word_bits); i++){
                                inject(b_src[i], word_bits, b_dst, b_dst_seen);
                            }

                            inject(b_src[b_src_seen / word_bits + (b_src_seen%word_bits? 1 : 0)], b_src_seen%word_bits, b_dst, b_dst_seen);

                        }
                    }
                };

                template<int UnitBits>
                struct injector<stream_endian::little_unit_little_bit<UnitBits>> {
                    static void inject(word_type w, length_type word_seen, block_type &b, length_type block_seen) {
                        //Insert word_seen-bit part of word into the block b according to endianness

                        // Check whether we fall out of the block
                        if (block_seen + word_seen <= block_bits) {
                            length_type last_word_ind = block_seen / word_bits;
                            length_type last_word_seen = block_seen % word_bits;
                            // Remove garbage
                            w &= right_bits<word_type>(word_seen);
                            b[last_word_ind] &= right_bits<word_type>(last_word_seen);
                            // Add significant word bits to block word
                            b[last_word_ind] |= (w << last_word_seen);
                            // If we fall out of the block word, push the remainder of element to the next block word
                            if (last_word_seen + word_seen > word_bits)
                                b[last_word_ind + 1] = w >> (word_bits - last_word_seen);

                            block_seen += word_seen;
                        }
                    }

                    static void inject(const block_type &b_src, length_type b_src_seen, block_type &b_dst, length_type &b_dst_seen) {
                        //Insert word_seen-bit part of word into the block b according to endianness

                        // Check whether we fall out of the block
                        if (b_src_seen + b_dst_seen <= block_bits) {
                            
                            for (length_type i = 0; i< (b_src_seen / word_bits); i++){
                                inject(b_src[i], word_bits, b_dst, b_dst_seen);
                            }

                            inject(b_src[b_src_seen / word_bits + (b_src_seen%word_bits? 1 : 0)], b_src_seen%word_bits, b_dst, b_dst_seen);

                        }
                    }
                };
        */            
        }        // namespace detail
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ACCUMULATORS_BLOCK_HPP