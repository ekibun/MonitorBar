/*
 * @Description: 
 * @Author: ekibun
 * @Date: 2019-10-21 22:16:34
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-22 00:29:45
 */
#include "DeskBand.h"
#include <sstream>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")
#include <gdiplus.h>
#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;

#define IDM_RESET 301
#define UPDATE_TIMER_ID 1

#define RECTWIDTH(x) ((x).right - (x).left)
#define RECTHEIGHT(x) ((x).bottom - (x).top)

extern HINSTANCE g_hInst;

HMENU CreateBandMenu()
{
    HMENU hmenu = CreatePopupMenu();
    AppendMenu(hmenu,
               MF_STRING,
               IDM_RESET,
               L"重置");
    return hmenu;
}

LRESULT CALLBACK CDeskBand::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDeskBand *pDeskBand = reinterpret_cast<CDeskBand *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_CREATE:
        pDeskBand = reinterpret_cast<CDeskBand *>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDeskBand));
        pDeskBand->OnCreate(hwnd);
        return pDeskBand->OnCreate(hwnd);
    case WM_SETFOCUS:
        return pDeskBand->OnFocus(TRUE);
    case WM_KILLFOCUS:
        return pDeskBand->OnFocus(FALSE);
    case WM_PAINT:
        return pDeskBand->OnPaint(hwnd);
    case WM_PRINTCLIENT:
        return pDeskBand->OnPaint(hwnd, reinterpret_cast<HDC>(wParam));
    case WM_ERASEBKGND:
        if (!pDeskBand->m_bCanCompositionEnabled)
            break;
        return 1;
    case WM_TIMER:
        if (wParam != UPDATE_TIMER_ID)
            break;
        {
            std::wostringstream ret;
            for (auto &m : pDeskBand->m_iMonitors)
                if (m)
                {
                    m->Update();
                    ret << m->ToLongString() << std::endl;
                }
            if (pDeskBand->m_hToolTip)
            {
                TOOLINFOW ti = {sizeof(TOOLINFOW), 0, pDeskBand->m_hwndParent, (UINT_PTR)pDeskBand->m_hwnd};
                ti.hinst = g_hInst;
                ti.lpszText = (LPWSTR)ret.str().c_str();
                SendMessageW(pDeskBand->m_hToolTip, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
            }
            pDeskBand->OnPaint(hwnd, reinterpret_cast<HDC>(wParam));
        }
    case WM_DESTROY:
        return pDeskBand->OnDestroy(hwnd);
    case WM_RBUTTONUP:
        return pDeskBand->OnRButtonUp(hwnd);
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_RESET:
            return pDeskBand->OnMenuReset();
        }
        break;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI MonitorThread(LPVOID lpParamter)
{
    HWND hwnd = (HWND)lpParamter;
    CDeskBand *pDeskBand = reinterpret_cast<CDeskBand *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    while (pDeskBand && IsWindow(hwnd))
    {
        std::wostringstream ret;
        for (auto &m : pDeskBand->m_iMonitors)
            if (m)
            {
                m->Update();
                ret << m->ToLongString() << std::endl;
            }
        if (pDeskBand->m_hToolTip)
        {
            TOOLINFOW ti = {sizeof(TOOLINFOW), 0, pDeskBand->m_hwndParent, (UINT_PTR)pDeskBand->m_hwnd};
            ti.hinst = g_hInst;
            ti.lpszText = (LPWSTR)ret.str().c_str();
            SendMessageW(pDeskBand->m_hToolTip, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
        }

        InvalidateRect((HWND)lpParamter, NULL, false);
        Sleep(1000);
    }
    return 0;
}

LRESULT CDeskBand::OnCreate(HWND hwnd)
{
    m_hwnd = hwnd;
    static ULONG_PTR gdiplusToken;
    if (!gdiplusToken)
    {
        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }

    m_hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr,
                                WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                hwnd, nullptr, g_hInst, nullptr);
    SetWindowPos(m_hToolTip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    TOOLINFO ti = {sizeof(TOOLINFO), TTF_SUBCLASS | TTF_IDISHWND, m_hwndParent, (UINT_PTR)hwnd};
    ti.hinst = g_hInst;
    ti.lpszText = TEXT("");
    GetClientRect(hwnd, &ti.rect);
    SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SendMessage(m_hToolTip, TTM_SETTITLE, TTI_NONE, (LPARAM)TEXT("详细信息"));
    SendMessage(m_hToolTip, TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
    m_hMenu = CreateBandMenu();
    //SetTimer(hwnd, UPDATE_TIMER_ID, 1000, nullptr);

    //Create Thread
    HANDLE hThread = CreateThread(NULL, 0, MonitorThread, hwnd, 0, NULL);
    CloseHandle(hThread);
    return 0;
}

void DrawString(Graphics &g, PWCHAR string, RectF rect, int size, StringAlignment sta)
{
    LOGFONTW lf;
    SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(LOGFONTW), &lf, 0);
    SolidBrush solidBrush(Color::White);
    FontFamily fontFamily(lf.lfFaceName);
    Font font(&fontFamily, (REAL)size, FontStyleRegular, UnitPixel);

    StringFormat sf(StringFormatFlags::StringFormatFlagsNoWrap);

    sf.SetAlignment(sta);
    sf.SetLineAlignment(StringAlignmentCenter);

    g.DrawString((WCHAR *)string, (INT)wcslen(string), &font, rect, &sf, &solidBrush);
}

LRESULT CDeskBand::OnPaint(HWND hwnd, HDC _hdc)
{
    HDC hdc = _hdc;
    PAINTSTRUCT paint;
    RECT &rc = paint.rcPaint;
    if (!_hdc)
        hdc = BeginPaint(hwnd, &paint);
    else
        GetClientRect(hwnd, &rc);
    if (hdc)
    {
        HTHEME hTheme = OpenThemeData(NULL, L"BUTTON");
        if (hTheme)
        {

            HDC hdcPaint = NULL;
            HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hdc, &rc, BPBF_TOPDOWNDIB, NULL, &hdcPaint);

            Rect rect = Rect(rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc));

            Graphics g(hdcPaint);

            g.Clear(Color::Transparent);

            std::wostringstream ret;
            for (auto &m : m_iMonitors)
                if (m)
                {
                    m->Update();
                    ret << m->ToString() << std::endl;
                }
            DrawString(g, (PWCHAR)ret.str().c_str(), RectF(0, 0, (REAL)rect.Width, (REAL)rect.Height), 10, StringAlignmentCenter);

            EndBufferedPaint(hBufferedPaint, TRUE);

            CloseThemeData(hTheme);
        }
    }
    if (!_hdc)
        EndPaint(hwnd, &paint);
    return 0;
}

LRESULT CDeskBand::OnFocus(BOOL fFocus)
{
    m_bHasFocus = fFocus;
    if (m_pSite)
        m_pSite->OnFocusChangeIS(dynamic_cast<IOleWindow *>(this), m_bHasFocus);
    return 0;
}

LRESULT CDeskBand::OnDestroy(HWND)
{
    if (m_hMenu)
    {
        DestroyMenu(m_hMenu);
        m_hMenu = nullptr;
    }
    return 0;
}

LRESULT CDeskBand::OnRButtonUp(HWND hwnd)
{
    if (m_hMenu)
    {
        POINT pt;
        GetCursorPos(&pt);
        RECT rc;
        GetClientRect(hwnd, &rc);
        TrackPopupMenu(m_hMenu, 0, pt.x, pt.y, 0, hwnd, &rc);
    }
    return 0;
}

LRESULT CDeskBand::OnMenuReset()
{
    for (auto &m : m_iMonitors)
        if (m)
            m->Reset();
    return 0;
}
