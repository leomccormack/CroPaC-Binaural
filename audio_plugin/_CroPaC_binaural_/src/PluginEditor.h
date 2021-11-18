/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 6.0.5

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --

#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "log2dSlider.h"
#include "../../resources/CroPaCLookAndFeel.h"

typedef enum _CroPaC_WARNINGS{
    k_warning_none,
    k_warning_frameSize,
    k_warning_supported_fs,
    k_warning_mismatch_fs,
    k_warning_NinputCH,
    k_warning_NoutputCH,
    k_warning_osc_connection_fail
}CroPaC_WARNINGS;

//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class PluginEditor  : public AudioProcessorEditor,
                      public MultiTimer,
                      private FilenameComponentListener,
                      public juce::Button::Listener,
                      public juce::ComboBox::Listener,
                      public juce::Slider::Listener
{
public:
    //==============================================================================
    PluginEditor (PluginProcessor* ownerFilter);
    ~PluginEditor() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.

    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    void sliderValueChanged (juce::Slider* sliderThatWasMoved) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    PluginProcessor* hVst;
    void* hCroPaC;
    void timerCallback(int timerID) override;
    std::unique_ptr<OpenGLGraphicsContextCustomShader> shader;
    OpenGLContext openGLContext;
    double progress = 0.0;
    ProgressBar progressbar;

    /* Look and Feel */
    CroPaCLookAndFeel LAF;

    /* sofa loading */
    FilenameComponent fileChooser;

    /* 2d slider handles */
    std::unique_ptr<log2dSlider> balance2dSlider;

    /* sofa file loading */
    void filenameComponentChanged (FilenameComponent*) override  {
        String directory = fileChooser.getCurrentFile().getFullPathName();
        const char* new_cstring = (const char*)directory.toUTF8();
        hcropaclib_setSofaFilePath(hCroPaC, new_cstring);
    }

    /* warnings */
    CroPaC_WARNINGS currentWarning;

    /* tooltips */
    SharedResourcePointer<TooltipWindow> tipWindow;
    std::unique_ptr<juce::ComboBox> pluginDescription; /* Dummy combo box to provide plugin description tooltip */
    HyperlinkButton publicationLink { "(Related Publication)", { "https://leomccormack.github.io/sparta-site/docs/help/related-publications/mccormack2019parametric.pdf" } };

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::ToggleButton> TBuseDefaultHRIRs;
    std::unique_ptr<juce::ComboBox> CBchFormat;
    std::unique_ptr<juce::ComboBox> CBnormScheme;
    std::unique_ptr<juce::ToggleButton> TBmaxRE;
    std::unique_ptr<juce::Slider> s_yaw;
    std::unique_ptr<juce::Slider> s_pitch;
    std::unique_ptr<juce::Slider> s_roll;
    std::unique_ptr<juce::ToggleButton> t_flipYaw;
    std::unique_ptr<juce::ToggleButton> t_flipPitch;
    std::unique_ptr<juce::ToggleButton> t_flipRoll;
    std::unique_ptr<juce::TextEditor> te_oscport;
    std::unique_ptr<juce::ToggleButton> TBrpyFlag;
    std::unique_ptr<juce::ToggleButton> TBenableRotation;
    std::unique_ptr<juce::Slider> s_cov_avg;
    std::unique_ptr<juce::Slider> s_diff2dir;
    std::unique_ptr<juce::Slider> s_ana_lim;
    std::unique_ptr<juce::ToggleButton> TBenableCroPaC;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

