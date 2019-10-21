/*
 * @Description: Cpu Monitor with WinRing0
 * @Author: ekibun
 * @Date: 2019-10-21 15:15:08
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-22 00:32:56
 */
#include "CpuMonitor.h"
#include <tchar.h>
#include "winring0/OlsApiInit.h"
#include <sstream>
#include <iomanip>

CpuMonitor::CpuMonitor()
{
}

CpuMonitor::~CpuMonitor()
{
    DeinitOpenLibSys(&m_hOpenLibSys);
}

void CpuMonitor::InitProcessorInfo()
{
    DWORD len;
    GetLogicalProcessorInformationEx(
        RelationProcessorCore,
        NULL,
        &len);

    PBYTE buffer = new BYTE[len];
    while (!GetLogicalProcessorInformationEx(
        RelationProcessorCore,
        reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer),
        &len))
        ;
    m_dwCpuCoreCount = 0;
    for (PBYTE bufferTmp = buffer;
         bufferTmp < buffer + len;
         bufferTmp += reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(bufferTmp)->Size)
        ++m_dwCpuCoreCount;
    m_pMasks = new KAFFINITY[m_dwCpuCoreCount];
    m_dwCpuCount = 0;
    size_t i = 0;
    for (PBYTE bufferTmp = buffer;
         bufferTmp < buffer + len;
         bufferTmp += reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(bufferTmp)->Size, ++i)
    {
        auto ptr = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(bufferTmp);
        if (ptr->Processor.GroupCount != 1)
            m_pMasks[i] = 0;
        else
        {
            m_pMasks[i] = ptr->Processor.GroupMask[0].Mask;
            for (auto mask = m_pMasks[i]; mask; mask &= mask - 1)
                ++m_dwCpuCount;
        }
    }
    delete[] buffer;
}

bool CpuMonitor::Init()
{
    BOOL ret = InitOpenLibSys(&m_hOpenLibSys);
    if (ret)
    {
        InitProcessorInfo();

        m_pTjMax = new BYTE[m_dwCpuCoreCount]();
        for (DWORD i = 0; i < m_dwCpuCoreCount; ++i)
        {
            DWORD eax, edx;
            RdmsrTx(0x1A2, &eax, &edx, m_pMasks[i]);
            m_pTjMax[i] = eax >> 16 & 0xFF;
        }
        Reset();
    }
    return ret;
}

void CpuMonitor::Update()
{
    if (!m_pTjMax || !m_pTemp || !m_pTempMin || !m_pTempMax)
        return;
    for (DWORD i = 0; i < m_dwCpuCoreCount; ++i)
    {
        DWORD eax, edx;
        RdmsrTx(0x19C, &eax, &edx, m_pMasks[i]);
        m_pTemp[i] = m_pTjMax[i] - (eax >> 16 & 0xFF);
        m_pTempMin[i] = min(m_pTempMin[i], m_pTemp[i]);
        m_pTempMax[i] = max(m_pTempMax[i], m_pTemp[i]);
    }
}

void CpuMonitor::Reset()
{
    if (!m_pTjMax)
        return;
    delete[] m_pTemp;
    delete[] m_pTempMin;
    delete[] m_pTempMax;

    m_pTemp = new BYTE[m_dwCpuCoreCount]();
    m_pTempMin = new BYTE[m_dwCpuCoreCount]();
    m_pTempMax = new BYTE[m_dwCpuCoreCount]();
    for (DWORD i = 0; i < m_dwCpuCoreCount; ++i)
    {
        DWORD eax, edx;
        RdmsrTx(0x19C, &eax, &edx, m_pMasks[i]);
        m_pTempMin[i] = m_pTempMax[i] = m_pTemp[i] = m_pTjMax[i] - (eax >> 16 & 0xFF);
    }
}

const std::wstring CpuMonitor::ToLongString() const
{
    std::wostringstream ret;
    ret << L"核心号\t当前温度\t最高温度\t最低温度" << std::endl;
    for (DWORD i = 0; i < m_dwCpuCoreCount; ++i)
    {
        ret << i << L"\t" << m_pTemp[i] << L"℃";
        ret << L"\t\t" << m_pTempMax[i] << L"℃";
        ret << L"\t\t" << m_pTempMin[i] << L"℃";
        ret << std::endl;
    }
    return ret.str();
}

const std::wstring CpuMonitor::ToString() const
{
    std::wostringstream ret;
    ret  << std::setiosflags(std::ios::fixed) << std::setprecision(1);
    ret << L"CPU: " << GetValue() << L"℃";
    return ret.str();
}

const double CpuMonitor::GetValue() const
{
    if (!m_dwCpuCoreCount || !m_pTemp)
        return 0.0;
    double sum = 0;
    for (DWORD i = 0; i < m_dwCpuCoreCount; ++i)
    {
        sum += m_pTemp[i];
    }
    return sum / m_dwCpuCoreCount;
}