#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <mutex>

enum class LogLevel
{
    INFO,
    WARN,
    ERR,
    TRADE
};

class Logger
{
private:
    std::ofstream log_file;
    std::mutex log_mutex;

    Logger()
    {
        log_file.open("order_matching_system.log", std::ios::out | std::ios::app);
        if (!log_file.is_open())
        {
            std::cerr << "CRITICAL: Could not open log file!" << std::endl;
        }
    }

public:
    static Logger &instance()
    {
        static Logger logger_instance;
        return logger_instance;
    }

    void log(LogLevel level, const std::string &message)
    {
        std::lock_guard<std::mutex> lock(log_mutex);

        std::string label;
        switch (level)
        {
        case LogLevel::INFO:
            label = "[INFO] ";
            break;
        case LogLevel::WARN:
            label = "[WARN] ";
            break;
        case LogLevel::ERR:
            label = "[ERR] ";
            break;
        case LogLevel::TRADE:
            label = "[TRADE]";
            break;
        }

        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S")
           << "." << std::setfill('0') << std::setw(3) << ms.count()
           << " " << label << " " << message;

        if (log_file.is_open())
        {
            log_file << ss.str() << std::endl;
            log_file.flush();
        }
    }

    void log_incoming_message(const MarketMessage &msg)
    {
        std::string sym(msg.symbol, 8);
        sym.erase(sym.find_first_of('\0'));

        log(LogLevel::INFO, "Received msg [seq=" + std::to_string(msg.seq_num) +
                                "|type=" + std::string(1, msg.msg_type) +
                                "|side=" + std::string(1, msg.side) +
                                "|symbol=" + sym +
                                "|order_id=" + std::to_string(msg.order_id) +
                                "|price=" + std::to_string(msg.price) +
                                "|qty=" + std::to_string(msg.qty) + "]");
    }

    static void log_trade(const Trade &trade)
    {
        std::string message = "TRADE id" + std::to_string(trade.trade_id) +
                              " aggressor id=" + std::to_string(trade.aggressor_id) +
                              " passive id=" + std::to_string(trade.passive_id) +
                              " price=" + std::to_string(trade.exec_price) +
                              " qty=" + std::to_string(trade.exec_qty);

        Logger::instance().log(LogLevel::TRADE, message);
    }

    ~Logger()
    {
        if (log_file.is_open())
            log_file.close();
    }
};
