/********************************************************************************/
/* Copyright 2018 Christofer Stegmayr                                           */
/*                                                                              */
/* Permission is hereby granted, free of charge, to any person obtaining a copy */
/* of this software and associated documentation files (the "Software"),        */
/* to deal in the Software without restriction, including without limitation    */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,     */
/* and/or sell copies of the Software, and to permit persons to whom the        */
/* Software is furnished to do so, subject to the following conditions:         */
/*                                                                              */
/* The above copyright notice and this permission notice shall be included in   */
/* all copies or substantial portions of the Software.                          */
/*                                                                              */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS */
/* IN THE SOFTWARE.                                                             */
/********************************************************************************/

#pragma once

#include <cstdint>        // For uint32_t/uint64_t
#include <cstring>        // For memset
#include <iterator>       // For std::iterator


#ifdef _MSC_VER           // Currently only valid for x64
#include <intrin.h>       // For _BitScanForward64
#define BT uint64_t
#define bsf(a, b) _BitScanForward64((unsigned long*)a,b)
#define bt(a,b) _bittest64((int64_t*)a,(int64_t)b)
#define btr(a,b) _bittestandreset64((int64_t*)a,(int64_t)b)
#define bts(a,b) _bittestandset64((int64_t*)a,(int64_t)b)
#define btc(a,b) _bittestandcomplement64((int64_t*)a,(int64_t)b)
#define popcnt(a) __popcnt64(a) // Risky instruction, as it may not be supported on all hardware (consider implementing using SSE instead: https://stackoverflow.com/questions/17354971/fast-counting-the-number-of-set-bits-in-m128i-register)
#else
#error No valid intrinsic substitute for compiler
#endif

namespace cs
{	
    template<uint32_t NBITS>
    class bitfield
    {
    private:
        using b_type = BT;
        static const b_type BIT_LEN = (b_type)(sizeof(b_type) * 8ull);
        inline void g_idx(uint32_t idx, b_type& i, b_type& tst) { i = (b_type)(idx / BIT_LEN); tst = (b_type)(idx%BIT_LEN); }
    public:
        using size_type = uint32_t;
        struct bits_container { b_type v[(NBITS + BIT_LEN - 1u) / (BIT_LEN)]; }; // Round up to nearest integer (i.e. LENGTH=55 => N=floor((55+63)/64)=1)

        bitfield() { reset(bits); }

        bool check_bit(size_type idx) { if (idx >= NBITS) return false; b_type i, tst;  g_idx(idx, i, tst); return !!bt(&bits.v[i], tst); }
        bool set_bit(size_type idx) { if (idx >= NBITS) return false; b_type i, tst;  g_idx(idx, i, tst); return !!bts(&bits.v[i], tst); }
        bool clear_bit(size_type idx) { if (idx >= NBITS) return false; b_type i, tst;  g_idx(idx, i, tst); return !!btr(&bits.v[i], tst); }
        bool toggle_bit(size_type idx) { if (idx >= NBITS) return false; b_type i, tst;  g_idx(idx, i, tst); return !!btc(&bits.v[i], tst); }

        size_type length() 
        {
            size_type len = 0u;
            const size_type n = (size_type)(sizeof(bits_container) / sizeof(b_type));

            for (size_type i = 0u; i < n; ++i)
            {
                auto tmp_len = popcnt(bits.v[i]);
                len += (size_type)tmp_len;
            }
            return len;
        }


        static void reset(bits_container& v) { memset(&v, 0, sizeof(bits_container)); }

        bits_container bits;

        /////////////////////////////////////////////////////////////////////////////////////

        class iterator : public std::iterator<std::input_iterator_tag, uint32_t, int32_t, const uint32_t*, uint32_t>
        {
        public:
            iterator() : _index(~0u) {}
            explicit iterator(bits_container v) : _bits(v), _index(~0u) { find_first_index(); }

            reference operator*() const { return _index; }
            iterator& operator++() { find_first_index(); return *this; }
            iterator operator++(int) { iterator r = *this; ++(*this); return r; }
            bool operator==(iterator other) const { return _index == other._index; }
            bool operator!=(iterator other) const { return !(*this == other); }


        private:
            void find_first_index()
            {
                _index = ~0u;
                const value_type n = (value_type)(sizeof(bits_container) / sizeof(b_type));

                for (value_type i = 0u; i < n; ++i)
                {
                    b_type idx = 0ull;
                    if (bsf(&idx, _bits.v[i]))
                    {
                        btr(&_bits.v[i], idx);
                        _index = (value_type)(i * BIT_LEN + idx);
                        return;
                    }
                }
            }

            bits_container _bits;
            value_type     _index;
        };

        using const_iterator = const iterator;
        /////////////////////////////////////////////////////////////////////////////////////
        
        iterator begin() { return iterator(bits); }
        const_iterator end() const { return iterator(); }
    };
}
    
#undef BT
#undef bfs
#undef bt
#undef bts
#undef btr
#undef btc
#undef popcnt

