# Test Plan

Run these on Windows 11 before release:

- Hold a key for 1 second: exactly one press sound.
- Type rapidly at 50+ key events/sec: no crash and bounded memory.
- Press multiple keys simultaneously: overlapping voices work.
- Test laptop and USB keyboards.
- Disconnect/reconnect the active audio endpoint.
- Import a valid PCM WAV and malformed/oversized WAV.
- Corrupt `%LOCALAPPDATA%\KEYFORGE\config.json` and verify safe recovery.
- Trigger `Ctrl+Alt+Shift+K` and verify audio stops while typing remains normal.
- Close/uninstall and verify no driver/service is installed.
