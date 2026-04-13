#pragma once

#include <array>

#include <juce_audio_processors/juce_audio_processors.h>

class MIDIKeySnapAudioProcessor final : public juce::AudioProcessor,
                                        public juce::ChangeBroadcaster
{
public:
    struct ScaleMatch
    {
        juce::String scaleName;

        bool isValid() const { return scaleName.isNotEmpty(); }
    };

    enum class SnapMode
    {
        wrap = 0,
        clampDown,
        clampUp,
        spread,
        random
    };

    enum class BlackKeyMode
    {
        passThrough = 0,
        block,
        snapDown,
        snapUp,
        nearest
    };

    MIDIKeySnapAudioProcessor();
    ~MIDIKeySnapAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;

    std::array<bool, 12> getSelectedNotes() const;
    void setSelectedNotes(const std::array<bool, 12>& newSelection);
    bool toggleSelectedNote(int pitchClass);
    void applyScaleByIndex(int scaleIndex);
    void applyScaleByName(const juce::String& scaleName);
    juce::String getCurrentScaleName() const;
    juce::String getDetectedScaleName() const;
    juce::String getSelectedNotesDebugString() const;
    int getTargetOutRootNote() const;

    SnapMode getSnapMode() const;
    BlackKeyMode getBlackKeyMode() const;

    bool isOutputPitchClassActive(int pitchClass) const;
    bool isInputPitchClassActive(int pitchClass) const;

private:
    std::array<bool, 12> selectedNotes { true, false, true, false, true, true, false, true, false, true, false, true };
    std::array<int, 128> activeMappings {};
    std::array<int, 128> activeDisplayMappings {};
    std::array<int, 12> activeOutputPitchClassCounts {};
    std::array<int, 12> activeInputPitchClassCounts {};
    mutable juce::CriticalSection stateLock;
    juce::String currentScaleName { "Major" };
    juce::Random rng;
    int lastRandomPitchClass = -1;

    static bool isWhiteKey(int pitchClass);
    static int wrapPitchClass(int pitchClass);
    static juce::String selectionToSignature(const std::array<bool, 12>& selection);

    std::vector<int> getSelectedPitchClassesSorted() const;
    ScaleMatch findMatchingScale(const std::array<bool, 12>& selection) const;
    std::array<int, 7> buildDegreeMap() const;
    int mapWhiteKeyPitch(int midiNote, const std::array<int, 7>& degreeMap) const;
    int mapRandomPitch(int midiNote);
    int mapBlackKeyPitch(int midiNote, bool& shouldBlock) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDIKeySnapAudioProcessor)
};
