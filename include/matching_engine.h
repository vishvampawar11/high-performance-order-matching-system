#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <vector>

#include "limit_order_book.h"
#include "trade.h"

class MatchingEngine
{
private:
    std::vector<Trade> match_buy(LimitOrderBook &book, Order *agg);
    std::vector<Trade> match_sell(LimitOrderBook &book, Order *agg);
    Trade make_trade(Order *agg, Order *passive, Qty qty);
    std::atomic<uint64_t> trade_seq_{0};

public:
    std::vector<Trade> on_new_order(LimitOrderBook &book, Order *aggressor);
};
