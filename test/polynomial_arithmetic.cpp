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

#define BOOST_TEST_MODULE polynomial_arithmetic_test

#include <vector>
#include <cstdint>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include <nil/crypto3/algebra/fields/arithmetic_params/bls12.hpp>

#include <nil/crypto3/math/polynomial/polynom.hpp>
#include <nil/crypto3/math/polynomial/basic_operations.hpp>
#include <nil/crypto3/math/polynomial/xgcd.hpp>

using namespace nil::crypto3::algebra;
using namespace nil::crypto3::math;

typedef fields::bls12<381> FieldType;
typedef fields::bls12_fr<381> ScalarFieldType;

BOOST_AUTO_TEST_SUITE(polynomial_addition_test_suite)

BOOST_AUTO_TEST_CASE(polynomial_addition_equal) {
    std::vector<typename FieldType::value_type> a = {1, 3, 4, 25, 6, 7, 7, 2};
    std::vector<typename FieldType::value_type> b = {9, 3, 11, 14, 7, 1, 5, 8};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::addition(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {10, 6, 15, 39, 13, 8, 12, 10};

    for (std::size_t i = 0; i < c.size(); ++i) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_addition_long_a) {

    std::vector<typename FieldType::value_type> a = {1, 3, 4, 25, 6, 7, 7, 2};
    std::vector<typename FieldType::value_type> b = {9, 3, 11, 14, 7};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::addition(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {10, 6, 15, 39, 13, 7, 7, 2};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_addition_long_b) {

    std::vector<typename FieldType::value_type> a = {1, 3, 4, 25, 6};
    std::vector<typename FieldType::value_type> b = {9, 3, 11, 14, 7, 1, 5, 8};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::addition(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {10, 6, 15, 39, 13, 1, 5, 8};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_addition_zero_a) {

    std::vector<typename FieldType::value_type> a = {0, 0, 0};
    std::vector<typename FieldType::value_type> b = {1, 3, 4, 25, 6, 7, 7, 2};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::addition(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {1, 3, 4, 25, 6, 7, 7, 2};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_addition_zero_b) {

    std::vector<typename FieldType::value_type> a = {1, 3, 4, 25, 6, 7, 7, 2};
    std::vector<typename FieldType::value_type> b = {0, 0, 0};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::addition(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {1, 3, 4, 25, 6, 7, 7, 2};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(polynomial_subtraction_test_suite)

BOOST_AUTO_TEST_CASE(polynomial_subtraction_equal) {

    std::vector<typename FieldType::value_type> a = {1, 3, 4, 25, 6, 7, 7, 2};
    std::vector<typename FieldType::value_type> b = {9, 3, 11, 14, 7, 1, 5, 8};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::subtraction(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {-8, 0, -7, 11, -1, 6, 2, -6};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_subtraction_long_a) {

    std::vector<typename FieldType::value_type> a = {1, 3, 4, 25, 6, 7, 7, 2};
    std::vector<typename FieldType::value_type> b = {9, 3, 11, 14, 7};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::subtraction(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {-8, 0, -7, 11, -1, 7, 7, 2};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_subtraction_long_b) {

    std::vector<typename FieldType::value_type> a = {1, 3, 4, 25, 6};
    std::vector<typename FieldType::value_type> b = {9, 3, 11, 14, 7, 1, 5, 8};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::subtraction(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {-8, 0, -7, 11, -1, -1, -5, -8};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_subtraction_zero_a) {

    std::vector<typename FieldType::value_type> a = {0, 0, 0};
    std::vector<typename FieldType::value_type> b = {1, 3, 4, 25, 6, 7, 7, 2};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::subtraction(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {-1, -3, -4, -25, -6, -7, -7, -2};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_subtraction_zero_b) {

    std::vector<typename FieldType::value_type> a = {1, 3, 4, 25, 6, 7, 7, 2};
    std::vector<typename FieldType::value_type> b = {0, 0, 0};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::subtraction(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {1, 3, 4, 25, 6, 7, 7, 2};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(polynomial_multiplication_test_suite)

BOOST_AUTO_TEST_CASE(polynomial_multiplication_long_a) {

    std::vector<typename FieldType::value_type> a = {5, 0, 0, 13, 0, 1};
    std::vector<typename FieldType::value_type> b = {13, 0, 1};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::multiplication(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {65, 0, 5, 169, 0, 26, 0, 1};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_multiplication_long_b) {

    std::vector<typename FieldType::value_type> a = {13, 0, 1};
    std::vector<typename FieldType::value_type> b = {5, 0, 0, 13, 0, 1};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::multiplication(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {65, 0, 5, 169, 0, 26, 0, 1};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_multiplication_zero_a) {

    std::vector<typename FieldType::value_type> a = {0};
    std::vector<typename FieldType::value_type> b = {5, 0, 0, 13, 0, 1};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::multiplication(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {0};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_multiplication_zero_b) {

    std::vector<typename FieldType::value_type> a = {5, 0, 0, 13, 0, 1};
    std::vector<typename FieldType::value_type> b = {0};
    std::vector<typename FieldType::value_type> c(1, FieldType::value_type::zero());

    polynomial::multiplication(c, a, b);

    std::vector<typename FieldType::value_type> c_ans = {0};

    for (std::size_t i = 0; i < c.size(); i++) {
        BOOST_CHECK_EQUAL(c_ans[i].data, c[i].data);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(polynomial_division1) {

    std::vector<typename FieldType::value_type> a = {5, 0, 0, 13, 0, 1};
    std::vector<typename FieldType::value_type> b = {13, 0, 1};

    std::vector<typename FieldType::value_type> Q(1, FieldType::value_type::zero());
    std::vector<typename FieldType::value_type> R(1, FieldType::value_type::zero());

    polynomial::division(Q, R, a, b);

    std::vector<typename FieldType::value_type> Q_ans = {0, 0, 0, 1};
    std::vector<typename FieldType::value_type> R_ans = {5};

    for (std::size_t i = 0; i < Q.size(); i++) {
        BOOST_CHECK_EQUAL(Q_ans[i].data, Q[i].data);
    }
    for (std::size_t i = 0; i < R.size(); i++) {
        BOOST_CHECK_EQUAL(R_ans[i].data, R[i].data);
    }
}

BOOST_AUTO_TEST_CASE(polynomial_division2) {

    std::vector<typename ScalarFieldType::value_type> a = {
        typename ScalarFieldType::integral_type(
            "34252716557043855342695290230103871228515105574773901488925285645652807153166"),
        typename ScalarFieldType::integral_type(
            "50987756412999415285078682237211178281888144743178199612615760467994903992082"),
        typename ScalarFieldType::integral_type(
            "24087040800774024553018233915566406475941909965665136675658456903584524936330"),
        typename ScalarFieldType::integral_type(
            "32590140112363357461550139815141042644159792097217024427836467483053975495010"),
        typename ScalarFieldType::integral_type(
            "27526504140082161585593358152024595445246036890034123406602170242512375908018"),
        typename ScalarFieldType::integral_type(
            "4164451897952655317482585500069509200655217678988255594017920258520056743793"),
        typename ScalarFieldType::integral_type(
            "51054915713378087731818477187716937873605692536880655507643872441109964651582"),
        typename ScalarFieldType::integral_type(
            "1492042980901913387940130157882635681388976151263533518877964808842231855703"),
    };
    std::vector<typename ScalarFieldType::value_type> b = {
        typename ScalarFieldType::integral_type(
            "41038196278518895001694922465381809300477619960735317337590217198846581910734"),
        ScalarFieldType::value_type::one(),
    };

    std::vector<typename ScalarFieldType::value_type> Q;
    std::vector<typename ScalarFieldType::value_type> R;

    _polynomial_division<ScalarFieldType>(Q, R, a, b);

    std::vector<typename ScalarFieldType::value_type> Q_ans = {
        typename ScalarFieldType::integral_type(
            "44128561650983680278455473768250070484575713190992177646786766650961845607451"),
        typename ScalarFieldType::integral_type(
            "5912625930387043008695069616452788359188722310701089700488078758315899816143"),
        typename ScalarFieldType::integral_type(
            "39762125371313198785149448291957682181469165476135258275197619225764589799494"),
        typename ScalarFieldType::integral_type(
            "51725389382610318382304285810094270528436001151627635322421659191513572176159"),
        typename ScalarFieldType::integral_type(
            "35201287514443338705756016880954205449193913746577965470906766233151144323350"),
        typename ScalarFieldType::integral_type(
            "29441384394867713260772806765848221977380145710997006705395504766798428408258"),
        typename ScalarFieldType::integral_type(
            "1492042980901913387940130157882635681388976151263533518877964808842231855703"),
    };
    std::vector<typename ScalarFieldType::value_type> R_ans = {ScalarFieldType::value_type::zero()};

    BOOST_CHECK(Q_ans == Q);
    BOOST_CHECK(R_ans == R);
}

BOOST_AUTO_TEST_CASE(extended_gcd) {

    std::vector<typename FieldType::value_type> a = {0, 0, 0, 0, 1};
    std::vector<typename FieldType::value_type> b = {1, -6, 11, -6};

    std::vector<typename FieldType::value_type> pg(1, FieldType::value_type::zero());
    std::vector<typename FieldType::value_type> pu(1, FieldType::value_type::zero());
    std::vector<typename FieldType::value_type> pv(1, FieldType::value_type::zero());

    extended_euclidean(a, b, pg, pu, pv);

    std::vector<typename FieldType::value_type> pv_ans = {1, 6, 25, 90};

    for (std::size_t i = 0; i < pv.size(); i++) {
        BOOST_CHECK_EQUAL(pv_ans[i].data, pv[i].data);
    }
}
