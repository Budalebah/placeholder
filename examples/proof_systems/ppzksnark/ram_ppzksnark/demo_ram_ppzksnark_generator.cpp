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

#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

#include <nil/crypto3/zk/snark/default_types/ram_ppzksnark_pp.hpp>
#include <nil/crypto3/zk/snark/relations/ram_computations/rams/tinyram/tinyram_params.hpp>
#include <nil/crypto3/zk/snark/proof_systems/ppzksnark/ram_ppzksnark/ram_ppzksnark.hpp>

#ifndef MINDEPS
namespace po = boost::program_options;

bool process_generator_command_line(const int argc, const char **argv, std::string &architecture_params_fn,
                                    std::string &computation_bounds_fn, std::string &proving_key_fn,
                                    std::string &verification_key_fn) {
    try {
        po::options_description desc("Usage");
        desc.add_options()("help", "print this help message")(
            "architecture_params", po::value<std::string>(&architecture_params_fn)->required())(
            "computation_bounds", po::value<std::string>(&computation_bounds_fn)->required())(
            "proving_key", po::value<std::string>(&proving_key_fn)->required())(
            "verification_key", po::value<std::string>(&verification_key_fn)->required());

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return false;
        }

        po::notify(vm);
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }

    return true;
}
#endif

using namespace nil::crypto3::zk::snark;

int main(int argc, const char *argv[]) {
#ifdef MINDEPS
    std::string architecture_params_fn = "architecture_params.txt";
    std::string computation_bounds_fn = "computation_bounds.txt";
    std::string proving_key_fn = "proving_key.txt";
    std::string verification_key_fn = "verification_key.txt";
#else
    std::string architecture_params_fn;
    std::string computation_bounds_fn;
    std::string proving_key_fn;
    std::string verification_key_fn;

    if (!process_generator_command_line(argc, argv, architecture_params_fn, computation_bounds_fn, proving_key_fn,
                                        verification_key_fn)) {
        return 1;
    }
#endif


    /* load everything */
    ram_ppzksnark_architecture_params<default_ram_ppzksnark_pp> ap;
    std::ifstream f_ap(architecture_params_fn);
    f_ap >> ap;

    std::ifstream f_rp(computation_bounds_fn);
    std::size_t tinyram_input_size_bound, tinyram_program_size_bound, time_bound;
    f_rp >> tinyram_input_size_bound >> tinyram_program_size_bound >> time_bound;

    const std::size_t boot_trace_size_bound = tinyram_program_size_bound + tinyram_input_size_bound;

    const ram_ppzksnark_keypair<default_ram_ppzksnark_pp> keypair =
        ram_ppzksnark_generator<default_ram_ppzksnark_pp>(ap, boot_trace_size_bound, time_bound);

    std::ofstream pk(proving_key_fn);
    pk << keypair.pk;
    pk.close();

    std::ofstream vk(verification_key_fn);
    vk << keypair.vk;
    vk.close();
}
