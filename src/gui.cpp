#include "gui.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <d3d11.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <algorithm>
#include <vector>

static ID3D11Device *g_pd3dDevice = nullptr;
static ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain *g_pSwapChain = nullptr;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;

static bool CreateRenderTarget()
{
    ID3D11Texture2D *pBackBuffer = nullptr;
    if (FAILED(g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
        return false;
    if (FAILED(g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView)))
    {
        pBackBuffer->Release();
        return false;
    }
    pBackBuffer->Release();
    return true;
}

static void CleanupRenderTarget()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

bool InitImGui(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
                                             featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
                                             &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext)))
    {
        return false;
    }

    if (!CreateRenderTarget())
        return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    return true;
}

void ShutdownImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupRenderTarget();
    if (g_pSwapChain)
    {
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }
    if (g_pd3dDeviceContext)
    {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = nullptr;
    }
    if (g_pd3dDevice)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
}

LRESULT WINAPI ImGuiWndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

static float ToRealPrice(Price price)
{
    return static_cast<float>(price) / PRICE_SCALE;
}

void RenderImGui(const LimitOrderBook &book, UIState &state)
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(1360, 760), ImGuiCond_FirstUseEver);
    ImGui::Begin("Market Dashboard");

    ImGui::Text("High Performance Order Matching System");
    ImGui::Separator();

    ImGui::Columns(2, nullptr, true);

    ImGui::BeginChild("OrderBookPanel", ImVec2(0, 0), true);
    {
        ImGui::Text("Order Book");
        ImGui::Separator();

        Qty total_buy_qty = 0;
        Qty total_sell_qty = 0;
        std::vector<std::pair<float, Qty>> buy_levels;
        std::vector<std::pair<float, Qty>> sell_levels;

        int max_levels = 5;
        for (const auto &entry : book.get_bids())
        {
            if (static_cast<int>(buy_levels.size()) >= max_levels)
                break;
            buy_levels.emplace_back(ToRealPrice(entry.first), entry.second.total_qty());
            total_buy_qty += entry.second.total_qty();
        }
        for (const auto &entry : book.get_asks())
        {
            if (static_cast<int>(sell_levels.size()) >= max_levels)
                break;
            sell_levels.emplace_back(ToRealPrice(entry.first), entry.second.total_qty());
            total_sell_qty += entry.second.total_qty();
        }

        ImGui::Columns(2, "book_columns", true);
        ImGui::Text("Buy Side");
        ImGui::NextColumn();
        ImGui::Text("Sell Side");
        ImGui::NextColumn();
        ImGui::Separator();

        for (int i = 0; i < max_levels; ++i)
        {
            if (i < static_cast<int>(buy_levels.size()))
                ImGui::Text("%llu @ %.2f", buy_levels[i].second, buy_levels[i].first);
            else
                ImGui::Text(" ");
            ImGui::NextColumn();

            if (i < static_cast<int>(sell_levels.size()))
                ImGui::Text("%.2f @ %llu", sell_levels[i].first, sell_levels[i].second);
            else
                ImGui::Text(" ");
            ImGui::NextColumn();
        }

        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::Text("Total Buy Quantity: %llu", total_buy_qty);
        ImGui::SameLine();
        ImGui::Text("Total Sell Quantity: %llu", total_sell_qty);
        ImGui::Spacing();

        Price last_price = state.last_trade_price.load();
        if (last_price != 0)
            ImGui::Text("Last Traded Price: %.2f", ToRealPrice(last_price));
        else
            ImGui::Text("Last Traded Price: N/A");

        std::vector<float> history;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            history.assign(state.price_history.begin(), state.price_history.end());
        }

        if (!history.empty())
        {
            float min_value = history.front();
            float max_value = history.front();
            for (float value : history)
            {
                min_value = std::min(min_value, value);
                max_value = std::max(max_value, value);
            }
            if (min_value == max_value)
            {
                min_value -= 1.0f;
                max_value += 1.0f;
            }
            ImGui::PlotLines("Price Trend", history.data(), static_cast<int>(history.size()), 0, nullptr, min_value, max_value, ImVec2(0, 160));
        }
        else
        {
            ImGui::Text("No price data yet.");
        }
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    ImGui::BeginChild("TradeFeedPanel", ImVec2(0, 0), true);
    {
        ImGui::Text("Trade Feed");
        ImGui::Separator();

        std::vector<Trade> trades;
        {
            std::lock_guard<std::mutex> lock(state.mutex);
            trades.assign(state.last_trades.begin(), state.last_trades.end());
        }

        ImGui::Columns(5, "trade_columns", true);
        ImGui::Text("Trade");
        ImGui::NextColumn();
        ImGui::Text("Agg");
        ImGui::NextColumn();
        ImGui::Text("Pass");
        ImGui::NextColumn();
        ImGui::Text("Price");
        ImGui::NextColumn();
        ImGui::Text("Qty");
        ImGui::NextColumn();
        ImGui::Separator();

        for (const auto &trade : trades)
        {
            ImGui::Text("%llu", trade.trade_id);
            ImGui::NextColumn();
            ImGui::Text("%llu", trade.aggressor_id);
            ImGui::NextColumn();
            ImGui::Text("%llu", trade.passive_id);
            ImGui::NextColumn();
            ImGui::Text("%.2f", ToRealPrice(trade.exec_price));
            ImGui::NextColumn();
            ImGui::Text("%llu", trade.exec_qty);
            ImGui::NextColumn();
        }
    }
    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::End();

    ImGui::Render();
    const float clear_color[4] = {0.08f, 0.10f, 0.14f, 1.0f};
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(1, 0);
}
