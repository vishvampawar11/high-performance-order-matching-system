#include <vector>
#include "matching_engine.h"

std::vector<Trade> MatchingEngine::match_buy(LimitOrderBook &book, Order *agg)
{
    std::vector<Trade> trades;
    const auto &asks = book.get_asks();

    while (agg->remaining_qty() > 0 && !asks.empty())
    {
        auto level_it = asks.begin(); // cheapest ask first
        if (agg->price < level_it->first)
            break; // no match possible

        const PriceLevel &level = level_it->second;
        bool restart = false;
        while (agg->remaining_qty() > 0 && level.order_count() > 0)
        {
            Order *passive = level.orders().front();
            Qty fill_qty = std::min<Qty>(agg->remaining_qty(), passive->remaining_qty());
            agg->filled_qty += fill_qty;
            passive->filled_qty += fill_qty;

            agg->update_status();
            passive->update_status();
            trades.push_back(make_trade(agg, passive, fill_qty));

            if (passive->remaining_qty() == 0)
            {
                Order *deleted = book.delete_order(passive->order_id); // remove filled resting order
                delete deleted;
                restart = true;
                break;
            }
        }
        if (restart)
            continue;

        if (asks.empty())
            break;

        if (level.order_count() == 0)
            continue;

        break;
    }

    if (agg->remaining_qty() > 0) // unfilled remainder rests in book
    {
        agg->update_status();
        book.add_order(agg);
    }
    else
    {
        agg->update_status();
    }

    return trades;
}

std::vector<Trade> MatchingEngine::match_sell(LimitOrderBook &book, Order *agg)
{
    std::vector<Trade> trades;
    const auto &bids = book.get_bids();

    while (agg->remaining_qty() > 0 && !bids.empty())
    {
        auto level_it = bids.begin(); // best bid first
        if (agg->price > level_it->first)
            break; // no match possible

        const PriceLevel &level = level_it->second;
        bool restart = false;
        while (agg->remaining_qty() > 0 && level.order_count() > 0)
        {
            Order *passive = level.orders().front();
            Qty fill_qty = std::min<Qty>(agg->remaining_qty(), passive->remaining_qty());
            agg->filled_qty += fill_qty;
            passive->filled_qty += fill_qty;

            agg->update_status();
            passive->update_status();
            trades.push_back(make_trade(agg, passive, fill_qty));

            if (passive->remaining_qty() == 0)
            {
                Order *deleted = book.delete_order(passive->order_id);
                delete deleted;
                restart = true;
                break;
            }
        }
        if (restart)
            continue;

        if (bids.empty())
            break;

        if (level.order_count() == 0)
            continue;

        break;
    }

    if (agg->remaining_qty() > 0)
    {
        agg->update_status();
        book.add_order(agg);
    }
    else
    {
        agg->update_status();
    }

    return trades;
}

Trade MatchingEngine::make_trade(Order *agg, Order *passive, Qty qty)
{
    Trade t;
    t.trade_id = trade_seq_.fetch_add(1, std::memory_order_relaxed);
    t.aggressor_id = agg->order_id;
    t.passive_id = passive->order_id;
    t.aggressor_side = agg->side;
    t.exec_price = passive->price; // fills at passive (resting) price
    t.exec_qty = qty;
    return t;
}

std::vector<Trade> MatchingEngine::on_new_order(LimitOrderBook &book, Order *aggressor)
{
    return (aggressor->side == Side::BUY) ? match_buy(book, aggressor) : match_sell(book, aggressor);
}
