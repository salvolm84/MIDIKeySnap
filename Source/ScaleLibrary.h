#pragma once

#include <juce_core/juce_core.h>

namespace ScaleLibrary
{
struct ScaleDefinition
{
    const char* name {};
    std::vector<int> semitones;
};

inline const std::vector<ScaleDefinition>& getScales()
{
    static const std::vector<ScaleDefinition> scales {
        { "Major Pentatonic",      { 0, 2, 4, 7, 9 } },
        { "Minor Pentatonic",      { 0, 3, 5, 7, 10 } },
        { "Egyptian Pentatonic",   { 0, 2, 5, 7, 10 } },
        { "Hirajoshi",             { 0, 2, 3, 7, 8 } },
        { "Iwato",                 { 0, 1, 5, 6, 10 } },
        { "Insen",                 { 0, 1, 5, 7, 10 } },
        { "Kumoi",                 { 0, 2, 3, 7, 9 } },
        { "Ritusen",               { 0, 2, 5, 7, 9 } },
        { "Blues Hexatonic",       { 0, 3, 5, 6, 7, 10 } },
        { "Whole Tone",            { 0, 2, 4, 6, 8, 10 } },
        { "Augmented",             { 0, 3, 4, 7, 8, 11 } },
        { "Prometheus",            { 0, 2, 4, 6, 9, 10 } },
        { "Major Blues",           { 0, 2, 3, 4, 7, 9 } },
        { "Minor Hexatonic",       { 0, 2, 3, 5, 7, 10 } },
        { "Major",                 { 0, 2, 4, 5, 7, 9, 11 } },
        { "Natural Minor",         { 0, 2, 3, 5, 7, 8, 10 } },
        { "Harmonic Minor",        { 0, 2, 3, 5, 7, 8, 11 } },
        { "Melodic Minor",         { 0, 2, 3, 5, 7, 9, 11 } },
        { "Dorian",                { 0, 2, 3, 5, 7, 9, 10 } },
        { "Phrygian",              { 0, 1, 3, 5, 7, 8, 10 } },
        { "Lydian",                { 0, 2, 4, 6, 7, 9, 11 } },
        { "Mixolydian",            { 0, 2, 4, 5, 7, 9, 10 } },
        { "Locrian",               { 0, 1, 3, 5, 6, 8, 10 } },
        { "Phrygian Dominant",     { 0, 1, 4, 5, 7, 8, 10 } },
        { "Hungarian Minor",       { 0, 2, 3, 6, 7, 8, 11 } },
        { "Double Harmonic",       { 0, 1, 4, 5, 7, 8, 11 } },
        { "Neapolitan Minor",      { 0, 1, 3, 5, 7, 8, 11 } },
        { "Neapolitan Major",      { 0, 1, 3, 5, 7, 9, 11 } },
        { "Acoustic",              { 0, 2, 4, 6, 7, 9, 10 } },
        { "Altered",               { 0, 1, 3, 4, 6, 8, 10 } },
        { "Enigmatic",             { 0, 1, 4, 6, 8, 10, 11 } },
        { "Persian",               { 0, 1, 4, 5, 6, 8, 11 } },
        { "Romanian Minor",        { 0, 2, 3, 6, 7, 9, 10 } },
        { "Ukrainian Dorian",      { 0, 2, 3, 6, 7, 9, 10 } },
        { "Major Locrian",         { 0, 2, 4, 5, 6, 8, 10 } },
        { "Lydian Dominant",       { 0, 2, 4, 6, 7, 9, 10 } },
        { "Mixolydian b6",         { 0, 2, 4, 5, 7, 8, 10 } },
        { "Minor Major",           { 0, 2, 3, 5, 7, 9, 11 } },
        { "Half Diminished",       { 0, 2, 3, 5, 6, 8, 10 } }
    };

    return scales;
}
} // namespace ScaleLibrary
