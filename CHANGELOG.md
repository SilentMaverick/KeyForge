# Changelog

## 0.1.1-hardening

- Fixed XAudio2 source-voice format initialization.
- Added canonical 48 kHz stereo PCM conversion during WAV load.
- Added 10-second decoded sound limit.
- Added 128 MiB LRU decoded-sample cache.
- Removed normal key-path disk I/O and heap allocation.
- Added negative caching for invalid sound assets.
- Added sound-pack path containment checks against traversal/reparse escape.
- Fixed default/imported sound-path layout.
- Fixed extended arrow-key identity handling.
- Fixed emergency-disable release-sound behavior.
- Hardened config-size recovery and atomic replacement.
- Fixed per-user installer path and installer AppId.
- Added `SECURITY_AUDIT.md` and `PERFORMANCE.md`.
