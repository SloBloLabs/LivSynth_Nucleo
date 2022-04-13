#include <bitset>
#include <cassert>
#include <iostream>

int main()
{
    // constructors:
    constexpr std::bitset<4> b1;
    constexpr std::bitset<4> b2{0xA}; // == 0B1010
    std::bitset<4> b3{"0011"}; // can't be constexpr yet
    std::bitset<8> b4{"ABBA", /*length*/4, /*0:*/'A', /*1:*/'B'}; // == 0B0000'0110
 
    // bitsets can be printed out to a stream:
    std::cout << "b1:" << b1 << "; b2:" << b2 << "; b3:" << b3 << "; b4:" << b4 << '\n';
 
    // bitset supports bitwise operations:
    b3 |= 0b0100; assert(b3 == 0b0111);
    b3 &= 0b0011; assert(b3 == 0b0011);
    b3 ^= std::bitset<4>{0b1100}; assert(b3 == 0b1111);
 
    // operations on the whole set:
    b3.reset(); assert(b3 == 0);
    b3.set(); assert(b3 == 0b1111);
    assert(b3.all() && b3.any() && !b3.none());
    b3.flip(); assert(b3 == 0);
 
    // operations on individual bits:
    b3.set(/* position = */ 1, true); assert(b3 == 0b0010);
    b3.set(/* position = */ 1, false); assert(b3 == 0);
    b3.flip(/* position = */ 2); assert(b3 == 0b0100);
    b3.reset(/* position = */ 2); assert(b3 == 0);
 
    // subscript operator[] is supported:
    b3[2] = true; assert(true == b3[2] && b3 == 0b0100);
 
    // other operations:
    assert(b3.count() == 1);
    assert(b3.size() == 4);
    assert(b3.to_ullong() == 0b0100ULL);
    assert(b3.to_string() == "0100");

    uint32_t x = 1UL;
    std::cout << x << ", " << !x << "\n";

    uint16_t y = (7372 % 0x1000) - 1; // 0x1000 = 4096
    std::cout << y << "\n";

    uint32_t freq = 25000000;
    uint32_t pllm = 4;
    uint32_t plln = 180;
    uint32_t pllp = 0;

#define __LL_RCC_CALC_PLLCLK_FREQ(__INPUTFREQ__, __PLLM__, __PLLN__, __PLLP__) \
    __INPUTFREQ__ / __PLLM__ * __PLLN__ / (((__PLLP__ >> 16U ) + 1U) * 2U)

    uint32_t result1 = __LL_RCC_CALC_PLLCLK_FREQ(freq, pllm, plln, pllp);
    std::cout << result1 << "\n";

    uint32_t result2 = freq / pllm * plln / (((pllp >> 16U) + 1U) * 2U);
    std::cout << result2 << "\n";
}