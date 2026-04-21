#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct MarketMessage
{
    char msg_type;
    uint32_t seq_num;
    uint64_t order_id;
    char symbol[4];
    char side;
    uint64_t price;
    uint32_t qty;
};
#pragma pack(pop)
