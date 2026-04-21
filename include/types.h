#pragma once
#include <cstdint>
#include <string>

static constexpr int64_t PRICE_SCALE = 100;

enum MessageType : char
{
    ADD = 'A',
    MODIFY = 'M',
    DEL = 'D'
};

enum class Side : uint8_t
{
    BUY = 0,
    SELL = 1
};

inline std::string side_str(Side s)
{
    return s == Side::BUY ? "BUY" : "SELL";
}
