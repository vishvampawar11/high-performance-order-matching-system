#include <cstdint>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "limit_order_book.h"
#include "market_data.h"
#include "types.h"
#include "logger.h"

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(15000);

    bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    u_long mode = 1;
    ioctlsocket(sockfd, FIONBIO, &mode);

    LimitOrderBook lob;
    char buffer[1024];

    while (true)
    {
        sockaddr_in cliaddr;
        int len = sizeof(cliaddr);

        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &len);

        if (n <= 0)
        {
            // no data
            continue;
        }

        if (n < sizeof(MarketMessage))
        {
            Logger::instance().log(LogLevel::ERR, "Received incomplete market message.");
            continue;
        }

        MarketMessage *msg = reinterpret_cast<MarketMessage *>(buffer);

        switch (msg->msg_type)
        {
        case MessageType::ADD:
        {
            Order *order = new Order(
                msg->order_id,
                msg->side == 'B' ? Side::BUY : Side::SELL,
                msg->price,
                msg->qty);

            if (lob.add_order(order))
            {
                Logger::instance().log(LogLevel::INFO, "ADD order_id=" + std::to_string(msg->order_id));
            }
            else
            {
                Logger::instance().log(LogLevel::ERR, "Failed ADD order_id=" + std::to_string(msg->order_id));
                delete order;
            }

            break;
        }

        case MessageType::DEL:
        {
            Order *deleted = lob.delete_order(msg->order_id);

            if (deleted)
            {
                Logger::instance().log(LogLevel::INFO, "DEL order_id=" + std::to_string(msg->order_id));

                delete deleted;
            }
            else
            {
                Logger::instance().log(LogLevel::ERR, "DEL failed order_id=" + std::to_string(msg->order_id));
            }

            break;
        }

        default:
            break;
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
