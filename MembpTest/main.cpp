#include <windows.h>
#include <stdio.h>

/*
#define PAGE_NOACCESS          0x01
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_WRITECOPY         0x08 <- not supported

#define PAGE_EXECUTE           0x10
#define PAGE_EXECUTE_READ      0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80 <- not supported

#define PAGE_GUARD            0x100 <- not supported with PAGE_NOACCESS
#define PAGE_NOCACHE          0x200 <- not supported with PAGE_GUARD or PAGE_WRITECOMBINE
#define PAGE_WRITECOMBINE     0x400 <- not supported with PAGE_GUARD or PAGE_NOCACHE
*/

static DWORD RemoveExecuteAccess(DWORD dwAccess)
{
    DWORD dwBase = dwAccess & 0xFF;
    DWORD dwHigh = dwAccess & 0xFFFFFF00;
    switch (dwBase)
    {
    case PAGE_EXECUTE:
        return dwHigh | PAGE_READONLY;
    case PAGE_EXECUTE_READ:
    case PAGE_EXECUTE_READWRITE:
    case PAGE_EXECUTE_WRITECOPY:
        return dwHigh | (dwBase >> 4);
    default:
        return dwAccess;
    }
}

static DWORD RemoveWriteAccess(DWORD dwAccess)
{
    DWORD dwBase = dwAccess & 0xFF;
    switch (dwBase)
    {
    case PAGE_READWRITE:
    case PAGE_EXECUTE_READWRITE:
        return (dwAccess & 0xFFFFFF00) | (dwBase >> 1);
    default:
        return dwAccess;
    }
}

int main()
{
    //GetProcessDEPPolicy

    DWORD lpFlags;
    BOOL bPermanent;
    if (!GetProcessDEPPolicy(GetCurrentProcess(), &lpFlags, &bPermanent))
        printf("GetProcessDEPPolicy failed (%08X)...\n", GetLastError());
    else
        printf("lpFlags: %d, bPermanent: %d\n", lpFlags, bPermanent);

    auto page = (unsigned char*)VirtualAlloc(nullptr, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    unsigned char code[] = { 0x90, 0x90, 0x90, 0xC3 };
    SIZE_T written;
    WriteProcessMemory(GetCurrentProcess(), page, code, sizeof(code), &written);

    //read
    printf("%02X\n", page[1]);

    //write
    page[1] = 0x66;

    //execute
    ((void(*)())page)();

    getchar();
    return 0;
}