#include "price_level.h"

std::list<Order *>::iterator PriceLevel::add_order(Order *order)
{
    orders_.push_back(order);
    total_qty_ += order->remaining_qty();
    return --orders_.end();
}

void PriceLevel::remove_order(std::list<Order *>::iterator it)
{
    Order *order = *it;
    total_qty_ -= order->remaining_qty();
    orders_.erase(it);
}

void PriceLevel::adjust_total_qty(Qty delta)
{
    total_qty_ += delta;
}