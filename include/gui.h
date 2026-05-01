#pragma once

#include <atomic>
#include <deque>
#include <mutex>
#include <Windows.h>

#include "limit_order_book.h"
#include "trade.h"
#include "types.h"

struct UIState
{
    std::deque<Trade> last_trades;
    std::deque<float> price_history;
    std::atomic<Price> last_trade_price{0};
    std::mutex mutex;
};

bool InitImGui(HWND hwnd);
void RenderImGui(const LimitOrderBook &book, UIState &state);
void ShutdownImGui();
LRESULT WINAPI ImGuiWndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
