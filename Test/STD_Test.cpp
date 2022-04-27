#define CONFIG_PPQN 192

#include "../App/Utils/Utils.h"
#include "../App/Utils/Groove.h"
#include <bitset>
#include <cassert>
#include <iostream>

void bpmPeriodError() {
    uint32_t naivePeriod;
    uint32_t roundedPeriod;
    for(float bpm = 20; bpm <= 200.f; bpm += .1f) {
        naivePeriod = (60 * 1000000) / (bpm * 192);
        roundedPeriod = std::round((60 * 1000000) / (bpm * 192));
        std::cout << "bpm: " << bpm << ", naive period: " << naivePeriod << ", rounded period: " << roundedPeriod;
        if(naivePeriod != roundedPeriod) std::cout << " -> error!";
        std::cout << std::endl;
    }
}

uint32_t nextTick, nextTickOn, nextTickOff;

void outputTick(uint32_t tick) {

    auto applySwing = [&] (uint32_t tick) {
        int swing = 75;
        return swing != 0 ? Groove::applySwing(tick, swing) : tick;
    };
    
    if(tick == nextTick) {
        uint32_t divisor = 48;
        // "target 1ms, at least one tick"
        uint32_t clockDuration = std::max(uint32_t(1), uint32_t(120.f * 192.f * 1.f / (60 * 1000)));
        //std::cout << "ClockDuration: " << clockDuration << std::endl;

        nextTickOn = applySwing(tick);
        nextTickOff = std::min(nextTickOn + clockDuration, applySwing(nextTick + divisor) - 1);

        nextTick += divisor;

        std::cout << "tick: " << tick << ", nextTick: " << nextTick << ", nextTickOn: " << nextTickOn << ", nextTickOff: " << nextTickOff << std::endl;
    }

    if(tick == nextTickOn) {
        std::cout << "set clock at " << tick << std::endl;
    }

    if(tick == nextTickOff) {
        std::cout << "reset clock at " << tick << std::endl;
    }
}

void testHSV2RGB() {
    for(float H = 0.f; H < 360.f; H += 10.f) {
        float S = 1., V = 1., R, G, B;
        HSVtoRGB(H, S, V, R, G, B);
        std::cout << "H: " << H << ", R: " << R << ", G: " << G << ", B: " << B << std::endl;
    }
}

void testHSV2RGB2() {
    float H = 0.f, S = 0., V = 1., R, G, B;
    HSVtoRGB(H, S, V, R, G, B);
    std::cout << "H: " << H << ", R: " << R << ", G: " << G << ", B: " << B << std::endl;
}

int main() {
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

    /*uint8_t curLed = 0, lastLed = 0;
    for(uint8_t i = 0; i < 50; ++i) {
        std::cout << "off: " << unsigned(lastLed) << "| on: " << unsigned(curLed) << std::endl;
        lastLed = curLed;
        if(!(++curLed % 15)) curLed++;
        if(curLed > 24) curLed = 0;
    }*/

    //testHSV2RGB();
    testHSV2RGB2();
    
    //bpmPeriodError();

    nextTick = 0;
    for(uint32_t tick = 0; tick < 200; ++tick) {
        outputTick(tick);
    }

    std::cout << uint32_t(120.f * 192.f * 1.f / (60 * 1000)) << std::endl;
}