# MASTER BUILD PROMPT

## Project: KEYFORGE — Real-Time Custom Mechanical Keyboard Sound Engine for Windows 11

You are an expert Windows desktop systems engineer, audio-engine developer, UX designer, and security-conscious software architect.

Build a complete, production-quality **Windows 11 desktop application** called **KEYFORGE**.

The goal is to transform ANY ordinary laptop/desktop keyboard into a customizable virtual mechanical keyboard by detecting keyboard presses in real time and playing extremely low-latency, customizable key sounds.

This is NOT a keyboard remapper.

This is NOT a macro tool.

This is NOT a keylogger.

It is an **audio feedback engine that reacts to keyboard events**.

---

# 1. CORE CONCEPT

When the user presses a physical keyboard key:

```
Physical Key Press
        ↓
Windows Keyboard Event
        ↓
KEYFORGE Event Engine
        ↓
Identify Key
        ↓
Select Assigned Sound
        ↓
Apply Velocity/Randomization/Modifiers
        ↓
Ultra-Low-Latency Audio Playback
        ↓
Mechanical Keyboard Sound

```

Example:

User presses:

```
A

```

KEYFORGE immediately plays:

```
click_A.wav

```

User holds A for 1 second:

```
key-down → sound plays immediately

```

DO NOT wait for:

```
key-up

```

DO NOT wait for:

```
Windows key-repeat

```

The sound must happen on the initial physical key-down event.

---

# 2. PRIMARY REQUIREMENT — EXTREMELY LOW LATENCY

The application must feel instantaneous.

Target:

```
Keyboard event → audible sound

```

Target latency:

```
ideally <10 ms

```

Acceptable:

```
<20 ms

```

Avoid architectures that introduce large buffering delays.

Do NOT:

- record microphone input
- continuously record keyboard data
- poll the keyboard at slow intervals
- wait for key release
- rely on key-repeat events
- spawn a new process per key press
- load an audio file from disk on every key press
- initialize an audio device for every key press
- create a new heavyweight audio engine for every event

Instead:

1. Load sounds into memory.
2. Pre-initialize the audio engine.
3. Keep the audio device alive.
4. Maintain an efficient sound cache.
5. Trigger playback from memory.
6. Use an appropriate low-latency Windows audio API/library.
7. Reuse audio resources.

---

# 3. WINDOWS TARGET

Target ONLY:

```
Windows 11

```

Architecture:

```
x64

```

Do not optimize for macOS/Linux.

Do not require Linux subsystems.

Do not require WSL.

Do not require administrator privileges unless absolutely unavoidable.

The application should work as a normal Windows desktop application.

---

# 4. KEYBOARD EVENT CAPTURE

Use a safe Windows-compatible keyboard event mechanism.

Prefer a non-invasive architecture such as:

```
Windows Raw Input

```

or another appropriate Windows API designed for keyboard input observation.

The application must:

- detect key-down
- detect key-up when necessary
- distinguish physical keys correctly
- support normal keyboards
- support laptop keyboards
- support external USB keyboards
- support mechanical keyboards
- support different keyboard layouts as far as technically possible

IMPORTANT:

KEYFORGE MUST NEVER:

- modify keyboard drivers
- install unknown kernel drivers
- modify Windows registry unnecessarily
- modify boot configuration
- disable Windows security
- intercept passwords for storage
- record typed text
- store keyboard input
- transmit keyboard input
- send keystrokes to another machine
- implement keylogging functionality

The program should only use keyboard events to trigger audio.

---

# 5. KEYBOARD PRIVACY

This is extremely important.

The application must NOT know or store:

```
what the user typed

```

It only needs:

```
physical key identity
key-down event
key-up event
timing information required for audio behavior

```

Do not create:

```
typing history
text logs
keypress database
clipboard monitoring
screenshot functionality
telemetry containing keyboard input

```

All keyboard event processing should happen locally.

---

# 6. SOUND ENGINE

Create a dedicated low-latency audio engine.

The engine should support:

- WAV
- preferably high-quality PCM WAV
- optional OGG/FLAC support if practical

Sounds should be decoded/preloaded into memory.

The engine should support multiple simultaneous sounds.

Example:

User types:

```
A S D F

```

The system should be capable of playing:

```
A sound
S sound
D sound
F sound

```

with overlapping playback if necessary.

Do NOT stop the previous sound just because another key is pressed.

---

# 7. SOUND CHARACTERISTICS

Each key should have configurable:

### Volume

Range:

```
0–100%

```

### Pitch

Example:

```
-12 semitones
to
+12 semitones

```

### Random Pitch

Example:

```
±2%

```

This prevents repetitive robotic sounds.

### Random Volume

Example:

```
±5%

```

### Playback Offset

Allow tiny random offsets where appropriate.

### Stereo Position

Allow:

```
Left
Center
Right

```

Optional per-key stereo positioning.

---

# 8. KEY-SPECIFIC SOUNDS

Every keyboard key should be independently configurable.

Example:

A → Click 01
B → Click 02
C → Click 01
Space → Spacebar
Enter → Enter
Backspace → Backspace
Shift → Shift
Ctrl → Ctrl
Alt → Alt
Tab → Tab
Esc → Esc

The user must be able to assign different sounds to individual keys.

---

# 9. SOUND PACK SYSTEM

Create a sound-pack architecture.

Example:

```
Sound Packs
   ├── Classic Blue
   ├── Red Linear
   ├── Brown Tactile
   ├── Creamy
   ├── Thock
   ├── Clicky
   ├── Typewriter
   └── Custom

```

A sound pack contains mappings such as:

```
A → sound_a.wav
B → sound_b.wav
...
Space → space.wav
Enter → enter.wav

```

Users should be able to create/import/export sound packs.

Recommended format:

```
JSON metadata
+
audio files

```

Example conceptual structure:

```
SoundPack/
    manifest.json
    sounds/
        a.wav
        b.wav
        c.wav
        space.wav
        enter.wav

```

---

# 10. SOUND VARIATIONS

Support multiple samples per key.

Example:

A:

```
a_01.wav
a_02.wav
a_03.wav
a_04.wav

```

On every press:

```
randomly select one

```

OR:

```
sequentially rotate samples

```

Modes:

```
Random
Sequential
Random-with-no-immediate-repeat

```

The "no immediate repeat" mode is preferred for realism.

---

# 11. PRESS / RELEASE SOUNDS

Support two sound modes.

### Mode A — Press Only

Play sound on:

```
key-down

```

### Mode B — Press + Release

Play:

```
key-down → press sound
key-up → release sound

```

The release sound must NOT play if the user disables the option.

This should be configurable globally and per key.

---

# 12. HOLD BEHAVIOR

Holding a key must NOT repeatedly trigger the sound simply because Windows generates keyboard repeat events.

Example:

User holds:

```
A

```

for 1 second.

Expected:

```
ONE press sound

```

Not:

```
A A A A A A A A...

```

unless the user specifically enables a repeat simulation feature.

Use physical key state/event handling rather than normal text-input repetition.

---

# 13. OPTIONAL KEYBOARD REPEAT SIMULATION

Provide an advanced optional setting:

```
Simulate Repeat

```

Default:

```
OFF

```

If enabled:

```
initial sound
    ↓
configurable delay
    ↓
repeated sounds

```

Settings:

```
Initial delay
Repeat interval
Maximum repeat rate

```

This feature must be completely optional.

---

# 14. GLOBAL CONTROLS

Main controls:

```
Master Volume
Sound Enable/Disable
Press Sounds
Release Sounds
Randomization
Pitch Variation
Volume Variation
Sound Pack
Audio Output Device

```

Include:

```
Test Sound

```

and:

```
Preview Key

```

---

# 15. PER-KEY EDITOR

Create a visual keyboard.

It should look like an actual modern keyboard.

Every key should be clickable.

Example:

```
┌────┐ ┌────┐ ┌────┐
│ Q  │ │ W  │ │ E  │
└────┘ └────┘ └────┘

```

Clicking Q opens:

```
Q
─────────────
Press Sound
Release Sound
Volume
Pitch
Randomization
Preview
Reset

```

The user must immediately understand which sound is assigned to which key.

---

# 16. DRAG AND DROP

Allow users to drag an audio file onto a key.

Example:

```
my_custom_click.wav

```

Drop onto:

```
A

```

Result:

```
A → my_custom_click.wav

```

Validate the audio file before accepting it.

Never execute files.

Only treat supported audio files as audio assets.

---

# 17. AUDIO FILE SAFETY

Audio importing must be defensive.

Reject:

- unsupported formats
- corrupted files
- maliciously malformed files where the decoder cannot safely process them
- excessively large files
- absurd sample rates
- unreasonable channel counts

Implement sensible limits.

Example:

Maximum individual sound size:

```
configurable but reasonable

```

Maximum sound-pack size:

```
configurable but reasonable

```

Do not allow imported files to cause unbounded memory allocation.

---

# 18. AUDIO NORMALIZATION

Provide optional normalization.

Example:

```
Normalize loudness

```

This prevents:

```
A = extremely loud
B = extremely quiet

```

Allow:

```
Off
Automatic
Manual

```

Do not permanently modify the user's original audio file.

---

# 19. AUDIO OUTPUT

Allow selecting the output device.

Example:

```
Speakers
Headphones
USB DAC
Bluetooth headset
External audio interface

```

Handle device changes gracefully.

If headphones are disconnected:

```
do not crash

```

If the selected device disappears:

```
gracefully fall back to the default Windows output device

```

When the device returns:

```
optionally restore the selected device

```

---

# 20. WINDOWS VOLUME SAFETY

Include a master volume slider.

Recommended:

```
0–100%

```

Optional safety limit:

```
Maximum application volume

```

Never attempt to bypass Windows volume controls.

Do not modify system volume without explicit user action.

---

# 21. GLOBAL HOTKEY

Provide a configurable global shortcut:

```
Toggle KEYFORGE

```

Example:

```
Ctrl + Alt + K

```

When disabled:

```
keyboard behaves completely normally

```

When enabled:

```
sound engine operates normally

```

The application must NEVER block normal keyboard functionality.

---

# 22. EXCLUSION / IGNORE MODE

Provide an exclusion system.

Examples:

```
Ignore when game is running
Ignore specific applications
Ignore specific windows

```

Example:

```
Ignore:
    Counter-Strike
    Valorant
    Anti-cheat protected games

```

The exact implementation must be safe and should not interfere with protected software.

A simpler initial implementation can provide:

```
Pause when foreground application matches exclusion list.

```

---

# 23. APPLICATION PROFILES

Allow different sound configurations per application.

Example:

```
Default Profile
├── Mechanical

Coding Profile
├── Thock

Gaming Profile
├── Minimal

Writing Profile
├── Typewriter

```

The application detects the foreground Windows application and switches profile.

IMPORTANT:

Profile switching must never interfere with keyboard functionality.

---

# 24. SYSTEM TRAY

The application should support Windows system tray.

Tray menu:

```
KEYFORGE
─────────────
Enable / Disable
Master Volume
Current Sound Pack
Open Settings
Pause
Exit

```

Closing the main window should optionally minimize to tray.

---

# 25. STARTUP

Optional:

```
Start KEYFORGE with Windows

```

This must be opt-in.

Do not secretly create startup persistence.

Use a standard Windows-supported startup mechanism.

---

# 26. UI DESIGN

Design language:

```
premium
futuristic
minimal
dark
clean
mechanical
technical

```

Avoid:

- bloated enterprise UI
- excessive animations
- unnecessary gradients
- confusing menus
- excessive rounded cards

The interface should feel like:

```
premium mechanical keyboard software

```

rather than:

```
generic Windows utility

```

---

# 27. MAIN WINDOW

Suggested structure:

```
┌──────────────────────────────────────────────┐
│ KEYFORGE                         ● ACTIVE    │
├──────────────────────────────────────────────┤
│                                              │
│       VISUAL KEYBOARD                        │
│                                              │
│  ESC F1 F2 F3 ...                            │
│  ┌─┐ ┌─┐ ┌─┐ ┌─┐                            │
│  │Q│ │W│ │E│ │R│ ...                        │
│  └─┘ └─┘ └─┘ └─┘                            │
│                                              │
├──────────────────────────────────────────────┤
│ SOUND PACK                                   │
│ [ Classic Mechanical ▼ ]                     │
│                                              │
│ Master Volume      ───────●────              │
│ Pitch Variation    ───●────────              │
│ Randomization      [ ON ]                    │
│                                              │
│ [Sound Library] [Key Editor] [Settings]      │
└──────────────────────────────────────────────┘

```

---

# 28. LIVE KEY VISUALIZATION

When a key is physically pressed:

```
corresponding UI key briefly lights/highlights

```

Example:

User presses:

```
W

```

The W key visually reacts immediately.

This visualization should be asynchronous and must NEVER block the keyboard/audio processing thread.

---

# 29. PERFORMANCE REQUIREMENTS

KEYFORGE must be lightweight.

Target:

```
very low CPU usage when idle

```

Avoid:

```
unnecessary background loops
busy waiting
constant polling
excessive UI updates

```

The keyboard event path should be extremely short.

Conceptually:

```
event
  ↓
identify key
  ↓
lookup sound
  ↓
trigger cached sample
  ↓
return

```

Do not perform expensive work inside the event callback.

---

# 30. THREADING ARCHITECTURE

Separate:

```
UI thread
Keyboard input thread
Audio engine
File loading
Profile manager

```

Never perform:

```
file I/O
expensive decoding
database operations
UI rendering

```

inside the real-time keyboard event path.

Use appropriate thread-safe queues or lock-free structures where beneficial.

---

# 31. AUDIO ENGINE ARCHITECTURE

Use a real-time-safe architecture.

Avoid allocations in the audio callback.

Avoid locks in the real-time audio callback where possible.

Avoid disk I/O in the audio callback.

Avoid unpredictable garbage collection pauses if the chosen language/runtime could cause them.

Preload audio samples.

Use pooled/reusable voice objects.

Implement a maximum simultaneous voice count.

Example:

```
Max voices = 32

```

If the limit is reached:

```
use a sensible voice-stealing policy

```

Never crash.

---

# 32. AUDIO MIXING

Support overlapping sounds.

Example:

```
Q + W + E + R

```

should produce:

```
Q sound
W sound
E sound
R sound

```

mixed together.

Prevent clipping.

Implement:

```
gain control
mixing
optional soft limiting

```

---

# 33. RAPID TYPING

The engine must handle:

```
10+
20+
50+

```

key events per second

without crashing or building an infinite queue.

If events arrive faster than audio can be played:

```
intelligently manage them

```

Never allow an unbounded event queue.

---

# 34. CRASH RESILIENCE

This is critical.

If:

```
sound file is corrupted
audio device disappears
sound pack is invalid
configuration file is corrupted
unexpected keyboard event occurs
application loses focus
Windows changes audio device
user rapidly changes settings

```

the application must fail gracefully.

Never:

```
BSOD
freeze Windows
disable keyboard
lock keyboard input
corrupt system files
modify drivers
crash Explorer
permanently intercept input

```

---

# 35. EMERGENCY FAILSAFE

Implement an emergency disable mechanism.

Example:

```
Ctrl + Alt + Shift + K

```

This immediately disables the audio engine.

The keyboard must continue working normally.

Also provide:

```
Disable on startup failure

```

If the audio subsystem cannot initialize:

```
KEYFORGE should start in disabled mode

```

rather than repeatedly crashing.

---

# 36. WATCHDOG

A lightweight internal health monitor may verify:

```
keyboard listener alive
audio engine alive
selected output device available

```

If something fails:

```
attempt recovery

```

If recovery fails:

```
disable audio

```

DO NOT repeatedly restart indefinitely.

---

# 37. CONFIGURATION

Store settings in an appropriate user-specific Windows application-data location.

Do NOT write configuration randomly into:

```
Windows
System32
Program Files

```

unless required by the installer.

Configuration should include:

```
master volume
selected sound pack
per-key assignments
audio device
hotkeys
profiles
randomization
startup setting
UI settings

```

Use versioned configuration files.

---

# 38. CONFIGURATION RECOVERY

If configuration becomes invalid:

```
detect it

```

Then:

```
backup invalid configuration
generate safe defaults
start normally

```

Never crash because a JSON/config file is malformed.

---

# 39. RESET FUNCTION

Provide:

```
Reset Current Profile

```

and:

```
Factory Reset

```

Factory reset should restore application configuration only.

It must NOT remove unrelated Windows settings.

---

# 40. INSTALLER

Create a proper Windows 11 installer.

Requirements:

- clean install
- clean uninstall
- no unnecessary services
- no hidden persistence
- no bundled junk
- no browser extensions
- no telemetry by default

Uninstaller must remove:

```
application files
application configuration if the user chooses
startup entry if created

```

Do not remove unrelated user files.

---

# 41. PORTABLE MODE

If practical, also support:

```
Portable Mode

```

where the application can run from a folder without installation.

If portable mode is implemented:

```
keep configuration within the portable application directory

```

without requiring administrator privileges.

---

# 42. LOGGING

Logging should be useful but privacy-safe.

Log:

```
application errors
audio initialization errors
device changes
configuration errors

```

NEVER log:

```
actual keys typed
text entered
passwords
keyboard sequences

```

Do not include keyboard contents in crash reports.

---

# 43. TELEMETRY

Default:

```
OFF

```

Prefer:

```
NO TELEMETRY

```

The application should work completely offline.

No cloud account should be required.

No internet connection should be required for basic functionality.

---

# 44. SOUND LIBRARY

Create an internal sound-library abstraction.

Example:

```
Sound
   ├── ID
   ├── File
   ├── Duration
   ├── Sample Rate
   ├── Channels
   ├── Peak
   └── Normalized Gain

```

Key mapping:

```
KeyCode → SoundAssignment

```

Sound assignment:

```
PressSample[]
ReleaseSample[]
Volume
Pitch
RandomPitch
RandomVolume

```

---

# 45. CUSTOM KEY MAPPING

Support:

```
Standard keys
Function keys
Navigation keys
Modifier keys
Numpad
Arrow keys
Insert/Delete/Home/End
Page Up/Page Down
Print Screen
Scroll Lock
Pause
Windows key
Context/Menu key

```

Where Windows exposes a distinct physical key identity, preserve that distinction.

---

# 46. LAPTOP COMPATIBILITY

Pay special attention to laptop keyboards.

Do not assume:

```
full-size desktop keyboard

```

The application should gracefully handle:

- compact keyboards
- 60%
- 65%
- 75%
- TKL
- full-size
- laptop layouts
- external keyboards

The visual keyboard is a representation, not a requirement that the physical keyboard match it.

---

# 47. MULTIPLE KEYBOARDS

If practical, support multiple physical keyboards.

Example:

```
Laptop keyboard
+
USB mechanical keyboard

```

Allow:

```
All keyboards

```

or optionally:

```
specific keyboard device

```

to trigger sounds.

Do not break either keyboard.

---

# 48. SOUND TESTING

Include a sound preview system.

When user clicks:

```
Preview

```

play the sound without requiring an actual key press.

Include:

```
Play
Stop
Volume
Pitch

```

---

# 49. SETTINGS SEARCH

If the application becomes large, provide a settings search.

Examples:

```
"latency"
"volume"
"startup"
"hotkey"
"sound pack"

```

Search should immediately locate the relevant setting.

---

# 50. ACCESSIBILITY

Support:

- keyboard navigation through the UI
- readable labels
- reasonable contrast
- tooltips
- scalable UI where practical

Do not rely solely on color to communicate state.

---

# 51. ERROR MESSAGES

Never display cryptic developer errors to ordinary users.

Bad:

```
HRESULT 0x80004005

```

Better:

```
"KEYFORGE couldn't initialize the selected audio device. Windows may have disconnected it. We've switched to the default device."

```

Provide:

```
[Retry]
[Open Audio Settings]

```

where appropriate.

---

# 52. SECURITY MODEL

Treat all imported sound packs as untrusted data.

Sound packs must NEVER contain executable code.

Do not implement:

```
scripts
plugins
arbitrary code execution
shell commands
macros

```

inside sound packs.

A sound pack should be data only.

---

# 53. NO KEYBOARD MANIPULATION

The application must NEVER alter what the keyboard does.

For example:

Press:

```
A

```

must still produce:

```
A

```

in the active application.

KEYFORGE only adds sound.

It must not:

- remap A → B
- suppress A
- inject text
- generate macros
- modify shortcuts

unless a future feature explicitly requires it, and that feature should NOT be implemented in this version.

---

# 54. GAME SAFETY

KEYFORGE should avoid behaving like a cheat or input manipulation tool.

It must not inject input into games.

It must not modify game memory.

It must not interact with anti-cheat systems.

It simply observes keyboard activity and produces local audio.

Provide an easy:

```
Pause KEYFORGE

```

option for games.

---

# 55. RESOURCE LIMITS

Implement hard safety limits.

Examples:

```
Maximum sound-pack memory
Maximum simultaneous voices
Maximum audio duration per sample
Maximum number of loaded samples
Maximum configuration size

```

The exact values should be chosen sensibly during implementation.

Never allow malformed input to cause unbounded memory growth.

---

# 56. TESTING REQUIREMENTS

Before considering the project complete, test:

### Keyboard

- every standard key
- rapid typing
- key holding
- simultaneous keys
- modifier combinations
- function keys
- numpad
- external USB keyboard
- laptop keyboard

### Audio

- speakers
- headphones
- Bluetooth
- USB audio device
- device disconnect
- device reconnect
- rapid sound playback
- simultaneous sounds

### Configuration

- valid configuration
- malformed configuration
- missing configuration
- corrupted sound file
- deleted sound file
- missing sound pack

### Windows

- startup
- shutdown
- sleep
- wake
- lock
- unlock
- user switching where applicable

### Performance

Measure:

```
startup time
idle CPU
idle RAM
typing CPU
audio latency
memory usage

```

---

# 57. CRITICAL TEST

Perform a stress test:

Generate extremely rapid keyboard input events.

The application must:

```
remain responsive
not crash
not freeze
not grow memory indefinitely
not break keyboard input

```

After stopping the stress test:

```
return to normal operation

```

---

# 58. ARCHITECTURE DOCUMENTATION

Create:

```
README.md

```

Include:

- project overview
- architecture
- build instructions
- dependencies
- Windows requirements
- security model
- audio architecture
- keyboard event architecture
- configuration format
- sound-pack format
- troubleshooting
- known limitations

Also create:

```
ARCHITECTURE.md

```

with diagrams such as:

```
Keyboard
    ↓
Windows Input Layer
    ↓
Event Processor
    ↓
Sound Resolver
    ↓
Audio Voice Manager
    ↓
Mixer
    ↓
Windows Audio
    ↓
Speakers / Headphones

```

---

# 59. CODE QUALITY

Write production-quality code.

Requirements:

- modular architecture
- clear naming
- error handling
- comments where useful
- no unnecessary comments
- no duplicated logic
- no giant monolithic files
- no magic numbers without explanation
- no unnecessary dependencies

Prefer stable, mature libraries.

Avoid obscure libraries unless there is a strong reason.

---

# 60. DEPENDENCY POLICY

Every dependency must have a reason.

Before adding a library ask:

```
Can Windows/.NET/C++ standard functionality handle this?

```

If yes:

```
prefer built-in functionality.

```

Minimize dependency count.

Do not include abandoned libraries.

Pin dependency versions where appropriate.

---

# 61. TECHNOLOGY CHOICE

Choose the technology stack that provides the best combination of:

```
low latency
Windows compatibility
stability
maintainability
modern UI
small footprint

```

Strong candidates include:

```
C++ + Win32/WinUI
C#/.NET + appropriate Windows APIs/audio library
Rust + Windows APIs

```

Choose ONE architecture and explain why.

Do not blindly use a technology just because it is popular.

For the audio/input critical path, prioritize deterministic low-latency behavior over UI convenience.

---

# 62. BUILD SYSTEM

The project must be reproducible.

Provide:

```
source code
dependency configuration
build configuration
release configuration

```

A fresh Windows 11 machine should be able to build the project by following README instructions.

Document:

```
compiler version
SDK version
runtime requirements
build commands

```

---

# 63. RELEASE BUILD

Produce a release configuration optimized for:

```
performance
reliability
reasonable binary size

```

Do not sacrifice safety for tiny binary size.

---

# 64. FIRST-RUN EXPERIENCE

On first launch:

```
Welcome to KEYFORGE

```

Then:

```
Select audio output
Choose sound pack
Test keyboard
Set master volume

```

Provide:

```
"Play a test sound"

```

The application should then be ready.

Do not force account creation.

Do not force internet access.

---

# 65. DEFAULT SOUND PACK

Include a legal, redistributable default sound pack.

Do NOT download copyrighted mechanical keyboard recordings from random websites.

If actual sound assets cannot legally be bundled:

```
provide placeholders
and a documented process for importing the user's own sounds.

```

---

# 66. IMPORT/EXPORT

Allow:

```
Import Sound Pack
Export Sound Pack

```

Validate everything before installing.

Never overwrite user files without confirmation.

If an imported pack has the same name:

```
offer:
    Replace
    Rename
    Cancel

```

---

# 67. BACKUP

Before destructive configuration changes:

```
create a lightweight backup

```

Allow:

```
Restore Previous Configuration

```

Do not create infinite backups.

Use sensible rotation.

---

# 68. UX PRINCIPLE

The user should be able to:

```
install
launch
choose a sound pack
type

```

within approximately:

```
30 seconds

```

The application should feel instant and fun.

---

# 69. ABSOLUTE SAFETY REQUIREMENT

The application must NEVER make the user regret installing it.

Therefore:

DO NOT:

- install drivers unnecessarily
- modify BIOS
- modify boot settings
- modify system files
- disable security software
- disable Defender
- modify firewall rules
- create hidden services
- create hidden scheduled tasks
- secretly run at startup
- collect typed text
- send keyboard data online
- inject keyboard input
- modify other applications
- install browser extensions
- bundle unrelated software
- require administrator privileges unnecessarily

If administrator privileges are genuinely required for some optional feature:

```
explain why
isolate that feature
keep the base application functional without it where possible

```

---

# 70. "KILL SWITCH" REQUIREMENT

The application must have an obvious:

```
DISABLE AUDIO

```

control.

Also implement the emergency hotkey.

If anything behaves unexpectedly:

```
audio can immediately be stopped.

```

Keyboard functionality must remain untouched.

---

# 71. FAILURE PHILOSOPHY

When uncertain:

```
choose the least invasive behavior.

```

When an audio operation fails:

```
disable audio rather than interfere with keyboard input.

```

When configuration is corrupted:

```
restore defaults rather than crash.

```

When a device disappears:

```
fallback rather than freeze.

```

When a sound is invalid:

```
skip the sound rather than crash.

```

When a feature cannot be implemented safely:

```
disable that feature rather than compromise system stability.

```

---

# 72. DELIVERABLES

The final project must contain:

```
/src
/assets
/sounds
/config
/tests
/docs

```

And:

```
README.md
ARCHITECTURE.md
SECURITY.md
CONTRIBUTING.md
CHANGELOG.md

```

Also provide:

```
installer configuration
build instructions
release instructions

```

---

# 73. FINAL ACCEPTANCE CRITERIA

Do NOT consider the project finished merely because it compiles.

It is finished only when:

[ ] Keyboard presses trigger sounds immediately

[ ] Holding a key does not produce unwanted repeated sounds

[ ] Normal keyboard typing remains completely functional

[ ] Sounds can be customized per key

[ ] Multiple samples can be assigned per key

[ ] Randomization works

[ ] Press/release sounds work

[ ] Sound packs work

[ ] Import/export works

[ ] Audio device switching works

[ ] Headphones work

[ ] Laptop keyboards work

[ ] External keyboards work

[ ] Rapid typing works

[ ] Simultaneous keys work

[ ] Configuration recovery works

[ ] Corrupted audio doesn't crash the app

[ ] Missing audio devices don't crash the app

[ ] Application can be disabled instantly

[ ] No keyboard text is stored

[ ] No keyboard data leaves the machine

[ ] No driver installation is required unless absolutely necessary

[ ] No dangerous Windows modifications are performed

[ ] CPU/RAM usage is reasonable

[ ] Windows 11 compatibility is verified

[ ] Installer works

[ ] Uninstaller works

[ ] README is complete

[ ] Security model is documented

[ ] Tests are included

---

# 74. DEVELOPMENT WORKFLOW

Do not attempt to blindly generate thousands of lines of code at once.

Build incrementally.

PHASE 1:

Create the architecture and minimal Windows application.

PHASE 2:

Implement keyboard event detection.

PHASE 3:

Implement basic audio playback.

PHASE 4:

Connect:

```
key-down → sound

```

PHASE 5:

Implement sound caching and low-latency playback.

PHASE 6:

Implement per-key mapping.

PHASE 7:

Implement visual keyboard.

PHASE 8:

Implement sound packs.

PHASE 9:

Implement customization.

PHASE 10:

Implement profiles.

PHASE 11:

Implement tray/settings/startup.

PHASE 12:

Implement safety/recovery.

PHASE 13:

Implement installer.

PHASE 14:

Stress-test everything.

At every phase:

```
compile
run
test
fix
continue

```

Never silently carry known compilation errors into the next phase.

---

# 75. IMPORTANT CODING-AGENT RULE

If you encounter a technical limitation:

DO NOT fake the feature.

Explain:

```
what Windows permits
what the limitation is
what the safest alternative is

```

Then implement the safest practical alternative.

Never use unsafe hacks merely to satisfy a feature requirement.

---

# 76. FINAL REQUEST

Build KEYFORGE as if it were going to be released publicly.

Prioritize:

```
LOW LATENCY
STABILITY
SAFETY
PRIVACY
WINDOWS 11 COMPATIBILITY
GOOD UX

```

over:

```
unnecessary features
flashy animations
excessive dependencies
clever but fragile implementations

```

The final product should feel like:

```
"I installed this and my normal keyboard became a mechanical keyboard."

```

It should NOT feel like:

```
"I installed this and Windows became unstable."

```

Produce the complete working project, not a conceptual prototype.

Before declaring completion, perform a final audit for:

```
compilation errors
race conditions
memory leaks
audio-resource leaks
thread-safety issues
keyboard-event duplication
unwanted key-repeat
device-disconnection failures
malformed configuration
malformed audio files
unsafe Windows modifications
privacy violations
excessive CPU usage
excessive memory usage

```

Fix all discovered issues before completion.