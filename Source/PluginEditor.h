#pragma once

#include <array>

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

class PianoKeyComponent final : public juce::Component
{
public:
    PianoKeyComponent(int pitchClass, bool whiteKey);

    std::function<void(int)> onClick;

    void setSelected(bool shouldBeSelected);
    void setOutputActive(bool shouldBeActive);
    void setInputActive(bool shouldBeActive);

    int getPitchClass() const noexcept;
    bool isWhite() const noexcept;

    void paint(juce::Graphics& g) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    int pitchClass = 0;
    bool white = true;
    bool selected = false;
    bool outputActive = false;
    bool inputActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoKeyComponent)
};

class PianoOctaveComponent final : public juce::Component,
                                   private juce::Timer
{
public:
    PianoOctaveComponent();

    std::function<void(int)> onKeyPressed;

    void setSelectedNotes(const std::array<bool, 12>& selection);
    void setOutputActiveStates(const std::array<bool, 12>& activeStates);
    void setInputActiveStates(const std::array<bool, 12>& activeStates);

    void resized() override;

private:
    std::array<std::unique_ptr<PianoKeyComponent>, 12> keys;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoOctaveComponent)
};

class MIDIKeySnapAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer,
                                              private juce::ChangeListener
{
public:
    explicit MIDIKeySnapAudioProcessorEditor(MIDIKeySnapAudioProcessor&);
    ~MIDIKeySnapAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MIDIKeySnapAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::ComboBox scaleBox;
    juce::ComboBox targetOutRootBox;
    juce::ComboBox snapModeBox;
    juce::ComboBox blackKeyModeBox;
    juce::Label targetOutRootLabel;
    juce::Label scaleLabel;
    juce::Label snapModeLabel;
    juce::Label blackKeyModeLabel;
    juce::Label hintLabel;
    juce::Label statusLabel;
    PianoOctaveComponent piano;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> snapModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> targetOutRootAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> blackKeyModeAttachment;

    void refreshFromProcessor();
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDIKeySnapAudioProcessorEditor)
};
