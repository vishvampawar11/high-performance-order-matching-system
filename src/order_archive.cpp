#include "order_archive.h"

void OrderArchive::add_to_filled_archive(Order *order, Qty filled_qty, Price fill_price)
{
    ArchiveRecord record;
    record.order_id = order->order_id;
    record.side = order->side;
    record.price = order->price;
    record.quantity = order->qty;
    record.filled_qty = filled_qty;
    record.fill_price = fill_price;
    record.final_status = OrderStatus::FILLED;

    filled_archive_[order->order_id] = record;
}

void OrderArchive::add_to_cancelled_archive(Order *order)
{
    ArchiveRecord record;
    record.order_id = order->order_id;
    record.side = order->side;
    record.price = order->price;
    record.quantity = order->qty;
    record.filled_qty = 0;
    record.fill_price = 0;
    record.final_status = OrderStatus::CANCELLED;

    cancelled_archive_[order->order_id] = record;
}
