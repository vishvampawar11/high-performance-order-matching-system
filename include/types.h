#pragma once
#include <cstdint>
#include <string>

static constexpr int64_t PRICE_SCALE = 100;

using Price = int64_t;
using Qty = uint64_t;
using OrderId = uint64_t;
using TradeId = uint64_t;

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

inline Side parse_side(char side_char)
{
    return (side_char == 'B') ? Side::BUY : Side::SELL;
}

enum class OrderStatus : uint8_t
{
    ACTIVE,
    PARTIAL_FILL,
    FILLED,
    CANCELLED
};
