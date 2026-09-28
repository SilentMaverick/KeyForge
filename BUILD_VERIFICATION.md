# Build Verification

## Performed in this environment

- Project tree and required source/docs/installer files checked.
- C++ source brace balance checked.
- Bundled PCM WAV files opened and validated for 16-bit audio and supported sample rates/channels.
- Static search performed for input-injection, process execution, registry persistence, driver/device-control, and network-download primitives.
- Security/performance audit completed; see `SECURITY_AUDIT.md` and `PERFORMANCE.md`.
- CMake configuration is Windows/x64-only by design.

## Requires Windows 11 verification

This execution environment is not Windows and does not provide MSVC, the Windows SDK, XAudio2, Raw Input, or the Win32 runtime. Therefore a real compile/run/stress test could not honestly be claimed here.

On Windows 11, run:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Then execute the tests in `tests/README.md` before shipping.


## Required Windows security/performance checks

- Build with `/W4 /permissive-` and treat warnings as review items.
- Run the WAV corpus against malformed RIFF/chunk sizes, truncated files, oversized files, and unsupported formats.
- Verify 32+ rapid voices never grow memory without bound.
- Verify emergency disable remains effective during rapid typing.
- Verify a sound-pack symlink/reparse point cannot escape the pack root.
- Verify unplugging/replugging the default audio endpoint does not affect keyboard input.
