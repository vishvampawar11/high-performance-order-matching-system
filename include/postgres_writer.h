#pragma once
#include <pqxx/pqxx>
#include <atomic>
#include <thread>
#include "trade_publisher.h"
#include "logger.h"

class PostgresWriter
{
public:
    PostgresWriter(TradePublisher &pub, const std::string &db_params) : pub(pub), conn(db_params), stop_flag(false) {}

    ~PostgresWriter()
    {
        stop();
    }

    void start()
    {
        writer_thread = std::thread(&PostgresWriter::run, this);
    }

    void stop()
    {
        stop_flag.store(true);
        if (writer_thread.joinable())
        {
            writer_thread.join();
        }
    }

private:
    void run()
    {
        while (!stop_flag.load())
        {
            Trade t;
            if (pub.pop(t)) // Check if a trade is available
            {
                try
                {
                    pqxx::work txn(conn);
                    txn.exec_params(
                        "INSERT INTO trades (aggressor_id, passive_id, price, quantity) "
                        "VALUES ($1, $2, $3, $4)",
                        t.aggressor_id,
                        t.passive_id,
                        t.exec_price,
                        t.exec_qty);
                    txn.commit();
                }
                catch (const std::exception &e)
                {
                    Logger::instance().log(LogLevel::ERR, e.what());
                }
            }
        }
    }

    TradePublisher &pub;
    pqxx::connection conn;
    std::atomic<bool> stop_flag;
    std::thread writer_thread;
};
