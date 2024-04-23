#ifndef ZKEMV_FRAMEWORK_BIN_ASSIGNER_INCLUDE_UTILS_HPP_
#define ZKEMV_FRAMEWORK_BIN_ASSIGNER_INCLUDE_UTILS_HPP_

#include <fstream>
#include <iostream>
#include <map>

#include "execution_state.hpp"
#include "vm.hpp"
#include "zkevm_framework/data_types/base.hpp"
#include "zkevm_framework/data_types/block.hpp"

struct vm_account {
    virtual ~vm_account() = default;

    // evmc::uint256be = evmc::bytes32 = uint8_t[32]

    evmc::uint256be balance = {};
    // code for execute call
    std::vector<uint8_t> code;

    // sload/sstore op codes
    std::map<evmc::bytes32, evmc::bytes32> storage;
    // tload/tstore op codes
    std::map<evmc::bytes32, evmc::bytes32> transient_storage;

    virtual evmc::bytes32 code_hash() const {
        // Extremely dumb "hash" function.
        evmc::bytes32 ret{};
        for (const auto v : code) ret.bytes[v % sizeof(ret.bytes)] ^= v;
        return ret;
    }
};

using vm_accounts = std::map<evmc::address, vm_account>;

class VmHost : public evmc::Host {
    vm_accounts accounts;
    evmc_tx_context tx_context{};

  public:
    VmHost() = default;
    explicit VmHost(evmc_tx_context& _tx_context) noexcept : tx_context{_tx_context} {}
    VmHost(evmc_tx_context& _tx_context, vm_accounts& _accounts) noexcept
        : accounts{_accounts}, tx_context{_tx_context} {}

    bool account_exists(const evmc::address& addr) const noexcept final {
        return accounts.find(addr) != accounts.end();
    }

    evmc::bytes32 get_storage(const evmc::address& addr,
                              const evmc::bytes32& key) const noexcept final {
        const auto account_iter = accounts.find(addr);
        if (account_iter == accounts.end()) {
            return {};
        }

        const auto storage_iter = account_iter->second.storage.find(key);
        if (storage_iter != account_iter->second.storage.end()) {
            return storage_iter->second;
        }
        return {};
    }

    evmc_storage_status set_storage(const evmc::address& addr, const evmc::bytes32& key,
                                    const evmc::bytes32& value) noexcept final {
        if (accounts.find(addr) == accounts.end()) {
            return EVMC_STORAGE_DELETED;
        }
        auto& account = accounts[addr];
        auto prev_value = account.storage[key];
        account.storage[key] = value;

        return (prev_value == value) ? EVMC_STORAGE_ASSIGNED : EVMC_STORAGE_MODIFIED;
    }

    evmc::uint256be get_balance(const evmc::address& addr) const noexcept final {
        auto it = accounts.find(addr);
        if (it != accounts.end()) {
            return it->second.balance;
        }
        return {};
    }

    size_t get_code_size(const evmc::address& addr) const noexcept final {
        auto it = accounts.find(addr);
        if (it != accounts.end()) {
            return it->second.code.size();
        }
        return 0;
    }

    evmc::bytes32 get_code_hash(const evmc::address& addr) const noexcept final {
        auto it = accounts.find(addr);
        if (it != accounts.end()) {
            return it->second.code_hash();
        }
        return {};
    }

    size_t copy_code(const evmc::address& addr, size_t code_offset, uint8_t* buffer_data,
                     size_t buffer_size) const noexcept final {
        const auto it = accounts.find(addr);
        if (it == accounts.end()) {
            return 0;
        }

        const auto& code = it->second.code;

        if (code_offset >= code.size()) {
            return 0;
        }

        const auto n = std::min(buffer_size, code.size() - code_offset);

        if (n > 0) {
            std::copy_n(&code[code_offset], n, buffer_data);
        }
        return n;
    }

    bool selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept final {
        (void)addr;
        (void)beneficiary;
        return false;
    }

    evmc::Result call(const evmc_message& msg) noexcept final {
        return evmc::Result{EVMC_REVERT, msg.gas, 0, msg.input_data, msg.input_size};
    }

    evmc_tx_context get_tx_context() const noexcept final { return tx_context; }

    evmc::bytes32 get_block_hash(int64_t number) const noexcept final {
        evmc::bytes32 ret{};
        return ret;
    }

    void emit_log(const evmc::address& addr, const uint8_t* data, size_t data_size,
                  const evmc::bytes32 topics[], size_t topics_count) noexcept final {
        (void)addr;
        (void)data;
        (void)data_size;
        (void)topics;
        (void)topics_count;
    }

    evmc_access_status access_account(const evmc::address& addr) noexcept final {
        (void)addr;
        return EVMC_ACCESS_COLD;
    }

    evmc_access_status access_storage(const evmc::address& addr,
                                      const evmc::bytes32& key) noexcept final {
        (void)addr;
        (void)key;
        return EVMC_ACCESS_COLD;
    }

    evmc::bytes32 get_transient_storage(const evmc::address& addr,
                                        const evmc::bytes32& key) const noexcept override {
        const auto account_iter = accounts.find(addr);
        if (account_iter == accounts.end()) {
            return {};
        }

        const auto transient_storage_iter = account_iter->second.transient_storage.find(key);
        if (transient_storage_iter != account_iter->second.transient_storage.end()) {
            return transient_storage_iter->second;
        }
        return {};
    }

    void set_transient_storage(const evmc::address& addr, const evmc::bytes32& key,
                               const evmc::bytes32& value) noexcept override {
        accounts[addr].transient_storage[key] = value;
    }
};

template<typename T>
evmc::uint256be to_uint256be(T v) {
    evmc::uint256be res;
    uint8_t mask = (1 << 8) - 1;
    for (uint8_t i = 0; i <= 31; i++) {
        res.bytes[i] = (v >> (i - 31) * 8);
    }
    return res;
}

evmc_address to_evmc_address(const data_types::Address& v) {
    evmc_address res = {
        v[0],  v[1],  v[2],  v[3],  v[4],  v[5],  v[6],  v[7],  v[8],  v[9],
        v[10], v[11], v[12], v[13], v[14], v[15], v[16], v[17], v[18], v[19],
    };
    return res;
}

std::string to_str(const evmc_address& v) {
    std::stringstream os;
    os << std::hex << "{ " << v.bytes[0] << " " << v.bytes[1] << " " << v.bytes[2] << " "
       << v.bytes[3] << " " << v.bytes[4] << " " << v.bytes[5] << " " << v.bytes[6] << " "
       << v.bytes[7] << " " << v.bytes[8] << " " << v.bytes[9] << " " << v.bytes[10] << " "
       << v.bytes[11] << " " << v.bytes[12] << " " << v.bytes[13] << " " << v.bytes[14] << " "
       << v.bytes[14] << " " << v.bytes[16] << " " << v.bytes[15] << " " << v.bytes[18] << " "
       << v.bytes[19] << " }";
    return os.str();
}

std::string to_str(const data_types::Address& v) {
    std::stringstream os;
    os << std::hex << "{ " << static_cast<int>(v[0]) << " " << static_cast<int>(v[1]) << " "
       << static_cast<int>(v[2]) << " " << static_cast<int>(v[3]) << " " << static_cast<int>(v[4])
       << " " << static_cast<int>(v[5]) << " " << static_cast<int>(v[6]) << " "
       << static_cast<int>(v[7]) << " " << static_cast<int>(v[8]) << " " << static_cast<int>(v[9])
       << " " << static_cast<int>(v[10]) << " " << static_cast<int>(v[11]) << " "
       << static_cast<int>(v[12]) << " " << static_cast<int>(v[13]) << " "
       << static_cast<int>(v[14]) << " " << static_cast<int>(v[15]) << " "
       << static_cast<int>(v[16]) << " " << static_cast<int>(v[17]) << " "
       << static_cast<int>(v[18]) << " " << static_cast<int>(v[19]) << " }";
    return os.str();
}

std::string to_str(const data_types::Transaction::Type& type) {
    switch (type) {
        case data_types::Transaction::Type::NullTransaction:
            return "NullTransaction";
        case data_types::Transaction::Type::ContractCreation:
            return "ContractCreation";
        case data_types::Transaction::Type::MessageCall:
            return "MessageCall";
        default:
            return "none";
    };
}

std::string to_str(evmc_status_code code) {
    switch (code) {
        case EVMC_SUCCESS:
            return "EVMC_SUCCESS";
        case EVMC_FAILURE:
            return "EVMC_FAILURE";
        case EVMC_REVERT:
            return "EVMC_REVERT";
        case EVMC_OUT_OF_GAS:
            return "EVMC_OUT_OF_GAS";
        case EVMC_INVALID_INSTRUCTION:
            return "EVMC_INVALID_INSTRUCTION";
        case EVMC_UNDEFINED_INSTRUCTION:
            return "EVMC_UNDEFINED_INSTRUCTION";
        case EVMC_STACK_OVERFLOW:
            return "EVMC_STACK_OVERFLOW";
        case EVMC_STACK_UNDERFLOW:
            return "EVMC_STACK_UNDERFLOW";
        case EVMC_BAD_JUMP_DESTINATION:
            return "EVMC_BAD_JUMP_DESTINATION";
        case EVMC_INVALID_MEMORY_ACCESS:
            return "EVMC_INVALID_MEMORY_ACCESS";
        case EVMC_CALL_DEPTH_EXCEEDED:
            return "EVMC_CALL_DEPTH_EXCEEDED";
        case EVMC_STATIC_MODE_VIOLATION:
            return "EVMC_STATIC_MODE_VIOLATION";
        case EVMC_PRECOMPILE_FAILURE:
            return "EVMC_PRECOMPILE_FAILURE";
        case EVMC_CONTRACT_VALIDATION_FAILURE:
            return "EVMC_CONTRACT_VALIDATION_FAILURE";
        case EVMC_ARGUMENT_OUT_OF_RANGE:
            return "EVMC_ARGUMENT_OUT_OF_RANGE";
        case EVMC_WASM_UNREACHABLE_INSTRUCTION:
            return "EVMC_WASM_UNREACHABLE_INSTRUCTION";
        case EVMC_WASM_TRAP:
            return "EVMC_WASM_TRAP";
        case EVMC_INSUFFICIENT_BALANCE:
            return "EVMC_INSUFFICIENT_BALANCE";
        case EVMC_INTERNAL_ERROR:
            return "EVMC_INTERNAL_ERROR";
        case EVMC_REJECTED:
            return "EVMC_REJECTED";
        case EVMC_OUT_OF_MEMORY:
            return "EVMC_OUT_OF_MEMORY";
        default:
            return "none";
    };
}

#endif  // ZKEMV_FRAMEWORK_BIN_ASSIGNER_INCLUDE_UTILS_HPP_
