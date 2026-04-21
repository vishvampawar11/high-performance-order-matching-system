#pragma once
#include <cstdint>
#include "types.h"

struct Order
{
    uint64_t order_id = 0;
    Side side = Side::BUY;
    uint64_t price = 0;
    uint32_t qty = 0;
    uint32_t filled_qty = 0;

    Order *next = nullptr;
    Order *prev = nullptr;

    uint32_t remaining_qty() const noexcept { return qty - filled_qty; }

    double real_price() const noexcept { return static_cast<double>(price) / PRICE_SCALE; }

    Order(uint64_t id, Side s, uint64_t p, uint32_t q)
        : order_id(id), side(s), price(p), qty(q),
          filled_qty(0),
          next(nullptr), prev(nullptr) {}
};
