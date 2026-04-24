#pragma once
#include <cstdint>
#include "types.h"

struct Order
{
    OrderId order_id = 0;
    Side side = Side::BUY;
    Price price = 0;
    Qty qty = 0;
    Qty filled_qty = 0;
    OrderStatus status = OrderStatus::ACTIVE;

    Qty remaining_qty() const noexcept { return qty - filled_qty; }

    double real_price() const noexcept { return static_cast<double>(price) / PRICE_SCALE; }

    void update_status()
    {
        if (remaining_qty() == 0)
            status = OrderStatus::FILLED;
        else if (filled_qty > 0)
            status = OrderStatus::PARTIAL_FILL;
        else
            status = OrderStatus::ACTIVE;
    }

    Order(OrderId id, Side s, Price p, Qty q) : order_id(id), side(s), price(p), qty(q), filled_qty(0) {}
};
