#include "limit_order_book.h"

void LimitOrderBook::add_order(Order *order)
{
    if (order->side == Side::BUY)
    {
        BidMap::iterator it = bids_.find(order->price);
        if (it == bids_.end())
            it = bids_.emplace(order->price, PriceLevel(order->price)).first;
        std::list<Order *>::iterator order_it = it->second.add_order(order);
        OrderLocation loc{order->side, it, order_it};
        order_map_[order->order_id] = loc;
    }
    else
    {
        auto it = asks_.find(order->price);
        if (it == asks_.end())
            it = asks_.emplace(order->price, PriceLevel(order->price)).first;
        auto order_it = it->second.add_order(order);
        OrderLocation loc{order->side, it, order_it};
        order_map_[order->order_id] = loc;
    }
}

Order *LimitOrderBook::delete_order(uint64_t order_id)
{
    auto map_it = order_map_.find(order_id);
    if (map_it == order_map_.end())
        return nullptr;
    OrderLocation &loc = map_it->second;
    Order *order = *loc.order_it;
    if (loc.side == Side::BUY)
    {
        auto level_it = loc.level_it;
        level_it->second.remove_order(loc.order_it);
        if (level_it->second.order_count() == 0)
            bids_.erase(level_it);
    }
    else
    {
        auto level_it = loc.level_it;
        level_it->second.remove_order(loc.order_it);
        if (level_it->second.order_count() == 0)
            asks_.erase(level_it);
    }
    order_map_.erase(map_it);
    return order;
}

bool LimitOrderBook::edit_order(uint64_t id, uint64_t new_qty)
{
    auto map_it = order_map_.find(id);
    if (map_it == order_map_.end())
        return false;

    OrderLocation &loc = map_it->second;
    Order *order = *loc.order_it;

    if (new_qty < order->filled_qty)
        return false;

    uint32_t old_remaining = order->remaining_qty();
    uint32_t new_remaining = new_qty - order->filled_qty;
    if (new_qty <= order->qty)
    {
        // Reducing or same qty, preserve priority
        order->qty = new_qty;
        int32_t delta = new_remaining - old_remaining;
        loc.level_it->second.adjust_total_qty(delta);
    }
    else
    {
        // Increasing qty, lose priority
        if (loc.side == Side::BUY)
        {
            auto level_it = loc.level_it;
            level_it->second.remove_order(loc.order_it);
            if (level_it->second.order_count() == 0)
                bids_.erase(level_it);
            order->qty = new_qty;
            auto it = bids_.find(order->price);
            if (it == bids_.end())
                it = bids_.emplace(order->price, PriceLevel(order->price)).first;
            auto order_it = it->second.add_order(order);
            loc.level_it = it;
            loc.order_it = order_it;
        }
        else
        {
            auto level_it = loc.level_it;
            level_it->second.remove_order(loc.order_it);
            if (level_it->second.order_count() == 0)
                asks_.erase(level_it);
            order->qty = new_qty;
            auto it = asks_.find(order->price);
            if (it == asks_.end())
                it = asks_.emplace(order->price, PriceLevel(order->price)).first;
            auto order_it = it->second.add_order(order);
            loc.level_it = it;
            loc.order_it = order_it;
        }
    }
    return true;
}
