# KEYFORGE Architecture

```text
Physical keyboard
      |
      v
Windows Raw Input
      |
      v
WM_INPUT / physical-key de-duplication
      |
      +----> asynchronous UI indication
      |
      v
Sound resolver
      |
      v
In-memory PCM cache
      |
      v
XAudio2 32-voice pool
      |
      v
Windows audio endpoint
      |
      v
Speakers / headphones
```

## Safety boundary

The input layer observes Raw Input only. No keyboard driver is installed and no input is injected back into Windows. If audio fails, the application disables audio rather than touching keyboard behavior.

## Real-time considerations

The current playback path performs only a bounded lookup, parameter calculation, and `SubmitSourceBuffer` against already decoded PCM. Disk I/O and WAV decoding occur in the cache loader. The voice pool is fixed at 32 entries to prevent unbounded growth.

## Configuration

User configuration is stored below `%LOCALAPPDATA%\KEYFORGE`. Sound assets are application-local and treated as untrusted data. The WAV decoder enforces format, size, channel, sample-rate, and bit-depth limits before caching.
