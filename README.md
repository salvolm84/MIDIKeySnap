# MIDIKeySnap

`MIDIKeySnap` is a JUCE-based VST3 MIDI FX plugin for macOS that remaps incoming MIDI notes into a user-defined 5, 6, or 7 note scale shape.

It is designed for fast scale-constrained performance workflows:

- click notes on a one-octave piano to define a custom C-based scale
- recall common pentatonic, hexatonic, and heptatonic scales from the preset list
- remap white-key input across the selected note set
- choose alternate sparse-scale behaviors when fewer than 7 notes are selected
- set independent handling rules for black-key input notes
- transpose the resulting output after mapping
- visualize both the incoming note and the mapped output note on the same keyboard

## Features

- `VST3` MIDI FX plugin
- built with `JUCE`
- full-octave clickable piano UI
- up to 7 selected notes at a time
- grouped preset list for 5-note, 6-note, and 7-note scales
- automatic recognition of known scales from custom clicked note sets
- sparse mapping modes:
  - `Wrap`
  - `Clamp Down`
  - `Clamp Up`
  - `Spread`
  - `Random`
- black-key input modes:
  - `Pass Through`
  - `Block`
  - `Snap Down`
  - `Snap Up`
  - `Nearest`
- output transpose control in semitone steps
- held-note visual feedback:
  - mapped output note highlight
  - raw input note bottom marker

## Musical Model

- Scale definitions are always treated as `C-based`.
- Incoming white keys are mapped into the selected scale notes.
- `Output Transpose` is applied after the scale mapping step.
- Black-key handling is controlled separately through the `Black Keys` option.

## Build

This project expects a local JUCE checkout and uses CMake.

### Requirements

- macOS
- CMake 3.22+
- Xcode / AppleClang
- JUCE checkout available locally

### Default JUCE Path

The project currently expects JUCE at:

`/Users/salvolm/JUCE`

You can override it at configure time:

```bash
cmake -S . -B build -DJUCE_DIR=/path/to/JUCE
```

### Build Command

```bash
cmake -S . -B build
cmake --build build --config Release --target MIDIKeySnap_VST3
```

The built plugin bundle is generated at:

`build/MIDIKeySnap_artefacts/VST3/MIDIKeySnap.vst3`

## Install

Copy the built bundle to:

`~/Library/Audio/Plug-Ins/VST3/MIDIKeySnap.vst3`

Then rescan plugins in your DAW.

## Repository Contents

- `CMakeLists.txt`: JUCE/CMake project configuration
- `Source/PluginProcessor.*`: MIDI mapping engine and plugin state
- `Source/PluginEditor.*`: UI and keyboard visualization
- `Source/ScaleLibrary.h`: bundled scale definitions

## License

This project is licensed under the GNU General Public License v3.0 or later.

That license choice is intended to stay compatible with JUCE usage under the JUCE open source licensing model. See `LICENSE`.
