//---------------------------------------------------------------------------//
// Copyright (c) 2020-2021 Mikhail Komarov <nemo@nil.foundation>
// Copyright (c) 2020-2021 Nikita Kaskov <nbering@nil.foundation>
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

#ifndef CRYPTO3_ALGEBRA_CURVES_SECP_K1_256_PARAMS_HPP
#define CRYPTO3_ALGEBRA_CURVES_SECP_K1_256_PARAMS_HPP

#include <nil/crypto3/algebra/curves/detail/secp/secp_k1/basic_params.hpp>

#include <nil/crypto3/detail/literals.hpp>

namespace nil {
    namespace crypto3 {
        namespace algebra {
            namespace curves {
                namespace detail {

                    template<std::size_t Version>
                    struct secp_k1_g1;

                    template<std::size_t Version>
                    struct secp_k1_g2;

                    template<std::size_t Version = 256>
                    struct secp_k1_short_weierstrass_params;

                    template<std::size_t Version = 256>
                    struct secp_k1_short_weierstrass_g1_affine_params;

                    template<std::size_t Version = 256>
                    struct secp_k1_short_weierstrass_g1_projective_params;

                    template<std::size_t Version = 256>
                    struct secp_k1_short_weierstrass_g1_jacobian_with_a4_0_params;

                    template<>
                    struct secp_k1_short_weierstrass_params<256> : public secp_k1_basic_params<256> {

                        using base_field_type = typename secp_k1_basic_params<256>::base_field_type;
                        using scalar_field_type = typename secp_k1_basic_params<256>::scalar_field_type;

                        constexpr static const typename base_field_type::modulus_type a =
                            typename base_field_type::modulus_type(0x00);    ///< coefficient of short Weierstrass curve $y^2=x^3+a*x+b$
                        constexpr static const typename base_field_type::modulus_type b = 
                            typename base_field_type::modulus_type(0x07);    ///< coefficient of short Weierstrass curve $y^2=x^3+a*x+b$
                    };

                    template<>
                    struct secp_k1_short_weierstrass_g1_jacobian_with_a4_0_params<256> : public secp_k1_short_weierstrass_params<256> {

                        using field_type = typename secp_k1_basic_params<256>::g1_field_type;
                        using group_type = secp_k1_g1<256>;

                        constexpr static const std::array<typename field_type::value_type, 3> zero_fill = {
                            field_type::value_type::zero(), field_type::value_type::one(),
                            field_type::value_type::zero()};

                        constexpr static const std::array<typename field_type::value_type, 3> one_fill = {
                            typename field_type::value_type(
                                0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798_cppui256),
                            typename field_type::value_type(
                                0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8_cppui256),
                            field_type::value_type::one()};
                    };

                    constexpr typename secp_k1_short_weierstrass_params<256>::base_field_type::modulus_type const secp_k1_short_weierstrass_params<256>::a;
                    constexpr typename secp_k1_short_weierstrass_params<256>::base_field_type::modulus_type const secp_k1_short_weierstrass_params<256>::b;

                    constexpr std::array<typename secp_k1_short_weierstrass_g1_jacobian_with_a4_0_params<256>::field_type::value_type, 3> const
                        secp_k1_short_weierstrass_g1_jacobian_with_a4_0_params<256>::zero_fill;
                    constexpr std::array<typename secp_k1_short_weierstrass_g1_jacobian_with_a4_0_params<256>::field_type::value_type, 3> const
                        secp_k1_short_weierstrass_g1_jacobian_with_a4_0_params<256>::one_fill;

                }    // namespace detail
            }        // namespace curves
        }            // namespace algebra
    }                // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_ALGEBRA_CURVES_SECP_K1_256_PARAMS_HPP
