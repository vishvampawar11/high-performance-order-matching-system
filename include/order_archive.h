#include <map>

#include "types.h"
#include "order.h"

struct ArchiveRecord
{
    OrderId order_id;
    Side side;
    Price price;
    Qty quantity;
    Qty filled_qty;
    Price fill_price;
    OrderStatus final_status;
};

class OrderArchive
{
private:
    OrderArchive() = default;

    std::map<OrderId, ArchiveRecord> filled_archive_;
    std::map<OrderId, ArchiveRecord> cancelled_archive_;

public:
    static OrderArchive &instance()
    {
        static OrderArchive inst;
        return inst;
    }

    void add_to_filled_archive(Order *order, Qty filled_qty, Price fill_price);
    void add_to_cancelled_archive(Order *order);
};
