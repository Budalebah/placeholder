///////////////////////////////////////////////////////////////
//  Copyright 2012 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt
//
// Comparison operators for big_integer:
//
#pragma once

#include <climits>
#include <cstring>
#include <functional>

#include "nil/crypto3/multiprecision/big_integer/big_integer_impl.hpp"
#include "nil/crypto3/multiprecision/big_integer/detail/config.hpp"
#include "nil/crypto3/multiprecision/big_integer/storage.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4319)
#endif

namespace nil::crypto3::multiprecision {
    template<unsigned Bits, class Op>
    constexpr void bitwise_op(big_integer<Bits>& result, const big_integer<Bits>& o,
                              Op op) noexcept {
        //
        // Both arguments are unsigned types, very simple case handled as a special case.
        //
        // First figure out how big the result needs to be and set up some data:
        //
        unsigned rs = result.size();
        unsigned os = o.size();
        unsigned m(0), x(0);
        boost::multiprecision::minmax(rs, os, m, x);
        typename big_integer<Bits>::limb_pointer pr = result.limbs();
        typename big_integer<Bits>::const_limb_pointer po = o.limbs();
        for (unsigned i = rs; i < x; ++i) {
            pr[i] = 0;
        }

        for (unsigned i = 0; i < os; ++i) {
            pr[i] = op(pr[i], po[i]);
        }
        for (unsigned i = os; i < x; ++i) {
            pr[i] = op(pr[i], limb_type(0));
        }
        result.normalize();
    }

    template<unsigned Bits>
    NIL_CO3_MP_FORCEINLINE constexpr void eval_bitwise_and(big_integer<Bits>& result,
                                                           const big_integer<Bits>& o) noexcept {
        bitwise_op(result, o, std::bit_and());
    }

    template<unsigned Bits>
    NIL_CO3_MP_FORCEINLINE constexpr void eval_bitwise_or(big_integer<Bits>& result,
                                                          const big_integer<Bits>& o) noexcept {
        bitwise_op(result, o, std::bit_or());
    }

    template<unsigned Bits>
    NIL_CO3_MP_FORCEINLINE constexpr void eval_bitwise_xor(big_integer<Bits>& result,
                                                           const big_integer<Bits>& o) noexcept {
        bitwise_op(result, o, std::bit_xor());
    }
    //
    // Again for operands which are single limbs:
    //
    template<unsigned Bits>
    NIL_CO3_MP_FORCEINLINE constexpr void eval_bitwise_and(big_integer<Bits>& result,
                                                           limb_type l) noexcept {
        result.limbs()[0] &= l;
        result.zero_after(1);
    }

    template<unsigned Bits>
    NIL_CO3_MP_FORCEINLINE constexpr void eval_bitwise_or(big_integer<Bits>& result,
                                                          limb_type l) noexcept {
        result.limbs()[0] |= l;
    }

    template<unsigned Bits>
    NIL_CO3_MP_FORCEINLINE constexpr void eval_bitwise_xor(big_integer<Bits>& result,
                                                           limb_type l) noexcept {
        result.limbs()[0] ^= l;
    }

    template<unsigned Bits>
    NIL_CO3_MP_FORCEINLINE constexpr void eval_complement(big_integer<Bits>& result,
                                                          const big_integer<Bits>& o) noexcept {
        unsigned os = o.size();
        for (unsigned i = 0; i < os; ++i) {
            result.limbs()[i] = ~o.limbs()[i];
        }
        result.normalize();
    }

    // Left shift will throw away upper Bits.
    // This function must be called only when s % 8 == 0, i.e. we shift bytes.
    template<unsigned Bits>
    inline void left_shift_byte(big_integer<Bits>& result, double_limb_type s) {
        typedef big_integer<Bits> Int;

        typename Int::limb_pointer pr = result.limbs();

        std::size_t bytes = static_cast<std::size_t>(s / CHAR_BIT);
        if (s >= Bits) {
            // Set result to 0.
            result.zero_after(0);
        } else {
            unsigned char* pc = reinterpret_cast<unsigned char*>(pr);
            std::memmove(pc + bytes, pc, result.size() * sizeof(limb_type) - bytes);
            std::memset(pc, 0, bytes);
        }
    }

    // Left shift will throw away upper Bits.
    // This function must be called only when s % limb_bits == 0, i.e. we shift limbs, which
    // are normally 64 bit.
    template<unsigned Bits>
    inline constexpr void left_shift_limb(big_integer<Bits>& result, double_limb_type s) {
        typedef big_integer<Bits> Int;

        limb_type offset = static_cast<limb_type>(s / Int::limb_bits);
        BOOST_ASSERT(static_cast<limb_type>(s % Int::limb_bits) == 0);

        typename Int::limb_pointer pr = result.limbs();

        if (s >= Bits) {
            // Set result to 0.
            result.zero_after(0);
        } else {
            unsigned i = offset;
            std::size_t rs = result.size() + offset;
            for (; i < result.size(); ++i) {
                pr[rs - 1 - i] = pr[result.size() - 1 - i];
            }
            for (; i < rs; ++i) {
                pr[rs - 1 - i] = 0;
            }
        }
    }

    // Left shift will throw away upper Bits.
    template<unsigned Bits>
    inline constexpr void left_shift_generic(big_integer<Bits>& result, double_limb_type s) {
        typedef big_integer<Bits> Int;

        if (s >= Bits) {
            // Set result to 0.
            result.zero_after(0);
        } else {
            limb_type offset = static_cast<limb_type>(s / Int::limb_bits);
            limb_type shift = static_cast<limb_type>(s % Int::limb_bits);

            typename Int::limb_pointer pr = result.limbs();
            std::size_t i = 0;
            std::size_t rs = result.size();
            // This code only works when shift is non-zero, otherwise we invoke undefined
            // behaviour!
            BOOST_ASSERT(shift);
            for (; rs - i >= 2 + offset; ++i) {
                pr[rs - 1 - i] = pr[rs - 1 - i - offset] << shift;
                pr[rs - 1 - i] |= pr[rs - 2 - i - offset] >> (Int::limb_bits - shift);
            }
            if (rs - i >= 1 + offset) {
                pr[rs - 1 - i] = pr[rs - 1 - i - offset] << shift;
                ++i;
            }
            for (; i < rs; ++i) {
                pr[rs - 1 - i] = 0;
            }
        }
    }

    // Shifting left throws away upper Bits.
    template<unsigned Bits>
    inline constexpr void eval_left_shift(big_integer<Bits>& result, double_limb_type s) noexcept {
        if (!s) {
            return;
        }

#if BOOST_ENDIAN_LITTLE_BYTE && defined(CRYPTO3_MP_USE_LIMB_SHIFT)
        constexpr const limb_type limb_shift_mask = big_integer<Bits>::limb_bits - 1;
        constexpr const limb_type byte_shift_mask = CHAR_BIT - 1;

        if ((s & limb_shift_mask) == 0) {
            left_shift_limb(result, s);
        }
#ifdef BOOST_MP_NO_CONSTEXPR_DETECTION
        else if ((s & byte_shift_mask) == 0)
#else
        else if (((s & byte_shift_mask) == 0) && !BOOST_MP_IS_CONST_EVALUATED(s))
#endif
        {
            left_shift_byte(result, s);
        }
#elif BOOST_ENDIAN_LITTLE_BYTE
        constexpr const limb_type byte_shift_mask = CHAR_BIT - 1;

#ifdef BOOST_MP_NO_CONSTEXPR_DETECTION
        if ((s & byte_shift_mask) == 0)
#else
        constexpr limb_type limb_shift_mask = big_integer<Bits>::limb_bits - 1;
        if (BOOST_MP_IS_CONST_EVALUATED(s) && ((s & limb_shift_mask) == 0)) {
            left_shift_limb(result, s);
        } else if (((s & byte_shift_mask) == 0) && !BOOST_MP_IS_CONST_EVALUATED(s))
#endif
        {
            left_shift_byte(result, s);
        }
#else
        constexpr const limb_type limb_shift_mask = big_integer<Bits>::limb_bits - 1;

        if ((s & limb_shift_mask) == 0) {
            left_shift_limb(result, s);
        }
#endif
        else {
            left_shift_generic(result, s);
        }
        result.normalize();
    }

    template<unsigned Bits>
    inline void right_shift_byte(big_integer<Bits>& result, double_limb_type s) {
        typedef big_integer<Bits> Int;

        limb_type offset = static_cast<limb_type>(s / Int::limb_bits);
        BOOST_ASSERT((s % CHAR_BIT) == 0);
        unsigned ors = result.size();
        unsigned rs = ors;
        if (offset >= rs) {
            result.zero_after(0);
            return;
        }
        rs -= offset;
        typename Int::limb_pointer pr = result.limbs();
        unsigned char* pc = reinterpret_cast<unsigned char*>(pr);
        limb_type shift = static_cast<limb_type>(s / CHAR_BIT);
        std::memmove(pc, pc + shift, ors * sizeof(pr[0]) - shift);
        shift = (sizeof(limb_type) - shift % sizeof(limb_type)) * CHAR_BIT;
        if (shift < Int::limb_bits) {
            pr[ors - offset - 1] &= (static_cast<limb_type>(1u) << shift) - 1;
            if (!pr[ors - offset - 1] && (rs > 1)) {
                --rs;
            }
        }
        // Set zeros after 'rs', alternative to resizing to size 'rs'.
        result.zero_after(rs);
    }

    template<unsigned Bits>
    inline constexpr void right_shift_limb(big_integer<Bits>& result, double_limb_type s) {
        typedef big_integer<Bits> Int;

        limb_type offset = static_cast<limb_type>(s / Int::limb_bits);
        BOOST_ASSERT((s % Int::limb_bits) == 0);
        unsigned ors = result.size();
        unsigned rs = ors;
        if (offset >= rs) {
            result.zero_after(0);
            return;
        }
        rs -= offset;
        typename Int::limb_pointer pr = result.limbs();
        unsigned i = 0;
        for (; i < rs; ++i) {
            pr[i] = pr[i + offset];
        }
        // Set zeros after 'rs', alternative to resizing to size 'rs'.
        result.zero_after(rs);
    }

    template<unsigned Bits>
    inline constexpr void right_shift_generic(big_integer<Bits>& result, double_limb_type s) {
        typedef big_integer<Bits> Int;
        limb_type offset = static_cast<limb_type>(s / Int::limb_bits);
        limb_type shift = static_cast<limb_type>(s % Int::limb_bits);
        unsigned ors = result.size();
        unsigned rs = ors;

        if (offset >= rs) {
            result = limb_type(0);
            return;
        }
        rs -= offset;
        typename Int::limb_pointer pr = result.limbs();
        if ((pr[ors - 1] >> shift) == 0) {
            if (--rs == 0) {
                result = limb_type(0);
                return;
            }
        }
        unsigned i = 0;

        // This code only works for non-zero shift, otherwise we invoke undefined behaviour!
        BOOST_ASSERT(shift);
        for (; i + offset + 1 < ors; ++i) {
            pr[i] = pr[i + offset] >> shift;
            pr[i] |= pr[i + offset + 1] << (Int::limb_bits - shift);
        }
        pr[i] = pr[i + offset] >> shift;

        // We cannot resize any more, so we need to set all the limbs to zero.
        result.zero_after(rs);
    }

    template<unsigned Bits>
    inline constexpr void eval_right_shift(big_integer<Bits>& result, double_limb_type s) noexcept {
        if (!s) {
            return;
        }

#if BOOST_ENDIAN_LITTLE_BYTE && defined(CRYPTO3_MP_USE_LIMB_SHIFT)
        constexpr const limb_type limb_shift_mask = big_integer<Bits>::limb_bits - 1;
        constexpr const limb_type byte_shift_mask = CHAR_BIT - 1;

        if ((s & limb_shift_mask) == 0) right_shift_limb(result, s);
#ifdef BOOST_MP_NO_CONSTEXPR_DETECTION
        else if ((s & byte_shift_mask) == 0)
#else
        else if (((s & byte_shift_mask) == 0) && !BOOST_MP_IS_CONST_EVALUATED(s))
#endif
        {
            right_shift_byte(result, s);
        }
#elif BOOST_ENDIAN_LITTLE_BYTE
        constexpr const limb_type byte_shift_mask = CHAR_BIT - 1;

#ifdef BOOST_MP_NO_CONSTEXPR_DETECTION
        if ((s & byte_shift_mask) == 0)
#else
        constexpr limb_type limb_shift_mask = big_integer<Bits>::limb_bits - 1;
        if (BOOST_MP_IS_CONST_EVALUATED(s) && ((s & limb_shift_mask) == 0)) {
            right_shift_limb(result, s);
        } else if (((s & byte_shift_mask) == 0) && !BOOST_MP_IS_CONST_EVALUATED(s))
#endif
        {
            right_shift_byte(result, s);
        }
#else
        constexpr const limb_type limb_shift_mask = big_integer<Bits>::limb_bits - 1;

        if ((s & limb_shift_mask) == 0) {
            right_shift_limb(result, s);
        }
#endif
        else {
            right_shift_generic(result, s);
        }
    }
}  // namespace nil::crypto3::multiprecision

#ifdef _MSC_VER
#pragma warning(pop)
#endif
