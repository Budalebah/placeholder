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

#ifndef CRYPTO3_RUN_USCS_PPZKSNARK_HPP
#define CRYPTO3_RUN_USCS_PPZKSNARK_HPP

#include <nil/crypto3/algebra/curves/public_params.hpp>

#include <nil/crypto3/zk/snark/relations/constraint_satisfaction_problems/uscs/examples/uscs_examples.hpp>

namespace nil {
    namespace crypto3 {
        namespace zk {
            namespace snark {

                /**
                 * Runs the ppzkSNARK (generator, prover, and verifier) for a given
                 * USCS example (specified by a constraint system, input, and witness).
                 */
                template<typename CurveType>
                bool run_uscs_ppzksnark(const uscs_example<typename CurveType::scalar_field_type> &example);

                /**
                 * The code below provides an example of all stages of running a USCS ppzkSNARK.
                 *
                 * Of course, in a real-life scenario, we would have three distinct entities,
                 * mangled into one in the demonstration below. The three entities are as follows.
                 * (1) The "generator", which runs the ppzkSNARK generator on input a given
                 *     constraint system CS to create a proving and a verification key for CS.
                 * (2) The "prover", which runs the ppzkSNARK prover on input the proving key,
                 *     a primary input for CS, and an auxiliary input for CS.
                 * (3) The "verifier", which runs the ppzkSNARK verifier on input the verification key,
                 *     a primary input for CS, and a proof.
                 */
                template<typename CurveType>
                bool run_uscs_ppzksnark(const uscs_example<typename CurveType::scalar_field_type> &example) {
                    std::cout << "Call to run_uscs_ppzksnark" << std::endl;

                    std::cout << "USCS ppzkSNARK Generator" << std::endl;
                    uscs_ppzksnark_keypair<CurveType> keypair = uscs_ppzksnark_generator<CurveType>(example.constraint_system);

                    std::cout << "Preprocess verification key" << std::endl;
                    uscs_ppzksnark_processed_verification_key<CurveType> pvk =
                        uscs_ppzksnark_verifier_process_vk<CurveType>(keypair.vk);

                    std::cout << "USCS ppzkSNARK Prover" << std::endl;
                    uscs_ppzksnark_proof<CurveType> proof =
                        uscs_ppzksnark_prover<CurveType>(keypair.pk, example.primary_input, example.auxiliary_input);

                    std::cout << "USCS ppzkSNARK Verifier" << std::endl;
                    bool ans = uscs_ppzksnark_verifier_strong_input_consistency<CurveType>(keypair.vk, example.primary_input, proof);
                    printf("* The verification result is: %s\n", (ans ? "PASS" : "FAIL"));

                    std::cout << "USCS ppzkSNARK Online Verifier" << std::endl;
                    bool ans2 = uscs_ppzksnark_online_verifier_strong_input_consistency<CurveType>(pvk, example.primary_input, proof);
                    assert(ans == ans2);

                    return ans;
                }

            }    // namespace snark
        }        // namespace zk
    }            // namespace crypto3
}    // namespace nil

#endif    // CRYPTO3_RUN_USCS_PPZKSNARK_HPP
