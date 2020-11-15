//---------------------------------------------------------------------------//
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#ifndef CRYPTO3_ZK_MEMORY_CONTENTS_EXAMPLES_HPP
#define CRYPTO3_ZK_MEMORY_CONTENTS_EXAMPLES_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <boost/multiprecision/integer.hpp>

#include <boost/multiprecision/number.hpp>

// temporary includes begin
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/modular/modular_adaptor.hpp>
// temporary includes end

#include <nil/crypto3/zk/snark/relations/variable.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace snark {

                typedef std::size_t lc_index_t;

                template<typename FieldType>
                class blueprint;

                template<typename FieldType>
                class blueprint_variable : public variable<FieldType> {
                public:
                    blueprint_variable(const var_index_t index = 0) : variable<FieldType>(index) {};

                    void allocate(blueprint<FieldType> &bp) {
                        this->index = bp.allocate_var_index();
                    }
                };

                template<typename FieldType>
                class blueprint_variable_vector : private std::vector<blueprint_variable<FieldType>> {
                    typedef FieldType field_type;
                    typedef typename field_type::value_type field_value_type;
                    typedef std::vector<blueprint_variable<field_type>> contents;
                public:
                    using typename contents::const_iterator;
                    using typename contents::const_reverse_iterator;
                    using typename contents::iterator;
                    using typename contents::reverse_iterator;

                    using contents::begin;
                    using contents::emplace_back;
                    using contents::empty;
                    using contents::end;
                    using contents::insert;
                    using contents::rbegin;
                    using contents::rend;
                    using contents::reserve;
                    using contents::size;
                    using contents::operator[];
                    using contents::resize;

                    blueprint_variable_vector() : contents() {};
                    blueprint_variable_vector(std::size_t count, const blueprint_variable<field_type> &value) :
                        contents(count, value) {};
                    blueprint_variable_vector(typename contents::const_iterator first,
                                              typename contents::const_iterator last) :
                        contents(first, last) {};
                    blueprint_variable_vector(typename contents::const_reverse_iterator first,
                                              typename contents::const_reverse_iterator last) :
                        contents(first, last) {};

                    /* allocates variable<field_type> array in MSB->LSB order */
                    void allocate(blueprint<field_type> &bp, const std::size_t n) {
                        (*this).resize(n);

                        for (std::size_t i = 0; i < n; ++i) {
                            (*this)[i].allocate(bp);
                        }
                    }

                    void fill_with_field_elements(blueprint<field_type> &bp,
                                                  const std::vector<field_value_type> &vals) const {
                        assert(this->size() == vals.size());
                        for (std::size_t i = 0; i < vals.size(); ++i) {
                            bp.val((*this)[i]) = vals[i];
                        }
                    }

                    void fill_with_bits(blueprint<field_type> &bp, const std::vector<bool> &bits) const {
                        assert(this->size() == bits.size());
                        for (std::size_t i = 0; i < bits.size(); ++i) {
                            bp.val((*this)[i]) =
                                (bits[i] ? field_value_type::one() : field_value_type::zero());
                        }
                    }

                    void fill_with_bits_of_ulong(blueprint<field_type> &bp, 
                                                 const unsigned long i) const {
                        this->fill_with_bits_of_field_element(bp, field_value_type(i));
                    }

                    void fill_with_bits_of_field_element(blueprint<field_type> &bp,
                                                         const field_value_type &r) const {
                        for (std::size_t i = 0; i < this->size(); ++i) {
                            bp.val((*this)[i]) = boost::multiprecision::bit_test(r, i) ? field_value_type::one() :
                                                                                         field_value_type::zero();
                        }
                    }

                    std::vector<field_value_type> get_vals(const blueprint<field_type> &bp) const {
                        std::vector<field_value_type> result(this->size());
                        for (std::size_t i = 0; i < this->size(); ++i) {
                            result[i] = bp.val((*this)[i]);
                        }
                        return result;
                    }

                    std::vector<bool> get_bits(const blueprint<field_type> &bp) const {
                        std::vector<bool> result;
                        for (std::size_t i = 0; i < this->size(); ++i) {
                            const field_value_type v = bp.val((*this)[i]);
                            assert(v.is_zero() || v.is_one());
                            result.push_back(v.is_one());
                        }
                        return result;
                    }

                    field_value_type get_field_element_from_bits(const blueprint<field_type> &bp) const {
                        field_value_type result = field_value_type::zero();

                        for (std::size_t i = 0; i < this->size(); ++i) {
                            /* push in the new bit */
                            const field_value_type v = bp.val((*this)[this->size() - 1 - i]);
                            assert(v.is_zero() || v.is_one());
                            result += result + v;
                        }

                        return result;
                    }
                };

                template<typename FieldType>
                class blueprint_linear_combination : public linear_combination<FieldType> {
                    typedef FieldType field_type;
                    typedef typename field_type::value_type field_value_type;
                public:
                    bool is_variable;
                    lc_index_t index;

                    blueprint_linear_combination() {
                        this->is_variable = false;
                    }

                    blueprint_linear_combination(const blueprint_variable<field_type> &var) {
                        this->is_variable = true;
                        this->index = var.index;
                        this->terms.emplace_back(linear_term<field_type>(var));
                    }

                    void assign(blueprint<field_type> &bp, const linear_combination<field_type> &lc) {
                        assert(this->is_variable == false);
                        this->index = bp.allocate_lc_index();
                        this->terms = lc.terms;
                    }

                    void evaluate(blueprint<field_type> &bp) const {
                        if (this->is_variable) {
                            return;    // do nothing
                        }

                        field_value_type sum = 0;
                        for (auto term : this->terms) {
                            sum += term.coeff * bp.val(blueprint_variable<field_type>(term.index));
                        }

                        bp.lc_val(*this) = sum;
                    }

                    bool is_constant() const {
                        if (is_variable) {
                            return (index == 0);
                        } else {
                            for (auto term : this->terms) {
                                if (term.index != 0) {
                                    return false;
                                }
                            }

                            return true;
                        }
                    }

                    field_value_type constant_term() const {
                        if (is_variable) {
                            return (index == 0 ? field_value_type::one() : field_value_type::zero());
                        } else {
                            field_value_type result = field_value_type::zero();
                            for (auto term : this->terms) {
                                if (term.index == 0) {
                                    result += term.coeff;
                                }
                            }
                            return result;
                        }
                    }
                };

                template<typename FieldType>
                class blueprint_linear_combination_vector : private std::vector
                                                                <blueprint_linear_combination<FieldType>> {

                    typedef FieldType field_type;
                    typedef typename field_type::value_type field_value_type;
                    typedef std::vector<blueprint_linear_combination<field_type>> contents;
                public:
                    using typename contents::const_iterator;
                    using typename contents::const_reverse_iterator;
                    using typename contents::iterator;
                    using typename contents::reverse_iterator;

                    using contents::begin;
                    using contents::emplace_back;
                    using contents::empty;
                    using contents::end;
                    using contents::insert;
                    using contents::rbegin;
                    using contents::rend;
                    using contents::reserve;
                    using contents::size;
                    using contents::operator[];
                    using contents::resize;

                    blueprint_linear_combination_vector() : contents() {};
                    blueprint_linear_combination_vector(const blueprint_variable_vector<field_type> &arr) {
                        for (auto &v : arr)
                            this->emplace_back(blueprint_linear_combination<field_type>(v));
                    };
                    blueprint_linear_combination_vector(std::size_t count) : contents(count) {};
                    blueprint_linear_combination_vector(std::size_t count,
                                                        const blueprint_linear_combination<field_type> &value) :
                        contents(count, value) {};
                    blueprint_linear_combination_vector(typename contents::const_iterator first,
                                                        typename contents::const_iterator last) :
                        contents(first, last) {};
                    blueprint_linear_combination_vector(typename contents::const_reverse_iterator first,
                                                        typename contents::const_reverse_iterator last) :
                        contents(first, last) {};

                    void evaluate(blueprint<field_type> &bp) const {
                        for (std::size_t i = 0; i < this->size(); ++i) {
                            (*this)[i].evaluate(bp);
                        }
                    }

                    void fill_with_field_elements(blueprint<field_type> &bp,
                                                  const std::vector<field_value_type> &vals) const {
                        assert(this->size() == vals.size());
                        for (std::size_t i = 0; i < vals.size(); ++i) {
                            bp.lc_val((*this)[i]) = vals[i];
                        }
                    }

                    void fill_with_bits(blueprint<field_type> &bp, const std::vector<bool> &bits) const {
                        assert(this->size() == bits.size());
                        for (std::size_t i = 0; i < bits.size(); ++i) {
                            bp.lc_val((*this)[i]) =
                                (bits[i] ? field_value_type::one() : field_value_type::zero());
                        }
                    }

                    void fill_with_bits_of_ulong(blueprint<field_type> &bp, const unsigned long i) const {
                        this->fill_with_bits_of_field_element(bp, field_value_type(i));
                    }

                    void fill_with_bits_of_field_element(blueprint<field_type> &bp,
                                                         const field_value_type &r) const {

                        // temporary added until fixed-precision modular adaptor is ready:
                        typedef boost::multiprecision::number<
                            boost::multiprecision::backends::cpp_int_backend<>> 
                            non_fixed_precision_modulus_type;

                        for (std::size_t i = 0; i < this->size(); ++i) {
                            bp.lc_val((*this)[i]) = 
                                boost::multiprecision::bit_test(non_fixed_precision_modulus_type(r.data), i) ?
                                                                field_value_type::one() :
                                                                field_value_type::zero();
                        }
                    }

                    std::vector<field_value_type> get_vals(const blueprint<field_type> &bp) const {
                        std::vector<field_value_type> result(this->size());
                        for (std::size_t i = 0; i < this->size(); ++i) {
                            result[i] = bp.lc_val((*this)[i]);
                        }
                        return result;
                    }

                    std::vector<bool> get_bits(const blueprint<field_type> &bp) const {
                        std::vector<bool> result;
                        for (std::size_t i = 0; i < this->size(); ++i) {
                            const field_value_type v = bp.lc_val((*this)[i]);
                            assert(v.is_zero() || v.is_one());
                            result.push_back(v.is_one());
                        }
                        return result;
                    }

                    field_value_type get_field_element_from_bits(const blueprint<field_type> &bp) const {
                        field_value_type result = field_value_type::zero();

                        for (std::size_t i = 0; i < this->size(); ++i) {
                            /* push in the new bit */
                            const field_value_type v = bp.lc_val((*this)[this->size() - 1 - i]);
                            std::cout << i << ": " << v.data << std::endl;
                            assert(v.is_zero() || v.is_one());
                            result += result + v;
                        }

                        return result;
                    }
                };

                template<typename FieldType>
                linear_combination<FieldType> blueprint_sum(
                    const blueprint_linear_combination_vector<FieldType> &v) {

                    linear_combination<FieldType> result;
                    for (auto &term : v) {
                        result = result + term;
                    }

                    return result;
                }

                template<typename FieldType>
                linear_combination<FieldType> blueprint_packing_sum(
                    const blueprint_linear_combination_vector<FieldType> &v) {

                    typename FieldType::value_type twoi =
                        FieldType::value_type::one();    // will hold 2^i entering each iteration
                    std::vector<linear_term<FieldType>> all_terms;
                    for (auto &lc : v) {
                        for (auto &term : lc.terms) {
                            all_terms.emplace_back(twoi * term);
                        }
                        twoi += twoi;
                    }

                    return linear_combination<FieldType>(all_terms);
                }

                template<typename FieldType>
                linear_combination<FieldType> blueprint_coeff_sum(
                    const blueprint_linear_combination_vector<FieldType> &v,
                    const std::vector<typename FieldType::value_type> &coeffs) {
                    
                    assert(v.size() == coeffs.size());
                    std::vector<linear_term<FieldType>> all_terms;

                    auto coeff_it = coeffs.begin();
                    for (auto &lc : v) {
                        for (auto &term : lc.terms) {
                            all_terms.emplace_back((*coeff_it) * term);
                        }
                        ++coeff_it;
                    }

                    return linear_combination<FieldType>(all_terms);
                }
            }    // namespace snark
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ZK_MEMORY_CONTENTS_EXAMPLES_HPP
