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

/*
 * Filename: hcropaclib.h (include header)
 * ---------------------------------------
 * A first-order parametric binaural Ambisonic decoder for reproducing ambisonic
 * signals over headphones. The algorithm is based on the segregation of the
 * direct and diffuse streams using the Cross-Pattern Coherence (CroPaC) spatial
 * post-filter.
 * The output of a linear binaural ambisonic decoder is then adaptively mixed,
 * in such a manner that the covariance matrix of the output stream is brought
 * closer to that of the target covariance matrix, derived from the
 * direct/diffuse analysis. For more information on the method, the reader is
 * directed to [1].
 *
 * Dependencies:
 *     Spatial_Audio_Framework
 * Author, date created:
 *     Leo McCormack, 12.01.2018
 *
 * [1] McCormack, L., Delikaris-Manias, S. (2019). "Parametric first-order
 *     ambisonic decoding for headphones utilising the Cross-Pattern Coherence
 *     algorithm". inProc 1st EAA Spatial Audio Signal Processing Symposium,
 *     Paris, France.
 */ 

#ifndef __HCROPACLIB_H_INCLUDED__
#define __HCROPACLIB_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* ========================================================================== */
/*                             Presets + Constants                            */
/* ========================================================================== */
    
/*
 * Enum: _CH_ORDER
 * ---------------
 * Available Ambisonic channel ordering conventions
 * Note: FuMa: CH_FUMA+NORM_FUMA, AmbiX: CH_ACN+NORM_SN3D
 *
 * Options:
 *     CH_ACN  - Ambisonic Channel Numbering (ACN)
 *     CH_FUMA - (Obsolete) Furse-Malham/B-format (WXYZ)
 */
#define HCROPAC_NUM_CH_ORDERINGS ( 2 )
typedef enum _CH_ORDER{
    CH_ACN = 1,
    CH_FUMA
}CH_ORDER;

/*
 * Enum: NORM_TYPES
 * ----------------
 * Available Ambisonic normalisation conventions
 * Note: NORM_FUMA does NOT have the 1/sqrt(2) scaling on the omni.
 * Further note: FuMa: CH_FUMA+NORM_FUMA, AmbiX: CH_ACN+NORM_SN3D
 *
 * Options:
 *     NORM_N3D  - orthonormalised (N3D)
 *     NORM_SN3D - Schmidt semi-normalisation (SN3D)
 *     NORM_FUMA - (Obsolete) Same as NORM_SN3D
 */
#define HCROPAC_NUM_NORM_TYPES ( 3 )
typedef enum _NORM_TYPES{
    NORM_N3D = 1,
    NORM_SN3D,
    NORM_FUMA   
}NORM_TYPES;

/*
 * Enum: CODEC_STATUS
 * ------------------
 * Current status of the codec.
 *
 * Options:
 *     CODEC_STATUS_INITIALISED     - Codec is initialised and ready to process
 *                                    input audio.
 *     CODEC_STATUS_NOT_INITIALISED - Codec has not yet been initialised, or
 *                                    the codec configuration has changed. Input
 *                                    audio should not be processed.
 *     CODEC_STATUS_INITIALISING    - Codec is currently being initialised,
 *                                    input audio should not be processed.
 */
typedef enum _CODEC_STATUS{
    CODEC_STATUS_INITIALISED = 0,
    CODEC_STATUS_NOT_INITIALISED,
    CODEC_STATUS_INITIALISING
}CODEC_STATUS;

#define HCROPAC_PROGRESSBARTEXT_CHAR_LENGTH ( 256 )
#define HCROPAC_ANA_LIMIT_MIN_VALUE ( 4000.0f )
#define HCROPAC_ANA_LIMIT_MAX_VALUE ( 20000.0f )
    
    
/* ========================================================================== */
/*                               Main Functions                               */
/* ========================================================================== */

/*
 * Function: hcropaclib_create
 * ---------------------------
 * Creates an instance of the mighty hcropaclib
 *
 * Input Arguments:
 *     phCroPaC - & address of hcropaclib handle
 */
void hcropaclib_create(void** const phCroPaC);

/*
 * Function: hcropaclib_destroy
 * ----------------------------
 * Destroys an instance of the mighty hcropaclib
 *
 * Input Arguments:
 *     phCroPaC - & address of hcropaclib handle
 */
void hcropaclib_destroy(void** const phCroPaC);

/*
 * Function: hcropaclib_init
 * -------------------------
 * Initialises an instance of hcropaclib with default settings
 *
 * Input Arguments:
 *     hCroPaC       - hcropaclib handle
 *     samplerate - host samplerate.
 */
void hcropaclib_init(void* const hCroPaC,
                     int samplerate);

/*
 * Function: hcropaclib_initCodec
 * ------------------------------
 * Intialises the codec variables, based on current global/user parameters
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 */
void hcropaclib_initCodec(void* const hCroPaC);

/*
 * Function: hcropaclib_process
 * ----------------------------
 * Performs CroPaC decoding [1] of input Ambisonic signals to the binaural
 * channels.
 *
 * Input Arguments:
 *     hCroPaC   - hcropaclib handle
 *     inputs    - input channel buffers; 2-D array: nInputs x nSamples
 *     outputs   - Output channel buffers; 2-D array: nOutputs x nSamples
 *     nInputs   - number of input channels
 *     nOutputs  - number of output channels
 *     nSamples  - number of samples in 'inputs'/'output' matrices
 *
 * [1] McCormack, L., Delikaris-Manias, S. (2019). "Parametric first-order
 *     ambisonic decoding for headphones utilising the Cross-Pattern Coherence
 *     algorithm". inProc 1st EAA Spatial Audio Signal Processing Symposium,
 *     Paris, France.
 */
void hcropaclib_process(void* const hCroPaC,
                        float** const inputs,
                        float** const outputs,
                        int nInputs,
                        int nOutputs,
                        int nSamples);

    
/* ========================================================================== */
/*                                Set Functions                               */
/* ========================================================================== */

/*
 * Function: hcropaclib_refreshParams
 * ----------------------------------
 * Sets all intialisation flags to 1. i.e. re-initialise all settings/variables
 * as hcropac is currently configured, at next available opportunity.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 */
void hcropaclib_refreshParams(void* const hCroPaC);
    
void hcropaclib_setEnableCroPaC(void* const hCroPaC, int newState);
    
/*
 * Function: hcropaclib_setBalance
 * -------------------------------
 * Sets the balance between direct and ambient streams (default=1, 50%/50%) for
 * ONE specific frequency band.
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newValue - new balance, 0: fully ambient, 1: balanced, 2: fully direct
 *     bandIdx  - frequency band index
 */
void hcropaclib_setBalance(void* const hCroPaC, float newValue, int bandIdx);

/*
 * Function: hcropaclib_setBalanceAllBands
 * ---------------------------------------
 * Sets the balance between direct and ambient streams (default=1, 50%/50%) for
 * ALL frequency bands.
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newValue - new balance, 0: fully ambient, 1: balanced, 2: fully direct
 */
void hcropaclib_setBalanceAllBands(void* const hCroPaC, float newValue);

/*
 * Function: hcropaclib_setCovAvg
 * ------------------------------
 * Sets the covariance matrix averaging coefficient.
 * Note: the coefficient is forced to not exceed 0.99, in order to avoid
 * infinite averaging.
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newValue - coefficient value 0..1; 0: no averaging, 1: "infinite"
 *                averaging
 */
void hcropaclib_setCovAvg(void* const hCroPaC, float newValue);

void hcropaclib_setAnaLimit(void* const hCroPaC, float newValue);

/*
 * Function: hcropaclib_setUseDefaultHRIRsflag
 * -------------------------------------------
 * Sets flag to dictate whether the default HRIRs in the Spatial_Audio_Framework
 * should be used, or a custom HRIR set loaded via a SOFA file.
 * Note: if the custom set failes to load correctly, hcropac will revert to the
 * defualt set. Use 'hcropaclib_getUseDefaultHRIRsflag' to check if loading was
 * successful.
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newState - 0: use custom HRIR set, 1: use default HRIR set
 */
void hcropaclib_setUseDefaultHRIRsflag(void* const hCroPaC, int newState);

/*
 * Function: hcropaclib_setSofaFilePath
 * ------------------------------------
 * Sets the file path for a .sofa file, in order to employ a custom HRIR set for
 * the decoding.
 * Note: if the custom set failes to load correctly, hcropac will revert to the
 * defualt set. Use 'hcropaclib_getUseDefaultHRIRsflag' to check if loading was
 * successful.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 *     path    - file path to .sofa file (WITH file extension)
 */
void hcropaclib_setSofaFilePath(void* const hCroPaC, const char* path);

/*
 * Function: hcropaclib_setChOrder
 * -------------------------------
 * Sets the Ambisonic channel ordering convention to decode with, in order to
 * match the convention employed by the input signals
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newOrder - convention to use (see 'CH_ORDER' enum)
 */
void hcropaclib_setChOrder(void* const hCroPaC, int newOrder);

/*
 * Function: hcropaclib_setNormType
 * --------------------------------
 * Sets the Ambisonic normalisation convention to decode with, in order to match
 * with the convention employed by the input signals.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 *     newType - convention to use (see 'NORM_TYPE' enum)
 */
void hcropaclib_setNormType(void* const hCroPaC, int newType);

/*
 * Function: hcropaclib_setEnableDiffCorrection
 * --------------------------------------------
 * Sets the flag, to say if a covariance diffuseness contraint [1] should be
 * applied to the prototype decoding matrix.
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newState - 0: disabled, 1: enabled
 *
 * [1] Zaunschirm M, Schörkhuber C, Höldrich R. Binaural rendering of
 *     Ambisonic signals by head-related impulse response time alignment and
 *     a diffuseness constraint. The Journal of the Acoustical Society of
 *     America. 2018 Jun 19;143(6):3616-27
 */
void hcropaclib_setEnableDiffCorrection(void* const hCroPaC, int newState);

/*
 * Function: hcropaclib_setEnableRotation
 * --------------------------------------
 * Sets the flag to enable/disable sound-field rotation.
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newState - 0: disable, 1: enable
 */
void hcropaclib_setEnableRotation(void* const hCroPaC, int newState);

/*
 * Function: hcropaclib_setYaw
 * ---------------------------
 * Sets the 'yaw' rotation angle
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 *     newYaw  - the 'yaw' rotation angle, in DEGREES
 */
void hcropaclib_setYaw(void* const hCroPaC, float newYaw);

/*
 * Function: hcropaclib_setPitch
 * -----------------------------
 * Sets the 'pitch' rotation angle
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newPitch - the 'pitch' rotation angle, in DEGREES
 */
void hcropaclib_setPitch(void* const hCroPaC, float newPitch);

/*
 * Function: hcropaclib_setRoll
 * ----------------------------
 * Sets the 'roll' rotation angle
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 *     newRoll - the 'roll' rotation angle, in DEGREES
 */
void hcropaclib_setRoll(void* const hCroPaC, float newRoll);

/*
 * Function: hcropaclib_setFlipYaw
 * -------------------------------
 * Sets a flag as to whether to "flip" the sign of the current 'yaw' angle
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newState - 0: do not flip sign, 1: flip the sign
 */
void hcropaclib_setFlipYaw(void* const hCroPaC, int newState);

/*
 * Function: hcropaclib_setFlipPitch
 * ---------------------------------
 * Sets a flag as to whether to "flip" the sign of the current 'pitch' angle
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newState - 0: do not flip sign, 1: flip the sign
 */
void hcropaclib_setFlipPitch(void* const hCroPaC, int newState);

/*
 * Function: hcropaclib_setFlipRoll
 * --------------------------------
 * Sets a flag as to whether to "flip" the sign of the current 'roll' angle
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newState - 0: do not flip sign, 1: flip the sign
 */
void hcropaclib_setFlipRoll(void* const hCroPaC, int newState);

/*
 * Function: hcropaclib_setRPYflag
 * -------------------------------
 * Sets a flag as to whether to use "yaw-pitch-roll" (0) or "roll-pitch-yaw" (1)
 * rotation order.
 *
 * Input Arguments:
 *     hCroPaC  - hcropaclib handle
 *     newState - 0: use "yaw-pitch-roll", 1: use "roll-pitch-yaw"
 */
void hcropaclib_setRPYflag(void* const hCroPaC, int newState);


/* ========================================================================== */
/*                                Get Functions                               */
/* ========================================================================== */

/*
 * Function: hcropaclib_getCodecStatus
 * -----------------------------------
 * Returns current codec status.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     codec status (see 'CODEC_STATUS' enum)
 */
CODEC_STATUS hcropaclib_getCodecStatus(void* const hCroPaC);

/*
 * Function: hcropaclib_getProgressBar0_1
 * --------------------------------------
 * (Optional) Returns current intialisation/processing progress, between 0..1
 * 0: intialisation/processing has started
 * 1: intialisation/processing has ended
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     current progress, 0..1
 */
float hcropaclib_getProgressBar0_1(void* const hCroPaC);

/*
 * Function: hcropaclib_getProgressBarText
 * ---------------------------------------
 * (Optional) Returns current intialisation/processing progress text
 * Note: "text" string should be (at least) of length:
 *     HCROPAC_PROGRESSBARTEXT_CHAR_LENGTH
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Output Arguments:
 *     text    - process bar text; HCROPAC_PROGRESSBARTEXT_CHAR_LENGTH x 1
 */
void hcropaclib_getProgressBarText(void* const hCroPaC, char* text);

int hcropaclib_getEnableCroPaC(void* const hCroPaC);

/*
 * Function: hcropaclib_getBalance
 * -------------------------------
 * Returns the balance between direct and ambient streams (default=1, 50%/50%)
 * for ONE specific frequency band.
 *
 * Input Arguments:
 *     hCroPaC    - hcropaclib handle
 *     bandIdx - frequency band index
 * Returns:
 *     The current balance value, 0: fully ambient, 1: balanced, 2: fully direct
 */
float hcropaclib_getBalance(void* const hCroPaC, int bandIdx);

/*
 * Function: hcropaclib_getBalanceAllBands
 * ---------------------------------------
 * Returns the balance between direct and ambient streams (default=1, 50%/50%)
 * for the FIRST frequency band.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *    Current balance, 0: fully ambient, 1: balanced, 2: fully direct
 */
float hcropaclib_getBalanceAllBands(void* const hCroPaC); /* returns the first value */

/*
 * Function: hcropaclib_getBalanceHandle
 * -------------------------------------
 * Returns a handle for the balance between direct and ambient streams
 * (default=1, 50%/50%) for ALL frequency bands.
 *
 * Input Arguments:
 *     hCroPaC      - hcropaclib handle
 * Output Arguments:
 *     pX_vector - & frequency vector; pNpoints x 1
 *     pY_values - & Balance values per frequency; pNpoints x 1
 *     pNpoints  - & number of frequencies/balance values
 */
void hcropaclib_getBalanceHandle(void* const hCroPaC,
                                  float** pX_vector,
                                  float** pY_values,
                                  int* pNpoints);
    
/*
 * Function: hcropaclib_getCovAvg
 * ------------------------------
 * Returns the covariance matrix averaging coefficient.
 * Note: the coefficient is forced to not exceed 0.99, in order to avoid
 * infinite averaging.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     coefficient value 0..1; 0: no averaging, 1: "infinite"  averaging
 */
float hcropaclib_getCovAvg(void* const hCroPaC);

float hcropaclib_getAnaLimit(void* const hCroPaC);

/*
 * Function: hcropaclib_getUseDefaultHRIRsflag
 * -------------------------------------------
 * Returns the value of a flag used to dictate whether the default HRIRs in the
 * Spatial_Audio_Framework should be used, or a custom HRIR set loaded via a
 * SOFA file.
 * Note: if the custom set failes to load correctly, hcropac will revert to the
 * defualt set.
 *
 * Input Arguments:
 *     hCroPaC     - hcropaclib handle
 * Returns:
 *     0: use custom HRIR set, 1: use default HRIR set
 */
int hcropaclib_getUseDefaultHRIRsflag(void* const hCroPaC);

/*
 * Function: hcropaclib_getSofaFilePath
 * ------------------------------------
 * Returns the file path for a .sofa file.
 * Note: if the custom set failes to load correctly, hcropac will revert to the
 * defualt set. Use 'hcropaclib_getUseDefaultHRIRsflag' to check if loading was
 * successful.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *      file path to .sofa file (WITH file extension)
 */
char* hcropaclib_getSofaFilePath(void* const hCroPaC);

/*
 * Function: hcropaclib_getChOrder
 * -------------------------------
 * Returns the Ambisonic channel ordering convention currently being used to
 * decode with, which should match the convention employed by the input signals
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     convention currently being used (see 'CH_ORDER' enum)
 */
int hcropaclib_getChOrder(void* const hCroPaC);

/*
 * Function: hcropaclib_getNormType
 * ---------------------------------
 * Returns the Ambisonic normalisation convention currently being usedto decode
 * with, which should match the convention employed by the input signals.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     convention currently being used (see 'NORM_TYPE' enum)
 */
int hcropaclib_getNormType(void* const hCroPaC);

/*
 * Function: hcropaclib_getEnableDiffCorrection
 * --------------------------------------------
 * Returns the flag, to say if a covariance diffuseness constraint [1] should be
 * applied to the prototype decoding matrix.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     0: disabled, 1: enabled
 *
 * [1] Zaunschirm M, Schörkhuber C, Höldrich R. Binaural rendering of
 *     Ambisonic signals by head-related impulse response time alignment and
 *     a diffuseness constraint. The Journal of the Acoustical Society of
 *     America. 2018 Jun 19;143(6):3616-27
 */
int hcropaclib_getEnableDiffCorrection(void* const hCroPaC);

int hcropaclib_getNumEars(void);

/*
 * Function: hcropaclib_getNumberOfBands
 * --------------------------------------
 * Returns the number of frequency bands employed by hcropaclib
 *
 * Returns:
 *     number of required spherical harmonic signals required by current
 *     decoding order
 */
int hcropaclib_getNumberOfBands(void);

/*
 * Function: hcropaclib_getNSHrequired
 * ------------------------------------
 * Returns the number of spherical harmonic signals required
 *
 * Returns:
 *     4
 */
int hcropaclib_getNSHrequired(void);

/*
 * Function: hcropaclib_getEnableRotation
 * ---------------------------------------
 * Returns the flag value which dictates whether to enable/disable sound-field
 * rotation.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     0: disable, 1: enable
 */
int hcropaclib_getEnableRotation(void* const hCroPaC);

/*
 * Function: hcropaclib_getYaw
 * ----------------------------
 * Returns the 'yaw' rotation angle
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     the 'yaw' rotation angle, in DEGREES
 */
float hcropaclib_getYaw(void* const hCroPaC);

/*
 * Function: hcropaclib_getPitch
 * ------------------------------
 * Returns the 'pitch' rotation angle
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     the 'pitch' rotation angle, in DEGREES
 */
float hcropaclib_getPitch(void* const hCroPaC);

/*
 * Function: hcropaclib_getRoll
 * -----------------------------
 * Returns the 'roll' rotation angle
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     the 'roll' rotation angle, in DEGREES
 */
float hcropaclib_getRoll(void* const hCroPaC);

/*
 * Function: hcropaclib_getFlipYaw
 * --------------------------------
 * Returns a flag as to whether to "flip" the sign of the current 'yaw' angle
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     0: do not flip sign, 1: flip the sign
 */
int hcropaclib_getFlipYaw(void* const hCroPaC);

/*
 * Function: hcropaclib_getFlipPitch
 * ----------------------------------
 * Returns a flag as to whether to "flip" the sign of the current 'pitch' angle
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     0: do not flip sign, 1: flip the sign
 */
int hcropaclib_getFlipPitch(void* const hCroPaC);

/*
 * Function: hcropaclib_getFlipRoll
 * ---------------------------------
 * Returns a flag as to whether to "flip" the sign of the current 'roll' angle
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     0: do not flip sign, 1: flip the sign
 */
int hcropaclib_getFlipRoll(void* const hCroPaC);

/*
 * Function: hcropaclib_getRPYflag
 * --------------------------------
 * Returns a flag as to whether to use "yaw-pitch-roll" (0) or "roll-pitch-yaw"
 * (1) rotation order.
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     0: use "yaw-pitch-roll", 1: use "roll-pitch-yaw"
 */
int hcropaclib_getRPYflag(void* const hCroPaC);

/*
 * Function: hcropaclib_getNDirs
 * ------------------------------
 * Returns the number of directions in the currently used HRIR set
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     number of HRIR directions
 */
int hcropaclib_getNDirs(void* const hCroPaC);

/*
 * Function: hcropaclib_getNTriangles
 * -----------------------------------
 * Returns the number of triangles given by the convex hull of the HRIR grid
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     number of HRIR grid triangles
 */
int hcropaclib_getNTriangles(void* const hCroPaC);

/*
 * Function: hcropaclib_getHRIRlength
 * -----------------------------------
 * Returns the length of HRIRs in time-domain samples
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     HRIR length in samples
 */
int hcropaclib_getHRIRlength(void* const hCroPaC);

/*
 * Function: hcropaclib_getHRIRsamplerate
 * ---------------------------------------
 * Returns the HRIR sample rate
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     HRIR sampling rate
 */
int hcropaclib_getHRIRsamplerate(void* const hCroPaC);

/*
 * Function: hcropaclib_getDAWsamplerate
 * --------------------------------------
 * Returns the DAW/Host sample rate
 *
 * Input Arguments:
 *     hCroPaC - hcropaclib handle
 * Returns:
 *     DAW/Host sampling rate
 */
int hcropaclib_getDAWsamplerate(void* const hCroPaC);

/*
 * Function: hcropaclib_getProcessingDelay
 * ----------------------------------------
 * Returns the processing delay in samples. May be used for delay compensation
 * features
 *
 * Returns:
 *     processing delay in samples
 */
int hcropaclib_getProcessingDelay(void);
    
    
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __HCROPACLIB_H_INCLUDED__ */
