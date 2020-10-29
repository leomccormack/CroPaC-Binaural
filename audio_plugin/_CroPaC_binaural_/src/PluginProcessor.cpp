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

PluginProcessor::PluginProcessor():
    AudioProcessor(BusesProperties()
    .withInput("Input", AudioChannelSet::discreteChannels(64), true)
               .withOutput("Output", AudioChannelSet::discreteChannels(64), true))
{
	hcropaclib_create(&hCroPaC);
    
    /* specify here on which UDP port number to receive incoming OSC messages */
    osc_port_ID = DEFAULT_OSC_PORT;
    osc.connect(osc_port_ID);
    /* tell the component to listen for OSC messages */
    osc.addListener(this);
    
    startTimer(TIMER_PROCESSING_RELATED, 40);
}

PluginProcessor::~PluginProcessor()
{
    osc.disconnect();
    osc.removeListener(this);
    
	hcropaclib_destroy(&hCroPaC);
}

void PluginProcessor::oscMessageReceived(const OSCMessage& message)
{
    /* if rotation angles are sent as an array \ypr[3] */
    if (message.size() == 3 && message.getAddressPattern().toString().compare("ypr")) {
        if (message[0].isFloat32())
            hcropaclib_setYaw(hCroPaC, message[0].getFloat32());
        if (message[1].isFloat32())
            hcropaclib_setPitch(hCroPaC, message[1].getFloat32());
        if (message[2].isFloat32())
            hcropaclib_setRoll(hCroPaC, message[2].getFloat32());
        return;
    }
    
    /* if rotation angles are sent individually: */
    if(message.getAddressPattern().toString().compare("yaw"))
        hcropaclib_setYaw(hCroPaC, message[0].getFloat32());
    else if(message.getAddressPattern().toString().compare("pitch"))
        hcropaclib_setPitch(hCroPaC, message[0].getFloat32());
    else if(message.getAddressPattern().toString().compare("roll"))
        hcropaclib_setRoll(hCroPaC, message[0].getFloat32());
}

void PluginProcessor::setParameter (int index, float newValue)
{
    switch (index) {
        case k_channelOrder:    hcropaclib_setChOrder(hCroPaC, (int)(newValue*(float)(HCROPAC_NUM_CH_ORDERINGS-1) + 1.5f)); break;
        case k_normType:        hcropaclib_setNormType(hCroPaC, (int)(newValue*(float)(HCROPAC_NUM_NORM_TYPES-1) + 1.5f)); break;
        case k_enableCroPaC:    hcropaclib_setEnableCroPaC(hCroPaC, (int)(newValue + 0.5f)); break;
        case k_AnaLimit:        hcropaclib_setAnaLimit(hCroPaC, newValue*(HCROPAC_ANA_LIMIT_MAX_VALUE-HCROPAC_ANA_LIMIT_MIN_VALUE)+HCROPAC_ANA_LIMIT_MIN_VALUE); break;
        case k_covAvgCoeff:     hcropaclib_setCovAvg(hCroPaC, newValue); break;
        case k_balance:         hcropaclib_setBalanceAllBands(hCroPaC, newValue*2.0f); break;
        case k_enableMaxRE:     hcropaclib_setEnableDiffCorrection(hCroPaC, (int)(newValue + 0.5f)); break;
        case k_enableRotation:  hcropaclib_setEnableRotation(hCroPaC, (int)(newValue + 0.5f)); break;
        case k_useRollPitchYaw: hcropaclib_setRPYflag(hCroPaC, (int)(newValue + 0.5f)); break;
        case k_yaw:             hcropaclib_setYaw(hCroPaC, (newValue-0.5f)*360.0f ); break;
        case k_pitch:           hcropaclib_setPitch(hCroPaC, (newValue - 0.5f)*180.0f); break;
        case k_roll:            hcropaclib_setRoll(hCroPaC, (newValue - 0.5f)*180.0f); break;
        case k_flipYaw:         hcropaclib_setFlipYaw(hCroPaC, (int)(newValue + 0.5f)); break;
        case k_flipPitch:       hcropaclib_setFlipPitch(hCroPaC, (int)(newValue + 0.5f)); break;
        case k_flipRoll:        hcropaclib_setFlipRoll(hCroPaC, (int)(newValue + 0.5f)); break;
            
        default: break;
    }
}

void PluginProcessor::setCurrentProgram (int /*index*/)
{
}

float PluginProcessor::getParameter (int index)
{
    switch (index) {
        case k_channelOrder:     return (float)(hcropaclib_getChOrder(hCroPaC)-1)/(float)(HCROPAC_NUM_CH_ORDERINGS-1);
        case k_normType:         return (float)(hcropaclib_getNormType(hCroPaC)-1)/(float)(HCROPAC_NUM_NORM_TYPES-1);
        case k_enableCroPaC:     return (float)hcropaclib_getEnableCroPaC(hCroPaC);
        case k_AnaLimit:         return (hcropaclib_getAnaLimit(hCroPaC)-HCROPAC_ANA_LIMIT_MIN_VALUE)/(HCROPAC_ANA_LIMIT_MAX_VALUE-HCROPAC_ANA_LIMIT_MIN_VALUE);
        case k_covAvgCoeff:      return hcropaclib_getCovAvg(hCroPaC);
        case k_balance:          return hcropaclib_getBalanceAllBands(hCroPaC)/2.0f;
        case k_enableMaxRE:      return (float)hcropaclib_getEnableDiffCorrection(hCroPaC);
        case k_enableRotation:   return (float)hcropaclib_getEnableRotation(hCroPaC);
        case k_useRollPitchYaw:  return (float)hcropaclib_getRPYflag(hCroPaC);
        case k_yaw:              return (hcropaclib_getYaw(hCroPaC)/360.0f) + 0.5f;
        case k_pitch:            return (hcropaclib_getPitch(hCroPaC)/180.0f) + 0.5f;
        case k_roll:             return (hcropaclib_getRoll(hCroPaC)/180.0f) + 0.5f;
        case k_flipYaw:          return (float)hcropaclib_getFlipYaw(hCroPaC);
        case k_flipPitch:        return (float)hcropaclib_getFlipPitch(hCroPaC);
        case k_flipRoll:         return (float)hcropaclib_getFlipRoll(hCroPaC);
        default: return 0.0f;
    }
}

int PluginProcessor::getNumParameters()
{
	return k_NumOfParameters;
}

const String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

const String PluginProcessor::getParameterName (int index)
{
    switch (index) {
        case k_channelOrder:    return "channel_order";
        case k_normType:        return "norm_type";
        case k_enableCroPaC:    return "enable_cropac";
        case k_AnaLimit:        return "analysis_limit";
        case k_covAvgCoeff:     return "cov_avg";
        case k_balance:         return "balance (diff-dir)";
        case k_enableMaxRE:     return "diffuseCorrection";
        case k_enableRotation:  return "enable_rotation";
        case k_useRollPitchYaw: return "use_rpy";
        case k_yaw:             return "yaw";
        case k_pitch:           return "pitch";
        case k_roll:            return "roll";
        case k_flipYaw:         return "flip_yaw";
        case k_flipPitch:       return "flip_pitch";
        case k_flipRoll:        return "flip_roll";
        default: return "NULL";
    }
}

const String PluginProcessor::getParameterText(int index)
{
    switch (index) {
        case k_channelOrder:
            switch(hcropaclib_getChOrder(hCroPaC)){
                case CH_ACN:  return "ACN";
                case CH_FUMA: return "FuMa";
                default: return "NULL";
            }
        case k_normType:
            switch(hcropaclib_getNormType(hCroPaC)){
                case NORM_N3D:  return "N3D";
                case NORM_SN3D: return "SN3D";
                case NORM_FUMA: return "FuMa";
                default: return "NULL";
            }
        case k_enableCroPaC:    return !hcropaclib_getEnableCroPaC(hCroPaC) ? "Off" : "On";
        case k_AnaLimit:        return String(hcropaclib_getAnaLimit(hCroPaC)) + " Hz";
        case k_covAvgCoeff:     return String(hcropaclib_getCovAvg(hCroPaC));
        case k_balance:         return String(hcropaclib_getBalanceAllBands(hCroPaC));
        case k_enableMaxRE:     return !hcropaclib_getEnableDiffCorrection(hCroPaC) ? "Off" : "On";
        case k_enableRotation:  return !hcropaclib_getEnableRotation(hCroPaC) ? "Off" : "On";
        case k_useRollPitchYaw: return !hcropaclib_getRPYflag(hCroPaC) ? "YPR" : "RPY";
        case k_yaw:             return String(hcropaclib_getYaw(hCroPaC));
        case k_pitch:           return String(hcropaclib_getPitch(hCroPaC));
        case k_roll:            return String(hcropaclib_getRoll(hCroPaC));
        case k_flipYaw:         return !hcropaclib_getFlipYaw(hCroPaC) ? "No-Flip" : "Flip";
        case k_flipPitch:       return !hcropaclib_getFlipPitch(hCroPaC) ? "No-Flip" : "Flip";
        case k_flipRoll:        return !hcropaclib_getFlipRoll(hCroPaC) ? "No-Flip" : "Flip";
            
        default: return "NULL";
    }
}

const String PluginProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String PluginProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
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


bool PluginProcessor::isInputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool PluginProcessor::isOutputChannelStereoPair (int /*index*/) const
{
    return true;
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

bool PluginProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

void PluginProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    nHostBlockSize = samplesPerBlock;
    nNumInputs =  getTotalNumInputChannels();
    nNumOutputs = getTotalNumOutputChannels();
    nSampleRate = (int)(sampleRate + 0.5);

	hcropaclib_init(hCroPaC, nSampleRate);
    AudioProcessor::setLatencySamples(hcropaclib_getProcessingDelay());
}

void PluginProcessor::releaseResources()
{
}

void PluginProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& /*midiMessages*/)
{
    int nCurrentBlockSize = nHostBlockSize = buffer.getNumSamples();
    nNumInputs = jmin(getTotalNumInputChannels(), buffer.getNumChannels());
    nNumOutputs = jmin(getTotalNumOutputChannels(), buffer.getNumChannels());
    float** bufferData = buffer.getArrayOfWritePointers();
    float* pFrameData[MAX_NUM_CHANNELS];
    int framesize = hcropaclib_getFrameSize();
    
    if(nCurrentBlockSize % framesize == 0) { /* divisible by frame size */
        for(int frame = 0; frame < nCurrentBlockSize/framesize; frame++) {
            for(int ch = 0; ch < buffer.getNumChannels(); ch++)
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
    return new PluginEditor (this);
}

//==============================================================================
void PluginProcessor::getStateInformation (MemoryBlock& destData)
{
    XmlElement xml("HCROPACPLUGINSETTINGS");
    
    xml.setAttribute("EnableCroPaC", hcropaclib_getEnableCroPaC(hCroPaC));
    for(int band=0; band<hcropaclib_getNumberOfBands(); band++){
        xml.setAttribute("Balance"+String(band), hcropaclib_getBalance(hCroPaC, band));
    }
    xml.setAttribute("CovAVG", hcropaclib_getCovAvg(hCroPaC));
    xml.setAttribute("AnaLim", hcropaclib_getAnaLimit(hCroPaC));
    
    xml.setAttribute("UseDefaultHRIRset", hcropaclib_getUseDefaultHRIRsflag(hCroPaC));
    xml.setAttribute("Norm", hcropaclib_getNormType(hCroPaC));
    xml.setAttribute("ChOrder", hcropaclib_getChOrder(hCroPaC));
    xml.setAttribute("maxrE", hcropaclib_getEnableDiffCorrection(hCroPaC));
    
    xml.setAttribute("ENABLEROT", hcropaclib_getEnableRotation(hCroPaC));
    xml.setAttribute("YAW", hcropaclib_getYaw(hCroPaC));
    xml.setAttribute("PITCH", hcropaclib_getPitch(hCroPaC));
    xml.setAttribute("ROLL", hcropaclib_getRoll(hCroPaC));
    xml.setAttribute("FLIP_YAW", hcropaclib_getFlipYaw(hCroPaC));
    xml.setAttribute("FLIP_PITCH", hcropaclib_getFlipPitch(hCroPaC));
    xml.setAttribute("FLIP_ROLL", hcropaclib_getFlipRoll(hCroPaC));
    xml.setAttribute("RPY_FLAG", hcropaclib_getRPYflag(hCroPaC));
    
    xml.setAttribute("OSC_PORT", osc_port_ID);
    
    if(!hcropaclib_getUseDefaultHRIRsflag(hCroPaC))
        xml.setAttribute("SofaFilePath", String(hcropaclib_getSofaFilePath(hCroPaC)));

    copyXmlToBinary(xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr) {
        if (xmlState->hasTagName("HCROPACPLUGINSETTINGS")) {
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
            
            hcropaclib_refreshParams(hCroPaC);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}

