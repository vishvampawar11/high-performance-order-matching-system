#pragma once
#include <cstdint>
#include "types.h"

struct Trade
{
    TradeId trade_id;
    OrderId aggressor_id, passive_id;
    Side aggressor_side;
    Price exec_price;
    Price exec_qty;
};
