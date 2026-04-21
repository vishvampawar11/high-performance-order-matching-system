#pragma once
#include <cstdint>
#include <unordered_map>
#include <map>

#include "order.h"
#include "price_level.h"

class LimitOrderBook
{
private:
    std::unordered_map<uint64_t, Order *> order_map_; // {order_id => Order}

    std::map<uint64_t, PriceLevel, std::greater<uint64_t>> bids_; // bids in descending order of price
    std::map<uint64_t, PriceLevel, std::less<uint64_t>> asks_;    // asks in ascending order of price

public:
    bool add_order(Order *Order);
    Order *delete_order(uint64_t order_id);

    const std::map<uint64_t, PriceLevel, std::greater<uint64_t>> &get_bids() const { return bids_; }
    const std::map<uint64_t, PriceLevel, std::less<uint64_t>> &get_asks() const { return asks_; }
};
