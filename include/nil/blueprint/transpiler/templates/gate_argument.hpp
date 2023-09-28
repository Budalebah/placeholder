#ifndef __MODULAR_GATE_ARGUMENT_CONTRACT_TEMPLATE_HPP__
#define __MODULAR_GATE_ARGUMENT_CONTRACT_TEMPLATE_HPP__

#include <string>

namespace nil {
    namespace blueprint {
        std::string modular_gate_argument_library_template = R"(
// SPDX-License-Identifier: Apache-2.0.
//---------------------------------------------------------------------------//
// Copyright (c) 2023 Generated by ZKLLVM-transpiler
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//---------------------------------------------------------------------------//
pragma solidity >=0.8.4;

import "../../types.sol";
import "../../basic_marshalling.sol";
import "../../interfaces/modular_gate_argument.sol";
import "hardhat/console.sol";

contract modular_gate_argument_$TEST_NAME$ is IGateArgument{
    uint256 constant modulus = $MODULUS$;

    // Append commitments
    function verify(
        bytes calldata blob,
        uint256 theta
    ) external view returns (uint256 F){
        console.log("Compute gate argument");
$GATE_ARGUMENT_COMPUTATION$
    }
}        )";
    }
}

#endif //__MODULAR_CONTRACT_TEMPLATE_HPP__