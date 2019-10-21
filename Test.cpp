/*
 * @Description: 
 * @Author: ekibun
 * @Date: 2019-10-21 09:21:57
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-22 00:19:13
 */

#include <stdio.h>
#include <Windows.h>
#include <Locale>
#include "CpuMonitor.h"

int main()
{
    setlocale(LC_ALL, "chs");
    CpuMonitor monitor = CpuMonitor();
    if (!monitor.Init())
        return -1;
    while (true)
    {
        monitor.Update();
        wprintf(L"%s", monitor.ToString().c_str());
        Sleep(1000);
    }
}