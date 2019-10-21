/*
 * @Description: Class Factory
 * @Author: ekibun
 * @Date: 2019-10-21 12:53:32
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-21 12:54:37
 */
#pragma once
#include <Unknwn.h>

class CClassFactory : public IClassFactory
{
public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG)
    AddRef();
    STDMETHODIMP_(ULONG)
    Release();

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown *, REFIID, void **);
    STDMETHODIMP LockServer(BOOL);

    CClassFactory();

protected:
    ~CClassFactory();

private:
    ULONG m_lRef;
};
