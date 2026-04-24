#include <map>
#include <list>
#include "types.h"
#include "price_level.h"

struct OrderLocation
{
    Side side;
    std::map<Price, PriceLevel>::iterator level_it;
    std::list<Order *>::iterator order_it;
};
