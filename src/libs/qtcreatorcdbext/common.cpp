// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "common.h"
#include "iinterfacepointer.h"
#include <sstream>

std::string winErrorMessage(unsigned long error)
{
    char *lpMsgBuf;
    const int len = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, 0, (LPSTR)&lpMsgBuf, 0, NULL);
    if (len) {
        const std::string rc(lpMsgBuf, len);
        LocalFree(lpMsgBuf);
        return rc;
    }
    std::ostringstream str;
    str << "Unknown error " << error;
    return str.str();
}

std::string winErrorMessage()
{
    return winErrorMessage(GetLastError());
}

std::string msgDebugEngineComResult(HRESULT hr)
{
    switch (hr) {
    case S_OK:
        return std::string("S_OK");
    case S_FALSE:
        return std::string("S_FALSE");
    case E_FAIL:
        break;
    case E_INVALIDARG:
        return std::string("E_INVALIDARG");
    case E_NOINTERFACE:
        return std::string("E_NOINTERFACE");
    case E_OUTOFMEMORY:
        return std::string("E_OUTOFMEMORY");
    case E_UNEXPECTED:
        return std::string("E_UNEXPECTED");
    case E_NOTIMPL:
        return std::string("E_NOTIMPL");
    }
    if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
        return std::string("ERROR_ACCESS_DENIED");
    if (hr == HRESULT_FROM_NT(STATUS_CONTROL_C_EXIT))
        return std::string("STATUS_CONTROL_C_EXIT");
    return std::string("E_FAIL ") + winErrorMessage(HRESULT_CODE(hr));
}

std::string msgDebugEngineComFailed(const char *func, HRESULT hr)
{
    std::string rc = func;
    rc += " failed: ";
    rc += msgDebugEngineComResult(hr);
    return rc;
}

ULONG currentThreadId(IDebugSystemObjects *sysObjects)
{
    ULONG id = 0;
    if (sysObjects->GetCurrentThreadId(&id) == S_OK)
        return id;
    return 0;
}

ULONG currentThreadId(CIDebugClient *client)
{
    IInterfacePointer<IDebugSystemObjects> sysObjects(client);
    if (sysObjects)
        return currentThreadId(sysObjects.data());
    return 0;
}

ULONG currentProcessId(IDebugSystemObjects *sysObjects)
{
    ULONG64 handle = 0;
    if (sysObjects->GetCurrentProcessHandle(&handle) == S_OK)
        return GetProcessId((HANDLE)handle);
    return 0;
}

ULONG currentProcessId(CIDebugClient *client)
{
    IInterfacePointer<IDebugSystemObjects> sysObjects(client);
    if (sysObjects)
        return currentProcessId(sysObjects.data());
    return 0;
}

std::string moduleNameByOffset(CIDebugSymbols *symbols, ULONG64 offset)
{
    enum { BufSize = 512 };
    ULONG index = 0;
    ULONG64 base = 0;
    // Convert module base address to module index
    HRESULT hr = symbols->GetModuleByOffset(offset, 0, &index, &base);
    if (FAILED(hr))
        return std::string();
    // Obtain module name
    char buf[BufSize];
    buf[0] = '\0';
    hr = symbols->GetModuleNameString(DEBUG_MODNAME_MODULE, index, base, buf, BufSize, 0);
    if (FAILED(hr))
        return std::string();
    return std::string(buf);
}

std::string sourceFileNameByOffset(CIDebugSymbols *symbols, ULONG64 offset, PULONG lineNumber)
{
    enum { BufSize = 512 };
    char buf[BufSize];
    buf[0] = '\0';
    HRESULT hr = symbols->GetLineByOffset(offset, lineNumber, buf, BufSize, NULL, NULL);
    if (FAILED(hr))
        return std::string();
    return std::string(buf);
}
