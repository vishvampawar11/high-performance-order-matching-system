#pragma once
#include <cstdint>

#include "order.h"

class PriceLevel
{
private:
    uint64_t price_;
    uint32_t total_qty_;
    size_t order_count_;
    Order *head_;
    Order *tail_;

public:
    explicit PriceLevel(int64_t price) : price_(price), total_qty_(0), order_count_(0), head_(nullptr), tail_(nullptr) {}

    void append(Order *order);
    void remove(Order *order);

    bool empty() const noexcept { return head_ == nullptr; }
    size_t order_count() const noexcept { return order_count_; }
    int64_t price() const noexcept { return price_; }
    uint64_t total_qty() const noexcept { return total_qty_; }
};
