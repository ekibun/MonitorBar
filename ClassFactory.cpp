/*
 * @Description: Class Factory
 * @Author: ekibun
 * @Date: 2019-10-21 12:54:46
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-21 23:56:18
 */
#include "ClassFactory.h"
#include "DeskBand.h"

extern ULONG g_lDllRef;

CClassFactory::CClassFactory()
    : m_lRef(1)
{
    InterlockedIncrement(&g_lDllRef);
}

CClassFactory::~CClassFactory()
{
    InterlockedDecrement(&g_lDllRef);
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;

    if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_IClassFactory, riid))
    {
        *ppv = static_cast<IUnknown *>(this);
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
        *ppv = NULL;
    }
    return hr;
}

STDMETHODIMP_(ULONG)
CClassFactory::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG)
CClassFactory::Release()
{
    auto l = InterlockedDecrement(&m_lRef);
    if (!l)
        delete this;
    return l;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;
    CDeskBand *pDeskBand = new CDeskBand;
    if (!pDeskBand)
        return E_OUTOFMEMORY;
    HRESULT hr = pDeskBand->QueryInterface(riid, ppv);
    pDeskBand->Release();
    return hr;
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement(&g_lDllRef);
    else
        InterlockedDecrement(&g_lDllRef);
    return S_OK;
}