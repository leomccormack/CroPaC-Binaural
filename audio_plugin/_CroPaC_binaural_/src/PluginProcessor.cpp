/*
 ==============================================================================
 
 This file is part of the CroPaC-Binaural
 Copyright (c) 2018 - Leo McCormack.
 
 CroPaC-Binaural is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 CroPaC-Binaural is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with CroPaC-Binaural.  If not, see <http://www.gnu.org/licenses/>.
 
 ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>("channelOrder", "ChannelOrder", juce::StringArray{"ACN", "FuMa"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("normType", "NormType", juce::StringArray{"N3D", "SN3D", "FuMa"}, 1));
    params.push_back(std::make_unique<juce::AudioParameterBool>("enableCroPaC", "EnableCroPaC", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("anaLimit", "AnaLimit", juce::NormalisableRange<float>(HCROPAC_ANA_LIMIT_MIN_VALUE, HCROPAC_ANA_LIMIT_MAX_VALUE, 1.0f), 18e3f, AudioParameterFloatAttributes().withLabel(" Hz")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("covAvgCoeff", "CovAvgCoeff", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.75f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("streamBalance", "StreamBalance", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("enableDiffCorrection", "EnableDiffCorrection", true, AudioParameterBoolAttributes().withAutomatable(false)));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("hrirPreproc", "HRIRPreproc",
                                                                  juce::StringArray{"Off","Diffuse-field EQ","Phase Simplification","EQ & Phase"}, 1,
                                                                  AudioParameterChoiceAttributes().withAutomatable(false)));
    params.push_back(std::make_unique<juce::AudioParameterBool>("enableRotation", "EnableRotation", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("useRollPitchYaw", "UseRollPitchYaw", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("yaw", "Yaw", juce::NormalisableRange<float>(-180.0f, 180.0f, 0.1f), 0.0f,
                                                                 AudioParameterFloatAttributes().withLabel(juce::String::fromUTF8(u8"\u00B0"))));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pitch", "Pitch", juce::NormalisableRange<float>(-180.0f, 180.0f, 0.1f), 0.0f,
                                                                 AudioParameterFloatAttributes().withLabel(juce::String::fromUTF8(u8"\u00B0"))));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("roll", "Roll", juce::NormalisableRange<float>(-180.0f, 180.0f, 0.1f), 0.0f,
                                                                 AudioParameterFloatAttributes().withLabel(juce::String::fromUTF8(u8"\u00B0"))));
    params.push_back(std::make_unique<juce::AudioParameterBool>("flipYaw", "FlipYaw", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("flipPitch", "FlipPitch", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("flipRoll", "FlipRoll", false));
    
    return { params.begin(), params.end() };
}

void PluginProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "channelOrder"){
        hcropaclib_setChOrder(hCroPaC, static_cast<int>(newValue+1.001f));
    }
    else if (parameterID == "normType"){
        hcropaclib_setNormType(hCroPaC, static_cast<int>(newValue+1.001f));
    }
    else if (parameterID == "enableCroPaC"){
        hcropaclib_setEnableCroPaC(hCroPaC, static_cast<int>(newValue+0.5f));
    }
    else if(parameterID == "anaLimit"){
        hcropaclib_setAnaLimit(hCroPaC, newValue);
    }
    else if(parameterID == "covAvgCoeff"){
        hcropaclib_setCovAvg(hCroPaC, newValue);
    }
    else if(parameterID == "streamBalance"){
        hcropaclib_setBalanceAllBands(hCroPaC, newValue);
    }
    else if (parameterID == "enableDiffCorrection"){
        hcropaclib_setEnableDiffCorrection(hCroPaC, static_cast<int>(newValue+0.5f));
    }
    else if (parameterID == "hrirPreproc"){
        hcropaclib_setHRIRsPreProc(hCroPaC, static_cast<HRIR_PREPROC_OPTIONS>(newValue+1.001f));
    }
    else if (parameterID == "enableRotation"){
        hcropaclib_setEnableRotation(hCroPaC, static_cast<int>(newValue+0.5f));
        checkAndUpdateOscStatus();
    }
    else if(parameterID == "useRollPitchYaw"){
        hcropaclib_setRPYflag(hCroPaC, static_cast<int>(newValue+0.5f));
    }
    else if(parameterID == "yaw"){
        hcropaclib_setYaw(hCroPaC, newValue);
    }
    else if(parameterID == "pitch"){
        hcropaclib_setPitch(hCroPaC, newValue);
    }
    else if(parameterID == "roll"){
        hcropaclib_setRoll(hCroPaC, newValue);
    }
    else if(parameterID == "flipYaw"){
        hcropaclib_setFlipYaw(hCroPaC, static_cast<int>(newValue+0.5f));
    }
    else if(parameterID == "flipPitch"){
        hcropaclib_setFlipPitch(hCroPaC, static_cast<int>(newValue+0.5f));
    }
    else if(parameterID == "flipRoll"){
        hcropaclib_setFlipRoll(hCroPaC, static_cast<int>(newValue+0.5f));
    }
}

void PluginProcessor::setParameterValuesUsingInternalState()
{
    setParameterValue("channelOrder", hcropaclib_getChOrder(hCroPaC)-1);
    setParameterValue("normType", hcropaclib_getNormType(hCroPaC)-1);
    setParameterValue("enableCroPaC", hcropaclib_getEnableCroPaC(hCroPaC));
    setParameterValue("anaLimit", hcropaclib_getAnaLimit(hCroPaC));
    setParameterValue("covAvgCoeff", hcropaclib_getCovAvg(hCroPaC));
    setParameterValue("streamBalance", hcropaclib_getBalanceAllBands(hCroPaC));
    setParameterValue("enableDiffCorrection", hcropaclib_getEnableDiffCorrection(hCroPaC));
    setParameterValue("hrirPreproc", hcropaclib_getHRIRsPreProc(hCroPaC)-1);
    setParameterValue("enableRotation", hcropaclib_getEnableRotation(hCroPaC));
    setParameterValue("useRollPitchYaw", hcropaclib_getRPYflag(hCroPaC));
    setParameterValue("yaw", hcropaclib_getYaw(hCroPaC));
    setParameterValue("pitch", hcropaclib_getPitch(hCroPaC));
    setParameterValue("roll", hcropaclib_getRoll(hCroPaC));
    setParameterValue("flipYaw", hcropaclib_getFlipYaw(hCroPaC));
    setParameterValue("flipPitch", hcropaclib_getFlipPitch(hCroPaC));
    setParameterValue("flipRoll", hcropaclib_getFlipRoll(hCroPaC));
    
    checkAndUpdateOscStatus();
}

void PluginProcessor::setInternalStateUsingParameterValues()
{
    hcropaclib_setChOrder(hCroPaC, getParameterChoice("channelOrder")+1);
    hcropaclib_setNormType(hCroPaC, getParameterChoice("normType")+1);
    hcropaclib_setEnableCroPaC(hCroPaC, getParameterBool("enableCroPaC"));
    hcropaclib_setAnaLimit(hCroPaC, getParameterFloat("anaLimit"));
    hcropaclib_setCovAvg(hCroPaC, getParameterFloat("covAvgCoeff"));
    hcropaclib_setBalanceAllBands(hCroPaC, getParameterFloat("streamBalance"));
    hcropaclib_setEnableDiffCorrection(hCroPaC, getParameterBool("enableDiffCorrection"));
    hcropaclib_setHRIRsPreProc(hCroPaC, static_cast<HRIR_PREPROC_OPTIONS>(getParameterChoice("hrirPreproc")+1));
    hcropaclib_setEnableRotation(hCroPaC, getParameterBool("enableRotation"));
    hcropaclib_setRPYflag(hCroPaC, getParameterBool("useRollPitchYaw"));
    hcropaclib_setYaw(hCroPaC, getParameterFloat("yaw"));
    hcropaclib_setPitch(hCroPaC, getParameterFloat("pitch"));
    hcropaclib_setRoll(hCroPaC, getParameterFloat("roll"));
    hcropaclib_setFlipYaw(hCroPaC, getParameterBool("flipYaw"));
    hcropaclib_setFlipPitch(hCroPaC, getParameterBool("flipPitch"));
    hcropaclib_setFlipRoll(hCroPaC, getParameterBool("flipRoll"));
    
    checkAndUpdateOscStatus();
}

PluginProcessor::PluginProcessor():
    AudioProcessor(BusesProperties()
        .withInput("Input", AudioChannelSet::discreteChannels(4), true)
        .withOutput("Output", AudioChannelSet::discreteChannels(2), true)),
    ParameterManager(*this, createParameterLayout())
{
	hcropaclib_create(&hCroPaC);
    
    /* OSC */
    osc.addListener(this);
    
    /* Grab defaults */
    setParameterValuesUsingInternalState();
    
    startTimer(40);
}

PluginProcessor::~PluginProcessor()
{
    if(osc_connected)
        osc.disconnect();
    osc.removeListener(this);
    
	hcropaclib_destroy(&hCroPaC);
}

void PluginProcessor::oscMessageReceived(const OSCMessage& message)
{
    /* if rotation angles are sent as an array \ypr[3] */
    if (message.size() == 3 && message.getAddressPattern().toString().compare("/ypr")==0) {
        if (message[0].isFloat32()){
            setParameterValue("yaw", message[0].getFloat32());
        }
        if (message[1].isFloat32()){
            setParameterValue("pitch", message[1].getFloat32());
        }
        if (message[2].isFloat32()){
            setParameterValue("roll", message[2].getFloat32());
        }
        return;
    }
    
    /* if rotation angles are sent individually: */
    if(message.getAddressPattern().toString().compare("/yaw")==0){
        setParameterValue("yaw", message[0].getFloat32());
    }
    else if(message.getAddressPattern().toString().compare("/pitch")==0){
        setParameterValue("pitch", message[0].getFloat32());
    }
    else if(message.getAddressPattern().toString().compare("/roll")==0){
        setParameterValue("roll", message[0].getFloat32());
    }
}

void PluginProcessor::setCurrentProgram (int /*index*/)
{
}

const String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 0;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

const String PluginProcessor::getProgramName (int /*index*/)
{
    return String();
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

void PluginProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    nHostBlockSize = samplesPerBlock;
    nNumInputs =  jmin(getTotalNumInputChannels(), 256);
    nNumOutputs = jmin(getTotalNumOutputChannels(), 256);
    nSampleRate = (int)(sampleRate + 0.5);

	hcropaclib_init(hCroPaC, nSampleRate);
    AudioProcessor::setLatencySamples(hcropaclib_getProcessingDelay());
}

void PluginProcessor::releaseResources()
{
}

void PluginProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& /*midiMessages*/)
{
    ScopedNoDenormals noDenormals;
    
    int nCurrentBlockSize = nHostBlockSize = buffer.getNumSamples();
    nNumInputs = jmin(getTotalNumInputChannels(), buffer.getNumChannels(), 256);
    nNumOutputs = jmin(getTotalNumOutputChannels(), buffer.getNumChannels(), 256);
    float* const* bufferData = buffer.getArrayOfWritePointers();
    float* pFrameData[256];
    int framesize = hcropaclib_getFrameSize();
    
    if(nCurrentBlockSize % framesize == 0) { /* divisible by frame size */
        for(int frame = 0; frame < nCurrentBlockSize/framesize; frame++) {
            for(int ch = 0; ch < jmin(buffer.getNumChannels(), 256); ch++)
                pFrameData[ch] = &bufferData[ch][frame*framesize];
            
            /* perform processing */
            hcropaclib_process(hCroPaC, pFrameData, pFrameData, nNumInputs, nNumOutputs, framesize);
        }
    }
    else
        buffer.clear();
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; 
}

AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (MemoryBlock& destData)
{
    juce::ValueTree state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setTagName("HCROPACPLUGINSETTINGS");
    xml->setAttribute("VersionCode", JucePlugin_VersionCode); // added since 0x10405
    
    /* Now for the other DSP object parameters (that have no JUCE parameter counterpart) */
    for(int band=0; band<hcropaclib_getNumberOfBands(); band++){
        xml->setAttribute("Balance"+String(band), hcropaclib_getBalance(hCroPaC, band));
    }
    xml->setAttribute("UseDefaultHRIRset", hcropaclib_getUseDefaultHRIRsflag(hCroPaC));
    if(!hcropaclib_getUseDefaultHRIRsflag(hCroPaC))
        xml->setAttribute("SofaFilePath", String(hcropaclib_getSofaFilePath(hCroPaC)));
    
    /* Other */
    xml->setAttribute("OSC_PORT", osc_port_ID);

    /* Save */
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    /* Load */
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName("HCROPACPLUGINSETTINGS")){
        if(!xmlState->hasAttribute("VersionCode")){ // pre-0x10405
            if(xmlState->hasAttribute("EnableCroPaC"))
                hcropaclib_setEnableCroPaC(hCroPaC, xmlState->getIntAttribute("EnableCroPaC", 1));
            
            for(int band=0; band<hcropaclib_getNumberOfBands(); band++){
                if(xmlState->hasAttribute("Balance"+String(band)))
                    hcropaclib_setBalance(hCroPaC, (float)xmlState->getDoubleAttribute("Balance"+String(band),0), band);
            }
            if(xmlState->hasAttribute("CovAVG"))
                hcropaclib_setCovAvg(hCroPaC, (float)xmlState->getDoubleAttribute("CovAVG", 0.5));
            if(xmlState->hasAttribute("AnaLim"))
                hcropaclib_setAnaLimit(hCroPaC, (float)xmlState->getDoubleAttribute("AnaLim", 10e3));
            
            if(xmlState->hasAttribute("UseDefaultHRIRset"))
                hcropaclib_setUseDefaultHRIRsflag(hCroPaC, xmlState->getIntAttribute("UseDefaultHRIRset", 1));
            if(xmlState->hasAttribute("Norm"))
                hcropaclib_setNormType(hCroPaC, xmlState->getIntAttribute("Norm", 1));
            if(xmlState->hasAttribute("ChOrder"))
                hcropaclib_setChOrder(hCroPaC, xmlState->getIntAttribute("ChOrder", 1));
            if(xmlState->hasAttribute("maxrE"))
                hcropaclib_setEnableDiffCorrection(hCroPaC,xmlState->getIntAttribute("maxrE", 1));
        
            if(xmlState->hasAttribute("ENABLEROT"))
                hcropaclib_setEnableRotation(hCroPaC, xmlState->getIntAttribute("ENABLEROT", 0));
            if(xmlState->hasAttribute("YAW"))
                hcropaclib_setYaw(hCroPaC, (float)xmlState->getDoubleAttribute("YAW", 0.0f));
            if(xmlState->hasAttribute("PITCH"))
                hcropaclib_setPitch(hCroPaC, (float)xmlState->getDoubleAttribute("PITCH", 0.0f));
            if(xmlState->hasAttribute("ROLL"))
                hcropaclib_setRoll(hCroPaC, (float)xmlState->getDoubleAttribute("ROLL", 0.0f));
            if(xmlState->hasAttribute("FLIP_YAW"))
                hcropaclib_setFlipYaw(hCroPaC, xmlState->getIntAttribute("FLIP_YAW", 0));
            if(xmlState->hasAttribute("FLIP_PITCH"))
                hcropaclib_setFlipPitch(hCroPaC, xmlState->getIntAttribute("FLIP_PITCH", 0));
            if(xmlState->hasAttribute("FLIP_ROLL"))
                hcropaclib_setFlipRoll(hCroPaC, xmlState->getIntAttribute("FLIP_ROLL", 0));
            if(xmlState->hasAttribute("RPY_FLAG"))
                hcropaclib_setRPYflag(hCroPaC, xmlState->getIntAttribute("RPY_FLAG", 0));
            
            if(xmlState->hasAttribute("OSC_PORT")){
                osc_port_ID = xmlState->getIntAttribute("OSC_PORT", DEFAULT_OSC_PORT);
                osc.connect(osc_port_ID);
            }
            
            if(xmlState->hasAttribute("SofaFilePath")){
                String directory = xmlState->getStringAttribute("SofaFilePath", "no_file");
                const char* new_cstring = (const char*)directory.toUTF8();
                hcropaclib_setSofaFilePath(hCroPaC, new_cstring);
            }
            
            setParameterValuesUsingInternalState();
        }
        else if(xmlState->getIntAttribute("VersionCode")>=0x10405){
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

            /* Now for the other DSP object parameters (that have no JUCE parameter counterpart) */
            if(xmlState->hasAttribute("UseDefaultHRIRset"))
                hcropaclib_setUseDefaultHRIRsflag(hCroPaC, xmlState->getIntAttribute("UseDefaultHRIRset", 1));
            if(xmlState->hasAttribute("SofaFilePath")){
                String directory = xmlState->getStringAttribute("SofaFilePath", "no_file");
                const char* new_cstring = (const char*)directory.toUTF8();
                hcropaclib_setSofaFilePath(hCroPaC, new_cstring);
            }
            
            /* Other */
            if(xmlState->hasAttribute("OSC_PORT")){
                osc_port_ID = xmlState->getIntAttribute("OSC_PORT", DEFAULT_OSC_PORT);
                osc.connect(osc_port_ID);
            }
            
            /* Many hosts will also trigger parameterChanged() for all parameters after calling setStateInformation() */
            /* However, some hosts do not. Therefore, it is better to ensure that the internal state is always up-to-date by calling: */
            setInternalStateUsingParameterValues();
        }
        
        hcropaclib_refreshParams(hCroPaC);
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}

