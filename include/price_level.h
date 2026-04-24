#pragma once
#include <cstdint>
#include <list>

#include "order.h"

class PriceLevel
{
private:
    Price price_;
    Qty total_qty_;
    std::list<Order *> orders_;

public:
    explicit PriceLevel(Price price) : price_(price), total_qty_(0) {}

    size_t order_count() const noexcept { return orders_.size(); }
    Price price() const noexcept { return price_; }
    Qty total_qty() const noexcept { return total_qty_; }
    const std::list<Order *> &orders() const { return orders_; }

    std::list<Order *>::iterator add_order(Order *order);
    void remove_order(std::list<Order *>::iterator it);
    void adjust_total_qty(Qty delta);
};
