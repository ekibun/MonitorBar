/*
 * @Description: 
 * @Author: ekibun
 * @Date: 2019-10-21 21:14:41
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-22 00:12:22
 */
#include "DeskBand.h"
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#include "CpuMonitor.h"
#include "NvMonitor.h"
#include "DeskBandWindow.h"

extern CLSID CLSID_DeskBand;

extern HINSTANCE g_hInst;
extern ULONG g_lDllRef;

static const LPCTSTR sm_lpszClassName = L"MonitorBandWindowClass";

CDeskBand::CDeskBand()
    : m_lRef(1), m_hwnd(nullptr), m_hwndParent(nullptr), m_dwBandID(0), m_bCanCompositionEnabled(FALSE), m_bIsDirty(FALSE), m_pSite(nullptr), m_bHasFocus(FALSE), m_bIsRegisterClassed(false), m_hToolTip(nullptr), m_hMenu(nullptr)
{
    OutputDebugString(L"CDeskBand init\n");
    m_iMonitors[0] = new CpuMonitor();
    m_iMonitors[1] = new NvMonitor();
    for (auto &m : m_iMonitors)
        if (!(m->Init()))
        {
            delete m;
            m = nullptr;
        }
    InterlockedIncrement(&g_lDllRef);
}

CDeskBand::~CDeskBand()
{
    if (m_pSite)
        m_pSite->Release();
    for (auto &m : m_iMonitors)
        if (m)
        {
            delete m;
            m = nullptr;
        }
    if (m_bIsRegisterClassed)
        UnregisterClass(sm_lpszClassName, g_hInst);
    InterlockedDecrement(&g_lDllRef);
}

STDMETHODIMP CDeskBand::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;

    if (IsEqualIID(IID_IUnknown, riid) ||
        IsEqualIID(IID_IOleWindow, riid) ||
        IsEqualIID(IID_IDockingWindow, riid) ||
        IsEqualIID(IID_IDeskBand, riid) ||
        IsEqualIID(IID_IDeskBand2, riid))
    {
        *ppv = static_cast<IOleWindow *>(this);
    }
    else if (IsEqualIID(IID_IPersist, riid) ||
             IsEqualIID(IID_IPersistStream, riid))
    {
        *ppv = static_cast<IPersist *>(this);
    }
    else if (IsEqualIID(IID_IObjectWithSite, riid))
    {
        *ppv = static_cast<IObjectWithSite *>(this);
    }
    else if (IsEqualIID(IID_IInputObject, riid))
    {
        *ppv = static_cast<IInputObject *>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
        *ppv = NULL;
    }
    if (*ppv)
    {
        AddRef();
    }
    return hr;
}

STDMETHODIMP_(ULONG)
CDeskBand::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG)
CDeskBand::Release()
{
    auto l = InterlockedDecrement(&m_lRef);
    if (!l)
        delete this;
    return l;
}

STDMETHODIMP CDeskBand::GetWindow(HWND *phwnd)
{
    if (!phwnd)
        return E_INVALIDARG;
    *phwnd = m_hwnd;
    return S_OK;
}

STDMETHODIMP CDeskBand::ShowDW(BOOL fShow)
{
    if (m_hwnd)
        ::ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);
    return S_OK;
}

STDMETHODIMP CDeskBand::CloseDW(DWORD)
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    return S_OK;
}

STDMETHODIMP CDeskBand::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO *pdbi)
{
    if (!pdbi)
        return E_INVALIDARG;
    else
    {
        m_dwBandID = dwBandID;
        m_dwViewMode = dwViewMode;

        POINT pt;

        pt.x = dwViewMode == DBIF_VIEWMODE_NORMAL ? 70 : 35;
        pt.y = dwViewMode == DBIF_VIEWMODE_NORMAL ? 35 : 70;

        if (pdbi->dwMask & DBIM_MINSIZE)
        {
            pdbi->ptMinSize.x = pt.x;
            pdbi->ptMinSize.y = pt.y;
        }
        if (pdbi->dwMask & DBIM_MAXSIZE)
            pdbi->ptMaxSize.y = -1;
        if (pdbi->dwMask & DBIM_INTEGRAL)
            pdbi->ptIntegral.y = 1;

        if (pdbi->dwMask & DBIM_ACTUAL)
        {
            pdbi->ptActual.x = pt.x;
            pdbi->ptActual.y = pt.y;
        }
        if (pdbi->dwMask & DBIM_TITLE)
            pdbi->dwMask &= ~DBIM_TITLE;
        if (pdbi->dwMask & DBIM_MODEFLAGS)
            pdbi->dwModeFlags = DBIMF_NORMAL | DBIMF_VARIABLEHEIGHT;
        if (pdbi->dwMask & DBIM_BKCOLOR)
            pdbi->dwMask &= ~DBIM_BKCOLOR;
        return S_OK;
    }
}

STDMETHODIMP CDeskBand::CanRenderComposited(BOOL *pfCanRenderComposited)
{
    return DwmIsCompositionEnabled(pfCanRenderComposited);
}

STDMETHODIMP CDeskBand::SetCompositionState(BOOL fCompositionEnabled)
{
    m_bCanCompositionEnabled = fCompositionEnabled;
    InvalidateRect(m_hwnd, nullptr, TRUE);
    UpdateWindow(m_hwnd);
    return S_OK;
}

STDMETHODIMP CDeskBand::GetCompositionState(BOOL *pfCompositionEnabled)
{
    if (!pfCompositionEnabled)
        return E_INVALIDARG;
    *pfCompositionEnabled = m_bCanCompositionEnabled;
    return S_OK;
}

STDMETHODIMP CDeskBand::GetClassID(CLSID *pclsid)
{
    if (!pclsid)
        return E_INVALIDARG;
    *pclsid = CLSID_DeskBand;
    return S_OK;
}

STDMETHODIMP CDeskBand::IsDirty()
{
    return m_bIsDirty ? S_OK : S_FALSE;
}

STDMETHODIMP CDeskBand::Save(IStream *, BOOL fClearDirty)
{
    if (fClearDirty)
        m_bIsDirty = FALSE;
    return S_OK;
}

STDMETHODIMP CDeskBand::SetSite(IUnknown *pUnkSite)
{
    if (m_pSite)
        m_pSite->Release();
    HRESULT hr = S_OK;
    if (!pUnkSite)
        return hr;
    do
    {
        IOleWindow *pow;
        hr = pUnkSite->QueryInterface(IID_IOleWindow, reinterpret_cast<void **>(&pow));
        if (FAILED(hr))
            break;
        hr = pow->GetWindow(&m_hwndParent);
        if (FAILED(hr))
            break;
        WNDCLASSW wc = {0};
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hInstance = g_hInst;
        wc.lpfnWndProc = WndProc;
        wc.lpszClassName = sm_lpszClassName;
        wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 0));
        hr = RegisterClassW(&wc);
        if (FAILED(hr))
            break;
        m_bIsRegisterClassed = true;
        hr = E_FAIL;
        m_hwnd = CreateWindow(sm_lpszClassName, nullptr,
                              WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                              0, 0, 0, 0, m_hwndParent, nullptr, g_hInst, this);
        if (!m_hwnd)
            break;
        pow->Release();
        hr = pUnkSite->QueryInterface(IID_IInputObjectSite,
                                      reinterpret_cast<void **>(&m_pSite));
    } while (false);
    return hr;
}

STDMETHODIMP CDeskBand::GetSite(REFIID riid, void **ppv)
{
    if (m_pSite)
        return m_pSite->QueryInterface(riid, ppv);
    else if (!ppv)
        return E_INVALIDARG;
    else
    {
        *ppv = nullptr;
        return E_FAIL;
    }
}

STDMETHODIMP CDeskBand::UIActivateIO(BOOL fActivate, MSG *)
{
    if (fActivate)
        SetFocus(m_hwnd);
    return S_OK;
}

STDMETHODIMP CDeskBand::HasFocusIO()
{
    return m_bHasFocus ? S_OK : S_FALSE;
}
