/*
 * @Description: Cpu Monitor with WinRing0
 * @Author: ekibun
 * @Date: 2019-10-21 15:15:16
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-21 22:43:43
 */
#include "IMonitor.h"
#include <Windows.h>

class CpuMonitor : public IMonitor
{
public:
    CpuMonitor();
    ~CpuMonitor();
    const std::wstring ToLongString() const;
    const std::wstring ToString() const;
    const double GetValue() const;
    bool Init();
    void Update();
    void Reset();

private:
    void InitProcessorInfo();

    HMODULE m_hOpenLibSys;
    DWORD TjMax = 0;
    DWORD m_dwCpuCoreCount;
    KAFFINITY *m_pMasks;
    DWORD m_dwCpuCount;
    PBYTE m_pTjMax;
    PBYTE m_pTemp;
    PBYTE m_pTempMin;
    PBYTE m_pTempMax;
};