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
 * @file hcropac_internal.h
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

#ifndef __HCROPAC_INTERNAL_H_INCLUDED__
#define __HCROPAC_INTERNAL_H_INCLUDED__

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "hcropaclib.h" 
#include "saf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* ========================================================================== */
/*                               Internal Enums                               */
/* ========================================================================== */

/**
 * Current status of the processing loop. 
 */
typedef enum _HCROPAC_PROC_STATUS{
    PROC_STATUS_ONGOING = 0, /**< Codec is processing input audio, and should
                              *   not be reinitialised at this time.*/
    PROC_STATUS_NOT_ONGOING  /**< Codec is not processing input audio, and may
                              *   be reinitialised if needed.*/
}HCROPAC_PROC_STATUS;

    
/* ========================================================================== */
/*                           Configurations Options                           */
/* ========================================================================== */

#define ENABLE_RESIDUAL_STREAM    /* comment out to disable */
//#define ENABLE_BINAURAL_DIFF_COH  /* binaural diffuse coherence, comment out to disable */


/* ========================================================================== */
/*                            Internal Parameters                             */
/* ========================================================================== */

#ifndef FRAME_SIZE
# define FRAME_SIZE ( 512 )
#endif
#define HOP_SIZE ( 128 )                                    /* STFT hop size = nBands */
#define HYBRID_BANDS ( HOP_SIZE + 5 )                       /* hybrid mode incurs an additional 5 bands  */
#define TIME_SLOTS ( FRAME_SIZE / HOP_SIZE )                /* 4/8/16 */
#define SH_ORDER ( 1 )                                      /* first-order only */
#define NUM_SH_SIGNALS ( (SH_ORDER+1)*(SH_ORDER+1) )
#define POST_GAIN_DB ( 3.0f )
#ifdef ENABLE_RESIDUAL_STREAM
# define NUM_DECOR_FRAMES ( 8 )
#endif
#ifndef DEG2RAD
# define DEG2RAD(x) (x * SAF_PI / 180.0f)
#endif
#ifndef RAD2DEG
# define RAD2DEG(x) (x * 180.0f / SAF_PI)
#endif
    
    
/* ========================================================================== */
/*                                 Structures                                 */
/* ========================================================================== */

/**
 * Contains variables for source DoA analysis, diffuse stream rendering, ERB
 * grouping, sofa file loading, HRTF rendering, HRTF interpolation.
 */
typedef struct _codecPars
{
    /* Prototype Decoder */
    float_complex M_dec[HYBRID_BANDS][NUM_EARS][NUM_SH_SIGNALS];
    float_complex M_dec_norm[HYBRID_BANDS][NUM_EARS][NUM_SH_SIGNALS];
    
    /* sofa file data */
    char* sofa_filepath;               /* absolute/relevative file path for a sofa file */
    float* hrirs;                      /* time domain HRIRs; N_hrir_dirs x 2 x hrir_len */
    float* hrir_dirs_deg;              /* directions of the HRIRs in degrees [azi elev]; N_hrir_dirs x 2 */
    int N_hrir_dirs;                   /* number of HRIR directions in the current sofa file */
    int hrir_len;                      /* length of the HRIRs, this can be truncated, see "saf_sofa_reader.h" */
    int hrir_fs;                       /* sampling rate of the HRIRs, should ideally match the host sampling rate, although not required */
    int N_Tri;
    
    /* hrir filterbank coefficients */
    float* itds_s;                     /* interaural-time differences for each HRIR (in seconds); N_hrirs x 1 */
    float_complex* hrtf_fb;            /* HRTF filterbank coeffs; HYBRID_BANDS x 2 x N_hrir_dirs  */
    float* hrtf_fb_mag;                /* abs(HRTF filterbank coeffs); HYBRID_BANDS x 2 x N_hrir_dirs */
#ifdef ENABLE_BINAURAL_DIFF_COH
    float binDiffuseCoh[HYBRID_BANDS]; /* binaural diffuse coherence per band; HYBRID_BANDS x 1 */
#endif
    
    /* for interpolation of HRTFs */ 
    float* vbap_gtableComp;
    int* vbap_gtableIdx;
    int N_hrtf_vbap_gtable;
    int hrtf_nTriangles;
    int az_res;
    int el_res;
    
    /* scanning grid */
    float* grid_dirs_deg;              /* grid_nDirs x 2 */
    int grid_nDirs;
    float_complex* pwdmap_cmplx;       /* TIME_SLOTS x grid_nDirs */
    float* Y_grid;                     /* NUM_SH_SIGNALS x grid_nDirs */
    float_complex* Y_grid_cmplx;       /* NUM_SH_SIGNALS x grid_nDirs */
    float_complex* M_rot;              /* grid_nDirs * NUM_SH_SIGNALS * NUM_SH_SIGNALS */
    
}codecPars;

/**
 * Main structure for hcropac. Contains variables for audio buffers, afSTFT,
 * mixing matrices, internal variables, flags, user parameters
 */
typedef struct _hcropaclib
{
    /* audio buffers + afSTFT time-frequency transform handle */
    float** SHFrameTD;
    float** binFrameTD;
    float_complex*** SHframeTF;
    float_complex** SHframeTF_rot;
    float_complex*** ambiframeTF;
    float_complex*** binframeTF;
    float interpolator[TIME_SLOTS];
    void* hSTFT;                             /* afSTFT handle */
    int afSTFTdelay;                         /* for host delay compensation */
    int fs;                                  /* host sampling rate */
    float freqVector[HYBRID_BANDS];          /* frequency vector for time-frequency transform, in Hz */
    
    /* our codec configuration */
    HCROPAC_CODEC_STATUS codecStatus;
    float progressBar0_1;
    char* progressBarText;
    codecPars* pars;                         /* codec parameters */
    void* hCdf;                              /* covariance domain framework handle */
#ifdef ENABLE_RESIDUAL_STREAM
    void* hCdf_res;                          /* covariance domain framework handle for the residual */
#endif
    
    /* internal */
    HCROPAC_PROC_STATUS procStatus;
    float_complex Cx[HYBRID_BANDS][NUM_SH_SIGNALS][NUM_SH_SIGNALS];
    float_complex Cy[HYBRID_BANDS][NUM_EARS][NUM_EARS];
    float_complex Cambi[HYBRID_BANDS][NUM_EARS][NUM_EARS];
    float_complex new_M[HYBRID_BANDS][NUM_EARS][NUM_EARS]; 
    float_complex current_M[HYBRID_BANDS][NUM_EARS][NUM_EARS];
#ifdef ENABLE_RESIDUAL_STREAM
    float new_Mr[HYBRID_BANDS][NUM_EARS][NUM_EARS];
    float current_Mr[HYBRID_BANDS][NUM_EARS][NUM_EARS];
    float_complex decorrelatedframeTF[HYBRID_BANDS][NUM_EARS][TIME_SLOTS];
    float_complex circBufferFrames[HYBRID_BANDS][NUM_EARS][(NUM_DECOR_FRAMES+1)*TIME_SLOTS];
    int decorrelationDelays[HYBRID_BANDS][NUM_EARS];
    float transientDetector1[HYBRID_BANDS][NUM_EARS];
    float transientDetector2[HYBRID_BANDS][NUM_EARS];
#endif
    float_complex M_rot[NUM_SH_SIGNALS][NUM_SH_SIGNALS];
    int recalc_M_rotFLAG;                    /* 0: no init required, 1: init required */
    
    /* user parameters */
    int enableCroPaC;                        /* 0: Ambisonic decoder, 1: CroPaC decoder */
    float EQ[HYBRID_BANDS];                  /* EQ curve */
    float balance[HYBRID_BANDS];             /* 0: only diffuse, 1: equal, 2: only directional */ 
    int diffCorrection;                      /* 0:disabled, 1: enabled */
    int useDefaultHRIRsFLAG;                 /* 1: use default HRIRs in database, 0: use those from SOFA file */
    HCROPAC_CH_ORDER chOrdering;             /* ACN or FuMa */
    HCROPAC_NORM_TYPES norm;                 /* N3D or SN3D */
    float covAvgCoeff;                       /* averaging coefficient for covarience matrix */
    float anaLimit_hz;                       /* frequency up to which to perform CroPaC analysis, Hz */
    int enableRotation;                      /* 1: enable rotation, 0: disable */
    float yaw, roll, pitch;                  /* rotation angles in degrees */
    int bFlipYaw, bFlipPitch, bFlipRoll;     /* flag to flip the sign of the individual rotation angles */
    int useRollPitchYawFlag;                 /* rotation order flag, 1: r-p-y, 0: y-p-r */
    
} hcropaclib_data;


/* ========================================================================== */
/*                             Internal Functions                             */
/* ========================================================================== */

/**
 * Sets codec status (see 'HCROPAC_CODEC_STATUS' enum)
 */
void hcropaclib_setCodecStatus(void* const hCroPaC,
                               HCROPAC_CODEC_STATUS newStatus);

/**
 * Interpolate HRTFs for each time slot */
void hcropaclib_interpHRTFs(void* const hCroPaC,
                            int band,
                            float secAzi[TIME_SLOTS],
                            float secElev[TIME_SLOTS],
                            float_complex h_intrp[TIME_SLOTS][NUM_EARS]);

    
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __HCROPAC_INTERNAL_H_INCLUDED__ */
