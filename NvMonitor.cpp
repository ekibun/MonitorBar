/*
 * @Description: Cpu Monitor with WinRing0
 * @Author: ekibun
 * @Date: 2019-10-21 15:15:08
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-22 00:30:56
 */
#include "NvMonitor.h"
#include <tchar.h>
#include <sstream>
#include <iomanip>
#pragma comment(lib, "nvapi/amd64/nvapi64.lib")

NvMonitor::NvMonitor()
{
}

NvMonitor::~NvMonitor()
{
}

bool NvMonitor::Init()
{
    NvAPI_Status ret = NvAPI_Initialize();
    char *step = "NvAPI_Initialize";
    NvU32 cnt;

    if (ret == NVAPI_OK)
    {
        step = "NvAPI_GetInterfaceVersionString";
        ret = NvAPI_GetInterfaceVersionString(m_gpuVer);
#ifdef _DEBUG
        printf("NVAPI Version: %s\n", m_gpuVer);
#endif
    }
    if (ret == NVAPI_OK)
    {
        step = "NvAPI_EnumPhysicalGPUs";
        ret = NvAPI_EnumPhysicalGPUs(&m_phys, &cnt);
    }
    if (ret == NVAPI_OK)
    {
        step = "NvAPI_GPU_GetFullName";
        ret = NvAPI_GPU_GetFullName(m_phys, m_gpuName);
#ifdef _DEBUG
        printf("NVAPI GPU Name: %s\n", m_gpuName);
#endif
    }
    if (ret == NVAPI_OK)
        Reset();

#ifdef _DEBUG
    if (ret != NVAPI_OK)
    {
        NvAPI_ShortString string;
        NvAPI_GetErrorMessage(ret, string);
        printf("Error %s: %s\n", step, string);
    }
#endif

    return ret == NVAPI_OK;
}

void NvMonitor::Update()
{
    if (!m_phys)
        return;

    NV_GPU_THERMAL_SETTINGS thermal;
    thermal.version = NV_GPU_THERMAL_SETTINGS_VER;
    NvAPI_Status ret = NvAPI_GPU_GetThermalSettings(m_phys, 0, &thermal);

    if (ret != NVAPI_OK)
    {
#ifdef _DEBUG
        NvAPI_ShortString string;
        NvAPI_GetErrorMessage(ret, string);
        printf("Error NvAPI_GPU_GetThermalSettings: %s\n", string);
#endif
        return;
    }
    else
    {
        m_pTemp = static_cast<unsigned>(thermal.sensor[0].currentTemp);
        m_pTempMin = min(m_pTempMin, m_pTemp);
        m_pTempMax = max(m_pTempMax, m_pTemp);
#ifdef _DEBUG
        printf("Temp: %d C\n", m_pTemp);
#endif
    }
}

void NvMonitor::Reset()
{
    if (!m_phys)
        return;

    NV_GPU_THERMAL_SETTINGS thermal;
    thermal.version = NV_GPU_THERMAL_SETTINGS_VER;
    NvAPI_Status ret = NvAPI_GPU_GetThermalSettings(m_phys, 0, &thermal);

    if (ret != NVAPI_OK)
    {
#ifdef _DEBUG
        NvAPI_ShortString string;
        NvAPI_GetErrorMessage(ret, string);
        printf("Error NvAPI_GPU_GetThermalSettings: %s\n", string);
#endif
        return;
    }
    else
    {
        m_pTemp = m_pTempMax = m_pTempMin = static_cast<unsigned>(thermal.sensor[0].currentTemp);
#ifdef _DEBUG
        printf("Temp: %d C\n", m_pTemp);
#endif
    }
}

const std::wstring NvMonitor::ToLongString() const
{
    std::wostringstream ret;
    ret << L"GPU\t" << m_pTemp << L"℃";
    ret << L"\t\t" << m_pTempMax << L"℃";
    ret << L"\t\t" << m_pTempMin << L"℃";
    ret << std::endl;
    return ret.str();
}

const std::wstring NvMonitor::ToString() const
{
    std::wostringstream ret;
    ret << std::setiosflags(std::ios::fixed) << std::setprecision(1);
    ret << L"GPU: " << GetValue() << L"℃";
    return ret.str();
}

const double NvMonitor::GetValue() const
{
    return m_pTemp;
}