/*
 * @Description: DLL Main
 * @Author: ekibun
 * @Date: 2019-10-21 12:37:18
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-21 23:56:01
 */
#include <Windows.h>
#include <Shlobj.h>
#include "ClassFactory.h"

// {60D53470-4142-4474-B2BC-6EA324C6439D}
CLSID CLSID_DeskBand = {0x60d53470, 0x4142, 0x4474, {0xb2, 0xbc, 0x6e, 0xa3, 0x24, 0xc6, 0x43, 0x9d}};
const wchar_t szAppName[] = L"监视器";

HINSTANCE g_hInst = NULL;
ULONG g_lDllRef = 0;

STDAPI_(BOOL)
DllMain(HINSTANCE hInstance, DWORD dwReason, void *)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if (!ppv)
        return E_INVALIDARG;
    if (!IsEqualCLSID(rclsid, CLSID_DeskBand))
        return CLASS_E_CLASSNOTAVAILABLE;
    CClassFactory *pClassFactory = new CClassFactory;
    if (!pClassFactory)
        return E_OUTOFMEMORY;
    HRESULT hr = pClassFactory->QueryInterface(riid, ppv);
    pClassFactory->Release();
    return hr;
}

STDAPI DllCanUnloadNow()
{
    return g_lDllRef ? S_FALSE : S_OK;
}

STDAPI DllRegisterServer()
{
    wchar_t szCLSID[] = L"CLSID\\",
            szInprocServer32[] = L"\\InprocServer32";
    wchar_t szSubKey[_countof(szCLSID) - 1 + 39 - 1 + _countof(szInprocServer32) - 1 + 1] = {0};
    wcscpy_s(szSubKey, szCLSID);
    StringFromGUID2(CLSID_DeskBand, szSubKey + _countof(szCLSID) - 1, 39);
    HKEY hKey;
    LSTATUS lRes;
    lRes = RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubKey, 0, nullptr,
                           REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey,
                           nullptr);
    if (ERROR_SUCCESS != lRes)
        return HRESULT_FROM_WIN32(lRes);
    lRes = RegSetValueExW(hKey, nullptr, 0, REG_SZ, (LPBYTE)szAppName, sizeof(szAppName));
    if (ERROR_SUCCESS != lRes)
    {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(lRes);
    }
    wcscpy_s(szSubKey + _countof(szCLSID) - 1 + 39 - 1, _countof(szInprocServer32), szInprocServer32);
    lRes = RegCreateKeyExW(HKEY_CLASSES_ROOT, szSubKey, 0, nullptr,
                           REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey,
                           nullptr);
    if (ERROR_SUCCESS != lRes)
        return HRESULT_FROM_WIN32(lRes);
    wchar_t szModule[MAX_PATH];
    if (!GetModuleFileNameW(g_hInst, szModule, _countof(szModule)))
    {
        if (GetLastError())
            return HRESULT_FROM_WIN32(GetLastError());
        else
            return E_FAIL;
    }
    lRes = RegSetValueExW(hKey, nullptr, 0, REG_SZ, (LPBYTE)szModule, (DWORD)(wcslen(szModule) * sizeof(wchar_t)));
    if (ERROR_SUCCESS != lRes)
    {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(lRes);
    }
    const wchar_t szModel[] = L"Apartment";
    lRes = RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, (LPBYTE)szModel, sizeof szModel);
    RegCloseKey(hKey);
    if (ERROR_SUCCESS != lRes)
        return HRESULT_FROM_WIN32(lRes);
    ICatRegister *pCatRegister;
    HRESULT hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, nullptr,
                                  CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCatRegister));
    if (FAILED(hr))
        return hr;
    CATID catid = CATID_DeskBand;
    hr = pCatRegister->RegisterClassImplCategories(CLSID_DeskBand, 1, &catid);
    pCatRegister->Release();
    return hr;
}

STDAPI DllUnregisterServer()
{
    wchar_t szCLSID[] = L"CLSID\\";
    wchar_t szSubKey[_countof(szCLSID) - 1 + 39 - 1 + 1] = {0};
    wcscpy_s(szSubKey, szCLSID);
    StringFromGUID2(CLSID_DeskBand, szSubKey + _countof(szCLSID) - 1, 39);
    return HRESULT_FROM_WIN32(RegDeleteTreeW(HKEY_CLASSES_ROOT, szSubKey));
}

STDAPI DllIsRegisterServer()
{
    wchar_t szCLSID[] = L"CLSID\\";
    wchar_t szSubKey[_countof(szCLSID) - 1 + 39 - 1 + 1] = {0};
    wcscpy_s(szSubKey, szCLSID);
    StringFromGUID2(CLSID_DeskBand, szSubKey + _countof(szCLSID) - 1, 39);
    HKEY hKey;
    LONG l = RegOpenKey(HKEY_CLASSES_ROOT, szSubKey, &hKey);
    if (!l)
    {
        RegCloseKey(hKey);
        return S_OK;
    }
    else if (2 == l)
        return S_FALSE;
    else
        return E_FAIL;
}

LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    LPCWPRETSTRUCT lpMsg = (LPCWPRETSTRUCT)lParam;
    if (HC_ACTION == nCode && lpMsg && WM_INITDIALOG == lpMsg->message && lpMsg->hwnd)
    {
        int titleLen = GetWindowTextLengthW(lpMsg->hwnd);
        wchar_t *lpStr = new wchar_t[titleLen + 1]();
        GetWindowTextW(lpMsg->hwnd, lpStr, titleLen + 1);
        if (!wcscmp(lpStr, szAppName))
        {
            HWND hD = FindWindowEx(lpMsg->hwnd, NULL, TEXT("DirectUIHWND"), NULL);
            if (hD)
            {
                for (HWND hc = FindWindowEx(hD, NULL, TEXT("CtrlNotifySink"), NULL);
                     hc; hc = FindWindowEx(hD, hc, TEXT("CtrlNotifySink"), NULL))
                {
                    HWND hb = FindWindowEx(hc, NULL, WC_BUTTON, NULL);
                    if (hb && GetWindowLongPtr(hb, GWL_STYLE) & BS_DEFPUSHBUTTON)
                    {
                        SendMessage(hc, WM_COMMAND, BN_CLICKED, (LPARAM)hb);
                        HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, TRUE, szAppName);
                        SetEvent(hEvent);
                        break;
                    }
                }
            }
        }
        delete[] lpStr;
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

STDAPI DllShowMonitorBar(BOOL fShowOrHide)
{
    ITrayDeskBand *pTrayDeskBand = NULL;
    CoInitialize(NULL);
    HRESULT hr = CoCreateInstance(CLSID_TrayDeskBand, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pTrayDeskBand));
    // Vista and higher operating system
    if (SUCCEEDED(hr))
    {
        if (TRUE == fShowOrHide)
        {
            hr = pTrayDeskBand->DeskBandRegistrationChanged();
            if (SUCCEEDED(hr))
            {
                int i = 5;
                for (hr = pTrayDeskBand->IsDeskBandShown(CLSID_DeskBand);
                     FAILED(hr) && i > 0;
                     --i, hr = pTrayDeskBand->IsDeskBandShown(CLSID_DeskBand))
                    Sleep(100);
                if (SUCCEEDED(hr) && (S_FALSE == hr))
                {
                    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, szAppName);
                    HHOOK hhk = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hInst, 0);
                    hr = pTrayDeskBand->ShowDeskBand(CLSID_DeskBand);
                    WaitForSingleObject(hEvent, 5000 /*INFINITE*/);
                    UnhookWindowsHookEx(hhk);
                    CloseHandle(hEvent);
                }
            }
        }
        else
        {
            hr = pTrayDeskBand->IsDeskBandShown(CLSID_DeskBand);
            if (SUCCEEDED(hr) && (S_OK == hr))
            {
                hr = pTrayDeskBand->HideDeskBand(CLSID_DeskBand);
            }
        }
        pTrayDeskBand->Release();
    }
    else
    {
        if (TRUE == fShowOrHide)
        {
            WCHAR *pBuf = new WCHAR[49];
            StringFromGUID2(CLSID_DeskBand, pBuf, 39);
            if (!GlobalFindAtom(pBuf))
                GlobalAddAtom(pBuf);
            // Beware! SHLoadInProc is not implemented under Vista and higher.
            hr = SHLoadInProc(CLSID_DeskBand);
        }
        else
        {
            IBandSite *spBandSite = nullptr;
            hr = CoCreateInstance(CLSID_TrayBandSiteService, NULL, CLSCTX_ALL, IID_PPV_ARGS(&spBandSite));
            if (SUCCEEDED(hr))
            {
                DWORD dwBandID = 0;
                const UINT nBandCount = spBandSite->EnumBands((UINT)-1, &dwBandID);

                for (UINT i = 0; i < nBandCount; ++i)
                {
                    spBandSite->EnumBands(i, &dwBandID);

                    IPersist *spPersist;
                    hr = spBandSite->GetBandObject(dwBandID, IID_PPV_ARGS(&spPersist));
                    if (SUCCEEDED(hr))
                    {
                        CLSID clsid = CLSID_NULL;
                        hr = spPersist->GetClassID(&clsid);
                        spPersist->Release();
                        if (SUCCEEDED(hr) && ::IsEqualCLSID(clsid, CLSID_DeskBand))
                        {
                            hr = spBandSite->RemoveBand(dwBandID);
                            break;
                        }
                    }
                }
                spBandSite->Release();
            }
        }
    }
    CoUninitialize();
    return hr;
}