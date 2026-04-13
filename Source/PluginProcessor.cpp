#include "PluginProcessor.h"

#include <limits>

#include "PluginEditor.h"
#include "ScaleLibrary.h"

namespace
{
constexpr std::array<int, 7> whiteKeyPitchClasses { 0, 2, 4, 5, 7, 9, 11 };
constexpr auto stateType = "MIDIKeySnapState";

juce::ValueTree makeSelectionTree(const std::array<bool, 12>& selection)
{
    juce::ValueTree notes("SelectedNotes");

    for (int i = 0; i < 12; ++i)
        notes.setProperty("pc" + juce::String(i), selection[(size_t) i], nullptr);

    return notes;
}

std::array<bool, 12> readSelectionTree(const juce::ValueTree& tree)
{
    std::array<bool, 12> selection {};

    for (int i = 0; i < 12; ++i)
        selection[(size_t) i] = (bool) tree.getProperty("pc" + juce::String(i), false);

    return selection;
}
} // namespace

MIDIKeySnapAudioProcessor::MIDIKeySnapAudioProcessor()
    : AudioProcessor(BusesProperties()),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    activeMappings.fill(-1);
    activeDisplayMappings.fill(-1);
    activeOutputPitchClassCounts.fill(0);
    activeInputPitchClassCounts.fill(0);
}

void MIDIKeySnapAudioProcessor::prepareToPlay(double, int)
{
}

void MIDIKeySnapAudioProcessor::releaseResources()
{
}

bool MIDIKeySnapAudioProcessor::isBusesLayoutSupported(const BusesLayout&) const
{
    return true;
}

void MIDIKeySnapAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const juce::ScopedLock lock(stateLock);
    const auto degreeMap = buildDegreeMap();
    const auto targetOutRoot = getTargetOutRootNote();
    const auto useRandomMode = getSnapMode() == SnapMode::random && getSelectedPitchClassesSorted().size() < 7;

    juce::MidiBuffer processed;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();

        if (message.isNoteOnOrOff())
        {
            const auto inputNote = message.getNoteNumber();
            auto outputNote = inputNote;
            auto displayedNote = inputNote;

            if (message.isNoteOn())
            {
                ++activeInputPitchClassCounts[(size_t) wrapPitchClass(inputNote)];
                bool shouldBlock = false;

                if (isWhiteKey(inputNote))
                    outputNote = useRandomMode ? mapRandomPitch(inputNote)
                                               : mapWhiteKeyPitch(inputNote, degreeMap);
                else
                    outputNote = mapBlackKeyPitch(inputNote, shouldBlock);

                if (shouldBlock)
                {
                    activeMappings[(size_t) inputNote] = -2;
                    activeDisplayMappings[(size_t) inputNote] = -1;
                    activeInputPitchClassCounts[(size_t) wrapPitchClass(inputNote)] = juce::jmax(0, activeInputPitchClassCounts[(size_t) wrapPitchClass(inputNote)] - 1);
                    continue;
                }

                displayedNote = outputNote;
                outputNote = juce::jlimit(0, 127, outputNote + targetOutRoot);
                activeMappings[(size_t) inputNote] = outputNote;
                activeDisplayMappings[(size_t) inputNote] = wrapPitchClass(displayedNote);
                ++activeOutputPitchClassCounts[(size_t) wrapPitchClass(displayedNote)];
                message.setNoteNumber(outputNote);
            }
            else if (message.isNoteOff())
            {
                const auto inputPitchClass = wrapPitchClass(inputNote);
                activeInputPitchClassCounts[(size_t) inputPitchClass] = juce::jmax(0, activeInputPitchClassCounts[(size_t) inputPitchClass] - 1);

                if (const auto mapped = activeMappings[(size_t) inputNote]; mapped >= 0)
                {
                    outputNote = mapped;
                    activeMappings[(size_t) inputNote] = -1;

                    if (const auto displayedPitchClass = activeDisplayMappings[(size_t) inputNote]; displayedPitchClass >= 0)
                    {
                        activeOutputPitchClassCounts[(size_t) displayedPitchClass] = juce::jmax(0, activeOutputPitchClassCounts[(size_t) displayedPitchClass] - 1);
                        activeDisplayMappings[(size_t) inputNote] = -1;
                    }
                }
                else if (mapped == -2)
                {
                    activeMappings[(size_t) inputNote] = -1;
                    activeDisplayMappings[(size_t) inputNote] = -1;
                    continue;
                }
                else
                {
                    outputNote = juce::jlimit(0, 127, inputNote + targetOutRoot);
                }

                message.setNoteNumber(outputNote);
            }
        }

        processed.addEvent(message, metadata.samplePosition);
    }

    midiMessages.swapWith(processed);
}

juce::AudioProcessorEditor* MIDIKeySnapAudioProcessor::createEditor()
{
    return new MIDIKeySnapAudioProcessorEditor(*this);
}

bool MIDIKeySnapAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String MIDIKeySnapAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MIDIKeySnapAudioProcessor::acceptsMidi() const
{
    return true;
}

bool MIDIKeySnapAudioProcessor::producesMidi() const
{
    return true;
}

bool MIDIKeySnapAudioProcessor::isMidiEffect() const
{
    return true;
}

double MIDIKeySnapAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MIDIKeySnapAudioProcessor::getNumPrograms()
{
    return 1;
}

int MIDIKeySnapAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MIDIKeySnapAudioProcessor::setCurrentProgram(int)
{
}

const juce::String MIDIKeySnapAudioProcessor::getProgramName(int)
{
    return {};
}

void MIDIKeySnapAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void MIDIKeySnapAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state(stateType);
    state.addChild(apvts.copyState(), -1, nullptr);

    {
        const juce::ScopedLock lock(stateLock);
        state.addChild(makeSelectionTree(selectedNotes), -1, nullptr);
        state.setProperty("currentScaleName", currentScaleName, nullptr);
    }

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MIDIKeySnapAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml(*xml);

        if (state.hasType(stateType))
        {
            if (const auto params = state.getChildWithName(apvts.state.getType()); params.isValid())
                apvts.replaceState(params);

            const juce::ScopedLock lock(stateLock);

            if (const auto notes = state.getChildWithName("SelectedNotes"); notes.isValid())
                selectedNotes = readSelectionTree(notes);

            currentScaleName = state.getProperty("currentScaleName", currentScaleName).toString();
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout MIDIKeySnapAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "snapMode",
        "Snap Mode",
        juce::StringArray { "Wrap", "Clamp Down", "Clamp Up", "Spread", "Random" },
        0));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "targetOutRoot",
        "Output Transpose",
        0,
        11,
        0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "blackKeyMode",
        "Black Key Mode",
        juce::StringArray { "Pass Through", "Block", "Snap Down", "Snap Up", "Nearest" },
        0));

    return { params.begin(), params.end() };
}

std::array<bool, 12> MIDIKeySnapAudioProcessor::getSelectedNotes() const
{
    const juce::ScopedLock lock(stateLock);
    return selectedNotes;
}

void MIDIKeySnapAudioProcessor::setSelectedNotes(const std::array<bool, 12>& newSelection)
{
    const auto selectedCount = std::count(newSelection.begin(), newSelection.end(), true);

    if (selectedCount == 0 || selectedCount > 7)
        return;

    const auto match = findMatchingScale(newSelection);

    {
        const juce::ScopedLock lock(stateLock);
        selectedNotes = newSelection;
        currentScaleName = match.isValid() ? match.scaleName : "Custom";
    }

    sendChangeMessage();
}

bool MIDIKeySnapAudioProcessor::toggleSelectedNote(int pitchClass)
{
    if (! juce::isPositiveAndBelow(pitchClass, 12))
        return false;

    auto updated = getSelectedNotes();
    updated[(size_t) pitchClass] = ! updated[(size_t) pitchClass];
    const auto selectedCount = std::count(updated.begin(), updated.end(), true);

    if (selectedCount == 0 || selectedCount > 7)
        return false;

    setSelectedNotes(updated);
    return true;
}

void MIDIKeySnapAudioProcessor::applyScaleByIndex(int scaleIndex)
{
    if (! juce::isPositiveAndBelow(scaleIndex, (int) ScaleLibrary::getScales().size()))
        return;

    const auto& scale = ScaleLibrary::getScales()[(size_t) scaleIndex];
    std::array<bool, 12> selection {};

    for (const auto semitone : scale.semitones)
        selection[(size_t) wrapPitchClass(semitone)] = true;

    {
        const juce::ScopedLock lock(stateLock);
        selectedNotes = selection;
        currentScaleName = scale.name;
    }

    sendChangeMessage();
}

void MIDIKeySnapAudioProcessor::applyScaleByName(const juce::String& scaleName)
{
    for (int i = 0; i < (int) ScaleLibrary::getScales().size(); ++i)
    {
        if (scaleName == juce::String(ScaleLibrary::getScales()[(size_t) i].name))
        {
            applyScaleByIndex(i);
            return;
        }
    }
}

juce::String MIDIKeySnapAudioProcessor::getCurrentScaleName() const
{
    const juce::ScopedLock lock(stateLock);
    return currentScaleName;
}

juce::String MIDIKeySnapAudioProcessor::getDetectedScaleName() const
{
    const juce::ScopedLock lock(stateLock);
    return findMatchingScale(selectedNotes).scaleName;
}

juce::String MIDIKeySnapAudioProcessor::getSelectedNotesDebugString() const
{
    const juce::ScopedLock lock(stateLock);
    juce::StringArray parts;

    for (int pitchClass = 0; pitchClass < 12; ++pitchClass)
    {
        if (selectedNotes[(size_t) pitchClass])
            parts.add(juce::String(pitchClass));
    }

    return parts.joinIntoString(", ");
}

int MIDIKeySnapAudioProcessor::getTargetOutRootNote() const
{
    return juce::roundToInt(apvts.getRawParameterValue("targetOutRoot")->load());
}

MIDIKeySnapAudioProcessor::SnapMode MIDIKeySnapAudioProcessor::getSnapMode() const
{
    return static_cast<SnapMode>((int) apvts.getRawParameterValue("snapMode")->load());
}

MIDIKeySnapAudioProcessor::BlackKeyMode MIDIKeySnapAudioProcessor::getBlackKeyMode() const
{
    return static_cast<BlackKeyMode>((int) apvts.getRawParameterValue("blackKeyMode")->load());
}

bool MIDIKeySnapAudioProcessor::isOutputPitchClassActive(int pitchClass) const
{
    if (! juce::isPositiveAndBelow(pitchClass, 12))
        return false;

    const juce::ScopedLock lock(stateLock);
    return activeOutputPitchClassCounts[(size_t) pitchClass] > 0;
}

bool MIDIKeySnapAudioProcessor::isInputPitchClassActive(int pitchClass) const
{
    if (! juce::isPositiveAndBelow(pitchClass, 12))
        return false;

    const juce::ScopedLock lock(stateLock);
    return activeInputPitchClassCounts[(size_t) pitchClass] > 0;
}

bool MIDIKeySnapAudioProcessor::isWhiteKey(int pitchClass)
{
    return std::find(whiteKeyPitchClasses.begin(), whiteKeyPitchClasses.end(), wrapPitchClass(pitchClass)) != whiteKeyPitchClasses.end();
}

int MIDIKeySnapAudioProcessor::wrapPitchClass(int pitchClass)
{
    return (pitchClass % 12 + 12) % 12;
}

juce::String MIDIKeySnapAudioProcessor::selectionToSignature(const std::array<bool, 12>& selection)
{
    juce::StringArray parts;

    for (int pitchClass = 0; pitchClass < 12; ++pitchClass)
    {
        if (selection[(size_t) pitchClass])
            parts.add(juce::String(pitchClass));
    }

    return parts.joinIntoString(",");
}

std::vector<int> MIDIKeySnapAudioProcessor::getSelectedPitchClassesSorted() const
{
    std::vector<int> selected;

    for (int i = 0; i < 12; ++i)
    {
        if (selectedNotes[(size_t) i])
            selected.push_back(i);
    }

    return selected;
}

MIDIKeySnapAudioProcessor::ScaleMatch MIDIKeySnapAudioProcessor::findMatchingScale(const std::array<bool, 12>& selection) const
{
    const auto selectionSignature = selectionToSignature(selection);

    for (const auto& scale : ScaleLibrary::getScales())
    {
        std::array<bool, 12> candidate {};

        for (const auto semitone : scale.semitones)
            candidate[(size_t) wrapPitchClass(semitone)] = true;

        if (selectionToSignature(candidate) == selectionSignature)
            return { scale.name };
    }

    return {};
}

std::array<int, 7> MIDIKeySnapAudioProcessor::buildDegreeMap() const
{
    const auto selected = getSelectedPitchClassesSorted();
    std::array<int, 7> map {};

    if (selected.empty())
    {
        map = whiteKeyPitchClasses;
        return map;
    }

    if (selected.size() >= 7)
    {
        for (int i = 0; i < 7; ++i)
            map[(size_t) i] = selected[(size_t) i];

        return map;
    }

    switch (getSnapMode())
    {
        case SnapMode::wrap:
            for (int i = 0; i < 7; ++i)
                map[(size_t) i] = selected[(size_t) (i % (int) selected.size())];
            break;

        case SnapMode::clampDown:
        {
            for (int i = 0; i < 7; ++i)
            {
                const auto sourceIndex = juce::jmin((int) selected.size() - 1,
                                                    (i * (int) selected.size()) / 7);
                map[(size_t) i] = selected[(size_t) sourceIndex];
            }
            break;
        }

        case SnapMode::clampUp:
        {
            for (int i = 0; i < 7; ++i)
            {
                const auto sourceIndex = juce::jmax(0,
                                                    ((i + 1) * (int) selected.size() + 6) / 7 - 1);
                map[(size_t) i] = selected[(size_t) sourceIndex];
            }
            break;
        }

        case SnapMode::spread:
        {
            if (selected.size() == 1)
            {
                map.fill(selected.front());
                break;
            }

            for (int i = 0; i < 7; ++i)
            {
                const auto position = (double) i * (double) (selected.size() - 1) / 6.0;
                const auto sourceIndex = juce::roundToInt(position);
                map[(size_t) i] = selected[(size_t) sourceIndex];
            }
            break;
        }

        case SnapMode::random:
            for (int i = 0; i < 7; ++i)
                map[(size_t) i] = selected[(size_t) (i % (int) selected.size())];
            break;
    }

    return map;
}

int MIDIKeySnapAudioProcessor::mapWhiteKeyPitch(int midiNote, const std::array<int, 7>& degreeMap) const
{
    const auto pitchClass = wrapPitchClass(midiNote);

    if (! isWhiteKey(pitchClass))
        return midiNote;

    int degreeIndex = 0;

    for (int i = 0; i < 7; ++i)
    {
        if (whiteKeyPitchClasses[(size_t) i] == pitchClass)
        {
            degreeIndex = i;
            break;
        }
    }

    const auto octave = midiNote / 12;
    auto mapped = octave * 12 + degreeMap[(size_t) degreeIndex];

    while (mapped - midiNote > 6)
        mapped -= 12;

    while (midiNote - mapped > 6)
        mapped += 12;

    return juce::jlimit(0, 127, mapped);
}

int MIDIKeySnapAudioProcessor::mapRandomPitch(int midiNote)
{
    const auto pitchClass = wrapPitchClass(midiNote);

    if (! isWhiteKey(pitchClass))
        return midiNote;

    const auto selected = getSelectedPitchClassesSorted();

    if (selected.empty())
        return midiNote;

    int degreeIndex = 0;

    for (int i = 0; i < 7; ++i)
    {
        if (whiteKeyPitchClasses[(size_t) i] == pitchClass)
        {
            degreeIndex = i;
            break;
        }
    }

    if (degreeIndex < (int) selected.size())
    {
        const auto octave = midiNote / 12;
        auto mapped = octave * 12 + selected[(size_t) degreeIndex];

        while (mapped - midiNote > 6)
            mapped -= 12;

        while (midiNote - mapped > 6)
            mapped += 12;

        return juce::jlimit(0, 127, mapped);
    }

    int chosenPitchClass = selected.front();

    if (selected.size() > 1)
    {
        std::vector<int> candidates;
        candidates.reserve(selected.size());

        for (const auto selectedPitchClass : selected)
        {
            if (selectedPitchClass != lastRandomPitchClass)
                candidates.push_back(selectedPitchClass);
        }

        if (candidates.empty())
            candidates = selected;

        chosenPitchClass = candidates[(size_t) rng.nextInt((int) candidates.size())];
    }

    lastRandomPitchClass = chosenPitchClass;

    const auto octave = midiNote / 12;
    auto mapped = octave * 12 + chosenPitchClass;

    while (mapped - midiNote > 6)
        mapped -= 12;

    while (midiNote - mapped > 6)
        mapped += 12;

    return juce::jlimit(0, 127, mapped);
}

int MIDIKeySnapAudioProcessor::mapBlackKeyPitch(int midiNote, bool& shouldBlock) const
{
    shouldBlock = false;

    const auto mode = getBlackKeyMode();

    if (mode == BlackKeyMode::passThrough)
        return midiNote;

    if (mode == BlackKeyMode::block)
    {
        shouldBlock = true;
        return midiNote;
    }

    const auto selected = getSelectedPitchClassesSorted();

    if (selected.empty())
        return midiNote;

    const auto inputPitchClass = wrapPitchClass(midiNote);
    const auto octave = midiNote / 12;

    auto makeCandidate = [octave](int pitchClass, int octaveOffset)
    {
        return (octave + octaveOffset) * 12 + pitchClass;
    };

    if (mode == BlackKeyMode::snapDown)
    {
        int best = midiNote;
        bool found = false;

        for (int octaveOffset = 0; octaveOffset >= -1; --octaveOffset)
        {
            for (auto it = selected.rbegin(); it != selected.rend(); ++it)
            {
                const auto candidate = makeCandidate(*it, octaveOffset);
                if (candidate <= midiNote)
                {
                    best = candidate;
                    found = true;
                    break;
                }
            }

            if (found)
                break;
        }

        return juce::jlimit(0, 127, best);
    }

    if (mode == BlackKeyMode::snapUp)
    {
        int best = midiNote;
        bool found = false;

        for (int octaveOffset = 0; octaveOffset <= 1; ++octaveOffset)
        {
            for (const auto selectedPitchClass : selected)
            {
                const auto candidate = makeCandidate(selectedPitchClass, octaveOffset);
                if (candidate >= midiNote)
                {
                    best = candidate;
                    found = true;
                    break;
                }
            }

            if (found)
                break;
        }

        return juce::jlimit(0, 127, best);
    }

    int best = midiNote;
    int bestDistance = std::numeric_limits<int>::max();

    for (int octaveOffset = -1; octaveOffset <= 1; ++octaveOffset)
    {
        for (const auto selectedPitchClass : selected)
        {
            const auto candidate = makeCandidate(selectedPitchClass, octaveOffset);
            const auto distance = std::abs(candidate - midiNote);

            if (distance < bestDistance || (distance == bestDistance && candidate < best))
            {
                best = candidate;
                bestDistance = distance;
            }
        }
    }

    juce::ignoreUnused(inputPitchClass);
    return juce::jlimit(0, 127, best);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MIDIKeySnapAudioProcessor();
}
