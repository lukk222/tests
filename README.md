# mymalwarewin (Windows 2000 / NT 4.0 Destroyer)

⚠️ **WARNUNG: Dieses Programm ist destruktiv und beschädigt das Betriebssystem permanent!**

## Zweck
Dieses Projekt wurde rein zu Bildungszwecken und für die Sicherheitsforschung auf Altsystemen (Windows 2000, ReactOS, NT 4.0) entwickelt. Es zeigt, wie ungeschützt GDI-Ressourcen und der Master Boot Record (MBR) in älteren Windows-Architekturen sind.

## Funktionen
* 20 Phasen intensives GDI-Grafikchaos
* Asynchrone WAV-Audio-Wiedergabe im Hintergrund (CRT-frei)
* Überschreibt Sektor 0 (MBR) mit einem benutzerdefinierten 16-Bit Real-Mode Bootloader
* Erzwingt nach 4:42 Minuten einen IRQL_NOT_LESS_OR_EQUAL Bluescreen via NtRaiseHardError

## Haftungsausschluss
Der Entwickler übernimmt keine Haftung für Schäden, die durch den Missbrauch dieses Quellcodes entstehen. Die Ausführung ist AUSSCHLIESSLICH in isolierten virtuellen Testumgebungen gestattet.
## Kompilieren unter Windows

Hier sind die exakten Befehle, um den Code direkt unter Windows in der Eingabeaufforderung (`cmd`, als Administrator starten) zu bauen.

### Methode 1: Über MinGW (GCC / G++)

#### 1. Compiler installieren
Nutze den in Windows eingebauten Paketmanager `winget`, um MSYS2 und die 32-Bit-Compiler-Toolchain für maximale Kompatibilität mit alten Betriebssystemen einzurichten:
```cmd
winget install MSYS2.MSYS2 --hush
C:\msys64\usr\bin\bash.exe -lc "pacman -S --noconfirm mingw-w64-i686-toolchain"
setx PATH "%PATH%;C:\msys64\mingw32\bin" /M
```
*(Hinweis: Starte die Eingabeaufforderung nach diesem Schritt einmal neu, damit der Pfad aktiv wird!)*

#### 2. Kompilieren
Führe diesen Befehl im Ordner mit deiner Quellcodedatei aus (ersetze `<datei>` durch deinen Dateinamen, z.B. `killer.cpp`):
```cmd
g++ -O2 -ffreestanding -nostdlib -mno-stack-arg-probe -Wl,--enable-stdcall-fixup <datei> -o mymalwarewin.exe -mwindows -lgdi32 -luser32 -lkernel32 -lwinmm -ladvapi32
```

---

### Methode 2: Über Microsoft Visual Studio (MSVC / cl.exe)

#### 1. Compiler installieren
Installiert lautlos im Hintergrund die offiziellen Microsoft C++ Build-Tools, ohne das schwere Visual-Studio-Hauptprogramm laden zu müssen:
```cmd
winget install Microsoft.VisualStudio.2022.BuildTools --override "--passive --add Microsoft.VisualStudio.Workload.VCTools"
```
*(Hinweis: Öffne nach der Installation die "Eingabeaufforderung für native Tools für x86" im Startmenü!)*

#### 2. Kompilieren
Führe diesen Befehl in der VS-Eingabeaufforderung aus:
```cmd
cl.exe /O2 /GS- /wd4201 <datei> /link /NODEFAULTLIB /ENTRY:WinMainCRTStartup /SUBSYSTEM:WINDOWS gdi32.lib user32.lib kernel32.lib winmm.lib advapi32.lib /OUT:mymalwarewin.exe
```
