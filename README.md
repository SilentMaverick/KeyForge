# KEYFORGE

KEYFORGE is a Windows 11 desktop audio-feedback engine that makes an ordinary keyboard feel mechanical by reacting to **physical keyboard events** and playing cached local sounds. It does not remap, inject, suppress, record, or transmit keystrokes.

## Current implementation

- C++20 / Win32 desktop application.
- Windows Raw Input for physical keyboard observation.
- XAudio2 with a bounded 32-voice pool using a canonical 48 kHz stereo source format.
- PCM 16-bit WAV validation, resampling, bounded 128 MiB LRU caching, and rejected-asset caching.
- Key-down playback without Windows text-repeat, with an allocation-free normal playback path.
- Optional press/release architecture.
- Per-key assignments and drag/drop/import path.
- Master volume and emergency disable hotkey (`Ctrl+Alt+Shift+K`).
- User configuration under `%LOCALAPPDATA%\KEYFORGE`.
- No administrator requirement, drivers, services, telemetry, registry persistence, or network dependency.

## Build

Requirements: Windows 11 x64, Visual Studio 2022 Build Tools or Visual Studio with the Desktop C++ workload, Windows 10/11 SDK, and CMake 3.25+.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Run `build\Release\KEYFORGE.exe` from the project/release directory containing the `sounds` folder.

## Audio format

The current safe decoder accepts PCM WAV only: 16-bit, 1–2 channels, 8–192 kHz. Individual files are limited to 64 MB and PCM payloads to 32 MB.

## Privacy

Raw Input events are used only transiently to identify a physical key and trigger audio. No typed characters, passwords, keyboard sequences, clipboard contents, screenshots, or telemetry are persisted or transmitted.

## Hardening audit

See `SECURITY_AUDIT.md` and `PERFORMANCE.md` for the vulnerability review, cache limits, complexity model, and residual Windows-only verification requirements.

## Important limitations in 0.1

This release establishes the safety-critical core. Explicit output-device selection, full profile switching, polished visual key animations, complete JSON sound-pack metadata parsing, startup integration, installer packaging, and comprehensive automated Windows integration tests are intentionally staged for subsequent increments rather than faked.

## Release packaging

Build Release first, then open `installer/KEYFORGE.iss` with Inno Setup 6. The installer is intentionally a normal per-user Windows application and does not install a keyboard driver or service.
