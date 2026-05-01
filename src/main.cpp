#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <string>
#include <deque>
#include <mutex>

#include "limit_order_book.h"
#include "matching_engine.h"
#include "market_data.h"
#include "order_archive.h"
#include "spsc_queue.h"
#include "logger.h"
#include "types.h"
#include "trade_publisher.h"
#include "postgres_writer.h"
#include "gui.h"

static constexpr size_t kQueueSize = 1024;
static SPSCQueue<MarketMessage, kQueueSize> message_queue;
static std::atomic<bool> running{true};

static void process_message(const MarketMessage &msg, LimitOrderBook &book, MatchingEngine &engine,
                            TradePublisher &trade_pub, UIState &ui_state)
{
    Logger::instance().log_incoming_message(msg);

    switch (msg.msg_type)
    {
    case MessageType::ADD:
    {
        if (msg.side != 'B' && msg.side != 'S')
        {
            Logger::instance().log(LogLevel::ERR, "Invalid side for ADD order_id=" + std::to_string(msg.order_id));
            return;
        }

        Order *order = new Order(msg.order_id, parse_side(msg.side), msg.price, msg.qty);
        auto trades = engine.on_new_order(book, order);

        for (auto &trade : trades)
        {
            trade_pub.push(trade);
            {
                std::scoped_lock lock(ui_state.mutex);
                ui_state.last_trades.push_front(trade);
                if (ui_state.last_trades.size() > 20)
                    ui_state.last_trades.pop_back();

                ui_state.price_history.push_front(static_cast<float>(trade.exec_price) / PRICE_SCALE);
                if (ui_state.price_history.size() > 60)
                    ui_state.price_history.pop_back();
            }
            ui_state.last_trade_price.store(trade.exec_price);
            Logger::instance().log_trade(trade);
        }

        if (order->remaining_qty() == 0)
        {
            Logger::instance().log(LogLevel::INFO,
                                   "ADD filled order_id=" + std::to_string(order->order_id) + " price=" + std::to_string(order->real_price()));

            OrderArchive::instance().add_to_filled_archive(order, order->filled_qty, order->price);

            delete order;
        }
        else
        {
            Logger::instance().log(LogLevel::INFO,
                                   "ADD queued order_id=" + std::to_string(order->order_id) + " price=" + std::to_string(order->real_price()) +
                                       " remaining qty=" + std::to_string(order->remaining_qty()));
        }
        break;
    }

    case MessageType::DEL:
    {
        Order *deleted = book.delete_order(msg.order_id);
        if (deleted)
        {
            Logger::instance().log(LogLevel::INFO, "DEL order_id=" + std::to_string(msg.order_id));
            OrderArchive::instance().add_to_cancelled_archive(deleted);
            delete deleted;
        }
        else
        {
            Logger::instance().log(LogLevel::ERR, "DEL failed order_id=" + std::to_string(msg.order_id));
        }
        break;
    }

    case MessageType::MODIFY:
    {
        if (!book.edit_order(msg.order_id, msg.qty))
        {
            Logger::instance().log(LogLevel::ERR, "MODIFY failed order_id=" + std::to_string(msg.order_id));
        }
        else
        {
            Logger::instance().log(LogLevel::INFO, "MODIFY order_id=" + std::to_string(msg.order_id) +
                                                       " qty=" + std::to_string(msg.qty));
        }
        break;
    }

    default:
        Logger::instance().log(LogLevel::WARN, "Unknown message type: " + std::string(1, msg.msg_type));
        break;
    }
}

static void consumer_thread(LimitOrderBook &book, MatchingEngine &engine, TradePublisher &trade_pub, UIState &ui_state)
{
    while (running.load())
    {
        MarketMessage msg;
        if (message_queue.pop(msg))
        {
            process_message(msg, book, engine, trade_pub, ui_state);
            continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET)
    {
        Logger::instance().log(LogLevel::ERR, "Failed to create UDP socket.");
        WSACleanup();
        return 1;
    }

    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(15000);

    if (bind(sockfd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr)) == SOCKET_ERROR)
    {
        Logger::instance().log(LogLevel::ERR, "Failed to bind UDP socket.");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    u_long mode = 1;
    ioctlsocket(sockfd, FIONBIO, &mode);

    TradePublisher trade_publisher;
    const char *conninfo = std::getenv("PG_CONN");
    PostgresWriter postgres_writer(trade_publisher, conninfo);
    postgres_writer.start();

    LimitOrderBook book;
    MatchingEngine engine(trade_publisher);
    UIState ui_state;
    std::thread consumer(consumer_thread, std::ref(book), std::ref(engine), std::ref(trade_publisher), std::ref(ui_state));

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ImGuiWndProcHandler;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"OrderMatchingSystemWindow";
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Order Matching System", WS_OVERLAPPEDWINDOW,
                              100, 100, 1400, 820, NULL, NULL, wc.hInstance, NULL);

    if (!hwnd)
    {
        Logger::instance().log(LogLevel::ERR, "Failed to create application window.");
        running.store(false);
        if (consumer.joinable())
            consumer.join();
        postgres_writer.stop();
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    if (!InitImGui(hwnd))
    {
        Logger::instance().log(LogLevel::ERR, "Failed to initialize ImGui.");
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        running.store(false);
        if (consumer.joinable())
            consumer.join();
        postgres_writer.stop();
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    char buffer[1024];
    sockaddr_in cliaddr{};
    int addr_len = sizeof(cliaddr);
    bool quit = false;

    while (!quit && running.load())
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                quit = true;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        int n = recvfrom(sockfd, buffer, static_cast<int>(sizeof(buffer)), 0, reinterpret_cast<sockaddr *>(&cliaddr), &addr_len);
        if (n == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS)
            {
                Logger::instance().log(LogLevel::ERR, "recvfrom failed with error " + std::to_string(error));
                break;
            }
        }
        else
        {
            if (n < static_cast<int>(sizeof(MarketMessage)))
            {
                Logger::instance().log(LogLevel::ERR, "Received incomplete market message.");
            }
            else
            {
                MarketMessage msg;
                std::memcpy(&msg, buffer, sizeof(MarketMessage));
                if (!message_queue.push(msg))
                {
                    Logger::instance().log(LogLevel::WARN, "Message queue full, dropping msg seq=" + std::to_string(msg.order_id));
                }
            }
        }

        RenderImGui(book, ui_state);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    running.store(false);
    if (consumer.joinable())
        consumer.join();

    ShutdownImGui();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    closesocket(sockfd);
    WSACleanup();

    // Stop PostgresWriter thread
    postgres_writer.stop();

    return 0;
}
