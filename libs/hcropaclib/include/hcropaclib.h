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

/**
 * @file hcropaclib.h  
 * @brief A first-order parametric binaural Ambisonic decoder for reproducing
 *        ambisonic signals over headphones.
 *
 * The algorithm is based on the segregation of the direct and diffuse streams
 * using the Cross-Pattern Coherence (CroPaC) spatial post-filter.
 *
 * The output of a linear binaural ambisonic decoder is then adaptively mixed,
 * in such a manner that the covariance matrix of the output stream is brought
 * closer to that of the target covariance matrix, derived from the
 * direct/diffuse analysis. For more information on the method, the reader is
 * directed to [1].
 *
 * @see [1] McCormack, L., Delikaris-Manias, S. (2019). "Parametric first-order
 *          ambisonic decoding for headphones utilising the Cross-Pattern
 *          Coherence algorithm". inProc 1st EAA Spatial Audio Signal Processing
 *          Symposium, Paris, France.
 *
 * @author Leo McCormack
 * @date 12.01.2018
 */ 

#ifndef __HCROPACLIB_H_INCLUDED__
#define __HCROPACLIB_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* ========================================================================== */
/*                             Presets + Constants                            */
/* ========================================================================== */
    
/**
 * Available Ambisonic channel ordering conventions
 *
 * @note CH_FUMA only supported for 1st order input.
 */
typedef enum _HCROPAC_CH_ORDER {
    CH_ACN = 1, /**< Ambisonic Channel Numbering (ACN) */
    CH_FUMA     /**< (Obsolete) Furse-Malham/B-format (WXYZ) */
    
} HCROPAC_CH_ORDER;

/** Number of channel ordering options */
#define HCROPAC_NUM_CH_ORDERINGS ( 2 )

/**
 * Available Ambisonic normalisation conventions
 *
 * @note NORM_FUMA only supported for 1st order input and does NOT have the
 *       1/sqrt(2) scaling on the omni.
 */
typedef enum _HCROPAC_NORM_TYPES {
    NORM_N3D = 1, /**< orthonormalised (N3D) */
    NORM_SN3D,    /**< Schmidt semi-normalisation (SN3D) */
    NORM_FUMA     /**< (Obsolete) Same as NORM_SN3D for 1st order */
    
} HCROPAC_NORM_TYPES;

/** Number of normalisation options */
#define HCROPAC_NUM_NORM_TYPES ( 3 )

/** Available HRIR pre-preprocessing options */
typedef enum {
    HRIR_PREPROC_OFF = 1,     /**< No pre-processing active */
    HRIR_PREPROC_EQ,          /**< Diffuse-field EQ (compensates CTF) */
    HRIR_PREPROC_PHASE,       /**< Phase simplification based on ITD */
    HRIR_PREPROC_ALL,         /**< Diffuse-field EQ AND phase-simplification */
}HRIR_PREPROC_OPTIONS;

/**
 * Current status of the codec.
 */
typedef enum _HCROPAC_CODEC_STATUS {
    CODEC_STATUS_INITIALISED = 0, /**< Codec is initialised and ready to process
                                   *   input audio. */
    CODEC_STATUS_NOT_INITIALISED, /**< Codec has not yet been initialised, or
                                   *   the codec configuration has changed.
                                   *   Input audio should not be processed. */
    CODEC_STATUS_INITIALISING     /**< Codec is currently being initialised,
                                   *   input audio should not be processed. */
} HCROPAC_CODEC_STATUS;

#define HCROPAC_PROGRESSBARTEXT_CHAR_LENGTH ( 256 )
#define HCROPAC_ANA_LIMIT_MIN_VALUE ( 4000.0f )
#define HCROPAC_ANA_LIMIT_MAX_VALUE ( 20000.0f )
    
    
/* ========================================================================== */
/*                               Main Functions                               */
/* ========================================================================== */

/**
 * Creates an instance of the mighty hcropaclib
 *
 * @param[in] phCroPaC (&) address of hcropaclib handle
 */
void hcropaclib_create(void** const phCroPaC);

/**
 * Destroys an instance of the mighty hcropaclib
 *
 * @param[in] phCroPaC (&) address of hcropaclib handle
 */
void hcropaclib_destroy(void** const phCroPaC);

/**
 * Initialises an instance of hcropaclib with default settings
 *
 * @param[in] hCroPaC    hcropaclib handle
 * @param[in] samplerate host samplerate.
 */
void hcropaclib_init(void* const hCroPaC,
                     int samplerate);

/**
 * Intialises the codec variables, based on current global/user parameters
 *
 * @param[in] hCroPaC hcropaclib handle
 */
void hcropaclib_initCodec(void* const hCroPaC);

/**
 * Performs CroPaC decoding [1] of input Ambisonic signals to the binaural
 * channels.
 *
 * @param[in] hCroPaC  hcropaclib handle
 * @param[in] inputs   Input channel buffers; 2-D array: nInputs x nSamples
 * @param[in] outputs  Output channel buffers; 2-D array: nOutputs x nSamples
 * @param[in] nInputs  Number of input channels
 * @param[in] nOutputs Number of output channels
 * @param[in] nSamples Number of samples in 'inputs'/'output' matrices
 *
 * @ see [1] McCormack, L., Delikaris-Manias, S. (2019). "Parametric first-order
 *           ambisonic decoding for headphones utilising the Cross-Pattern
 *           Coherence algorithm". inProc 1st EAA Spatial Audio Signal
 *           Processing Symposium, Paris, France.
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

/**
 * Sets all intialisation flags to 1; re-initialise all settings/variables
 * as hcropac is currently configured, at next available opportunity.
 *
 * @param[in] hCroPaC hcropaclib handle
 */
void hcropaclib_refreshParams(void* const hCroPaC);
    
/**
 * Enables/Disables CroPaC processing; if disabled, then the Magnitude least-
 * squares decoder is used instead.
 */
void hcropaclib_setEnableCroPaC(void* const hCroPaC, int newState);
    
/**
 * Sets the balance between direct and ambient streams (default=1) for ONE
 * specific frequency band.
 *
 * @param[in] hCroPaC  hcropaclib handle
 * @param[in] newValue New balance, 0: fully ambient, 1: balanced,
 *                     2: fully direct
 * @param[in] bandIdx  Frequency band index
 */
void hcropaclib_setBalance(void* const hCroPaC, float newValue, int bandIdx);

/**
 * Sets the balance between direct and ambient streams (default=1) for ALL
 * frequency bands (0: fully ambient, 1: balanced, 2: fully direct)
 */
void hcropaclib_setBalanceAllBands(void* const hCroPaC, float newValue);

/**
 * Sets the covariance matrix averaging coefficient.
 *
 * @note The coefficient is capped to not exceed 0.99, in order to avoid
 *       infinite averaging.
 */
void hcropaclib_setCovAvg(void* const hCroPaC, float newValue);

/**
 * Sets the maximum analysis frequency, in Hz.
 */
void hcropaclib_setAnaLimit(void* const hCroPaC, float newValue);

/**
 * Sets flag to dictate whether the default HRIRs in the Spatial_Audio_Framework
 * should be used, or a custom HRIR set loaded via a SOFA file.
 *
 * (0: use custom HRIR set, 1: use default HRIR set)
 * @note If the custom set fails to load correctly, hcropac will revert to the
 *       defualt set. Use 'hcropaclib_getUseDefaultHRIRsflag()' to check if
 *       loading was  successful.
 */
void hcropaclib_setUseDefaultHRIRsflag(void* const hCroPaC, int newState);

/**
 * Sets the file path for a .sofa file, in order to employ a custom HRIR set for
 * the decoding.
 *
 * @note If the custom set fails to load correctly, hcropac will revert to the
 *       defualt set. Use 'hcropaclib_getUseDefaultHRIRsflag()' to check if
 *       loading was  successful.
 *
 * @param[in] hCroPaC hcropaclib handle
 * @param[in] path    File path to .sofa file (WITH file extension)
 */
void hcropaclib_setSofaFilePath(void* const hCroPaC, const char* path);

/**
 * Sets the Ambisonic channel ordering convention to decode with, in order to
 * match the convention employed by the input signals (see 'HCROPAC_CH_ORDER'
 * enum)
 */
void hcropaclib_setChOrder(void* const hCroPaC, int newOrder);

/**
 * Sets the Ambisonic normalisation convention to decode with, in order to match
 * with the convention employed by the input signals (see 'HCROPAC_NORM_TYPE'
 * enum)
 */
void hcropaclib_setNormType(void* const hCroPaC, int newType);

/**
 * Sets the flag, to say if a covariance diffuseness contraint [1] should be
 * applied to the prototype decoding matrix.
 *
 * @see [1] Zaunschirm M, Scho"rkhuber C, Ho"ldrich R. Binaural rendering of
 *          Ambisonic signals by head-related impulse response time alignment
 *          and a diffuseness constraint. The Journal of the Acoustical Society
 *          of America. 2018 Jun 19;143(6):3616-27
 */
void hcropaclib_setEnableDiffCorrection(void* const hCroPaC, int newState);

/** See #HRIR_PREPROC_OPTIONS */
void hcropaclib_setHRIRsPreProc(void* const hCroPaC, HRIR_PREPROC_OPTIONS newState);

/**
 * Sets the flag to enable/disable sound-field rotation.
 */
void hcropaclib_setEnableRotation(void* const hCroPaC, int newState);

/**
 * Sets the 'yaw' rotation angle, in DEGREES
 */
void hcropaclib_setYaw(void* const hCroPaC, float newYaw);

/**
 * Sets the 'pitch' rotation angle, in DEGREES
 */
void hcropaclib_setPitch(void* const hCroPaC, float newPitch);

/**
 * Sets the 'roll' rotation angle, in DEGREES
 */
void hcropaclib_setRoll(void* const hCroPaC, float newRoll);

/**
 * Sets a flag as to whether to "flip" the sign of the current 'yaw' angle
 * (0: do not flip sign, 1: flip the sign).
 */
void hcropaclib_setFlipYaw(void* const hCroPaC, int newState);

/**
 * Sets a flag as to whether to "flip" the sign of the current 'pitch' angle
 * (0: do not flip sign, 1: flip the sign).
 */
void hcropaclib_setFlipPitch(void* const hCroPaC, int newState);

/**
 * Sets a flag as to whether to "flip" the sign of the current 'roll' angle
 * (0: do not flip sign, 1: flip the sign).
 */
void hcropaclib_setFlipRoll(void* const hCroPaC, int newState);

/**
 * Sets a flag as to whether to use "yaw-pitch-roll" (0) or "roll-pitch-yaw" (1)
 * rotation order.
 */
void hcropaclib_setRPYflag(void* const hCroPaC, int newState);


/* ========================================================================== */
/*                                Get Functions                               */
/* ========================================================================== */

/**
 * Returns the processing framesize (i.e., number of samples processed with
 * every _process() call )
 */
int hcropaclib_getFrameSize(void);

/**
 * Returns current codec status (see #_HCROPAC_CODEC_STATUS enum)
 */
HCROPAC_CODEC_STATUS hcropaclib_getCodecStatus(void* const hCroPaC);

/**
 * (Optional) Returns current intialisation/processing progress, between 0..1
 *  - 0: intialisation/processing has started
 *  - 1: intialisation/processing has ended
 */
float hcropaclib_getProgressBar0_1(void* const hCroPaC);

/**
 * (Optional) Returns current intialisation/processing progress text
 *
 * @note "text" string should be (at least) of length:
 *       #HCROPAC_PROGRESSBARTEXT_CHAR_LENGTH
 */
void hcropaclib_getProgressBarText(void* const hCroPaC, char* text);

/**
 * Flag whether CroPaC is enabled (1), or disabled and Mag-LS is used instead
 * (0)
 */
int hcropaclib_getEnableCroPaC(void* const hCroPaC);

/**
 * Returns the balance between direct and ambient streams (default=1, 50%/50%)
 * for ONE specific frequency band.
 */
float hcropaclib_getBalance(void* const hCroPaC, int bandIdx);

/**
 * Returns the balance between direct and ambient streams (default=1, 50%/50%)
 * for the FIRST frequency band.
 */
float hcropaclib_getBalanceAllBands(void* const hCroPaC);

/**
 * Returns a handle for the balance between direct and ambient streams
 * (default=1, 50%/50%) for ALL frequency bands.
 *
 * @param[in]  hCroPaC   hcropaclib handle
 * @param[out] pX_vector (&) frequency vector; pNpoints x 1
 * @param[out] pY_values (&) Balance values per frequency; pNpoints x 1
 * @param[out] pNpoints  (&) number of frequencies/balance values
 */
void hcropaclib_getBalanceHandle(void* const hCroPaC,
                                  float** pX_vector,
                                  float** pY_values,
                                  int* pNpoints);
    
/**
 * Returns the covariance matrix averaging coefficient.
 *
 @note The coefficient is forced to not exceed 0.99, in order to avoid
 *     infinite averaging.
 */
float hcropaclib_getCovAvg(void* const hCroPaC);

/**
 * Returns the maximum CroPaC analysis frequency, in Hz
 */
float hcropaclib_getAnaLimit(void* const hCroPaC);

/**
 * Returns the value of a flag used to dictate whether the default HRIRs in the
 * Spatial_Audio_Framework should be used, or a custom HRIR set loaded via a
 * SOFA file.
 *
 * @note If the custom set fails to load correctly, hcropac will revert to the
 *       default set.
 */
int hcropaclib_getUseDefaultHRIRsflag(void* const hCroPaC);

/**
 * Returns the file path for a .sofa file (WITH file extension).
 *
 * @note If the custom set failes to load correctly, hcropac will revert to the
 *       default set. Use 'hcropaclib_getUseDefaultHRIRsflag()' to check if
 *       loading was successful.
 */
char* hcropaclib_getSofaFilePath(void* const hCroPaC);

/**
 * Returns the Ambisonic channel ordering convention currently being used to
 * decode with, which should match the convention employed by the input signals
 * (see 'HCROPAC_CH_ORDER' enum)
 */
int hcropaclib_getChOrder(void* const hCroPaC);

/**
 * Returns the Ambisonic normalisation convention currently being used to decode
 * with, which should match the convention employed by the input signals
 * convention currently being used (see 'HCROPAC_NORM_TYPE' enum)
 */
int hcropaclib_getNormType(void* const hCroPaC);

/**
 * Returns the flag, to say if a covariance diffuseness constraint [1] should be
 * applied to the prototype decoding matrix.
 *
 * @see [1] Zaunschirm M, Scho"rkhuber C, Ho"ldrich R. Binaural rendering of
 *          Ambisonic signals by head-related impulse response time alignment
 *          and a diffuseness constraint. The Journal of the Acoustical Society
 *          of America. 2018 Jun 19;143(6):3616-27
 */
int hcropaclib_getEnableDiffCorrection(void* const hCroPaC);

/** Returns current HRIR_PREPROC_OPTIONS option */
HRIR_PREPROC_OPTIONS hcropaclib_getHRIRsPreProc(void* const hCroPaC);

int hcropaclib_getNumEars(void);

/**
 * Returns the number of frequency bands employed by hcropaclib
 */
int hcropaclib_getNumberOfBands(void);

/*
 *
 * Returns the number of spherical harmonic signals required
 */
int hcropaclib_getNSHrequired(void);

/**
 * Returns the flag value which dictates whether to enable/disable sound-field
 * rotation.
 */
int hcropaclib_getEnableRotation(void* const hCroPaC);

/**
 * Returns the 'yaw' rotation angle, in DEGREES
 */
float hcropaclib_getYaw(void* const hCroPaC);

/**
 * Returns the 'pitch' rotation angle, in DEGREES
 */
float hcropaclib_getPitch(void* const hCroPaC);

/**
 * Returns the 'roll' rotation angle, in DEGREES
 */
float hcropaclib_getRoll(void* const hCroPaC);

/**
 * Returns a flag as to whether to "flip" the sign of the current 'yaw' angle
 * (0: do not flip sign, 1: flip the sign)
 */
int hcropaclib_getFlipYaw(void* const hCroPaC);

/**
 * Returns a flag as to whether to "flip" the sign of the current 'pitch' angle
 * (0: do not flip sign, 1: flip the sign)
 */
int hcropaclib_getFlipPitch(void* const hCroPaC);

/**
 * Returns a flag as to whether to "flip" the sign of the current 'roll' angle
 * (0: do not flip sign, 1: flip the sign)
 */
int hcropaclib_getFlipRoll(void* const hCroPaC);

/**
 * Returns a flag as to whether to use "yaw-pitch-roll" (0) or "roll-pitch-yaw"
 * (1) rotation order.
 */
int hcropaclib_getRPYflag(void* const hCroPaC);

/**
 * Returns the number of directions in the currently used HRIR set
 */
int hcropaclib_getNDirs(void* const hCroPaC);

/**
 * Returns the number of triangles given by the convex hull of the HRIR grid
 */
int hcropaclib_getNTriangles(void* const hCroPaC);

/**
 * Returns the length of HRIRs in time-domain samples
 */
int hcropaclib_getHRIRlength(void* const hCroPaC);

/**
 * Returns the HRIR sample rate
 */
int hcropaclib_getHRIRsamplerate(void* const hCroPaC);

/**
 * Returns the DAW/Host sample rate
 */
int hcropaclib_getDAWsamplerate(void* const hCroPaC);

/**
 * Returns the processing delay in samples; may be used for delay compensation
 * features
 */
int hcropaclib_getProcessingDelay(void);
    
    
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __HCROPACLIB_H_INCLUDED__ */
