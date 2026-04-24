#pragma once
#include <cstdint>
#include <unordered_map>
#include <map>

#include "order.h"
#include "order_location.h"

class LimitOrderBook
{
public:
    using BidMap = std::map<Price, PriceLevel, std::greater<Price>>; // bids in descending order of price
    using AskMap = std::map<Price, PriceLevel>;                      // asks in ascending order of price

    void add_order(Order *Order);
    Order *delete_order(OrderId order_id);
    bool edit_order(OrderId order_id, Qty new_qty);

    const BidMap &get_bids() const { return bids_; }
    const AskMap &get_asks() const { return asks_; }

private:
    std::unordered_map<OrderId, OrderLocation> order_map_;

    BidMap bids_;
    AskMap asks_;
};
