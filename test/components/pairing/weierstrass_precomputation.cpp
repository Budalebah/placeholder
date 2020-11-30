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

#define BOOST_TEST_MODULE weierstrass_precomputation_components_test

#include <boost/test/unit_test.hpp>

#include <nil/crypto3/algebra/curves/bn128.hpp>
#include <nil/crypto3/algebra/curves/edwards.hpp>
#include <nil/crypto3/algebra/curves/mnt4.hpp>
#include <nil/crypto3/algebra/curves/mnt6.hpp>

#include <nil/crypto3/zk/snark/components/pairing/weierstrass_precomputation_components.hpp>

using namespace nil::crypto3;
using namespace nil::crypto3::zk::snark;
using namespace nil::crypto3::algebra;

template<typename CurveType>
void test_G1_variable_precomp(const std::string &annotation) {
    blueprint<typename CurveType::scalar_field_type> bp;
    typename other_curve<CurveType>::g1_type::value_type g_val =
        algebra::random_element<typename other_curve<CurveType>::scalar_field_type>() *
        other_curve<CurveType>::g1_type::value_type::one();

    G1_variable<CurveType> g(bp);
    G1_precomputation<CurveType> precomp;
    precompute_G1_component<CurveType> do_precomp(bp, g, precomp);
    do_precomp.generate_r1cs_constraints();

    g.generate_r1cs_witness(g_val);
    do_precomp.generate_r1cs_witness();
    assert(bp.is_satisfied());

    G1_precomputation<CurveType> const_precomp(bp, g_val);

    algebra::affine_ate_G1_precomp<other_curve<CurveType>> native_precomp =
        other_curve<CurveType>::affine_ate_precompute_G1(g_val);
    assert(precomp.PY_twist_squared->get_element() == native_precomp.PY_twist_squared);
    assert(const_precomp.PY_twist_squared->get_element() == native_precomp.PY_twist_squared);
}

template<typename CurveType>
void test_G2_variable_precomp(const std::string &annotation) {
    blueprint<typename CurveType::scalar_field_type> bp;
    typename other_curve<CurveType>::g2_type::value_type g_val =
        algebra::random_element<typename other_curve<CurveType>::scalar_field_type>() *
        other_curve<CurveType>::g2_type::value_type::one();

    G2_variable<CurveType> g(bp, "g");
    G2_precomputation<CurveType> precomp;
    precompute_G2_component<CurveType> do_precomp(bp, g, precomp);
    do_precomp.generate_r1cs_constraints();

    g.generate_r1cs_witness(g_val);
    do_precomp.generate_r1cs_witness();
    assert(bp.is_satisfied());

    algebra::affine_ate_G2_precomp<other_curve<CurveType>> native_precomp =
        other_curve<CurveType>::affine_ate_precompute_G2(g_val);

    assert(precomp.coeffs.size() - 1 ==
           native_precomp.coeffs.size());    // the last precomp is unused, but remains for convenient programming
    for (std::size_t i = 0; i < native_precomp.coeffs.size(); ++i) {
        assert(precomp.coeffs[i]->RX->get_element() == native_precomp.coeffs[i].old_RX);
        assert(precomp.coeffs[i]->RY->get_element() == native_precomp.coeffs[i].old_RY);
        assert(precomp.coeffs[i]->gamma->get_element() == native_precomp.coeffs[i].gamma);
        assert(precomp.coeffs[i]->gamma_X->get_element() == native_precomp.coeffs[i].gamma_X);
    }

    printf("number of constraints for G2 precomp (Fr is %s)  = %zu\n", annotation.c_str(), bp.num_constraints());
}

BOOST_AUTO_TEST_SUITE(weierstrass_precomputation_components_test_suite)

BOOST_AUTO_TEST_CASE(weierstrass_precomputation_components_test) {
}

BOOST_AUTO_TEST_SUITE_END()