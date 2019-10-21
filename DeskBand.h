/*
 * @Description: DeskBand
 * @Author: ekibun
 * @Date: 2019-10-21 21:14:47
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-21 23:10:22
 */
#pragma once
#include <Windows.h>
#include <ShlObj.h>
#include "IMonitor.h"

class CDeskBand : public IDeskBand2,
                  public IPersistStream,
                  public IObjectWithSite,
                  public IInputObject
{
public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG)
    AddRef();
    STDMETHODIMP_(ULONG)
    Release();

    // IOleWindow
    STDMETHODIMP GetWindow(HWND *);
    STDMETHODIMP ContextSensitiveHelp(BOOL) { return E_NOTIMPL; }

    // IDockingWindow
    STDMETHODIMP ShowDW(BOOL);
    STDMETHODIMP CloseDW(DWORD);
    STDMETHODIMP ResizeBorderDW(const RECT *, IUnknown *, BOOL) { return E_NOTIMPL; }

    // IDeskBand (needed for all deskbands)
    STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD, DESKBANDINFO *);

    // IDeskBand2 (needed for glass deskband)
    STDMETHODIMP CanRenderComposited(BOOL *);
    STDMETHODIMP SetCompositionState(BOOL);
    STDMETHODIMP GetCompositionState(BOOL *);

    // IPersist
    STDMETHODIMP GetClassID(CLSID *);

    // IPersistStream
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load(IStream *) { return S_OK; }
    STDMETHODIMP Save(IStream *, BOOL);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER *) { return E_NOTIMPL; }

    // IObjectWithSite
    STDMETHODIMP SetSite(IUnknown *);
    STDMETHODIMP GetSite(REFIID, void **);

    // IInputObject
    STDMETHODIMP UIActivateIO(BOOL, MSG *);
    STDMETHODIMP HasFocusIO();
    STDMETHODIMP TranslateAcceleratorIO(MSG *) { return S_FALSE; }

    CDeskBand();

    IMonitor *m_iMonitors[2]; // Monitors

    ~CDeskBand();

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT OnCreate(HWND);
    LRESULT OnPaint(HWND, HDC = nullptr);
    LRESULT OnFocus(BOOL);
    LRESULT OnDestroy(HWND);
    LRESULT OnRButtonUp(HWND);
    LRESULT OnMenuReset();

    LONG m_lRef;                   // ref count of deskband
    IInputObjectSite *m_pSite;     // parent site that contains deskband
    BOOL m_bHasFocus;              // whether deskband window currently has focus
    BOOL m_bIsDirty;               // whether deskband setting has changed
    BOOL m_bCanCompositionEnabled; // whether glass is currently enabled in deskband
    DWORD m_dwBandID;              // ID of deskband
    DWORD m_dwViewMode;            // View Mode of deskband
    HWND m_hwnd;                   // main window of deskband
    HWND m_hwndParent;             // parent window of deskband

    BOOL m_bIsRegisterClassed; // wheter registered Window Class
    HWND m_hToolTip;           // handle of tool tip
    HMENU m_hMenu;             // handle of menu
};
