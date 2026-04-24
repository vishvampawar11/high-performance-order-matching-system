struct alignas(64) MarketMessage
{
    uint8_t msg_type;
    uint8_t side;
    uint16_t _p0 = 0;
    uint32_t seq_num = 0;
    uint64_t order_id;
    char symbol[8];
    uint64_t price;
    uint64_t qty;
    uint8_t _p1[24] = {};
};
static_assert(sizeof(MarketMessage) == 64);
