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
    fileChooser.setBounds (458, 86, 168, 20);
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
    TBuseDefaultHRIRs = nullptr;
    CBchFormat = nullptr;
    CBnormScheme = nullptr;
    TBmaxRE = nullptr;
    s_yaw = nullptr;
    s_pitch = nullptr;
    s_roll = nullptr;
    t_flipYaw = nullptr;
    t_flipPitch = nullptr;
    t_flipRoll = nullptr;
    te_oscport = nullptr;
    TBrpyFlag = nullptr;
    TBenableRotation = nullptr;
    s_cov_avg = nullptr;
    s_diff2dir = nullptr;
    s_ana_lim = nullptr;
    TBenableCroPaC = nullptr;

    setLookAndFeel(nullptr);
    balance2dSlider = nullptr;
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);

    {
        int x = 0, y = 154, width = 656, height = 124;
        juce::Colour fillColour1 = juce::Colour (0xff19313f), fillColour2 = juce::Colour (0xff041518);
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             8.0f - 0.0f + x,
                                             280.0f - 154.0f + y,
                                             fillColour2,
                                             8.0f - 0.0f + x,
                                             208.0f - 154.0f + y,
                                             false));
        g.fillRect (x, y, width, height);
    }

    {
        int x = 0, y = 30, width = 656, height = 124;
        juce::Colour fillColour1 = juce::Colour (0xff19313f), fillColour2 = juce::Colour (0xff041518);
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             8.0f - 0.0f + x,
                                             32.0f - 30.0f + y,
                                             fillColour2,
                                             8.0f - 0.0f + x,
                                             104.0f - 30.0f + y,
                                             false));
        g.fillRect (x, y, width, height);
    }

    {
        float x = 1.0f, y = 2.0f, width = 654.0f, height = 31.0f;
        juce::Colour fillColour1 = juce::Colour (0xff041518), fillColour2 = juce::Colour (0xff19313f);
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             0.0f - 1.0f + x,
                                             32.0f - 2.0f + y,
                                             fillColour2,
                                             656.0f - 1.0f + x,
                                             24.0f - 2.0f + y,
                                             false));
        g.fillRoundedRectangle (x, y, width, height, 5.000f);
        g.setColour (strokeColour);
        g.drawRoundedRectangle (x, y, width, height, 5.000f, 2.000f);
    }

    {
        int x = 12, y = 58, width = 207, height = 68;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 12, y = 125, width = 424, height = 39;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 12, y = 58, width = 207, height = 32;
        juce::Colour fillColour = juce::Colour (0x08f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 218, y = 58, width = 218, height = 68;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 446, y = 58, width = 196, height = 54;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 164, y = 32, width = 149, height = 30;
        juce::String text (TRANS("Decoding Settings"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (15.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 520, y = 32, width = 113, height = 30;
        juce::String text (TRANS("Output"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (15.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 459, y = 56, width = 165, height = 30;
        juce::String text (TRANS("Use Default HRIR set:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (15.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 19, y = 129, width = 125, height = 30;
        juce::String text (TRANS("Cov Avg:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (14.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 225, y = 59, width = 132, height = 30;
        juce::String text (TRANS("HRIR PreProc:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (14.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 225, y = 92, width = 132, height = 30;
        juce::String text (TRANS("Format:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (14.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 19, y = 92, width = 205, height = 30;
        juce::String text (TRANS("Diffuse Cov. Constraint:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (14.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 446, y = 111, width = 196, height = 157;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 446, y = 160, width = 196, height = 108;
        juce::Colour fillColour = juce::Colour (0x08f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 471, y = 156, width = 49, height = 30;
        juce::String text (TRANS("\\ypr[0]"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (10.00f, juce::Font::plain).withStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 511, y = 156, width = 46, height = 30;
        juce::String text (TRANS("Pitch"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (12.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 567, y = 156, width = 54, height = 30;
        juce::String text (TRANS("Roll"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (12.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 567, y = 240, width = 63, height = 30;
        juce::String text (TRANS("+/-"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (13.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 439, y = 240, width = 63, height = 30;
        juce::String text (TRANS("+/-"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (13.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 503, y = 240, width = 63, height = 30;
        juce::String text (TRANS("+/-"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (13.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 535, y = 130, width = 91, height = 35;
        juce::String text (TRANS("OSC port:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (11.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 434, y = 156, width = 65, height = 30;
        juce::String text (TRANS("Yaw"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (12.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 544, y = 156, width = 40, height = 30;
        juce::String text (TRANS("\\ypr[1]"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (10.00f, juce::Font::plain).withStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 601, y = 156, width = 40, height = 30;
        juce::String text (TRANS("\\ypr[2]"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (10.00f, juce::Font::plain).withStyle ("Regular"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 447, y = 130, width = 54, height = 35;
        juce::String text (TRANS("R-P-Y:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (11.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centred, true);
    }

    {
        int x = 455, y = 109, width = 160, height = 30;
        juce::String text (TRANS("Enable Rotation:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (15.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 12, y = 163, width = 424, height = 105;
        juce::Colour fillColour = juce::Colour (0x10f4f4f4);
        juce::Colour strokeColour = juce::Colour (0x67a0a0a0);
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 21, y = 159, width = 283, height = 30;
        juce::String text (TRANS("Diffuse to Direct Balance Per Band"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (15.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 414, y = 240, width = 40, height = 30;
        juce::String text (TRANS("Diff"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (11.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 414, y = 168, width = 40, height = 30;
        juce::String text (TRANS("Dir"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (11.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 19, y = 243, width = 37, height = 30;
        juce::String text (TRANS("100"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (12.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 174, y = 243, width = 37, height = 30;
        juce::String text (TRANS("1k"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (12.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 328, y = 243, width = 37, height = 30;
        juce::String text (TRANS("10k"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (12.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 210, y = 243, width = 93, height = 30;
        juce::String text (TRANS("Frequency (Hz)"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (12.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 371, y = 243, width = 37, height = 30;
        juce::String text (TRANS("20k"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (12.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 16, y = 1, width = 100, height = 32;
        juce::String text (TRANS("CroPaC|"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (18.80f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 86, y = 1, width = 112, height = 32;
        juce::String text (TRANS("Binaural"));
        juce::Colour fillColour = juce::Colour (0xffdbed5c);
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (18.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 227, y = 129, width = 125, height = 30;
        juce::String text (TRANS("Ana Lim:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (14.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 19, y = 58, width = 181, height = 30;
        juce::String text (TRANS("Enable CroPaC Decoding:"));
        juce::Colour fillColour = juce::Colours::white;
        g.setColour (fillColour);
        g.setFont (juce::FontOptions (14.00f, juce::Font::plain).withStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 0, y = 0, width = 656, height = 2;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 0, y = 0, width = 2, height = 278;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 654, y = 0, width = 2, height = 278;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 0, y = 276, width = 656, height = 2;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

	g.setColour(Colours::white);
	g.setFont(juce::FontOptions (11.00f, Font::plain));
	g.drawText(TRANS("Ver ") + JucePlugin_VersionString + BUILD_VER_SUFFIX + TRANS(", Build Date ") + __DATE__ + TRANS(" "),
		165, 16, 530, 11,
		Justification::centredLeft, true);

    /* display warning message */
    g.setColour(Colours::red);
    g.setFont(juce::FontOptions (11.00f, Font::plain));
    switch (currentWarning){
        case k_warning_none:
            break;
        case k_warning_frameSize:
            g.drawText(TRANS("Set frame size to multiple of ") + String(hcropaclib_getFrameSize()),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_supported_fs:
            g.drawText(TRANS("Sample rate (") + String(hcropaclib_getDAWsamplerate(hCroPaC)) + TRANS(") is unsupported"),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_mismatch_fs:
            g.drawText(TRANS("Sample rate mismatch between DAW/HRIRs"),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NinputCH:
            g.drawText(TRANS("Insufficient number of input channels (") + String(processor.getTotalNumInputChannels()) +
                       TRANS("/") + String(hcropaclib_getNSHrequired()) + TRANS(")"),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NoutputCH:
            g.drawText(TRANS("Insufficient number of output channels (") + String(processor.getTotalNumOutputChannels()) +
                       TRANS("/") + String(hcropaclib_getNumEars()) + TRANS(")"),
                       getBounds().getWidth()-225, 16, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_osc_connection_fail:
            g.drawText(TRANS("OSC failed to connect, or port is already taken"),
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
    else if ( !((hcropaclib_getDAWsamplerate(hCroPaC) == 44.1e3) || (hcropaclib_getDAWsamplerate(hCroPaC) == 48e3)) ){
        currentWarning = k_warning_supported_fs;
        repaint(0,0,getWidth(),32);
    }
    else if (hcropaclib_getDAWsamplerate(hCroPaC) != hcropaclib_getHRIRsamplerate(hCroPaC)){
        currentWarning = k_warning_mismatch_fs;
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
    else if(currentWarning){
        currentWarning = k_warning_none;
        repaint(0,0,getWidth(),32);
    }

    /* check if OSC port has changed */
    if(processor.getOscPortID() != te_oscport->getText().getIntValue())
        processor.setOscPortID(te_oscport->getText().getIntValue());
}
