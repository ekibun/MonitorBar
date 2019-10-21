/*
 * @Description: Monitor Interface
 * @Author: ekibun
 * @Date: 2019-10-21 14:20:24
 * @LastEditors: ekibun
 * @LastEditTime: 2019-10-21 22:43:22
 */
#pragma once
#include <string>

class IMonitor
{
public:
    virtual ~IMonitor() {}
    virtual const std::wstring ToString() const = 0;
    virtual const std::wstring ToLongString() const = 0;
    virtual const double GetValue() const = 0;
    virtual bool Init() = 0;
    virtual void Update() = 0;
    virtual void Reset() = 0;
};