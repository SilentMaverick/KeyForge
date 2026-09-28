# KEYFORGE Security & Vulnerability Audit

Audit scope: source tree after the hardening pass, focused on Windows 11 x64 behavior, keyboard observation, audio asset parsing, persistence, installer privileges, and real-time performance.

## 1. Findings fixed

| ID | Severity | Finding | Fix |
|---|---|---|---|
| KF-001 | High | XAudio2 source voices were created without a concrete source format. This could make audio initialization fail or reject submitted buffers. | All voices now use a canonical 48 kHz / 16-bit / stereo PCM format; WAVs are converted once at load time. |
| KF-002 | High | Default sound assignments used `sounds/click.wav` while the sound-pack root was already the `sounds` directory. | Default and imported assignments now use paths relative to the sound-pack root, e.g. `click.wav`. |
| KF-003 | High | A persisted assignment could contain an absolute or `..` path and cause KEYFORGE to load an arbitrary local file as audio. | Asset paths are required to remain inside the sound-pack root using canonicalization + relative-path validation. |
| KF-004 | High | Cache growth was effectively unbounded. Repeatedly importing unique sounds could retain decoded PCM indefinitely. | Added a 128 MiB LRU cache with bounded eviction and a negative-cache for rejected assets. |
| KF-005 | Medium | Every key press could perform filesystem canonicalization and potentially disk access. | Assignments are resolved/preloaded during startup/import; normal key processing uses in-memory maps only. |
| KF-006 | Medium | The original audio path allocated a heap context for every playback. | Each of the 32 reusable voices owns a fixed callback token; normal playback performs no heap allocation. |
| KF-007 | Medium | `StopAll()` could release a voice before its asynchronous buffer callback arrived, allowing stale callbacks to interfere with a newly reused voice. | Voices remain occupied until the flush/end callback releases them; immutable per-voice tokens prevent stale-state confusion. |
| KF-008 | Medium | Imported/replaced sounds could remain stale in the cache because the cache key was the same path. | Import now explicitly invalidates the cached sample before validation/reload. |
| KF-009 | Medium | Holding a key was de-duplicated, but the implementation used a heap allocation for every Raw Input packet. | Keyboard processing now uses a fixed 1 KiB stack buffer for the Raw Input packet. |
| KF-010 | Medium | Emergency disable did not guard the release-sound branch, so a release sound could still occur after disabling audio. | Release playback now requires the engine to be enabled and ready. |
| KF-011 | Medium | Installer used an invalid GUID-like AppId and a machine-oriented default directory despite `PrivilegesRequired=lowest`. | Replaced with a valid GUID and per-user installation directory under LocalAppData. |
| KF-012 | Low | Configuration replacement removed the existing config before a second rename, creating an avoidable failure window. | Configuration now uses `MoveFileExW(..., MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)`. |
| KF-013 | Low | Oversized configuration files were not backed up before recovery. | Files above 1 MiB are backed up as `.invalid` and defaults are restored. |
| KF-014 | Low | Extended keyboard identities were inconsistent with the assignment map, so arrow keys could fail to resolve. | E0 physical identities use a consistent `0x100` namespace (e.g. Left = `0x14B`). |

## 2. Security properties

### Keyboard

- Uses Windows Raw Input observation.
- Does not install a keyboard driver.
- Does not suppress, remap, or inject keystrokes.
- Does not call `SendInput`, `keybd_event`, or low-level keyboard hooks.
- Does not persist key sequences or text.
- Does not send keyboard information over the network.

### Audio assets

Imported assets are treated as untrusted data:

- PCM WAV only.
- 16-bit PCM only.
- 1–2 channels accepted as input.
- 8–192 kHz input range.
- 64 MiB physical file limit.
- 32 MiB decoded PCM ceiling.
- 10 second decoded duration ceiling.
- Canonical conversion to 48 kHz stereo PCM before playback.
- Invalid assets are rejected and negatively cached to avoid repeated disk/decoder work.
- No scripts, plugins, shell commands, or executable payloads are supported.

### Path security

Sound assignments are data-only relative paths. Before an asset is loaded, KEYFORGE resolves the path and verifies that the resulting path remains inside the configured sound-pack root. Absolute paths and traversal outside the root are rejected.

### Persistence

Configuration is stored below `%LOCALAPPDATA%\KEYFORGE`. It is capped at 1 MiB on load. Invalid/oversized configurations are backed up and replaced with safe defaults.

### Installer

The installer requests the lowest privilege level and installs into the per-user LocalAppData program directory. No service, scheduled task, driver, firewall rule, Defender setting, or hidden persistence mechanism is created.

## 3. Residual risks / verification still required

These require an actual Windows 11 test environment and cannot be proven by static inspection alone:

1. ASan/UBSan or equivalent Windows-native fuzzing of the WAV parser.
2. Repeated XAudio2 device disconnect/reconnect testing.
3. Sleep/wake and fast user switching.
4. Raw Input behavior across laptop OEM keyboards and unusual HID devices.
5. Inno Setup installation/uninstallation verification on a clean standard-user account.
6. Measured keyboard-to-audio latency with a hardware or timestamped audio test rig.
7. Thread/race stress testing under simultaneous key events and audio callbacks.

## 4. Security posture

The code has no obvious remote-code-execution, command-execution, input-injection, or network-exfiltration primitive in the reviewed source tree. The largest risks in this class of application are local untrusted asset parsing, uncontrolled memory growth, accidental input interference, and real-time callback races; those areas were specifically hardened in this pass.
