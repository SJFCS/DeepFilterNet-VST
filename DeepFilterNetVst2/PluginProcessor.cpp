#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
constexpr double targetSampleRate = 48000.0;
}

DeepFilterNetVst2AudioProcessor::DeepFilterNetVst2AudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    attenLimDbParam_ = parameters_.getRawParameterValue(attenParamId);
    postFilterBetaParam_ = parameters_.getRawParameterValue(postParamId);
}

DeepFilterNetVst2AudioProcessor::~DeepFilterNetVst2AudioProcessor() = default;

void DeepFilterNetVst2AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine_.setSampleRate(sampleRate);
    engine_.setMaximumBlockSize(samplesPerBlock);
    engine_.prepare();
    setLatencySamples(engine_.getLatencySamples());
}

void DeepFilterNetVst2AudioProcessor::releaseResources()
{
    engine_.release();
    setLatencySamples(0);
}

bool DeepFilterNetVst2AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();

    if (input != output)
        return false;

    return output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo();
}

void DeepFilterNetVst2AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    for (auto channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (attenLimDbParam_ != nullptr && postFilterBetaParam_ != nullptr)
        engine_.updateParameters(attenLimDbParam_->load(), postFilterBetaParam_->load());

    engine_.process(buffer);
    setLatencySamples(engine_.getLatencySamples());
}

juce::AudioProcessorEditor* DeepFilterNetVst2AudioProcessor::createEditor()
{
    return new DeepFilterNetVst2AudioProcessorEditor(*this);
}

bool DeepFilterNetVst2AudioProcessor::hasEditor() const
{
    return true;
}

const juce::String DeepFilterNetVst2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DeepFilterNetVst2AudioProcessor::acceptsMidi() const
{
    return false;
}

bool DeepFilterNetVst2AudioProcessor::producesMidi() const
{
    return false;
}

bool DeepFilterNetVst2AudioProcessor::isMidiEffect() const
{
    return false;
}

double DeepFilterNetVst2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DeepFilterNetVst2AudioProcessor::getNumPrograms()
{
    return 1;
}

int DeepFilterNetVst2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void DeepFilterNetVst2AudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String DeepFilterNetVst2AudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return "Default";
}

void DeepFilterNetVst2AudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void DeepFilterNetVst2AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (const auto xml = parameters_.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void DeepFilterNetVst2AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    const auto xmlState = getXmlFromBinary(data, sizeInBytes);
    if (xmlState == nullptr)
        return;

    if (!xmlState->hasTagName(parameters_.state.getType()))
        return;

    parameters_.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState& DeepFilterNetVst2AudioProcessor::getParametersState()
{
    return parameters_;
}

double DeepFilterNetVst2AudioProcessor::getCurrentSampleRateHz() const
{
    return getSampleRate();
}

bool DeepFilterNetVst2AudioProcessor::isSampleRateCompatible() const
{
    return std::abs(getCurrentSampleRateHz() - targetSampleRate) <= 1.0;
}

bool DeepFilterNetVst2AudioProcessor::isDenoiserReady() const
{
    return engine_.isReady();
}

juce::AudioProcessorValueTreeState::ParameterLayout DeepFilterNetVst2AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(attenParamId, 1),
        "Denoise Strength",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float value, int)
            {
                return juce::String(value, 0) + " dB";
            })));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(postParamId, 1),
        "Post Filter",
        juce::NormalisableRange<float>(0.0f, 0.05f, 0.0005f),
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float value, int)
            {
                return juce::String(value, 3);
            })));

    return { parameters.begin(), parameters.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DeepFilterNetVst2AudioProcessor();
}
