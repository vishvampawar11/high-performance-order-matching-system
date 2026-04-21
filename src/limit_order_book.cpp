#include "limit_order_book.h"

bool LimitOrderBook::add_order(Order *order)
{
    if (order_map_.find(order->order_id) != order_map_.end())
        return false;

    order->prev = nullptr;
    order->next = nullptr;

    if (order->side == Side::BUY)
    {
        auto [it, _] = bids_.try_emplace(order->price, order->price);
        it->second.append(order);
    }
    else
    {
        auto [it, _] = asks_.try_emplace(order->price, order->price);
        it->second.append(order);
    }

    order_map_.emplace(order->order_id, order);
    return true;
}

Order *LimitOrderBook::delete_order(uint64_t order_id)
{
    auto it = order_map_.find(order_id);
    if (it == order_map_.end())
        return nullptr;

    Order *order = it->second;
    order_map_.erase(it);

    if (order->side == Side::BUY)
    {
        auto lvl_it = bids_.find(order->price);
        lvl_it->second.remove(order);
        if (lvl_it->second.empty())
            bids_.erase(lvl_it);
    }
    else
    {
        auto lvl_it = asks_.find(order->price);
        lvl_it->second.remove(order);
        if (lvl_it->second.empty())
            asks_.erase(lvl_it);
    }

    return order;
}
