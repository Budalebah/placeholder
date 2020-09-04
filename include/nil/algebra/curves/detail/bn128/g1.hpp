//---------------------------------------------------------------------------//
// Copyright (c) 2020 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020 Nikita Kaskov <nbering@nil.foundation>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//---------------------------------------------------------------------------//

#ifndef ALGEBRA_CURVES_BN128_G1_HPP
#define ALGEBRA_CURVES_BN128_G1_HPP

#include <boost/multiprecision/cpp_int/multiply.hpp>
#include <boost/multiprecision/modular/base_params.hpp>

#include <nil/algebra/curves/detail/element/curve_weierstrass.hpp>

#include <nil/algebra/detail/mp_def.hpp>

namespace nil {
    namespace algebra {
        namespace curves {
            namespace detail {

                template<typename PairingParams>
                struct bn128_g1 : public element_curve_weierstrass<typename PairingParams::g1_type> {

                    using policy_type = PairingParams;
                    using element_type = element_curve_weierstrass<typename policy_type::g1_type>;
                    using underlying_field_type_value = typename element_type::underlying_field_type_value;

                    bn128_g1() : bn128_g1(zero_fill[0], zero_fill[1], zero_fill[2]) {};

                    bn128_g1(underlying_field_type_value X, underlying_field_type_value Y,
                             underlying_field_type_value Z) :
                        element_type(X, Y, Z) {};

                    static bn128_g1 zero() {
                        return bn128_g1();
                    }

                    static bn128_g1 one() {
                        return bn128_g1(one_fill[0], one_fill[1], one_fill[2]);
                    }

                private:
                    constexpr static const underlying_field_type_value zero_fill = {
                        underlying_field_type_value::one(), underlying_field_type_value::one(),
                        underlying_field_type_value::zero()};

                    constexpr static const underlying_field_type_value one_fill = {
                        underlying_field_type_value(1), underlying_field_type_value(2), underlying_field_type_value(1)};
                };

            }    // namespace detail
        }        // namespace curves
    }            // namespace algebra
}    // namespace nil
#endif    // ALGEBRA_CURVES_BN128_G1_HPP
