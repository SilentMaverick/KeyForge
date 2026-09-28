# Security Model

KEYFORGE is deliberately designed as an audio-only observer.

## Never done

- No kernel driver installation.
- No keyboard remapping or suppression.
- No text/keypress logging.
- No clipboard or screen monitoring.
- No network transmission of keyboard data.
- No shell/plugin/script execution from sound packs.
- No hidden services or scheduled tasks.
- No Defender/firewall/security-policy changes.

## Untrusted audio

Imported audio is copied only as an audio asset and validated as PCM WAV before decoding. The decoder rejects unsupported formats, malformed chunks, excessive file sizes, invalid sample rates, unsupported channel counts, and non-16-bit PCM. Decoded audio is normalized to a fixed 48 kHz stereo playback format and capped at 10 seconds per sample.

The decoded cache is bounded to 128 MiB with LRU eviction; rejected files are negatively cached to avoid repeated decoder/file work.

## Failure policy

A malformed configuration should fall back to defaults. An invalid sound should be skipped. An unavailable audio device should disable audio rather than interfere with input. The emergency hotkey immediately stops audio and disables the engine.

## Path boundary

Sound assignment paths are data-only relative paths. Before loading, KEYFORGE canonicalizes the asset and verifies that it remains below the sound-pack root. Absolute paths and traversal outside the root are rejected.

## Audit

See `SECURITY_AUDIT.md` for the detailed finding/fix matrix and residual Windows-only verification requirements.
