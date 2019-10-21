/*
 * @Description: Cpu Monitor with WinRing0
 * @Author: ekibun
 * @Date: 2019-10-21 15:15:16
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-21 23:26:13
 */
#include "IMonitor.h"
#include "nvapi/nvapi.h"
#include <Windows.h>

class NvMonitor : public IMonitor
{
public:
    NvMonitor();
    ~NvMonitor();
    const std::wstring ToLongString() const;
    const std::wstring ToString() const;
    const double GetValue() const;
    bool Init();
    void Update();
    void Reset();

private:
    void InitProcessorInfo();

    NvPhysicalGpuHandle m_phys;
    NvAPI_ShortString m_gpuVer;
    NvAPI_ShortString m_gpuName;
    int m_pTemp;
    int m_pTempMin;
    int m_pTempMax;
};