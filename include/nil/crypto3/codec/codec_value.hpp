//---------------------------------------------------------------------------//
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nilfoundation.org>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_CODEC_VALUE_HPP
#define CRYPTO3_CODEC_VALUE_HPP

#include <boost/assert.hpp>
#include <boost/concept_check.hpp>

#include <boost/range/concepts.hpp>

#include <nil/crypto3/codec/accumulators/codec.hpp>

namespace nil {
    namespace crypto3 {
        namespace codec {
            namespace detail {
                template<typename CodecAccumulatorSet>
                struct ref_codec_impl {
                    typedef CodecAccumulatorSet accumulator_set_type;
                    typedef typename boost::mpl::front<
                            typename accumulator_set_type::features_type>::type accumulator_type;

                    typedef typename accumulator_type::mode_type mode_type;
                    typedef typename mode_type::codec_type codec_type;

                    ref_codec_impl(const accumulator_set_type &acc) : accumulator_set(acc) {

                    }

                    accumulator_set_type &accumulator_set;
                };

                template<typename CodecAccumulatorSet>
                struct value_codec_impl {
                    typedef CodecAccumulatorSet accumulator_set_type;
                    typedef typename boost::mpl::front<
                            typename accumulator_set_type::features_type>::type accumulator_type;

                    typedef typename accumulator_type::mode_type mode_type;
                    typedef typename mode_type::codec_type codec_type;

                    value_codec_impl(const accumulator_set_type &acc) : accumulator_set(acc) {

                    }

                    mutable accumulator_set_type accumulator_set;
                };

                template<typename CodecStateImpl>
                struct range_codec_impl : public CodecStateImpl {
                    typedef CodecStateImpl codec_state_impl_type;

                    typedef typename codec_state_impl_type::accumulator_type accumulator_type;
                    typedef typename codec_state_impl_type::accumulator_set_type accumulator_set_type;

                    typedef typename codec_state_impl_type::mode_type mode_type;
                    typedef typename codec_state_impl_type::codec_type codec_type;

                    typedef typename boost::mpl::apply<accumulator_set_type,
                                                       accumulator_type>::type::result_type result_type;

                    template<typename SinglePassRange>
                    range_codec_impl(const SinglePassRange &range, const accumulator_set_type &ise)
                            : CodecStateImpl(ise) {
                        BOOST_RANGE_CONCEPT_ASSERT((boost::SinglePassRangeConcept<const SinglePassRange>));

                        typedef typename std::iterator_traits<
                                typename SinglePassRange::iterator>::value_type value_type;
                        BOOST_STATIC_ASSERT(std::numeric_limits<value_type>::is_specialized);
                        typedef typename codec_type::template stream_processor<mode_type, accumulator_set_type,
                                                                               std::numeric_limits<value_type>::digits +
                                                                               std::numeric_limits<
                                                                                       value_type>::is_signed>::type stream_processor;


                        stream_processor(this->accumulator_set)(range.begin(), range.end());
                    }

                    template<typename InputIterator>
                    range_codec_impl(InputIterator first, InputIterator last, const accumulator_set_type &ise)
                            : CodecStateImpl(ise) {
                        BOOST_CONCEPT_ASSERT((boost::InputIteratorConcept<InputIterator>));

                        typedef typename std::iterator_traits<InputIterator>::value_type value_type;
                        BOOST_STATIC_ASSERT(std::numeric_limits<value_type>::is_specialized);
                        typedef typename codec_type::template stream_processor<mode_type, accumulator_set_type,
                                                                               std::numeric_limits<value_type>::digits +
                                                                               std::numeric_limits<
                                                                                       value_type>::is_signed>::type stream_processor;

                        stream_processor(this->accumulator_set)(first, last);
                    }

                    template<typename OutputRange>
                    operator OutputRange() const {
                        result_type result = boost::accumulators::extract_result<accumulator_type>(
                                this->accumulator_set);
                        return OutputRange(result.cbegin(), result.cend());
                    }

                    operator result_type() const {
                        return boost::accumulators::extract_result<accumulator_type>(this->accumulator_set);
                    }

                    operator accumulator_set_type() const {
                        return this->accumulator_set;
                    }

#ifdef CRYPTO3_ASCII_STRING_CODEC_OUTPUT

                    template<typename Char, typename CharTraits, typename Alloc>
                    operator std::basic_string<Char, CharTraits, Alloc>() const {
                        return std::to_string(boost::accumulators::extract_result<accumulator_type>(
                                this->accumulator_set));
                    }

#endif
                };

                template<typename CodecStateImpl, typename OutputIterator>
                struct itr_codec_impl : public CodecStateImpl {
                private:
                    mutable OutputIterator out;

                public:
                    typedef CodecStateImpl codec_state_impl_type;

                    typedef typename codec_state_impl_type::accumulator_type accumulator_type;
                    typedef typename codec_state_impl_type::accumulator_set_type accumulator_set_type;

                    typedef typename codec_state_impl_type::mode_type mode_type;
                    typedef typename codec_state_impl_type::codec_type codec_type;

                    typedef typename boost::mpl::apply<accumulator_set_type,
                                                       accumulator_type>::type::result_type result_type;

                    template<typename SinglePassRange>
                    itr_codec_impl(const SinglePassRange &range, OutputIterator out, const accumulator_set_type &ise)
                            : CodecStateImpl(ise), out(std::move(out)) {
                        BOOST_CONCEPT_ASSERT((boost::SinglePassRangeConcept<const SinglePassRange>));

                        typedef typename std::iterator_traits<
                                typename SinglePassRange::iterator>::value_type value_type;
                        BOOST_STATIC_ASSERT(std::numeric_limits<value_type>::is_specialized);
                        typedef typename codec_type::template stream_processor<mode_type, accumulator_set_type,
                                                                               std::numeric_limits<value_type>::digits +
                                                                               std::numeric_limits<
                                                                                       value_type>::is_signed>::type stream_processor;


                        stream_processor(this->accumulator_set)(range.begin(), range.end());
                    }

                    template<typename InputIterator>
                    itr_codec_impl(InputIterator first, InputIterator last, OutputIterator out,
                                   const accumulator_set_type &ise)
                            : CodecStateImpl(ise), out(std::move(out)) {
                        BOOST_CONCEPT_ASSERT((boost::InputIteratorConcept<InputIterator>));

                        typedef typename std::iterator_traits<InputIterator>::value_type value_type;
                        BOOST_STATIC_ASSERT(std::numeric_limits<value_type>::is_specialized);
                        typedef typename codec_type::template stream_processor<mode_type, accumulator_set_type,
                                                                               std::numeric_limits<value_type>::digits +
                                                                               std::numeric_limits<
                                                                                       value_type>::is_signed>::type stream_processor;


                        stream_processor(this->accumulator_set)(first, last);
                    }

                    operator OutputIterator() const {
                        result_type result = boost::accumulators::extract_result<accumulator_type>(
                                this->accumulator_set);

                        return std::move(result.cbegin(), result.cend(), out);
                    }
                };
            }
        }
    }
}

#endif //CRYPTO3_CODEC_VALUE_HPP
