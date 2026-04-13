#include "PluginEditor.h"

#include "ScaleLibrary.h"

namespace
{
constexpr std::array<bool, 12> whiteKeyMask { true, false, true, false, true, true, false, true, false, true, false, true };
constexpr int customScaleItemId = 1;
constexpr int firstPresetItemId = 100;
const juce::Colour backgroundTop   = juce::Colour::fromRGB(31, 34, 37);
const juce::Colour backgroundBottom= juce::Colour::fromRGB(20, 22, 24);
const juce::Colour panelOuter      = juce::Colour::fromRGB(49, 53, 57);
const juce::Colour panelInner      = juce::Colour::fromRGB(38, 41, 45);
const juce::Colour panelStroke     = juce::Colour::fromRGB(80, 84, 89);
const juce::Colour panelHeader     = juce::Colour::fromRGB(60, 64, 69);
const juce::Colour accentOrange    = juce::Colour::fromRGB(255, 132, 36);
const juce::Colour accentAmber     = juce::Colour::fromRGB(255, 174, 72);
const juce::Colour textPrimary     = juce::Colour::fromRGB(234, 236, 239);
const juce::Colour textMuted       = juce::Colour::fromRGB(157, 164, 170);

juce::String pitchClassName(int pitchClass)
{
    static const juce::StringArray names { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return names[pitchClass % 12];
}

void styleComboBox(juce::ComboBox& box)
{
    box.setColour(juce::ComboBox::backgroundColourId, panelInner);
    box.setColour(juce::ComboBox::outlineColourId, panelStroke);
    box.setColour(juce::ComboBox::textColourId, textPrimary);
    box.setColour(juce::ComboBox::arrowColourId, accentOrange);
}

juce::String formatScaleLabel(const ScaleLibrary::ScaleDefinition& scale)
{
    return juce::String(scale.name) + "  [" + juce::String((int) scale.semitones.size()) + "]";
}
} // namespace

PianoKeyComponent::PianoKeyComponent(int pc, bool isWhiteKey)
    : pitchClass(pc), white(isWhiteKey)
{
}

void PianoKeyComponent::setSelected(bool shouldBeSelected)
{
    if (selected != shouldBeSelected)
    {
        selected = shouldBeSelected;
        repaint();
    }
}

void PianoKeyComponent::setOutputActive(bool shouldBeActive)
{
    if (outputActive != shouldBeActive)
    {
        outputActive = shouldBeActive;
        repaint();
    }
}

void PianoKeyComponent::setInputActive(bool shouldBeActive)
{
    if (inputActive != shouldBeActive)
    {
        inputActive = shouldBeActive;
        repaint();
    }
}

int PianoKeyComponent::getPitchClass() const noexcept
{
    return pitchClass;
}

bool PianoKeyComponent::isWhite() const noexcept
{
    return white;
}

void PianoKeyComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto base = white ? juce::Colour::fromRGB(214, 216, 218)
                      : juce::Colour::fromRGB(52, 56, 60);

    if (selected)
        base = white ? juce::Colour::fromRGB(255, 164, 62)
                     : juce::Colour::fromRGB(255, 132, 36);

    g.setColour(base);
    g.fillRoundedRectangle(bounds.reduced(1.0f), white ? 4.0f : 3.0f);

    if (outputActive)
    {
        g.setColour(juce::Colours::white.withAlpha(white ? 0.72f : 0.32f));
        g.fillRoundedRectangle(bounds.reduced(4.0f), white ? 3.0f : 2.0f);

        g.setColour(accentAmber.withAlpha(white ? 0.90f : 1.00f));
        g.drawRoundedRectangle(bounds.reduced(2.0f), white ? 4.0f : 3.0f, white ? 2.0f : 1.8f);
    }

    if (inputActive)
    {
        auto marker = bounds.reduced(8.0f, 0.0f);
        marker = marker.withY(bounds.getBottom() - (white ? 18.0f : 12.0f)).withHeight(white ? 5.0f : 4.0f);
        g.setColour(juce::Colours::black.withAlpha(white ? 0.95f : 0.85f));
        g.fillRoundedRectangle(marker, 2.0f);
    }

    g.setColour(white ? juce::Colour::fromRGB(106, 110, 114) : juce::Colour::fromRGB(22, 24, 26));
    g.drawRoundedRectangle(bounds.reduced(1.0f), white ? 4.0f : 3.0f, 1.0f);

    g.setColour(white ? juce::Colour::fromRGB(44, 46, 49) : textPrimary);
    g.setFont(juce::FontOptions(white ? 13.0f : 11.0f, juce::Font::bold));
    g.drawFittedText(pitchClassName(pitchClass), getLocalBounds().reduced(2), juce::Justification::centredBottom, 1);
}

void PianoKeyComponent::mouseUp(const juce::MouseEvent& event)
{
    if (! event.mouseWasDraggedSinceMouseDown() && onClick != nullptr)
        onClick(pitchClass);
}

PianoOctaveComponent::PianoOctaveComponent()
{
    for (int pitchClass = 0; pitchClass < 12; ++pitchClass)
    {
        auto key = std::make_unique<PianoKeyComponent>(pitchClass, whiteKeyMask[(size_t) pitchClass]);
        key->onClick = [this](int pc)
        {
            if (onKeyPressed != nullptr)
                onKeyPressed(pc);
        };

        addAndMakeVisible(*key);
        keys[(size_t) pitchClass] = std::move(key);
    }

    startTimerHz(30);
}

void PianoOctaveComponent::setSelectedNotes(const std::array<bool, 12>& selection)
{
    for (int i = 0; i < 12; ++i)
        keys[(size_t) i]->setSelected(selection[(size_t) i]);
}

void PianoOctaveComponent::setOutputActiveStates(const std::array<bool, 12>& activeStates)
{
    for (int i = 0; i < 12; ++i)
        keys[(size_t) i]->setOutputActive(activeStates[(size_t) i]);
}

void PianoOctaveComponent::setInputActiveStates(const std::array<bool, 12>& activeStates)
{
    for (int i = 0; i < 12; ++i)
        keys[(size_t) i]->setInputActive(activeStates[(size_t) i]);
}

void PianoOctaveComponent::resized()
{
    auto bounds = getLocalBounds();
    const auto whiteWidth = bounds.getWidth() / 7;
    const auto whiteHeight = bounds.getHeight();
    const auto blackWidth = juce::roundToInt((float) whiteWidth * 0.62f);
    const auto blackHeight = juce::roundToInt((float) whiteHeight * 0.62f);

    int whiteIndex = 0;

    for (int pitchClass = 0; pitchClass < 12; ++pitchClass)
    {
        if (whiteKeyMask[(size_t) pitchClass])
        {
            keys[(size_t) pitchClass]->setBounds(whiteIndex * whiteWidth, 0, whiteWidth + 1, whiteHeight);
            ++whiteIndex;
        }
    }

    auto setBlack = [this, blackWidth, blackHeight](int pitchClass, int anchorWhite)
    {
        const auto x = anchorWhite - blackWidth / 2;
        keys[(size_t) pitchClass]->setBounds(x, 0, blackWidth, blackHeight);
        keys[(size_t) pitchClass]->toFront(false);
    };

    setBlack(1, whiteWidth);
    setBlack(3, whiteWidth * 2);
    setBlack(6, whiteWidth * 4);
    setBlack(8, whiteWidth * 5);
    setBlack(10, whiteWidth * 6);
}

void PianoOctaveComponent::timerCallback()
{
}

MIDIKeySnapAudioProcessorEditor::MIDIKeySnapAudioProcessorEditor(MIDIKeySnapAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(760, 360);

    titleLabel.setText("MIDIKeySnap", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, textPrimary);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Define scales as C-based note sets, then apply a post-mapping output transpose in semitones.", juce::dontSendNotification);
    subtitleLabel.setColour(juce::Label::textColourId, textMuted);
    addAndMakeVisible(subtitleLabel);

    scaleLabel.setText("Scale Recall", juce::dontSendNotification);
    scaleLabel.setColour(juce::Label::textColourId, accentAmber);
    addAndMakeVisible(scaleLabel);

    scaleBox.addItem("Custom", customScaleItemId);
    scaleBox.addSeparator();

    bool addedFiveNoteHeading = false;
    bool addedSixNoteHeading = false;
    bool addedSevenNoteHeading = false;

    for (int i = 0; i < (int) ScaleLibrary::getScales().size(); ++i)
    {
        const auto& scale = ScaleLibrary::getScales()[(size_t) i];
        const auto noteCount = (int) scale.semitones.size();

        if (noteCount == 5 && ! addedFiveNoteHeading)
        {
            scaleBox.addSectionHeading("5-Note Scales");
            addedFiveNoteHeading = true;
        }
        else if (noteCount == 6 && ! addedSixNoteHeading)
        {
            scaleBox.addSectionHeading("6-Note Scales");
            addedSixNoteHeading = true;
        }
        else if (noteCount == 7 && ! addedSevenNoteHeading)
        {
            scaleBox.addSectionHeading("7-Note Scales");
            addedSevenNoteHeading = true;
        }

        scaleBox.addItem(formatScaleLabel(scale), firstPresetItemId + i);
    }

    scaleBox.onChange = [this]
    {
        const auto selectedId = scaleBox.getSelectedId();

        if (selectedId >= firstPresetItemId)
            audioProcessor.applyScaleByIndex(selectedId - firstPresetItemId);
    };
    addAndMakeVisible(scaleBox);
    styleComboBox(scaleBox);

    targetOutRootLabel.setText("Output Transpose", juce::dontSendNotification);
    targetOutRootLabel.setColour(juce::Label::textColourId, accentAmber);
    addAndMakeVisible(targetOutRootLabel);

    for (int pitchClass = 0; pitchClass < 12; ++pitchClass)
        targetOutRootBox.addItem(pitchClassName(pitchClass), pitchClass + 1);

    addAndMakeVisible(targetOutRootBox);
    styleComboBox(targetOutRootBox);

    snapModeLabel.setText("Sparse Mode", juce::dontSendNotification);
    snapModeLabel.setColour(juce::Label::textColourId, accentAmber);
    addAndMakeVisible(snapModeLabel);

    snapModeBox.addItemList({ "Wrap", "Clamp Down", "Clamp Up", "Spread", "Random" }, 1);
    styleComboBox(snapModeBox);
    addAndMakeVisible(snapModeBox);

    blackKeyModeLabel.setText("Black Keys", juce::dontSendNotification);
    blackKeyModeLabel.setColour(juce::Label::textColourId, accentAmber);
    addAndMakeVisible(blackKeyModeLabel);

    blackKeyModeBox.addItemList({ "Pass Through", "Block", "Snap Down", "Snap Up", "Nearest" }, 1);
    styleComboBox(blackKeyModeBox);
    addAndMakeVisible(blackKeyModeBox);

    hintLabel.setText("Scales are always defined from C. Output Transpose is applied after note conversion. Black keys can pass, block, or snap musically.", juce::dontSendNotification);
    hintLabel.setColour(juce::Label::textColourId, textMuted);
    addAndMakeVisible(hintLabel);

    statusLabel.setJustificationType(juce::Justification::centredRight);
    statusLabel.setColour(juce::Label::textColourId, accentOrange);
    statusLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(statusLabel);

    piano.onKeyPressed = [this](int pitchClass)
    {
        audioProcessor.toggleSelectedNote(pitchClass);
    };
    addAndMakeVisible(piano);

    snapModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "snapMode", snapModeBox);
    targetOutRootAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "targetOutRoot", targetOutRootBox);
    blackKeyModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "blackKeyMode", blackKeyModeBox);

    audioProcessor.addChangeListener(this);
    startTimerHz(20);
    refreshFromProcessor();
}

MIDIKeySnapAudioProcessorEditor::~MIDIKeySnapAudioProcessorEditor()
{
    audioProcessor.removeChangeListener(this);
}

void MIDIKeySnapAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    juce::ColourGradient gradient(backgroundTop, 0.0f, 0.0f, backgroundBottom, area.getRight(), area.getBottom(), false);
    gradient.addColour(0.68, juce::Colour::fromRGB(24, 27, 30));
    g.setGradientFill(gradient);
    g.fillAll();

    auto shell = area.reduced(16.0f);
    g.setColour(panelOuter);
    g.fillRoundedRectangle(shell, 10.0f);

    g.setColour(panelStroke);
    g.drawRoundedRectangle(shell, 10.0f, 1.0f);

    auto topStrip = shell.removeFromTop(42.0f);
    g.setColour(panelHeader);
    g.fillRoundedRectangle(topStrip, 10.0f);

    g.setColour(accentOrange.withAlpha(0.22f));
    g.fillRect(topStrip.removeFromRight(150.0f).reduced(0.0f, 10.0f));

    auto content = area.reduced(28.0f);
    auto controls = content.removeFromTop(126.0f);
    g.setColour(panelInner.brighter(0.04f));
    g.fillRoundedRectangle(controls, 6.0f);
    g.setColour(panelStroke.withAlpha(0.75f));
    g.drawRoundedRectangle(controls, 6.0f, 1.0f);

    content.removeFromTop(10.0f);
    g.setColour(panelInner);
    g.fillRoundedRectangle(content.removeFromTop(164.0f), 6.0f);
}

void MIDIKeySnapAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(26);

    auto topRow = bounds.removeFromTop(32);
    titleLabel.setBounds(topRow.removeFromLeft(230));
    statusLabel.setBounds(topRow);

    auto subtitleArea = bounds.removeFromTop(26);
    subtitleLabel.setBounds(subtitleArea);

    auto controls = bounds.removeFromTop(92).reduced(10, 10);
    const auto columnWidth = controls.getWidth() / 4;

    auto scaleArea = controls.removeFromLeft(columnWidth).reduced(0, 6);
    scaleLabel.setBounds(scaleArea.removeFromTop(20));
    scaleBox.setBounds(scaleArea.removeFromTop(32));

    auto targetRootArea = controls.removeFromLeft(columnWidth).reduced(10, 6);
    targetOutRootLabel.setBounds(targetRootArea.removeFromTop(20));
    targetOutRootBox.setBounds(targetRootArea.removeFromTop(32));

    auto snapArea = controls.removeFromLeft(columnWidth).reduced(10, 6);
    snapModeLabel.setBounds(snapArea.removeFromTop(20));
    snapModeBox.setBounds(snapArea.removeFromTop(32));

    auto blackKeyArea = controls.removeFromLeft(columnWidth).reduced(10, 6);
    blackKeyModeLabel.setBounds(blackKeyArea.removeFromTop(20));
    blackKeyModeBox.setBounds(blackKeyArea.removeFromTop(32));

    bounds.removeFromTop(10);
    piano.setBounds(bounds.removeFromTop(164).reduced(10, 8));
    bounds.removeFromTop(10);
    hintLabel.setBounds(bounds.removeFromTop(22));
}

void MIDIKeySnapAudioProcessorEditor::refreshFromProcessor()
{
    const auto selection = audioProcessor.getSelectedNotes();
    piano.setSelectedNotes(selection);

    const auto detectedScaleName = audioProcessor.getDetectedScaleName();
    targetOutRootBox.setSelectedId(audioProcessor.getTargetOutRootNote() + 1, juce::dontSendNotification);

    const auto selectedCount = std::count(selection.begin(), selection.end(), true);
    statusLabel.setText("C SCALE   |   +" + juce::String(audioProcessor.getTargetOutRootNote()) + " ST (" + pitchClassName(audioProcessor.getTargetOutRootNote()) + ")" + "   |   " + juce::String(selectedCount) + "/7 NOTES", juce::dontSendNotification);
    auto matchedId = customScaleItemId;

    for (int i = 0; i < (int) ScaleLibrary::getScales().size(); ++i)
    {
        const auto knownName = juce::String(ScaleLibrary::getScales()[(size_t) i].name);

        if (detectedScaleName == knownName)
        {
            matchedId = firstPresetItemId + i;
            break;
        }
    }

    scaleBox.setSelectedId(matchedId, juce::dontSendNotification);
}

void MIDIKeySnapAudioProcessorEditor::timerCallback()
{
    std::array<bool, 12> outputStates {};
    std::array<bool, 12> inputStates {};

    for (int pitchClass = 0; pitchClass < 12; ++pitchClass)
    {
        outputStates[(size_t) pitchClass] = audioProcessor.isOutputPitchClassActive(pitchClass);
        inputStates[(size_t) pitchClass] = audioProcessor.isInputPitchClassActive(pitchClass);
    }

    piano.setOutputActiveStates(outputStates);
    piano.setInputActiveStates(inputStates);
}

void MIDIKeySnapAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == static_cast<juce::ChangeBroadcaster*>(&audioProcessor))
        refreshFromProcessor();
}
