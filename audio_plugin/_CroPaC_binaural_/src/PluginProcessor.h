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

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include <JuceHeader.h>
#include "../../resources/ParameterManager.h"
#include "hcropaclib.h"
#include <string.h>
#include <thread>
#define BUILD_VER_SUFFIX ""            /* String to be added before the version name on the GUI (e.g. beta, alpha etc..) */
#define MAX_NUM_CHANNELS 64
#define DEFAULT_OSC_PORT 9000
#ifndef MIN
# define MIN(a,b) (( (a) < (b) ) ? (a) : (b))
#endif
#ifndef MAX
# define MAX(a,b) (( (a) > (b) ) ? (a) : (b))
#endif

typedef enum _TIMERS{
    TIMER_PROCESSING_RELATED = 1,
    TIMER_GUI_RELATED
}TIMERS;

class PluginProcessor  : public AudioProcessor,
                         public MultiTimer,
                         private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
                         public VST2ClientExtensions,
                         public ParameterManager
{
public:
    /* Get functions */
    void* getFXHandle() { return hCroPaC; }
    int getCurrentBlockSize(){ return nHostBlockSize; }
    int getCurrentNumInputs(){ return nNumInputs; }
    int getCurrentNumOutputs(){ return nNumOutputs; }
    
    /* VST CanDo */
    pointer_sized_int handleVstManufacturerSpecific (int32 /*index*/, pointer_sized_int /*value*/, void* /*ptr*/, float /*opt*/) override { return 0; }
    pointer_sized_int handleVstPluginCanDo (int32 /*index*/, pointer_sized_int /*value*/, void* ptr, float /*opt*/) override{
        auto text = (const char*) ptr;
        auto matches = [=](const char* s) { return strcmp (text, s) == 0; };
        if (matches ("wantsChannelCountNotifications"))
            return 1;
        return 0;
    }
    VST2ClientExtensions* getVST2ClientExtensions() override {return this;}
    
    /* OSC */
    void oscMessageReceived(const OSCMessage& message) override;
    void setOscPortID(int newID){
        osc.disconnect();
        osc_port_ID = newID;
        osc.connect(osc_port_ID);
    }
    int getOscPortID(){ return osc_port_ID; }
    
private:
    void* hCroPaC;       /* hcropaclib handle */
    int nNumInputs;      /* current number of input channels */
    int nNumOutputs;     /* current number of output channels */
    int nSampleRate;     /* current host sample rate */
    int nHostBlockSize;  /* typical host block size to expect, in samples */
    OSCReceiver osc;
    int osc_port_ID;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void setParameterValuesUsingInternalState();
    void setInternalStateUsingParameterValues();

    void timerCallback(int timerID) override {
        switch(timerID){
            case TIMER_PROCESSING_RELATED:
                /* reinitialise codec if needed */
                if(hcropaclib_getCodecStatus(hCroPaC) == CODEC_STATUS_NOT_INITIALISED){
                    try{
                        std::thread threadInit(hcropaclib_initCodec, hCroPaC);
                        threadInit.detach();
                    } catch (const std::exception& exception) {
                        std::cout << "Could not create thread" << exception.what() << std::endl;
                    }
                }
                break;
                
            case TIMER_GUI_RELATED:
                /* handled in PluginEditor; */
                break;
        }
    }
    
    /***************************************************************************\
                                    JUCE Functions
    \***************************************************************************/
public:
    PluginProcessor();
    ~PluginProcessor() override;
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
