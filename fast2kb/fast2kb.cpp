#include "stdafx.h"
#include "keymap_config.h"



#define VERSION_DESCRIPTION "2021.04.04.00 \"Easter\""

#ifdef _WIN64
#define ARCHITECTURE_DESCRIPTION "x64"
#else
#define ARCHITECTURE_DESCRIPTION "x86"
#endif

#ifdef _DEBUG
#define PROFILE_DESCRIPTION "Debug"
#else
#define PROFILE_DESCRIPTION "Release"
#endif


#define STATUS_SUCCESS 0


#define POLLING_INTERVAL 2ul

#define POLLING_INTERVAL_RESOLUTION 5000ul


#define JVS_FRIENDLY

#ifdef JVS_FRIENDLY
#define JVS_PORT L"COM2"

#define JVS_HEARTBEAT_INTERVAL 60000ul
#define JVS_HEARTBEAT_INTERVAL_RESOLUTION 30000ul
#endif


//#define VERBOSE

#ifdef VERBOSE
#define LOG_VERBOSE(...) (fprintf_s(stdout, __VA_ARGS__))
#define LOG_LEVEL_DESCRIPTION "Verbose"
#else
#define LOG_VERBOSE(...)
#define LOG_LEVEL_DESCRIPTION "Default"
#endif

#define LOG_INFO(...) (fprintf_s(stdout, __VA_ARGS__))
#define LOG_ERROR(...) (fprintf_s(stderr, __VA_ARGS__))


#define CHECK_BINDING(binding, bit, name) \
    if (binding.defined && (xorButtonsData & bit)) \
    { \
        if (buttonsData & bit) \
        { \
            LOG_VERBOSE("Pressed: " name "\n"); \
            diEvent.ki.dwFlags = binding.flags; \
        } \
        else \
        { \
            LOG_VERBOSE("Released: " name "\n"); \
            diEvent.ki.dwFlags = binding.flags | KEYEVENTF_KEYUP; \
        } \
        diEvent.ki.wScan = binding.scanCode; \
        SendInput(1, &diEvent, sizeof(INPUT)); \
    }

#define CHECK_ESCAPE(binding, bitStart, bitBtn1, bitBtn3, name, startBit, btn1Bit, btn3Bit) \
    if (binding.defined && (xorButtonsData & startBit || xorButtonsData & btn1Bit || xorButtonsData & btn3Bit)) \
    { \
        if (buttonsData & startBit && buttonsData & btn1Bit && buttonsData & btn3Bit) \
        { \
            LOG_VERBOSE("Pressed: " name "\n"); \
            diEvent.ki.dwFlags = binding.flags; \
            diEvent.ki.wScan = binding.scanCode; \
            SendInput(1, &diEvent, sizeof(INPUT)); \
        } \
        else if (prevButtonsData & startBit && prevButtonsData & btn1Bit && prevButtonsData & btn3Bit) \
        { \
            LOG_VERBOSE("Released: " name "\n"); \
            diEvent.ki.dwFlags = binding.flags | KEYEVENTF_KEYUP; \
            diEvent.ki.wScan = binding.scanCode; \
            SendInput(1, &diEvent, sizeof(INPUT)); \
        } \
    }


typedef NTSTATUS(__stdcall *ntQueryTimerRes)(PULONG MinimumResolution,
    PULONG MaximumResolution, PULONG CurrentResolution);
typedef NTSTATUS(__stdcall *ntSetTimerRes)(ULONG DesiredResolution,
    BOOLEAN SetResolution, PULONG CurrentResolution);

HMODULE ntdll;

ntQueryTimerRes NtQueryTimerResolution;
ntSetTimerRes NtSetTimerResolution;


typedef int(*dmacOpen)(int, LPVOID, LPVOID);
typedef int(*dmacRead)(int, DWORD, LPVOID, LPVOID);
typedef int(*dmacWrite)(int, DWORD, int, LPVOID);
typedef int(*dmacClose)(int, LPVOID);

HMODULE iDmacDrv;

dmacOpen iDmacDrvOpen;
dmacRead iDmacDrvRegisterRead;
dmacWrite iDmacDrvRegisterWrite;
dmacClose iDmacDrvClose;


int deviceIndex = 1;
int deviceId;
int buttonsAddressP1P2;
int buttonsAddressP3P4;

HANDLE hMainThread;
BOOL keepPolling = TRUE;



BOOL loadNtdll()
{
    ntdll = LoadLibrary(TEXT("ntdll.dll"));
    if (ntdll == NULL)
    {
        LOG_ERROR("Failed to load ntdll.dll. Error code: %d\n", GetLastError());
        return FALSE;
    }

    NtQueryTimerResolution = (ntQueryTimerRes)GetProcAddress(
        ntdll, "NtQueryTimerResolution");
    if (NtQueryTimerResolution == NULL)
    {
        LOG_ERROR("Failed to get address of NtQueryTimerResolution. Error code: %d\n", GetLastError());
        return FALSE;
    }

    NtSetTimerResolution = (ntSetTimerRes)GetProcAddress(
        ntdll, "NtSetTimerResolution");
    if (NtSetTimerResolution == NULL)
    {
        LOG_ERROR("Failed to get address of NtSetTimerResolution. Error code: %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}


BOOL loadIDmacDrv()
{
#ifdef _WIN64
    iDmacDrv = LoadLibrary(TEXT("iDmacDrv64.dll"));
    if (iDmacDrv == NULL)
    {
        LOG_ERROR("Failed to load iDmacDrv64.dll. Error code: %d\n", GetLastError());
        return FALSE;
    }
#else
    iDmacDrv = LoadLibrary(TEXT("iDmacDrv32.dll"));
    if (iDmacDrv == NULL)
    {
        LOG_ERROR("Failed to load iDmacDrv32.dll. Error code: %d\n", GetLastError());
        return FALSE;
    }
#endif

    iDmacDrvOpen = (dmacOpen)GetProcAddress(iDmacDrv, "iDmacDrvOpen");
    if (iDmacDrvOpen == NULL)
    {
        LOG_ERROR("Failed to get address of iDmacDrvOpen. Error code: %d\n", GetLastError());
        return FALSE;
    }

    iDmacDrvRegisterRead = (dmacRead)GetProcAddress(iDmacDrv, "iDmacDrvRegisterRead");
    if (iDmacDrvRegisterRead == NULL)
    {
        LOG_ERROR("Failed to get address of iDmacRegisterRead. Error code: %d\n", GetLastError());
        return FALSE;
    }

    iDmacDrvRegisterWrite = (dmacWrite)GetProcAddress(iDmacDrv, "iDmacDrvRegisterWrite");
    if (iDmacDrvRegisterWrite == NULL)
    {
        LOG_ERROR("Failed to get address of iDmacRegisterWrite. Error code: %d\n", GetLastError());
        return FALSE;
    }

    iDmacDrvClose = (dmacClose)GetProcAddress(iDmacDrv, "iDmacDrvClose");
    if (iDmacDrvClose == NULL)
    {
        LOG_ERROR("Failed to get address of iDmacDrvClose. Error code: %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}


BOOL WINAPI ctrlHandler(DWORD signal)
{
    UNREFERENCED_PARAMETER(signal);

    keepPolling = FALSE;
    WaitForSingleObject(hMainThread, INFINITE);
    CloseHandle(hMainThread);

    return TRUE;
}


#ifdef JVS_FRIENDLY
HANDLE JVS_Open()
{
    HANDLE hCom = CreateFile(JVS_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, 0, NULL);
    if (hCom == INVALID_HANDLE_VALUE)
    {
        LOG_VERBOSE("JVS_Open: CreateFile failed. Error code: %d\n", GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    DWORD errors;
    if (!ClearCommError(hCom, &errors, NULL))
    {
        LOG_ERROR("JVS_Open: ClearCommError failed. Error code: %d\n", GetLastError());
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    if (!SetupComm(hCom, 516ul, 516ul))
    {
        LOG_ERROR("JVS_Open: SetupComm failed. Error code: %d\n", GetLastError());
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hCom, &dcb))
    {
        LOG_ERROR("JVS_Open: GetCommState failed. Error code: %d\n", GetLastError());
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8u;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(hCom, &dcb))
    {
        LOG_ERROR("JVS_Open: SetCommState failed. Error code: %d\n", GetLastError());
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    if (!EscapeCommFunction(hCom, CLRRTS))
    {
        LOG_ERROR("JVS_Open: EscapeCommFunction failed. Error code: %d\n", GetLastError());
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    if (!EscapeCommFunction(hCom, SETDTR))
    {
        LOG_ERROR("JVS_Open: EscapeCommFunction failed. Error code: %d\n", GetLastError());
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    if (!SetCommMask(hCom, EV_RXCHAR))
    {
        LOG_ERROR("JVS_Open: SetCommMask failed. Error code: %d\n", GetLastError());
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    COMMTIMEOUTS comTimeouts = { 0 };
    if (!SetCommTimeouts(hCom, &comTimeouts))
    {
        LOG_ERROR("JVS_Open: SetCommTimeouts failed. Error code: %d\n", GetLastError());
        CloseHandle(hCom);
        return INVALID_HANDLE_VALUE;
    }

    return hCom;
}


void JVS_Close(HANDLE hCom)
{
    EscapeCommFunction(hCom, CLRDTR);
    CloseHandle(hCom);
}


BOOL JVS_Write(HANDLE hCom, const byte *message, DWORD messageLength)
{
    if (!EscapeCommFunction(hCom, SETRTS))
    {
        LOG_ERROR("JVS_Write: EscapeCommFunction failed. Error code: %d\n", GetLastError());
        return FALSE;
    }

    DWORD numBytesWritten;
    if (!WriteFile(hCom, message, messageLength, &numBytesWritten, NULL))
    {
        LOG_ERROR("JVS_Write: WriteFile failed. Error code: %d\n", GetLastError());
        return FALSE;
    }

    if (numBytesWritten != messageLength)
    {
        LOG_ERROR("JVS_Write: WriteFile failed to write enough bytes: messageLength=%d, numBytesWritten=%d\n",
            messageLength, numBytesWritten);
        return FALSE;
    }

    if (!FlushFileBuffers(hCom))
    {
        LOG_ERROR("JVS_Write: FlushFileBuffers failed. Error code: %d\n", GetLastError());
        return FALSE;
    }

    DWORD errors;
    if (!ClearCommError(hCom, &errors, NULL))
    {
        LOG_ERROR("JVS_Write: ClearCommError failed. Error code: %d\n", GetLastError());
        return FALSE;
    }

    if (!EscapeCommFunction(hCom, SETRTS))
    {
        LOG_ERROR("JVS_Write: EscapeCommFunction failed. Error code: %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}


BOOL JVS_Reset(HANDLE hCom)
{
    static const DWORD resetMessageLength = 6ul;
    static const byte resetMessage[resetMessageLength] = {
        0xe0, 0xff, 0x03, 0xf0, 0xd9, 0xcb
    };

    if (!JVS_Write(hCom, resetMessage, resetMessageLength))
    {
        LOG_ERROR("JVS_Reset: Failed\n");
        return FALSE;
    }

    LOG_VERBOSE("JVS_Reset: Succeeded\n");
    return TRUE;
}


BOOL JVS_Register(HANDLE hCom)
{
    static const DWORD registerMessageLength = 6ul;
    static const byte registerMessage[registerMessageLength] = {
        0xe0, 0xff, 0x03, 0xf1, 0x01, 0xf4
    };

    if (!JVS_Write(hCom, registerMessage, registerMessageLength))
    {
        LOG_ERROR("JVS_Register: Failed\n");
        return FALSE;
    }

    LOG_VERBOSE("JVS_Register: Succeeded\n");
    return TRUE;
}


void CALLBACK JVS_HeartbeatCallback(UINT uTimerID, UINT uMsg,
    DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    UNREFERENCED_PARAMETER(uTimerID);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(dw1);
    UNREFERENCED_PARAMETER(dw2);

    HANDLE hCom = (HANDLE)dwUser;
    JVS_Reset(hCom);
}
#endif


BOOL __cdecl FIO_Open()
{
    int flags = 0x0;
    return iDmacDrvOpen(deviceIndex, &deviceId, &flags) == 0;
}


BOOL __cdecl FIO_RegRead(DWORD address, int &data_out)
{
    int flags = 0x0;
    return iDmacDrvRegisterRead(deviceId, address, &data_out, &flags) == 0;
}


BOOL assignPlayers()
{
    int data;
    BOOL port1HasConnection = FIO_RegRead(0x4000, data) && data & 0xff;
    BOOL port2HasConnection = FIO_RegRead(0x4004, data) && data & 0xff;

    if (port1HasConnection)
    {
        buttonsAddressP1P2 = 0x4120;
        LOG_VERBOSE("Fast I/O Port 1: Player 1 + Player 2\n");
        if (port2HasConnection)
        {
            buttonsAddressP3P4 = 0x41a0;
            LOG_VERBOSE("Fast I/O Port 2: Player 3 + Player 4\n");
        }
        else
        {
            buttonsAddressP3P4 = NULL;
            LOG_VERBOSE("Fast I/O Port 2: Empty\n");
        }
    }
    else
    {
        buttonsAddressP3P4 = NULL;
        LOG_VERBOSE("Fast I/O Port 1: Empty\n");
        if (port2HasConnection)
        {
            buttonsAddressP1P2 = 0x41a0;
            LOG_VERBOSE("Fast I/O Port 2: Player 1 + Player 2\n");
        }
        else
        {
            buttonsAddressP1P2 = NULL;
            LOG_VERBOSE("Fast I/O Port 2: Empty\n");
            return FALSE;
        }
    }

    return TRUE;
}


void pollP1P2(int &buttonsData, int &prevButtonsData, INPUT &diEvent)
{
    prevButtonsData = buttonsData;
    if (!FIO_RegRead(buttonsAddressP1P2, buttonsData) ||
        buttonsData == prevButtonsData)
    {
        return;
    }
    LOG_VERBOSE("pollP1P2: buttonsData=0x%x\n", buttonsData);

    int xorButtonsData = buttonsData ^ prevButtonsData;

    CHECK_ESCAPE(g_keymap.s_esc1, 0x10, 0x10000, 0x100000,
        "Escape 1 (Player 1 Start + Button 1 + Button 3)", 0x10, 0x10000, 0x100000);
    CHECK_ESCAPE(g_keymap.s_esc2, 0x20, 0x20000, 0x200000,
        "Escape 2 (Player 2 Start + Button 1 + Button 3)", 0x20, 0x20000, 0x200000);

    CHECK_BINDING(g_keymap.s_coin1, 0x1, "Coin 1");
    CHECK_BINDING(g_keymap.s_coin2, 0x2, "Coin 2");
    CHECK_BINDING(g_keymap.s_service1, 0x4, "Service 1");
    CHECK_BINDING(g_keymap.s_service2, 0x8, "Service 2");
    CHECK_BINDING(g_keymap.p1_start, 0x10, "Player 1 Start");
    CHECK_BINDING(g_keymap.p2_start, 0x20, "Player 2 Start");
    CHECK_BINDING(g_keymap.s_test1, 0x40, "Test 1");
    CHECK_BINDING(g_keymap.s_tilt1, 0x80, "Tilt 1");
    CHECK_BINDING(g_keymap.p1_up, 0x100, "Player 1 Up");
    CHECK_BINDING(g_keymap.p2_up, 0x200, "Player 2 Up");
    CHECK_BINDING(g_keymap.p1_down, 0x400, "Player 1 Down");
    CHECK_BINDING(g_keymap.p2_down, 0x800, "Player 2 Down");
    CHECK_BINDING(g_keymap.p1_left, 0x1000, "Player 1 Left");
    CHECK_BINDING(g_keymap.p2_left, 0x2000, "Player 2 Left");
    CHECK_BINDING(g_keymap.p1_right, 0x4000, "Player 1 Right");
    CHECK_BINDING(g_keymap.p2_right, 0x8000, "Player 2 Right");
    CHECK_BINDING(g_keymap.p1_btn1, 0x10000, "Player 1 Button 1");
    CHECK_BINDING(g_keymap.p2_btn1, 0x20000, "Player 2 Button 1");
    CHECK_BINDING(g_keymap.p1_btn2, 0x40000, "Player 1 Button 2");
    CHECK_BINDING(g_keymap.p2_btn2, 0x80000, "Player 2 Button 2");
    CHECK_BINDING(g_keymap.p1_btn3, 0x100000, "Player 1 Button 3");
    CHECK_BINDING(g_keymap.p2_btn3, 0x200000, "Player 2 Button 3");
    CHECK_BINDING(g_keymap.p1_btn4, 0x400000, "Player 1 Button 4");
    CHECK_BINDING(g_keymap.p2_btn4, 0x800000, "Player 2 Button 4");
    CHECK_BINDING(g_keymap.p1_btn5, 0x1000000, "Player 1 Button 5");
    CHECK_BINDING(g_keymap.p2_btn5, 0x2000000, "Player 2 Button 5");
    CHECK_BINDING(g_keymap.p1_btn6, 0x4000000, "Player 1 Button 6");
    CHECK_BINDING(g_keymap.p2_btn6, 0x8000000, "Player 2 Button 6");
    CHECK_BINDING(g_keymap.p1_btn7, 0x10000000, "Player 1 Button 7");
    CHECK_BINDING(g_keymap.p2_btn7, 0x20000000, "Player 2 Button 7");
    CHECK_BINDING(g_keymap.p1_btn8, 0x40000000, "Player 1 Button 8");
    CHECK_BINDING(g_keymap.p2_btn8, 0x80000000, "Player 2 Button 8");
}


void pollP3P4(int& buttonsData, int& prevButtonsData, INPUT& diEvent)
{
    prevButtonsData = buttonsData;
    if (!FIO_RegRead(buttonsAddressP3P4, buttonsData) ||
        buttonsData == prevButtonsData)
    {
        return;
    }
    LOG_VERBOSE("pollP3P4: buttonsData=0x%x\n", buttonsData);

    int xorButtonsData = buttonsData ^ prevButtonsData;

    CHECK_ESCAPE(g_keymap.s_esc3, 0x10, 0x10000, 0x100000,
        "Escape 3 (Player 3 Start + Button 1 + Button 3)", 0x10, 0x10000, 0x100000);
    CHECK_ESCAPE(g_keymap.s_esc4, 0x20, 0x20000, 0x200000,
        "Escape 4 (Player 4 Start + Button 1 + Button 3)", 0x20, 0x20000, 0x200000);

    CHECK_BINDING(g_keymap.s_coin3, 0x1, "Coin 3");
    CHECK_BINDING(g_keymap.s_coin4, 0x2, "Coin 4");
    CHECK_BINDING(g_keymap.s_service3, 0x4, "Service 3");
    CHECK_BINDING(g_keymap.s_service4, 0x8, "Service 4");
    CHECK_BINDING(g_keymap.p3_start, 0x10, "Player 3 Start");
    CHECK_BINDING(g_keymap.p4_start, 0x20, "Player 4 Start");
    CHECK_BINDING(g_keymap.s_test2, 0x40, "Test 2");
    CHECK_BINDING(g_keymap.s_tilt2, 0x80, "Tilt 2");
    CHECK_BINDING(g_keymap.p3_up, 0x100, "Player 3 Up");
    CHECK_BINDING(g_keymap.p4_up, 0x200, "Player 4 Up");
    CHECK_BINDING(g_keymap.p3_down, 0x400, "Player 3 Down");
    CHECK_BINDING(g_keymap.p4_down, 0x800, "Player 4 Down");
    CHECK_BINDING(g_keymap.p3_left, 0x1000, "Player 3 Left");
    CHECK_BINDING(g_keymap.p4_left, 0x2000, "Player 4 Left");
    CHECK_BINDING(g_keymap.p3_right, 0x4000, "Player 3 Right");
    CHECK_BINDING(g_keymap.p4_right, 0x8000, "Player 4 Right");
    CHECK_BINDING(g_keymap.p3_btn1, 0x10000, "Player 3 Button 1");
    CHECK_BINDING(g_keymap.p4_btn1, 0x20000, "Player 4 Button 1");
    CHECK_BINDING(g_keymap.p3_btn2, 0x40000, "Player 3 Button 2");
    CHECK_BINDING(g_keymap.p4_btn2, 0x80000, "Player 4 Button 2");
    CHECK_BINDING(g_keymap.p3_btn3, 0x100000, "Player 3 Button 3");
    CHECK_BINDING(g_keymap.p4_btn3, 0x200000, "Player 4 Button 3");
    CHECK_BINDING(g_keymap.p3_btn4, 0x400000, "Player 3 Button 4");
    CHECK_BINDING(g_keymap.p4_btn4, 0x800000, "Player 4 Button 4");
    CHECK_BINDING(g_keymap.p3_btn5, 0x1000000, "Player 3 Button 5");
    CHECK_BINDING(g_keymap.p4_btn5, 0x2000000, "Player 4 Button 5");
    CHECK_BINDING(g_keymap.p3_btn6, 0x4000000, "Player 3 Button 6");
    CHECK_BINDING(g_keymap.p4_btn6, 0x8000000, "Player 4 Button 6");
    CHECK_BINDING(g_keymap.p3_btn7, 0x10000000, "Player 3 Button 7");
    CHECK_BINDING(g_keymap.p4_btn7, 0x20000000, "Player 4 Button 7");
    CHECK_BINDING(g_keymap.p3_btn8, 0x40000000, "Player 3 Button 8");
    CHECK_BINDING(g_keymap.p4_btn8, 0x80000000, "Player 4 Button 8");
}


int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "/?") == 0 || strcmp(argv[i], "/h") == 0)
        {
            LOG_INFO(
                "--------------------------------------------->\n"
                "- FastIO2KB --------------------------------->\n"
                "--------------------------------------------->\n"
                "  Version:       %s\n"
                "  Architecture:  %s\n"
                "  Profile:       %s\n"
                "  Keymap:        %s\n"
                "  Log Level:     %s\n",
                VERSION_DESCRIPTION,
                ARCHITECTURE_DESCRIPTION,
                PROFILE_DESCRIPTION,
                GetKeymapName(),
                LOG_LEVEL_DESCRIPTION);
            return 0;
        }
    }

    LoadKeymapFromINI(KEYMAP_INI_FILENAME);

    if (!loadNtdll())
    {
        LOG_ERROR("Windows Native API (ntdll) is missing or incompatible\n");
        return 1;
    }

    if (!loadIDmacDrv())
    {
        LOG_ERROR("DMAC Driver Interface (iDmacDrv) is missing or incompatible\n");
        return 1;
    }

    HANDLE pseudoHProcess = GetCurrentProcess();
    HANDLE pseudoHMainThread = GetCurrentThread();

    if (!DuplicateHandle(pseudoHProcess, pseudoHMainThread, pseudoHProcess,
        &hMainThread, NULL, FALSE, DUPLICATE_SAME_ACCESS))
    {
        LOG_ERROR("Failed to get handle to main thread. Error code: %d\n", GetLastError());
        return 1;
    }

    if (!SetConsoleCtrlHandler(ctrlHandler, TRUE))
    {
        LOG_ERROR("Failed to set control handler. Error code: %d\n", GetLastError());
        return 1;
    }

    if (!FIO_Open())
    {
        LOG_ERROR("Failed to open iDmacDrv device. Error code: %d\n", GetLastError());
        return 1;
    }

    if (!assignPlayers())
    {
        LOG_ERROR("Failed to detect Fast I/O connection for any player\n");
        return 1;
    }

#ifdef JVS_FRIENDLY
    HANDLE hCom = JVS_Open();
    UINT uTimerID;
    if (hCom == INVALID_HANDLE_VALUE)
    {
        LOG_VERBOSE("JVS Port: Closed\n");
        uTimerID = NULL;
    }
    else
    {
        LOG_VERBOSE("JVS Port: Open\n");
        JVS_Reset(hCom);
        uTimerID = timeSetEvent(JVS_HEARTBEAT_INTERVAL,
            JVS_HEARTBEAT_INTERVAL_RESOLUTION, JVS_HeartbeatCallback,
            (DWORD_PTR)hCom, TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);
        if (uTimerID == NULL)
        {
            LOG_ERROR("Failed to set periodic callback for JVS heartbeat\n");
        }
    }
#endif

    int buttonsDataP1P2 = 0x0;
    int prevButtonsDataP1P2 = 0x0;
    int buttonsDataP3P4 = 0x0;
    int prevButtonsDataP3P4 = 0x0;

    INPUT diEvent = { 0 };
    diEvent.type = INPUT_KEYBOARD;

    ULONG minTimerResolution;
    ULONG maxTimerResolution;
    ULONG currentTimerResolution;
    ULONG preferredTimerResolution;
    if (NtQueryTimerResolution(&minTimerResolution, &maxTimerResolution,
            &currentTimerResolution) != STATUS_SUCCESS)
    {
        preferredTimerResolution = POLLING_INTERVAL_RESOLUTION;
    }
    else
    {
        preferredTimerResolution = min(
            max(maxTimerResolution, POLLING_INTERVAL_RESOLUTION),
            minTimerResolution);
    }

    if (NtSetTimerResolution(preferredTimerResolution, TRUE,
            &currentTimerResolution) != STATUS_SUCCESS)
    {
        LOG_VERBOSE("Timer resolution: Unknown\n");
    }
    else
    {
        LOG_VERBOSE("Timer resolution: %.4fms\n",
            preferredTimerResolution * 0.0001);
    }

    LOG_VERBOSE("Starting to poll input registers...\n");
    if (buttonsAddressP3P4 == NULL)
    {
        while (keepPolling)
        {
            pollP1P2(buttonsDataP1P2, prevButtonsDataP1P2, diEvent);
            Sleep(POLLING_INTERVAL);
        }
    }
    else
    {
        while (keepPolling)
        {
            pollP1P2(buttonsDataP1P2, prevButtonsDataP1P2, diEvent);
            pollP3P4(buttonsDataP3P4, prevButtonsDataP3P4, diEvent);
            Sleep(POLLING_INTERVAL);
        }
    }
    LOG_VERBOSE("Finished polling input registers\n");

#ifdef JVS_FRIENDLY
    if (hCom != INVALID_HANDLE_VALUE)
    {
        if (uTimerID != NULL)
        {
            timeKillEvent(uTimerID);
        }
        JVS_Close(hCom);
    }
#endif

    NtSetTimerResolution(preferredTimerResolution, FALSE,
        &currentTimerResolution);

    return 0;
}
