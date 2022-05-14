////---------------------------------------------------------------------------//
//// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//// Copyright (c) 2021-2022 Aleksei Moskvin <alalmoskvin@nil.foundation>
////
//// MIT License
////
//// Permission is hereby granted, free of charge, to any person obtaining a copy
//// of this software and associated documentation files (the "Software"), to deal
//// in the Software without restriction, including without limitation the rights
//// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//// copies of the Software, and to permit persons to whom the Software is
//// furnished to do so, subject to the following conditions:
////
//// The above copyright notice and this permission notice shall be included in all
//// copies or substantial portions of the Software.
////
//// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//// SOFTWARE.
////---------------------------------------------------------------------------//

#include <nil/crypto3/hash/sha2.hpp>
#include <nil/crypto3/hash/md5.hpp>
#include <nil/crypto3/hash/blake2b.hpp>
//#include <nil/crypto3/hash/pedersen.hpp>
//#include <nil/crypto3/hash/find_group_hash.hpp>

#include <nil/actor/testing/test_case.hh>
#include <nil/actor/testing/thread_test_case.hh>
#include <nil/actor/core/memory.hh>
#include <nil/actor/core/smp.hh>
#include <nil/actor/detail/memory_diagnostics.hh>

#include <nil/crypto3/container/merkle/tree.hpp>
#include <nil/crypto3/container/merkle/proof.hpp>
#include <nil/actor/container/merkle/tree.hpp>

#include <vector>

#include <cstdlib>

template<typename ValueType, std::size_t N>
typename std::enable_if<std::is_unsigned<ValueType>::value, std::vector<std::array<ValueType, N>>>::type
    generate_random_data(std::size_t leaf_number) {
    std::vector<std::array<ValueType, N>> v;
    for (std::size_t i = 0; i < leaf_number; ++i) {
        std::array<ValueType, N> leaf {};
        std::generate(std::begin(leaf), std::end(leaf),
                      [&]() { return std::rand() % (std::numeric_limits<ValueType>::max() + 1); });
        v.emplace_back(leaf);
    }
    return v;
}

template<typename Hash, size_t Arity, typename ValueType, std::size_t N>
void testing_validate_template_random_data(std::size_t leaf_number) {
    std::array<ValueType, N> data_not_in_tree = {0};
    auto data = generate_random_data<ValueType, N>(leaf_number);
    auto tree = nil::actor::containers::make_merkle_tree<Hash, Arity>(data.begin(), data.end());

    std::size_t proof_idx = std::rand() % leaf_number;
    nil::crypto3::containers::merkle_proof<Hash, Arity> proof(tree, proof_idx);
    bool good_validate = proof.validate(data[proof_idx]);
    bool wrong_leaf_validate = proof.validate(data[(proof_idx + 1) % leaf_number]);
    bool wrong_data_validate = proof.validate(data_not_in_tree);
    BOOST_CHECK(good_validate);
    BOOST_CHECK(!wrong_leaf_validate);
    BOOST_CHECK(!wrong_data_validate);
}

template<typename Hash, size_t Arity, typename Element>
void testing_validate_template(std::vector<Element> data) {
    std::array<uint8_t, 7> data_not_in_tree = {'\x6d', '\x65', '\x73', '\x73', '\x61', '\x67', '\x65'};
    nil::crypto3::containers::merkle_tree<Hash, Arity> tree = nil::actor::containers::make_merkle_tree<Hash, Arity>(data.begin(), data.end());
    nil::crypto3::containers::merkle_proof<Hash, Arity> proof(tree, 0);
    bool good_validate = proof.validate(data[0]);
    bool wrong_leaf_validate = proof.validate(data[1]);
    bool wrong_data_validate = proof.validate(data_not_in_tree);
    BOOST_CHECK(true == good_validate);
    BOOST_CHECK(false == wrong_leaf_validate);
    BOOST_CHECK(false == wrong_data_validate);
}

template<typename Hash, size_t Arity, typename Element>
void testing_hash_template(std::vector<Element> data, std::string result) {
    nil::crypto3::containers::merkle_tree<Hash, Arity> tree = nil::actor::containers::make_merkle_tree<Hash, Arity>(data.begin(), data.end());
    BOOST_CHECK(result == std::to_string(tree.root()));
}

using namespace nil::actor;

ACTOR_THREAD_TEST_CASE(merkletree_validate_test_1) {
    std::vector<std::array<char, 1>> v = {{'0'}, {'1'}, {'2'}, {'3'}, {'4'}, {'5'}, {'6'}, {'7'}};
    testing_validate_template<nil::crypto3::hashes::sha2<256>, 2>(v);
    testing_validate_template<nil::crypto3::hashes::md5, 2>(v);
    testing_validate_template<nil::crypto3::hashes::blake2b<224>, 2>(v);

    std::size_t leaf_number = 64;
    testing_validate_template_random_data<nil::crypto3::hashes::sha2<256>, 2, std::uint8_t, 1>(leaf_number);
    testing_validate_template_random_data<nil::crypto3::hashes::md5, 2, std::uint8_t, 1>(leaf_number);
    testing_validate_template_random_data<nil::crypto3::hashes::blake2b<224>, 2, std::uint8_t, 1>(leaf_number);
}

ACTOR_THREAD_TEST_CASE(merkletree_validate_test_2) {
    std::vector<std::array<char, 1>> v = {{'0'}, {'1'}, {'2'}, {'3'}, {'4'}, {'5'}, {'6'}, {'7'}, {'8'}};
    testing_validate_template<nil::crypto3::hashes::sha2<256>, 3>(v);
    testing_validate_template<nil::crypto3::hashes::md5, 3>(v);
    testing_validate_template<nil::crypto3::hashes::blake2b<224>, 3>(v);

    std::size_t leaf_number = 81;
    testing_validate_template_random_data<nil::crypto3::hashes::sha2<256>, 3, std::uint8_t, 1>(leaf_number);
    testing_validate_template_random_data<nil::crypto3::hashes::md5, 3, std::uint8_t, 1>(leaf_number);
    testing_validate_template_random_data<nil::crypto3::hashes::blake2b<224>, 3, std::uint8_t, 1>(leaf_number);
}

//ACTOR_THREAD_TEST_CASE(merkletree_validate_test_3) {
//    using hash_type = nil::crypto3::hashes::pedersen<
//        nil::crypto3::hashes::find_group_hash_default_params, nil::crypto3::hashes::sha2<256>,
//        nil::crypto3::algebra::curves::jubjub::template g1_type<nil::crypto3::algebra::curves::coordinates::affine,
//                                                  nil::crypto3::algebra::curves::forms::twisted_edwards>>;
//    std::size_t leaf_number = 8;
//    testing_validate_template_random_data<hash_type, 2, bool, hash_type::digest_bits>(leaf_number);
//    return make_ready_future<>();
//}

ACTOR_THREAD_TEST_CASE(merkletree_hash_test_1) {
    std::vector<std::array<char, 1>> v = {{'0'}, {'1'}, {'2'}, {'3'}, {'4'}, {'5'}, {'6'}, {'7'}};
    testing_hash_template<nil::crypto3::hashes::sha2<256>, 2>(v, "3b828c4f4b48c5d4cb5562a474ec9e2fd8d5546fae40e90732ef635892e42720");
    testing_hash_template<nil::crypto3::hashes::md5, 2>(v, "11ee8b50825ce6f816a1ae06d4aa0045");
    testing_hash_template<nil::crypto3::hashes::blake2b<224>, 2>(v, "0ed2a2145cae554ca57f08420d6cb58629ca1e89dc92f819c6c1d13d");
}

ACTOR_THREAD_TEST_CASE(merkletree_hash_test_2) {
    std::vector<std::array<char, 1>> v = {{'0'}, {'1'}, {'2'}, {'3'}, {'4'}, {'5'}, {'6'}, {'7'}, {'8'}};
    testing_hash_template<nil::crypto3::hashes::sha2<256>, 3>(v, "6831d4d32538bedaa7a51970ac10474d5884701c840781f0a434e5b6868d4b73");
    testing_hash_template<nil::crypto3::hashes::md5, 3>(v, "0733c4cd580b1523cfbb9751f42e9420");
    testing_hash_template<nil::crypto3::hashes::blake2b<224>, 3>(v, "d9d0ff26d10aaac2882c08eb2b55e78690c949d1a73b1cfc0eb322ee");
}