#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <vector>

#include "limit_order_book.h"
#include "trade_publisher.h"
#include "trade.h"

class MatchingEngine
{
private:
    std::vector<Trade> match_buy(LimitOrderBook &book, Order *agg);
    std::vector<Trade> match_sell(LimitOrderBook &book, Order *agg);
    Trade make_trade(Order *agg, Order *passive, Qty qty);

    TradePublisher &trade_publisher;
    std::atomic<uint64_t> trade_seq_{0};

public:
    MatchingEngine(TradePublisher &publisher) : trade_publisher(publisher) {}
    std::vector<Trade> on_new_order(LimitOrderBook &book, Order *aggressor);
};
