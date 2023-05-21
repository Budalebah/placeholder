//---------------------------------------------------------------------------//
// Copyright (c) 2023 Dmitrii Tabalin <d.tabalin@nil.foundation>
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

#ifndef CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_NON_NATIVE_LESS_THAN_CONSTANT_HPP
#define CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_NON_NATIVE_LESS_THAN_CONSTANT_HPP

#include <nil/crypto3/zk/snark/arithmetization/plonk/constraint_system.hpp>

#include <nil/marshalling/algorithms/pack.hpp>

#include <nil/blueprint/blueprint/plonk/circuit.hpp>
#include <nil/blueprint/blueprint/plonk/assignment.hpp>
#include <nil/blueprint/component.hpp>

#include <type_traits>
#include <utility>

namespace nil {
    namespace blueprint {
        namespace components {
            template<typename ArithmetizationType, std::uint32_t WitnessesAmount, std::size_t R>
            class less_than_constant;

            /*
                Checks if input x is less than some predefined constant c, which has R bits.
                Constant c should be less than 2^{modulus_bits - 2} (so R < modulus_bits - 1).

                We first find the smallest n such that 2^{n} > c.
                We check that both x and c - x are less than 2^{n} and c - x != 0.
                The first check is done by splitting x (c-x) into bit chunks and checking that their weighted sum is
                equal to x (c-x). The second check is done by providing an inverse for c - x.

                The component is multiple copies of the following gate (illustrated for WitnessesAmount = 15):
                +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                |x|y|p|p|p|p|p|p|p|p|p|p|p|p|p|
                +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                |p|o|o|o|o|o|o|o|o|o|o|o|o|o|o|
                +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                |x|y| | | | | | | | | | | | | |
                +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                Where x and y are weighted sum of bit chunks for x and c - x respectively, and o/p are the bit chunks
                of x and c - x respectively. Empty spaces are not constrained.
                Starting sums for x and y are constrained to be zero.

                We use the third cell in the final row to store the inverse of c - x, in order to check that c-x != 0.
            */
            template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                     std::size_t R>
            class less_than_constant<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                                 ArithmetizationParams>,
                                     WitnessesAmount, R> :
                public plonk_component<BlueprintFieldType, ArithmetizationParams, WitnessesAmount, 1, 0> {

                using component_type = plonk_component<BlueprintFieldType, ArithmetizationParams,
                                                       WitnessesAmount, 1, 0>;
                using value_type = typename BlueprintFieldType::value_type;
            public:
                using var = typename component_type::var;

                constexpr static const std::size_t chunk_size = 2;
                constexpr static const std::size_t chunk_amount = (R + chunk_size - 1) / chunk_size;
                // Techincally, this is average chunks per row after first.
                constexpr static const std::size_t chunks_per_row = WitnessesAmount - 1;
                constexpr static const std::size_t bits_per_row = chunks_per_row * chunk_size;
                // We need to pad each of x, c-x up to the nearest multiple of WitnessAmount - 1.
                constexpr static const std::size_t padded_chunks =
                    (chunk_amount + WitnessesAmount - 2) / (WitnessesAmount - 1) * (WitnessesAmount - 1);
                constexpr static const std::size_t padding_size = padded_chunks - chunk_amount;
                constexpr static const std::size_t padding_bits = padded_chunks * chunk_size - R;

                constexpr static const std::size_t rows_amount = 1 + 2 * padded_chunks / (WitnessesAmount - 1);
                constexpr static const bool needs_first_chunk_constraint =
                    (R % chunk_size ? 1 : 0) &&
                    (R + ((chunk_size - R % chunk_size) % chunk_size) >= BlueprintFieldType::modulus_bits - 1);
                constexpr static const std::size_t gates_amount = 2 + needs_first_chunk_constraint;

                struct input_type {
                    var x;
                    value_type constant;
                };

                struct result_type {
                    result_type(const less_than_constant &component, std::size_t start_row_index) {}
                };

                template <typename ContainerType>
                    less_than_constant(ContainerType witness):
                        component_type(witness, {}, {}) {};

                template <typename WitnessContainerType, typename ConstantContainerType, typename PublicInputContainerType>
                    less_than_constant(WitnessContainerType witness, ConstantContainerType constant, PublicInputContainerType public_input):
                        component_type(witness, constant, public_input) {};

                less_than_constant(
                    std::initializer_list<typename component_type::witness_container_type::value_type> witnesses,
                    std::initializer_list<typename component_type::constant_container_type::value_type> constants,
                    std::initializer_list<typename component_type::public_input_container_type::value_type>
                        public_inputs) : component_type(witnesses, constants, public_inputs) {};

            };

            template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                     std::size_t R>
            using plonk_less_than_constant =
                less_than_constant<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                               ArithmetizationParams>,
                                   WitnessesAmount, R>;

            template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                     std::size_t R,
                     std::enable_if_t<R < BlueprintFieldType::modulus_bits - 1, bool> = true>
            void generate_gates(
                const plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                               WitnessesAmount, R>
                    &component,
                circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                    ArithmetizationParams>>
                    &bp,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                       ArithmetizationParams>>
                    &assignment,
                const typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                                        WitnessesAmount, R>::input_type
                    &instance_input,
                const std::size_t first_selector_index) {

                using var = typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                                              WitnessesAmount, R>::var;
                using constraint_type = crypto3::zk::snark::plonk_constraint<BlueprintFieldType>;
                using gate_type = typename crypto3::zk::snark::plonk_gate<BlueprintFieldType, constraint_type>;

                typename BlueprintFieldType::value_type base_two = 2;
                std::vector<constraint_type> constraints;
                constraints.reserve(WitnessesAmount * 2 - 2);

                auto generate_chunk_size_constraint = [](var v, std::size_t size) {
                    constraint_type constraint = v;
                    for (std::size_t i = 1; i < (1 << size); i++) {
                        constraint = constraint * (v - i);
                    }
                    return constraint;
                };

                // Assert chunk size.
                for (std::size_t row_idx = 0; row_idx < 2; row_idx++) {
                    for (std::size_t i = 2 * (1 - row_idx); i < WitnessesAmount; i++) {
                        constraint_type chunk_range_constraint =
                            generate_chunk_size_constraint(var(component.W(i), int(row_idx) - 1, true),
                                                           component.chunk_size);

                        constraints.push_back(bp.add_constraint(chunk_range_constraint));
                    }
                }
                // Assert sums. var_idx = 0 is x, var_idx = 1 is y.
                for (int var_idx = 0; var_idx < 2; var_idx++) {
                    constraint_type sum_constraint = var(component.W(1 + var_idx), -var_idx, true);
                    for (std::size_t i = 2 + var_idx; i < WitnessesAmount; i++) {
                        sum_constraint = base_two.pow(component.chunk_size) * sum_constraint +
                                         var(component.W(i), -var_idx, true);
                    }
                    if (var_idx == 1) {
                        sum_constraint = base_two.pow(component.chunk_size) * sum_constraint +
                                         var(component.W(0), 0, true);
                    }
                    sum_constraint = sum_constraint +
                                        base_two.pow(component.chunk_size * component.chunks_per_row) *
                                                    var(component.W(var_idx), -1, true) -
                                        var(component.W(var_idx), 1, true);
                    constraints.push_back(bp.add_constraint(sum_constraint));
                }

                gate_type gate(first_selector_index, constraints);
                bp.add_gate(gate);

                std::vector<constraint_type> correctness_constraints;
                constraint_type non_zero_constraint = var(component.W(1), 0, true) * var(component.W(2), 0, true) - 1,
                                c_minus_x_constraint = instance_input.constant - var(component.W(0), 0, true) -
                                                       var(component.W(1), 0, true);
                correctness_constraints.push_back(non_zero_constraint);
                correctness_constraints.push_back(c_minus_x_constraint);
                gate = gate_type(first_selector_index + 1, correctness_constraints);
                bp.add_gate(gate);

                if (!component.needs_first_chunk_constraint) return;
                // If R is not divisible by chunk size, the first chunk of both x/c-x should be constrained to be
                // less than 2^{R % component.chunk_size}.
                // We actually only need this constraint when c - x can do an unsafe overflow.
                // Otherwise the constraint on c - x takes care of this.
                std::vector<constraint_type> first_chunk_range_constraints;

                var size_constraint_var = component.padding_size != WitnessesAmount - 2 ?
                                            var(component.W(2 + component.padding_size), 0, true)
                                          : var(component.W(0), 1, true);
                constraint_type first_chunk_range_constraint = generate_chunk_size_constraint(
                    size_constraint_var, R % component.chunk_size);
                first_chunk_range_constraints.push_back(first_chunk_range_constraint);

                size_constraint_var = var(component.W(1 + component.padding_size), 1, true);
                first_chunk_range_constraint =
                    generate_chunk_size_constraint(size_constraint_var, R % component.chunk_size);
                first_chunk_range_constraints.push_back(first_chunk_range_constraint);

                gate = gate_type(first_selector_index + 2, first_chunk_range_constraints);
                bp.add_gate(gate);
            }

            template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                     std::size_t R,
                     std::enable_if_t<R < BlueprintFieldType::modulus_bits - 1, bool> = true>
            void generate_copy_constraints(
                const plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                               WitnessesAmount, R>
                    &component,
                circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                    ArithmetizationParams>>
                    &bp,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                       ArithmetizationParams>>
                    &assignment,
                const typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                                        WitnessesAmount, R>::input_type
                    &instance_input,
                const std::uint32_t start_row_index) {

                using var = typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                                              WitnessesAmount, R>::var;
                std::uint32_t row = start_row_index;
                var zero(0, start_row_index, false, var::column_type::constant);

                bp.add_copy_constraint({zero, var(component.W(0), start_row_index, false)});
                bp.add_copy_constraint({zero, var(component.W(1), start_row_index, false)});

                // Padding constraints for x
                for (std::size_t i = 0; i < component.padding_size; i++) {
                    bp.add_copy_constraint({zero, var(component.W(i + 1), start_row_index + 1, false)});
                }
                // Padding constraints for y
                for (std::size_t i = 0; i < component.padding_size; i++) {
                    bp.add_copy_constraint({zero, var(component.W(i + 2), start_row_index, false)});
                }

                row += component.rows_amount - 1;
                bp.add_copy_constraint({instance_input.x, var(component.W(0), row, false)});
            }

            template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                     std::size_t R,
                     std::enable_if_t<R < BlueprintFieldType::modulus_bits - 1, bool> = true>
            typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                              WitnessesAmount, R>::result_type
            generate_circuit(
                const plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                               WitnessesAmount, R>
                    &component,
                circuit<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                    ArithmetizationParams>>
                    &bp,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                        ArithmetizationParams>>
                    &assignment,
                const typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                                        WitnessesAmount, R>::input_type
                    &instance_input,
                const std::uint32_t start_row_index) {

                auto selector_iterator = assignment.find_selector(component);
                std::size_t first_selector_index;

                if (selector_iterator == assignment.selectors_end()) {
                    first_selector_index = assignment.allocate_selector(component, component.gates_amount);
                    generate_gates(component, bp, assignment, instance_input, first_selector_index);
                } else {
                    first_selector_index = selector_iterator->second;
                }

                assignment.enable_selector(first_selector_index, start_row_index + 1,
                                           start_row_index + component.rows_amount - 2, 2);
                assignment.enable_selector(first_selector_index + 1, start_row_index + component.rows_amount - 1);

                if (component.needs_first_chunk_constraint) {
                    assignment.enable_selector(first_selector_index + 2, start_row_index);
                }

                generate_copy_constraints(component, bp, assignment, instance_input, start_row_index);
                generate_assignments_constants(component, assignment, instance_input, start_row_index);

                return typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                                         WitnessesAmount, R>::result_type(
                        component, start_row_index);
            }

            template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                     std::size_t R,
                     std::enable_if_t<R < BlueprintFieldType::modulus_bits - 1, bool> = true>
            typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                              WitnessesAmount, R>::result_type
            generate_assignments(
                const plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                               WitnessesAmount, R>
                    &component,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                        ArithmetizationParams>>
                    &assignment,
                const typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                                        WitnessesAmount, R>::input_type
                    &instance_input,
                const std::uint32_t start_row_index) {

                std::size_t row = start_row_index;

                using component_type = plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                                                WitnessesAmount, R>;
                using value_type = typename BlueprintFieldType::value_type;
                using integral_type = typename BlueprintFieldType::integral_type;
                using chunk_type = std::uint8_t;
                BOOST_ASSERT(component.chunk_size <= 8);

                value_type x = var_value(assignment, instance_input.x),
                           y = instance_input.constant - x;

                std::array<integral_type, 2> integrals = {integral_type(x.data), integral_type(y.data)};

                std::array<std::array<bool, R + component_type::padding_bits>, 2> bits;
                for (std::size_t i = 0; i < 2; i++) {
                    bits[i].fill(0);

                    nil::marshalling::status_type status;
                    std::array<bool, BlueprintFieldType::modulus_bits> bytes_all =
                        nil::marshalling::pack<nil::marshalling::option::big_endian>(integrals[i], status);
                    std::copy(bytes_all.end() - R, bytes_all.end(),
                              bits[i].begin() + component.padding_bits);
                    assert(status == nil::marshalling::status_type::success);
                }

                std::array<std::array<chunk_type, component_type::padded_chunks>, 2> chunks;
                for (std::size_t i = 0; i < 2; i++) {
                    for (std::size_t j = 0; j < component.padded_chunks; j++) {
                        chunk_type chunk_value = 0;
                        for (std::size_t k = 0; k < component.chunk_size; k++) {
                            chunk_value <<= 1;
                            chunk_value |= bits[i][j * component.chunk_size + k];
                        }
                        chunks[i][j] = chunk_value;
                    }
                }

                assignment.witness(component.W(0), row) = assignment.witness(component.W(1), row) = 0;

                std::array<value_type, 2> sum = {0, 0};
                for (std::size_t i = 0; i < (component.rows_amount - 1) / 2; i++) {
                    // Filling the first row.
                    for (std::size_t j = 0; j < component.chunks_per_row - 1; j++) {
                        assignment.witness(component.W(j + 2), row) =
                            chunks[1][i * component.chunks_per_row + j];
                        sum[1] *= (1 << component.chunk_size);
                        sum[1] += chunks[1][i * component.chunks_per_row + j];
                    }
                    row++;
                    // Filling the second row.
                    assignment.witness(component.W(0), row) = chunks[1][i * component.chunks_per_row +
                                                                        component.chunks_per_row - 1];
                    sum[1] *= (1 << component.chunk_size);
                    sum[1] += chunks[1][i * component.chunks_per_row + component.chunks_per_row - 1];

                    for (std::size_t j = 0; j < component.chunks_per_row; j++) {
                        assignment.witness(component.W(j + 1), row) =
                            chunks[0][i * component.chunks_per_row + j];
                        sum[0] *= (1 << component.chunk_size);
                        sum[0] += chunks[0][i * component.chunks_per_row + j];
                    }
                    row++;
                    // Filling the sums
                    assignment.witness(component.W(0), row) = sum[0];
                    assignment.witness(component.W(1), row) = sum[1];
                }
                assignment.witness(component.W(2), row) = 1 / y;
                row++;
                BOOST_ASSERT(row == start_row_index + component.rows_amount);

                return typename component_type::result_type(component, start_row_index);
            }

            template<typename BlueprintFieldType, typename ArithmetizationParams, std::uint32_t WitnessesAmount,
                     std::size_t R,
                     std::enable_if_t<R < BlueprintFieldType::modulus_bits - 1, bool> = true>
            void generate_assignments_constants(
                const plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                               WitnessesAmount, R>
                    &component,
                assignment<crypto3::zk::snark::plonk_constraint_system<BlueprintFieldType,
                                                                        ArithmetizationParams>>
                    &assignment,
                const typename plonk_less_than_constant<BlueprintFieldType, ArithmetizationParams,
                                                        WitnessesAmount, R>::input_type
                    &instance_input,
                const std::uint32_t start_row_index) {

                assignment.constant(component.C(0), start_row_index) = 0;
            }
        }    // namespace components
    }        // namespace blueprint
}   // namespace nil

#endif    // CRYPTO3_BLUEPRINT_COMPONENTS_PLONK_NON_NATIVE_LESS_THAN_CONSTANT_HPP
