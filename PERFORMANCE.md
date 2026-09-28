# KEYFORGE Performance & Complexity Model

## Normal key-down path

```text
Raw Input
   ↓
O(1) physical-key normalization
   ↓
O(1) repeat-state check
   ↓
O(1) key → assignment lookup
   ↓
O(1) key → preloaded sample lookup
   ↓
O(32) voice scan = O(1) bounded constant
   ↓
XAudio2 SubmitSourceBuffer / Start
```

### Complexity

| Operation | Complexity | Disk I/O? | Allocation? |
|---|---:|---:|---:|
| Raw Input packet handling | O(1) | No | No |
| Key-repeat suppression | O(1) | No | No |
| Assignment lookup | O(1) average | No | No |
| Preloaded sample lookup | O(1) average | No | No |
| Voice selection | O(32) | No | No |
| Audio submission | O(1) bounded by XAudio2 | No | No application heap allocation |
| Normal key release | O(1) | No | No |

The 32-voice scan is intentionally fixed-size. It is therefore a constant-time upper bound rather than an unbounded search.

## Startup/import path

WAV parsing and resampling are intentionally moved away from the real-time event path.

For an input file containing `N` samples and `M` output samples:

- WAV validation: O(file size)
- PCM conversion/resampling: O(M)
- Cache insertion: O(1) average
- LRU trimming: O(number of cache entries examined), bounded by one cache pass per trim

The normal key path never performs this work.

## Cache properties

- Maximum decoded cache: 128 MiB.
- LRU eviction.
- Maximum decoded sample duration: 10 seconds.
- Maximum simultaneous voices: 32.
- Invalid/rejected assets are negatively cached to prevent repeated decoder/file work.
- Imported files invalidate the corresponding cached entry before reload.

## Why the architecture is faster

The previous design could execute filesystem canonicalization and sample loading from `HandleKey()`, which made first-use and invalid-file latency unpredictable. The hardened design resolves and preloads all configured samples during startup and replaces that work with direct in-memory lookups.

The audio engine also uses one canonical source format. This avoids creating voices for different sample rates/channel counts and avoids format negotiation in the playback path.
