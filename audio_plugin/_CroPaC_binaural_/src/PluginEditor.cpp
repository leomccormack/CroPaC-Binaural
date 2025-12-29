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

#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor(p), processor(p), progressbar(progress), fileChooser ("File", File(), true, false, false,
      "*.sofa;*.nc;", String(),
      "Load SOFA File")
{
    TBuseDefaultHRIRs.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (TBuseDefaultHRIRs.get());
    TBuseDefaultHRIRs->setButtonText (juce::String());
    TBuseDefaultHRIRs->addListener (this);
    TBuseDefaultHRIRs->setBounds (614, 60, 21, 24);

    CBchFormat = std::make_unique<ComboBoxWithAttachment>(p.parameters, "channelOrder");
    addAndMakeVisible (CBchFormat.get());
    CBchFormat->setEditableText (false);
    CBchFormat->setJustificationType (juce::Justification::centredLeft);
    CBchFormat->setBounds (280, 98, 73, 20);

    CBnormScheme = std::make_unique<ComboBoxWithAttachment>(p.parameters, "normType");
    addAndMakeVisible (CBnormScheme.get());
    CBnormScheme->setEditableText (false);
    CBnormScheme->setJustificationType (juce::Justification::centredLeft);
    CBnormScheme->setBounds (357, 98, 73, 20);

    TBmaxRE = std::make_unique<ToggleButtonWithAttachment>(p.parameters, "enableDiffCorrection");
    addAndMakeVisible (TBmaxRE.get());
    TBmaxRE->setBounds (192, 96, 22, 24);

    s_yaw = std::make_unique<SliderWithAttachment>(p.parameters, "yaw");
    addAndMakeVisible (s_yaw.get());
    s_yaw->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s_yaw->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    s_yaw->setBounds (453, 179, 58, 63);

    s_pitch= std::make_unique<SliderWithAttachment>(p.parameters, "pitch");
    addAndMakeVisible (s_pitch.get());
    s_pitch->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s_pitch->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    s_pitch->setBounds (516, 179, 58, 63);

    s_roll = std::make_unique<SliderWithAttachment>(p.parameters, "roll");
    addAndMakeVisible (s_roll.get());
    s_roll->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s_roll->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 15);
    s_roll->setBounds (579, 179, 58, 63);

    t_flipYaw = std::make_unique<ToggleButtonWithAttachment>(p.parameters, "flipYaw");
    addAndMakeVisible (t_flipYaw.get());
    t_flipYaw->setBounds (483, 243, 23, 24);

    t_flipPitch = std::make_unique<ToggleButtonWithAttachment>(p.parameters, "flipPitch");
    addAndMakeVisible (t_flipPitch.get());
    t_flipPitch->setBounds (546, 243, 23, 24);

    t_flipRoll = std::make_unique<ToggleButtonWithAttachment>(p.parameters, "flipRoll");
    addAndMakeVisible (t_flipRoll.get());
    t_flipRoll->setBounds (609, 243, 23, 24);

    te_oscport.reset (new juce::TextEditor ("new text editor"));
    addAndMakeVisible (te_oscport.get());
    te_oscport->setMultiLine (false);
    te_oscport->setReturnKeyStartsNewLine (false);
    te_oscport->setReadOnly (false);
    te_oscport->setScrollbarsShown (true);
    te_oscport->setCaretVisible (false);
    te_oscport->setPopupMenuEnabled (true);
    te_oscport->setText (TRANS("9000"));
    te_oscport->setBounds (587, 137, 44, 18);

    TBrpyFlag = std::make_unique<ToggleButtonWithAttachment>(p.parameters, "useRollPitchYaw");
    addAndMakeVisible (TBrpyFlag.get());
    TBrpyFlag->setBounds (492, 135, 32, 24);

    TBenableRotation = std::make_unique<ToggleButtonWithAttachment>(p.parameters, "enableRotation");
    addAndMakeVisible (TBenableRotation.get());
    TBenableRotation->setBounds (575, 113, 22, 24);

    s_cov_avg = std::make_unique<SliderWithAttachment>(p.parameters, "covAvgCoeff");
    addAndMakeVisible (s_cov_avg.get());
    s_cov_avg->setSliderStyle (juce::Slider::LinearHorizontal);
    s_cov_avg->setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
    s_cov_avg->setBounds (80, 130, 132, 32);

    s_diff2dir = std::make_unique<SliderWithAttachment>(p.parameters, "streamBalance");
    addAndMakeVisible (s_diff2dir.get());
    s_diff2dir->setSliderStyle (juce::Slider::LinearVertical);
    s_diff2dir->setTextBoxStyle (juce::Slider::NoTextBox, false, 80, 20);
    s_diff2dir->addListener (this);
    s_diff2dir->setBounds (386, 179, 40, 80);

    s_ana_lim = std::make_unique<SliderWithAttachment>(p.parameters, "anaLimit");
    addAndMakeVisible (s_ana_lim.get());
    s_ana_lim->setSliderStyle (juce::Slider::LinearHorizontal);
    s_ana_lim->setTextBoxStyle (juce::Slider::TextBoxRight, false, 74, 20);
    s_ana_lim->setBounds (283, 130, 146, 32);

    TBenableCroPaC = std::make_unique<ToggleButtonWithAttachment>(p.parameters, "enableCroPaC");
    addAndMakeVisible (TBenableCroPaC.get());
    TBenableCroPaC->setBounds (192, 63, 22, 24);
    
    CBhrirPreProc = std::make_unique<ComboBoxWithAttachment>(p.parameters, "hrirPreproc");
    addAndMakeVisible (CBhrirPreProc.get());
    CBhrirPreProc->setEditableText (false);
    CBhrirPreProc->setBounds (312, 65, 118, 20);

    setSize (656, 278);

    /* handle to object */
    hCroPaC = processor.getFXHandle();

    /* init OpenGL */
#ifndef PLUGIN_EDITOR_DISABLE_OPENGL
    openGLContext.setMultisamplingEnabled(true);
    openGLContext.attachTo(*this);
#endif

    /* Look and Feel */
    setLookAndFeel(&LAF);

    /* file loader */
    addAndMakeVisible (fileChooser);
    fileChooser.addListener (this);
    fileChooser.setBounds (458, 86, 174, 20);
    StringArray filenames;
    filenames.add("/Spatial_Audio_Framework/Default");
    filenames.add(hcropaclib_getSofaFilePath(hCroPaC));
    fileChooser.setRecentlyUsedFilenames(filenames);
    fileChooser.setFilenameIsEditable(true);

    /* create 2d Sliders */
    int nPoints;
    float* pX_vector;
    float* pY_values;

    /* create 2d Slider for the balance parameter */
    balance2dSlider.reset (new log2dSlider(360, 62, 100, 20e3, 0, 2, 2));
    addAndMakeVisible (balance2dSlider.get());
    balance2dSlider->setAlwaysOnTop(true);
    balance2dSlider->setTopLeftPosition(25, 188);
    hcropaclib_getBalanceHandle(hCroPaC, &pX_vector, &pY_values, &nPoints);
    balance2dSlider->setDataHandles(pX_vector, pY_values, nPoints);

    /* grab current parameter settings */
    TBuseDefaultHRIRs->setToggleState(hcropaclib_getUseDefaultHRIRsflag(hCroPaC), dontSendNotification);
    te_oscport->setText(String(processor.getOscPortID()), dontSendNotification);

    /* tooltips */
    TBenableCroPaC->setTooltip("Enables/Disables the parameteric rendering. When disabled, the plug-in outputs ambisonic decoded audio using the MagLS decoder.");
    TBmaxRE->setTooltip("Enables/Disables the diffuse covariance constraint applied to the decoding matrix. This is the 'C' part of the 'TAC' decoder. Note, this is not the same as applying diffuse-field EQ on the HRIRs; this is mainly a \"spatial\" manipulation, not a timbral one. Also note that, while it may make recodings sound broader/wider at lower-orders, it does so at the cost of greatly damaging the spatial properties of the recording (pulling everything to the sides: almost stereo-widening); therefore, we would argue that it is not \"correct\" to enable this by default... although, it can sound pretty good in some cases.");
    TBuseDefaultHRIRs->setTooltip("If this is 'ticked', the plug-in is using the default HRIR set from the Spatial_Audio_Framework.");
    fileChooser.setTooltip("Optionally, a custom HRIR set may be loaded via the SOFA standard. Note that if the plug-in fails to load the specified .sofa file, it will revert to the default HRIR data.");
    s_diff2dir->setTooltip("This sets the diffuse-to-direct balance for all frequencies (default is in the middle). Use the 2-D slider to change the balance for specific frequencies.");
    s_cov_avg->setTooltip("Covariance matrix averaging coefficient (one-pole).");
    s_ana_lim->setTooltip("Maximum analysis frequency (in Hz). Optionally, set this to the spatial aliasing frequency of the microphone array used for the recording to save CPU.");
    CBchFormat->setTooltip("Ambisonic channel ordering convention (Note that AmbiX: ACN/SN3D).");
    CBnormScheme->setTooltip("Ambisonic normalisation scheme (Note that AmbiX: ACN/SN3D).");
    TBenableRotation->setTooltip("Enables/Disables sound-field rotation prior to decoding.");
    s_yaw->setTooltip("Sets the 'Yaw' rotation angle (in degrees).");
    s_pitch->setTooltip("Sets the 'Pitch' rotation angle (in degrees).");
    s_roll->setTooltip("Sets the 'Roll' rotation angle (in degrees).");
    t_flipYaw->setTooltip("Flips the sign (+/-) of the 'Yaw' rotation angle.");
    t_flipPitch->setTooltip("Flips the sign (+/-) of the 'Pitch' rotation angle.");
    t_flipRoll->setTooltip("Flips the sign (+/-) of the 'Roll' rotation angle.");
    te_oscport->setTooltip("The OSC port at which to receive the rotation angles. To facilitate head-tracking, send the rotation angles (in degrees) to this port ID as a 3-element vector 'ypr[3]', following the yaw-pitch-roll convention.");
    TBrpyFlag->setTooltip("If enabled, the plug-in will use the roll-pitch-yaw rotation order convention. If disabled, it will use the yaw-pitch-roll convention.");
    CBhrirPreProc->setTooltip("Pre-processing options for the HRIRs. Diffuse-field EQ is based on a weighted summation of all the HRTF magnitudes in the currently loaded set (i.e., removing the common/direction-independent filtering of the HRTFs). The phase-simplification involves estimating the ITDs for all the HRIRs, removing the phase from the HRTFs, but then re-introducing the phase as IPDs per frequency-bin.");
    
    /* Plugin description */
    pluginDescription.reset (new juce::ComboBox ("new combo box"));
    addAndMakeVisible (pluginDescription.get());
    pluginDescription->setBounds (0, 0, 200, 32);
    pluginDescription->setAlpha(0.0f);
    pluginDescription->setEnabled(false);
    pluginDescription->setTooltip(TRANS("This plug-in is an implementation of a parametric binaural Ambisonic decoder, which aims to go beyond conventional linear and time-invarient Ambisonic decoding by estimating spatial parameters (that describe the input sound scene) over time and frequency. These spatial parameters are then used to conduct the mapping of the input Ambisonic signals to the output binaural channels in an adaptive and informed manner. The employed parametric approach is inspired by the COMPASS method. However, along with the Cross-Pattern Coherence (CroPaC) spatial post-filter, it also employs instantaneous source direction estimation and synthesises the output in a linear manner as much as possible; in order to improve the fidelity of the output signals. It is intended for first-order input only. \n\n") +
                                  TRANS("The plugin first generates intermediate/prototype binaural signals to serve as a good \"starting guess\", in this case the Magnitude-Least-Squares (MagLS) Ambisonic decoder (as found in sparta_ambiBIN) is used. The signal statistics (covariance matrices per frequency band) of these prototype binaural signals are then computed, followed by defining new target covariance matrices that are formed based on the analysed spatial parameters. The problem of applying mixing matrices to the prototype signals such that their narrow-band covariance matrices are brought closer to the target covariance matrices, is then solved by using the covariance-domain framework for spatial audio processing (CDF4SAP); also referred to as \"optimal-mixing\". The approach aims to synthesise signals that exhibit the target covariance matrices first via a linear combination of them as much as possible, followed by \"filling in the gaps\" with a decorrelated version of the prototype signals; as described by a residual mixing matrix (describing what is left). Such processing aims to  improve signal fidelity and also mitigate artefacts arrising due to signal decorrelation.\n\n")+
                                  TRANS("The \"Diffuse-to-Direct\" control allows the user to give more prominence to the direct sound components (an effect similar to de-reverberation), or to the ambient component (an effect similar to emphasising reverberation in the recording). When set in the middle, the two are balanced. \n\n "));
    addAndMakeVisible (publicationLink);
    publicationLink.setColour (HyperlinkButton::textColourId, Colours::lightblue);
    publicationLink.setBounds(getBounds().getWidth()-80, 4, 80, 12);
    publicationLink.setJustificationType(Justification::centredLeft);

    /* ProgressBar */
    progress = 0.0;
    progressbar.setBounds(getLocalBounds().getCentreX()-175, getLocalBounds().getCentreY()-17, 350, 35);
    progressbar.ProgressBar::setAlwaysOnTop(true);
    progressbar.setColour(ProgressBar::backgroundColourId, Colours::gold);
    progressbar.setColour(ProgressBar::foregroundColourId, Colours::white);

    /* Specify screen refresh rate */
    startTimer(20);

    /* warnings */
    currentWarning = k_warning_none;
}

PluginEditor::~PluginEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void PluginEditor::paint (juce::Graphics& g)
{
    using namespace ColoursUI;

    drawPluginBackgroundAndBanner(g, getBounds());

    /* Panels */
    drawPanel(g, {12, 58,207, 68}, panelFill,      panelStroke);
    drawPanel(g, {12, 58,207, 32}, panelFillLight, panelStroke);
    drawPanel(g, {218,58,218,68}, panelFill, panelStroke);
    drawPanel(g, {446,58,196,54}, panelFill, panelStroke);
    drawPanel(g, {12,125,424,39}, panelFill, panelStroke);
    drawPanel(g, {446,111,196,157}, panelFill,      panelStroke);
    drawPanel(g, {446,160,196,108}, panelFillLight, panelStroke);
    drawPanel(g, {12,163,424,105}, panelFill, panelStroke);

    /* Title */
    drawLabel(g, {16,1,100,32}, "CroPaC|", 18.8f);
    drawLabel(g, {86,1,112,32}, "Binaural", 18.f, juce::Justification::centredLeft, juce::Colour(0xffdbed5c));

    /* Section titles */
    drawLabel(g, {164,32,149,30}, "Decoding Settings", 15.f);
    drawLabel(g, {520,32,113,30}, "Output",            15.f);

    /* Top-left labels */
    drawLabel(g, {19,58,181,30}, "Enable CroPaC Decoding:", 14.f);
    drawLabel(g, {19,92,205,30}, "Diffuse Cov. Constraint:", 14.f);
    drawLabel(g, {19,129,125,30}, "Cov Avg:", 14.f);

    /* Top-middle labels */
    drawLabel(g, {225,59,132,30}, "HRIR PreProc:", 14.f);
    drawLabel(g, {225,92,132,30}, "Format:",       14.f);

    /* Top-right labels */
    drawLabel(g, {459,56,165,30}, "Use Default HRIR set:", 15.f);

    /* Rotation block labels */
    drawLabel(g, {455,109,160,30}, "Enable Rotation:", 15.f);
    drawLabel(g, {447,130,54,35},  "R-P-Y:",           11.f, juce::Justification::centred);
    drawLabel(g, {535,130,91,35},  "OSC port:",        11.f);
    drawLabel(g, {434,156,65,30}, "Yaw",   12.f, juce::Justification::centred);
    drawLabel(g, {471,156,49,30}, "\\ypr[0]", 10.f, juce::Justification::centred);
    drawLabel(g, {511,156,46,30}, "Pitch", 12.f, juce::Justification::centred);
    drawLabel(g, {544,156,40,30}, "\\ypr[1]", 10.f, juce::Justification::centred);
    drawLabel(g, {567,156,54,30}, "Roll",  12.f, juce::Justification::centred);
    drawLabel(g, {601,156,40,30}, "\\ypr[2]", 10.f, juce::Justification::centred);
    drawLabel(g, {439,240,63,30}, "+/-", 13.f, juce::Justification::centred);
    drawLabel(g, {503,240,63,30}, "+/-", 13.f, juce::Justification::centred);
    drawLabel(g, {567,240,63,30}, "+/-", 13.f, juce::Justification::centred);

    /* Diffuse to Direct block */
    drawLabel(g, {21,159,283,30}, "Diffuse to Direct Balance Per Band", 15.f);
    drawLabel(g, {414,168,40,30}, "Dir",  11.f);
    drawLabel(g, {414,240,40,30}, "Diff", 11.f);
    drawLabel(g, {19,243,37,30},  "100", 12.f);
    drawLabel(g, {174,243,37,30}, "1k",  12.f);
    drawLabel(g, {328,243,37,30}, "10k", 12.f);
    drawLabel(g, {210,243,93,30}, "Frequency (Hz)", 12.f);
    drawLabel(g, {371,243,37,30}, "20k", 12.f);

    /* Additional label */
    drawLabel(g, {227,129,125,30}, "Ana Lim:", 14.f);

    g.setColour(Colours::white);
    g.setFont(juce::FontOptions (11.00f, Font::plain));
    g.drawText(TRANS("Ver ") + JucePlugin_VersionString + BUILD_VER_SUFFIX + TRANS(", Build Date ") + __DATE__ + TRANS(" "),
        165, 16, 530, 11,
        Justification::centredLeft, true);

    /* display warning message */
    g.setFont(juce::FontOptions (11.00f, Font::plain));
    switch (currentWarning){
        case k_warning_none:
            break;
        case k_warning_frameSize:
            g.setColour(Colours::red);
            g.drawText(TRANS("Set frame size to multiple of ") + String(hcropaclib_getFrameSize()),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NinputCH:
            g.setColour(Colours::red);
            g.drawText(TRANS("Insufficient number of input channels (") + String(processor.getTotalNumInputChannels()) +
                       TRANS("/") + String(hcropaclib_getNSHrequired()) + TRANS(")"),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NoutputCH:
            g.setColour(Colours::red);
            g.drawText(TRANS("Insufficient number of output channels (") + String(processor.getTotalNumOutputChannels()) +
                       TRANS("/") + String(hcropaclib_getNumEars()) + TRANS(")"),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_osc_connection_fail:
            g.setColour(Colours::red);
            g.drawText(TRANS("OSC failed to connect, or port is already taken"),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_supported_fs:
            g.setColour(Colours::yellow);
            g.drawText(TRANS("Sample rate \"") + String(hcropaclib_getDAWsamplerate(hCroPaC)) + TRANS("\" is not recommended"),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_mismatch_fs:
            g.setColour(Colours::yellow);
            g.drawText(TRANS("Resampled HRIRs to match host samplerate"),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
    }
}

void PluginEditor::resized()
{
}

void PluginEditor::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == TBuseDefaultHRIRs.get())
    {
        hcropaclib_setUseDefaultHRIRsflag(hCroPaC, (int)TBuseDefaultHRIRs->getToggleState());
    }
}

void PluginEditor::comboBoxChanged (juce::ComboBox* /*comboBoxThatHasChanged*/)
{
}

void PluginEditor::sliderValueChanged (juce::Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == s_diff2dir.get())
    {
        balance2dSlider->setRefreshValuesFLAG(true);
    }
}

void PluginEditor::timerCallback()
{
    /* parameters whos values can change internally should be periodically refreshed */
    TBuseDefaultHRIRs->setToggleState(hcropaclib_getUseDefaultHRIRsflag(hCroPaC), dontSendNotification);

    /* Progress bar */
    if(hcropaclib_getCodecStatus(hCroPaC)==CODEC_STATUS_INITIALISING){
        addAndMakeVisible(progressbar);
        progress = (double)hcropaclib_getProgressBar0_1(hCroPaC);
        char text[HCROPAC_PROGRESSBARTEXT_CHAR_LENGTH];
        hcropaclib_getProgressBarText(hCroPaC, (char*)text);
        progressbar.setTextToDisplay(String(text));
    }
    else
        removeChildComponent(&progressbar);

    /* Some parameters shouldn't be editable during initialisation*/
    if(hcropaclib_getCodecStatus(hCroPaC)==CODEC_STATUS_INITIALISING){
        if(fileChooser.isEnabled())
            fileChooser.setEnabled(false);
        if(TBuseDefaultHRIRs->isEnabled())
            TBuseDefaultHRIRs->setEnabled(false);
        if(TBmaxRE->isEnabled())
            TBmaxRE->setEnabled(false);
    }
    else {
        if(!fileChooser.isEnabled())
            fileChooser.setEnabled(true);
        if(!TBuseDefaultHRIRs->isEnabled())
            TBuseDefaultHRIRs->setEnabled(true);
        if(!TBmaxRE->isEnabled())
            TBmaxRE->setEnabled(true);
    }

    /* refresh 2d slider */
    if (balance2dSlider->getRefreshValuesFLAG()){
        balance2dSlider->repaint();
        balance2dSlider->setRefreshValuesFLAG(false);
    }

    /* display warning message, if needed */
    if ((processor.getCurrentBlockSize() % hcropaclib_getFrameSize()) != 0){
        currentWarning = k_warning_frameSize;
        repaint(0,0,getWidth(),32);
    }
    else if ((processor.getCurrentNumInputs() < hcropaclib_getNSHrequired())){
        currentWarning = k_warning_NinputCH;
        repaint(0,0,getWidth(),32);
    }
    else if ((processor.getCurrentNumOutputs() < hcropaclib_getNumEars())){
        currentWarning = k_warning_NoutputCH;
        repaint(0,0,getWidth(),32);
    }
    else if(!processor.getOscPortConnected() && hcropaclib_getEnableRotation(hCroPaC)){
        currentWarning = k_warning_osc_connection_fail;
        repaint(0,0,getWidth(),32);
    }
    else if ( !((hcropaclib_getDAWsamplerate(hCroPaC) == 44.1e3) || (hcropaclib_getDAWsamplerate(hCroPaC) == 48e3)) ){
        currentWarning = k_warning_supported_fs;
        repaint(0,0,getWidth(),32);
    }
    else if (hcropaclib_getDAWsamplerate(hCroPaC) != hcropaclib_getHRIRsamplerate(hCroPaC)){
        currentWarning = k_warning_mismatch_fs;
        repaint(0,0,getWidth(),32);
    }
    else if(currentWarning){
        currentWarning = k_warning_none;
        repaint(0,0,getWidth(),32);
    }

    /* check if OSC port has changed */
    if(processor.getOscPortID() != te_oscport->getText().getIntValue())
        processor.setOscPortID(te_oscport->getText().getIntValue());
}
