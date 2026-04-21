#include "price_level.h"

void PriceLevel::append(Order *order)
{
    order->prev = tail_;
    order->next = nullptr;

    if (tail_)
        tail_->next = order;
    tail_ = order;
    if (!head_)
        head_ = order;

    total_qty_ += order->remaining_qty();
    ++order_count_;
}

void PriceLevel::remove(Order *order)
{
    if (order->prev)
        order->prev->next = order->next;
    if (order->next)
        order->next->prev = order->prev;
    if (head_ == order)
        head_ = order->next;
    if (tail_ == order)
        tail_ = order->prev;

    order->next = nullptr;
    order->prev = nullptr;

    total_qty_ -= order->remaining_qty();
    --order_count_;
}
