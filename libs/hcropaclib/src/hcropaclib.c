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
 * @file hcropaclib.c
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
 
#include "hcropac_internal.h"

void hcropaclib_create
(
    void ** const phCroPaC
)
{
    hcropaclib_data* pData = (hcropaclib_data*)malloc1d(sizeof(hcropaclib_data));
    *phCroPaC = (void*)pData;
    int band;

    /* default user parameters */
    pData->enableCroPaC = 1;
    for (band = 0; band<HYBRID_BANDS; band++){
        pData->EQ[band] = 1.0f;
        pData->balance[band] = 1.0f;
    }
    pData->useDefaultHRIRsFLAG = 1; /* pars->sofa_filepath must be valid to set this to 0 */
    pData->chOrdering = CH_ACN;
    pData->norm = NORM_SN3D;
    pData->diffCorrection = 0;
    pData->hrirProcMode = HRIR_PREPROC_ALL;
    pData->covAvgCoeff = 0.75f;
    pData->anaLimit_hz = 18e3f;
    pData->enableRotation = 0;
    pData->yaw = 0.0f;
    pData->pitch = 0.0f;
    pData->roll = 0.0f;
    pData->bFlipYaw = 0;
    pData->bFlipPitch = 0;
    pData->bFlipRoll = 0;
    pData->useRollPitchYawFlag = 0;
    
    /* afSTFT stuff */
    afSTFT_create(&(pData->hSTFT), NUM_SH_SIGNALS, NUM_EARS, HOP_SIZE, 0, 1, AFSTFT_BANDS_CH_TIME);
    pData->SHFrameTD = (float**)malloc2d(NUM_SH_SIGNALS, FRAME_SIZE, sizeof(float));
    pData->binFrameTD = (float**)malloc2d(NUM_EARS, FRAME_SIZE, sizeof(float));
    pData->SHframeTF = (float_complex***)malloc3d(HYBRID_BANDS, NUM_SH_SIGNALS, TIME_SLOTS, sizeof(float_complex));
    pData->SHframeTF_rot = (float_complex**)malloc2d(NUM_SH_SIGNALS, TIME_SLOTS, sizeof(float_complex));
    pData->ambiframeTF = (float_complex***)malloc3d(HYBRID_BANDS, NUM_EARS, TIME_SLOTS, sizeof(float_complex));
    pData->binframeTF= (float_complex***)malloc3d(HYBRID_BANDS, NUM_EARS, TIME_SLOTS, sizeof(float_complex));

    /* codec data */
    pData->progressBar0_1 = 0.0f;
    pData->progressBarText = malloc1d(HCROPAC_PROGRESSBARTEXT_CHAR_LENGTH*sizeof(char));
    strcpy(pData->progressBarText,"");
    pData->pars = (codecPars*)malloc1d(sizeof(codecPars));
    codecPars* pars = pData->pars; 
    pars->sofa_filepath = NULL;
    pars->hrirs = NULL;
    pars->hrir_dirs_deg = NULL;
    pars->itds_s = NULL;
    pars->hrtf_fb = NULL;
    pars->hrtf_fb_mag = NULL;
    pars->pwdmap_cmplx = NULL;
    pars->M_rot = NULL;
    pars->vbap_gtableComp = NULL;
    pars->vbap_gtableIdx = NULL;
    pars->Y_grid = NULL;
    pars->Y_grid_cmplx = NULL;
    cdf4sap_cmplx_create(&(pData->hCdf), NUM_EARS, NUM_EARS);
#ifdef ENABLE_RESIDUAL_STREAM
    cdf4sap_create(&(pData->hCdf_res), NUM_EARS, NUM_EARS);
#endif
    
    /* flags */
    pData->procStatus = PROC_STATUS_NOT_ONGOING;
    pData->codecStatus = CODEC_STATUS_NOT_INITIALISED;
    pData->recalc_M_rotFLAG = 1;
}

void hcropaclib_destroy
(
    void ** const phCroPaC
)
{
    hcropaclib_data *pData = (hcropaclib_data*)(*phCroPaC);
    codecPars *pars;
    
    if (pData != NULL) {
        /* not safe to free memory during intialisation/processing loop */
        while (pData->codecStatus == CODEC_STATUS_INITIALISING ||
               pData->procStatus == PROC_STATUS_ONGOING){
            SAF_SLEEP(10);
        }
        
        /* free afSTFT and buffers */
        if(pData->hSTFT!=NULL)
            afSTFT_destroy(&(pData->hSTFT));
        free(pData->SHFrameTD);
        free(pData->binFrameTD);
        free(pData->SHframeTF);
        free(pData->SHframeTF_rot);
        free(pData->ambiframeTF);
        free(pData->binframeTF);

        pars = pData->pars;
        free(pars->hrtf_fb);
        free(pars->hrtf_fb_mag);
        free(pars->itds_s);
        free(pars->hrirs);
        free(pars->hrir_dirs_deg);
        free(pars->pwdmap_cmplx);
        free(pars->M_rot);
        free(pars->vbap_gtableComp);
        free(pars->vbap_gtableIdx);
        free(pars->Y_grid);
        free(pars->Y_grid_cmplx);
        free(pars);
        
        cdf4sap_cmplx_destroy(&(pData->hCdf));
#ifdef ENABLE_RESIDUAL_STREAM
        cdf4sap_destroy(&(pData->hCdf_res));
#endif
        free(pData->progressBarText);
        free(pData);
        pData = NULL;
    }
}

void hcropaclib_init
(
    void * const hCroPaC,
    int          sampleRate
)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    int t;
    
    /* define frequency vector */
    pData->fs = sampleRate;
    afSTFT_getCentreFreqs(pData->hSTFT, (float)sampleRate, HYBRID_BANDS, pData->freqVector);
    
    /* default starting values */
    memset(pData->Cx, 0, HYBRID_BANDS*NUM_SH_SIGNALS*NUM_SH_SIGNALS*sizeof(float_complex));
    memset(pData->Cy, 0, HYBRID_BANDS*NUM_EARS*NUM_EARS*sizeof(float_complex));
    memset(pData->Cambi, 0, HYBRID_BANDS*NUM_EARS*NUM_EARS*sizeof(float_complex));
    memset(pData->current_M, 0, HYBRID_BANDS*NUM_EARS*NUM_EARS*sizeof(float_complex));
#ifdef ENABLE_RESIDUAL_STREAM
    memset(pData->current_Mr, 0, HYBRID_BANDS*NUM_EARS*NUM_EARS*sizeof(float));
    memset(pData->transientDetector1, 0, HYBRID_BANDS*NUM_EARS*sizeof(float));
    memset(pData->transientDetector2, 0, HYBRID_BANDS*NUM_EARS*sizeof(float));
    memset(pData->circBufferFrames, 0, HYBRID_BANDS*NUM_EARS*TIME_SLOTS*(NUM_DECOR_FRAMES+1)*sizeof(float_complex));
#endif
    memset(pData->M_rot, 0, NUM_SH_SIGNALS*NUM_SH_SIGNALS*sizeof(float_complex));
    pData->recalc_M_rotFLAG = 1;
    
    /* interpolator */
    for(t=0; t<TIME_SLOTS; t++)
        pData->interpolator[t] = ((float)t+1.0f)/TIME_SLOTS;
}

void hcropaclib_initCodec
(
    void* const hCroPaC
)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    codecPars* pars = pData->pars;
    int i, j, k, band;
    float Rxyz[3][3];
    float* M_rot_tmp;
#ifdef SAF_ENABLE_SOFA_READER_MODULE
    SAF_SOFA_ERROR_CODES error;
    saf_sofa_container sofa;
#endif
    
    if (pData->codecStatus != CODEC_STATUS_NOT_INITIALISED)
        return; /* re-init not required, or already happening */
    while (pData->procStatus == PROC_STATUS_ONGOING){
        /* re-init required, but we need to wait for the current processing loop to end */
        pData->codecStatus = CODEC_STATUS_INITIALISING; /* indicate that we want to init */
        SAF_SLEEP(10);
    }
    
    /* for progress bar */
    pData->codecStatus = CODEC_STATUS_INITIALISING;
    strcpy(pData->progressBarText,"Preparing HRIRs");
    pData->progressBar0_1 = 0.0f;
    
    /* clear afSTFT buffers */
    afSTFT_clearBuffers(pData->hSTFT);
    
    /* ----- LOAD HRIRs ----- */
    /* load sofa file or load default hrir data */
#ifdef SAF_ENABLE_SOFA_READER_MODULE
    if(!pData->useDefaultHRIRsFLAG && pars->sofa_filepath!=NULL){
        /* Load SOFA file */
        error = saf_sofa_open(&sofa, pars->sofa_filepath, SAF_SOFA_READER_OPTION_DEFAULT);

        /* Load defaults instead */
        if(error!=SAF_SOFA_OK || sofa.nReceivers!=NUM_EARS){
            pData->useDefaultHRIRsFLAG = 1;
            saf_print_warning("Unable to load the specified SOFA file, or it contained something other than 2 channels. Using default HRIR data instead.");
        }
        else{
            /* Copy SOFA data */
            pars->hrir_fs = (int)sofa.DataSamplingRate;
            pars->hrir_len = sofa.DataLengthIR;
            pars->N_hrir_dirs = sofa.nSources;
            pars->hrirs = realloc1d(pars->hrirs, pars->N_hrir_dirs*NUM_EARS*(pars->hrir_len)*sizeof(float));
            memcpy(pars->hrirs, sofa.DataIR, pars->N_hrir_dirs*NUM_EARS*(pars->hrir_len)*sizeof(float));
            pars->hrir_dirs_deg = realloc1d(pars->hrir_dirs_deg, pars->N_hrir_dirs*2*sizeof(float));
            cblas_scopy(pars->N_hrir_dirs, sofa.SourcePosition, 3, pars->hrir_dirs_deg, 2); /* azi */
            cblas_scopy(pars->N_hrir_dirs, &sofa.SourcePosition[1], 3, &pars->hrir_dirs_deg[1], 2); /* elev */
        }

        /* Clean-up */
        saf_sofa_close(&sofa);
    }
#else
    pData->useDefaultHRIRsFLAG = 1; /* Can only load the default HRIR data */
#endif
    if(pData->useDefaultHRIRsFLAG){
        /* Copy default HRIR data */
        pars->hrir_fs = __default_hrir_fs;
        pars->hrir_len = __default_hrir_len;
        pars->N_hrir_dirs = __default_N_hrir_dirs;
        pars->hrirs = realloc1d(pars->hrirs, pars->N_hrir_dirs*NUM_EARS*(pars->hrir_len)*sizeof(float));
        memcpy(pars->hrirs, (float*)__default_hrirs, pars->N_hrir_dirs*NUM_EARS*(pars->hrir_len)*sizeof(float));
        pars->hrir_dirs_deg = realloc1d(pars->hrir_dirs_deg, pars->N_hrir_dirs*2*sizeof(float));
        memcpy(pars->hrir_dirs_deg, (float*)__default_hrir_dirs_deg, pars->N_hrir_dirs*2*sizeof(float));
    }
    
    /* estimate the ITDs for each HRIR */
    pars->itds_s = realloc1d(pars->itds_s, pars->N_hrir_dirs*sizeof(float));
    estimateITDs(pars->hrirs, pars->N_hrir_dirs, pars->hrir_len, pars->hrir_fs, pars->itds_s);
    
    pData->progressBar0_1 = 0.4f;
    
    /* convert hrirs to filterbank coefficients */
    pars->hrtf_fb = realloc1d(pars->hrtf_fb, HYBRID_BANDS * NUM_EARS * (pars->N_hrir_dirs)*sizeof(float_complex));
    HRIRs2HRTFs_afSTFT(pars->hrirs, pars->N_hrir_dirs, pars->hrir_len, HOP_SIZE, 0, 1, pars->hrtf_fb);
    diffuseFieldEqualiseHRTFs(pars->N_hrir_dirs, pars->itds_s, pData->freqVector, HYBRID_BANDS, NULL,
                              pData->hrirProcMode == HRIR_PREPROC_ALL || pData->hrirProcMode == HRIR_PREPROC_EQ ? 1 : 0,
                              pData->hrirProcMode == HRIR_PREPROC_ALL || pData->hrirProcMode == HRIR_PREPROC_PHASE ? 1 : 0,
                              pars->hrtf_fb);
    pars->hrtf_fb_mag = realloc1d(pars->hrtf_fb_mag, HYBRID_BANDS*NUM_EARS* (pars->N_hrir_dirs)*sizeof(float));
    for(i=0; i<HYBRID_BANDS*NUM_EARS* (pars->N_hrir_dirs); i++)
        pars->hrtf_fb_mag[i] = cabsf(pars->hrtf_fb[i]);
#ifdef ENABLE_BINAURAL_DIFF_COH
    binauralDiffuseCoherence(pars->hrtf_fb, pars->itds_s, pData->freqVector, pars->N_hrir_dirs, HYBRID_BANDS, (float*)pars->binDiffuseCoh);
#endif
    
    /* compressed HRTF interpolation table */
    strcpy(pData->progressBarText,"Computing interpolation table");
    pData->progressBar0_1 = 0.85f;
    float* hrtf_vbap_gtable = NULL;
    pars->az_res = 1;
    pars->el_res = 4;
    generateVBAPgainTable3D(pars->hrir_dirs_deg,  pars->N_hrir_dirs, pars->az_res, pars->el_res, 0, 0, 0.0f,
                            &hrtf_vbap_gtable, &(pars->N_hrtf_vbap_gtable), &(pars->hrtf_nTriangles));
    pars->vbap_gtableComp = realloc1d(pars->vbap_gtableComp, pars->N_hrtf_vbap_gtable*3*sizeof(float));
    pars->vbap_gtableIdx = realloc1d(pars->vbap_gtableIdx, pars->N_hrtf_vbap_gtable*3*sizeof(int));
    compressVBAPgainTable3D(hrtf_vbap_gtable, pars->N_hrtf_vbap_gtable, pars->N_hrir_dirs, pars->vbap_gtableComp, pars->vbap_gtableIdx);
    free(hrtf_vbap_gtable);

    /* get integration weights */
    float* weights;
    if(pars->N_hrir_dirs<1800){
        weights = malloc1d(pars->N_hrir_dirs*sizeof(float));
        getVoronoiWeights(pars->hrir_dirs_deg, pars->N_hrir_dirs, 0, weights);
    }
    else
        weights = NULL;
    
    /* ----- COMPUTE PROTO DECODER ----- */
    float_complex* decMtx;
    decMtx = calloc1d(HYBRID_BANDS*NUM_EARS*NUM_SH_SIGNALS, sizeof(float_complex));
    getBinauralAmbiDecoderMtx(pars->hrtf_fb, pars->hrir_dirs_deg, pars->N_hrir_dirs, HYBRID_BANDS, BINAURAL_DECODER_MAGLS, SH_ORDER, pData->freqVector, pars->itds_s, weights, pData->diffCorrection, 1, decMtx);
    
    /* replace current decoder */
    memset(pars->M_dec, 0, HYBRID_BANDS*NUM_EARS*NUM_SH_SIGNALS*sizeof(float_complex));
    for(band=0; band<HYBRID_BANDS; band++)
        for(i=0; i<NUM_EARS; i++)
            for(j=0; j<NUM_SH_SIGNALS; j++)
                pars->M_dec[band][i][j] = decMtx[band*NUM_EARS*NUM_SH_SIGNALS + i*NUM_SH_SIGNALS + j];
    free(decMtx);
    free(weights);
  
    /* ----- SCANNING GRID ----- */
    strcpy(pData->progressBarText,"Computing scanning grid");
    pData->progressBar0_1 = 0.95f;
    int geosphere_ico_freq = 12;
    pars->grid_dirs_deg = (float*)__HANDLES_geosphere_ico_dirs_deg[geosphere_ico_freq];
    pars->grid_nDirs = __geosphere_ico_nPoints[geosphere_ico_freq];
    pars->Y_grid = realloc1d(pars->Y_grid, NUM_SH_SIGNALS*(pars->grid_nDirs)*sizeof(float));
    getRSH(SH_ORDER, pars->grid_dirs_deg, pars->grid_nDirs, pars->Y_grid);
    pars->Y_grid_cmplx = realloc1d(pars->Y_grid_cmplx, NUM_SH_SIGNALS * (pars->grid_nDirs)*sizeof(float_complex));
    for(i=0; i<NUM_SH_SIGNALS; i++)
        for(j=0; j<pars->grid_nDirs; j++)
            pars->Y_grid_cmplx[i*(pars->grid_nDirs)+j] = cmplxf(pars->Y_grid[i*(pars->grid_nDirs)+j], 0.0f);
    pars->pwdmap_cmplx = realloc1d(pars->pwdmap_cmplx, TIME_SLOTS*(pars->grid_nDirs)*sizeof(float_complex));
    pars->M_rot = realloc1d(pars->M_rot, pars->grid_nDirs*NUM_SH_SIGNALS*NUM_SH_SIGNALS*sizeof(float_complex));
    
    /* rotation matrices for each grid direction */
    M_rot_tmp = malloc1d(NUM_SH_SIGNALS*NUM_SH_SIGNALS * sizeof(float));
    for(i=0; i<pars->grid_nDirs; i++){
        yawPitchRoll2Rzyx(pars->grid_dirs_deg[i*2]*SAF_PI/180.0f, -pars->grid_dirs_deg[i*2+1]*SAF_PI/180.0f, 0.0f, 0, Rxyz);
        getSHrotMtxReal(Rxyz, M_rot_tmp, SH_ORDER);
        for (j = 0; j < NUM_SH_SIGNALS; j++)
            for (k = 0; k < NUM_SH_SIGNALS; k++)
                pars->M_rot[i*NUM_SH_SIGNALS*NUM_SH_SIGNALS + j*NUM_SH_SIGNALS + k] = cmplxf(M_rot_tmp[j*NUM_SH_SIGNALS + k], 0.0f);
    }
    free(M_rot_tmp);
    
    /* ----- RESIDUAL PROCESSING ----- */
#ifdef ENABLE_RESIDUAL_STREAM
    getDecorrelationDelays(NUM_EARS, pData->freqVector, HYBRID_BANDS, (float)pData->fs, NUM_DECOR_FRAMES*TIME_SLOTS, HOP_SIZE, &(pData->decorrelationDelays[0][0]));
#endif
    
    /* done! */
    strcpy(pData->progressBarText,"Done!");
    pData->progressBar0_1 = 1.0f;
    pData->codecStatus = CODEC_STATUS_INITIALISED;
}

void hcropaclib_process
(
    void  *  const hCroPaC,
    float ** const inputs,
    float ** const outputs,
    int            nInputs,
    int            nOutputs,
    int            nSamples
)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    codecPars* pars = pData->pars;
    int n, t, ch, i, j, band;
    int o[SH_ORDER + 2], dir_max_idx[TIME_SLOTS];
    const float_complex calpha = cmplxf(1.0f, 0.0f), cbeta = cmplxf(0.0f, 0.0f);
    float inputEnergy, G, Ex, Eambi;
    float Rxyz[3][3]; // ambiFrame_norm[NUM_EARS][TIME_SLOTS],
    float azi[TIME_SLOTS], elev[TIME_SLOTS]; 
#ifdef ENABLE_RESIDUAL_STREAM
    float_complex Cr[NUM_EARS][NUM_EARS];
    float Cr_real[NUM_EARS][NUM_EARS];
    float real_eye2[NUM_EARS][NUM_EARS] = { {1.0f, 0.0f}, {0.0f, 1.0f} };
#endif
    float_complex Cx_new[NUM_SH_SIGNALS][NUM_SH_SIGNALS], Cambi_new[NUM_EARS][NUM_EARS];
    float_complex inputFrame_s[NUM_SH_SIGNALS], inputFrame_rot[NUM_SH_SIGNALS];
    float_complex Cdir[NUM_EARS][NUM_EARS], Cdiff[NUM_EARS][NUM_EARS], hrtf_interp[TIME_SLOTS][NUM_EARS];
    float_complex inFrame_t[NUM_EARS], outFrame_t[NUM_EARS], interp_M[NUM_EARS][NUM_EARS];
    float_complex eye2[NUM_EARS][NUM_EARS];
    float_complex B, GB[TIME_SLOTS], w[NUM_SH_SIGNALS], y[TIME_SLOTS][NUM_SH_SIGNALS];
    float_complex y_dir[TIME_SLOTS][NUM_EARS], y_diff[TIME_SLOTS][NUM_EARS];
    float_complex a_diff[NUM_SH_SIGNALS];
#ifdef ENABLE_BINAURAL_DIFF_COH
    float_complex U[NUM_EARS][NUM_EARS], U_Cdiff[NUM_EARS][NUM_EARS];
#endif
    float* M_rot_tmp;
    for (i = 0; i < NUM_EARS; i++)
        for (j = 0; j < NUM_EARS; j++)
            eye2[i][j] = i == j ? cmplxf(1.0f, 0.0f) : cmplxf(0.0f, 0.0f);

    /* local copies of user parameters */
    int enableRot, enableCroPaC;
    float covAvgCoeff, anaLim;
    float balance[HYBRID_BANDS];
    HCROPAC_NORM_TYPES norm;
    HCROPAC_CH_ORDER chOrdering;
    
    /* decode audio to headphones */
    if ( (nSamples == FRAME_SIZE) && (pData->codecStatus == CODEC_STATUS_INITIALISED) ) {
        pData->procStatus = PROC_STATUS_ONGOING;
        
        /* copy user parameters to local variables */
        for(n=0; n<SH_ORDER+2; n++){  o[n] = n*n;  }
        norm = pData->norm;
        chOrdering = pData->chOrdering;
        enableRot = pData->enableRotation;
        covAvgCoeff = pData->covAvgCoeff;
        enableCroPaC = pData->enableCroPaC;
        anaLim = pData->anaLimit_hz;
        memcpy(balance, pData->balance, HYBRID_BANDS*sizeof(float));

        /* Load time-domain data */
        for(i=0; i < SAF_MIN(NUM_SH_SIGNALS, nInputs); i++)
            utility_svvcopy(inputs[i], FRAME_SIZE, pData->SHFrameTD[i]);
        for(; i<NUM_SH_SIGNALS; i++)
            memset(pData->SHFrameTD[i], 0, FRAME_SIZE * sizeof(float)); /* fill remaining channels with zeros */

        /* account for channel order convention */
        switch(chOrdering){
            case CH_ACN:
                convertHOAChannelConvention(FLATTEN2D(pData->SHFrameTD), SH_ORDER, FRAME_SIZE, HOA_CH_ORDER_ACN, HOA_CH_ORDER_ACN);
                break;
            case CH_FUMA:
                convertHOAChannelConvention(FLATTEN2D(pData->SHFrameTD), SH_ORDER, FRAME_SIZE, HOA_CH_ORDER_FUMA, HOA_CH_ORDER_ACN);
                break;
        }

        /* account for input normalisation scheme */
        switch(norm){
            case NORM_N3D:  /* already in N3D, do nothing */
                break;
            case NORM_SN3D: /* convert to N3D */
                convertHOANormConvention(FLATTEN2D(pData->SHFrameTD), SH_ORDER, FRAME_SIZE, HOA_NORM_SN3D, HOA_NORM_N3D);
                break;
            case NORM_FUMA: /* only for first-order, convert to N3D */
                convertHOANormConvention(FLATTEN2D(pData->SHFrameTD), SH_ORDER, FRAME_SIZE, HOA_NORM_FUMA, HOA_NORM_N3D);
                break;
        }
        
        /* Apply time-frequency transform (TFT) */
        afSTFT_forward(pData->hSTFT, pData->SHFrameTD, FRAME_SIZE, pData->SHframeTF);
    
        /* Main processing: */
        /* Apply rotation */
        if (enableRot) {
            if(pData->recalc_M_rotFLAG){
                M_rot_tmp = malloc1d(NUM_SH_SIGNALS*NUM_SH_SIGNALS * sizeof(float));
                yawPitchRoll2Rzyx(pData->yaw, pData->pitch, pData->roll, pData->useRollPitchYawFlag, Rxyz);
                getSHrotMtxReal(Rxyz, M_rot_tmp, SH_ORDER);
                for (i = 0; i < NUM_SH_SIGNALS; i++)
                    for (j = 0; j < NUM_SH_SIGNALS; j++)
                        pData->M_rot[i][j] = cmplxf(M_rot_tmp[i*NUM_SH_SIGNALS + j], 0.0f);
                free(M_rot_tmp);
                pData->recalc_M_rotFLAG = 0;
            }
            for (band = 0; band < HYBRID_BANDS; band++) {
                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_SH_SIGNALS, TIME_SLOTS, NUM_SH_SIGNALS, &calpha,
                            pData->M_rot, NUM_SH_SIGNALS,
                            FLATTEN2D(pData->SHframeTF[band]), TIME_SLOTS, &cbeta,
                            FLATTEN2D(pData->SHframeTF_rot), TIME_SLOTS);
                memcpy(FLATTEN2D(pData->SHframeTF[band]), FLATTEN2D(pData->SHframeTF_rot), NUM_SH_SIGNALS*TIME_SLOTS*sizeof(float_complex));
            }
        }

        /* mix to headphones via linear decoding */
        for (band = 0; band < HYBRID_BANDS; band++) {
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_EARS, TIME_SLOTS, NUM_SH_SIGNALS, &calpha,
                        pars->M_dec[band], NUM_SH_SIGNALS,
                        FLATTEN2D(pData->SHframeTF[band]), TIME_SLOTS, &cbeta,
                        FLATTEN2D(pData->ambiframeTF[band]), TIME_SLOTS);
#ifdef ENABLE_RESIDUAL_STREAM
            for(i=0; i<NUM_EARS; i++){
                for(t=0; t<TIME_SLOTS; t++)
                    pData->decorrelatedframeTF[band][i][t] = pData->circBufferFrames[band][i][TIME_SLOTS*(NUM_DECOR_FRAMES)+t-pData->decorrelationDelays[band][i]];
            }
            for(i=0; i<NUM_EARS; i++){
                for(t=0; t<TIME_SLOTS*NUM_DECOR_FRAMES; t++)
                    pData->circBufferFrames[band][i][t] = pData->circBufferFrames[band][i][t+TIME_SLOTS];
                memcpy(&(pData->circBufferFrames[band][i][NUM_DECOR_FRAMES*TIME_SLOTS]), pData->ambiframeTF[band][i], TIME_SLOTS*sizeof(float_complex));
            }
#endif
        }
            
        /* update covarience matrix per band */
        for(band=0; band<HYBRID_BANDS; band++){
            /* For input SH */
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, NUM_SH_SIGNALS, NUM_SH_SIGNALS, TIME_SLOTS, &calpha,
                        FLATTEN2D(pData->SHframeTF[band]), TIME_SLOTS,
                        FLATTEN2D(pData->SHframeTF[band]), TIME_SLOTS, &cbeta,
                        Cx_new, NUM_SH_SIGNALS);
            for(i=0; i<NUM_SH_SIGNALS; i++)
                for(j=0; j<NUM_SH_SIGNALS; j++)
                    pData->Cx[band][i][j] = ccaddf(crmulf(pData->Cx[band][i][j], covAvgCoeff), crmulf(Cx_new[i][j], 1.0f-covAvgCoeff));
                
            /* For prototype */
            cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasConjTrans, NUM_EARS, NUM_EARS, TIME_SLOTS, &calpha,
                        FLATTEN2D(pData->ambiframeTF[band]), TIME_SLOTS,
                        FLATTEN2D(pData->ambiframeTF[band]), TIME_SLOTS, &cbeta,
                        Cambi_new, NUM_EARS);
            for(i=0; i<NUM_EARS; i++)
                for(j=0; j<NUM_EARS; j++)
                    pData->Cambi[band][i][j] = ccaddf(crmulf(pData->Cambi[band][i][j], covAvgCoeff), crmulf(Cambi_new[i][j], 1.0f-covAvgCoeff));
        }
        
        /* CroPaC analysis/synthesis per band */
        for(band=0; band<HYBRID_BANDS; band++){
            if(pData->freqVector[band] < anaLim){
                /* optain powermap */
                cblas_cgemm(CblasRowMajor, CblasTrans, CblasNoTrans, TIME_SLOTS, pars->grid_nDirs, NUM_SH_SIGNALS, &calpha,
                            FLATTEN2D(pData->SHframeTF[band]), TIME_SLOTS,
                            pars->Y_grid_cmplx, pars->grid_nDirs, &cbeta,
                            pars->pwdmap_cmplx, pars->grid_nDirs);
                
                /* determine which directions have the most energy per time instance */
                for(i=0; i<TIME_SLOTS; i++){
                    utility_cimaxv(&pars->pwdmap_cmplx[i*(pars->grid_nDirs)], pars->grid_nDirs, &dir_max_idx[i]);
                    azi[i] = pars->grid_dirs_deg[dir_max_idx[i]*2];
                    elev[i] = pars->grid_dirs_deg[dir_max_idx[i]*2+1];
                }
 
                /* calculate CroPaC Gains, G */
                for(i=0; i<TIME_SLOTS; i++){
                    for(j=0; j<NUM_SH_SIGNALS; j++)
                        inputFrame_s[j] = pData->SHframeTF[band][j][i];
                    inputEnergy = powf(cabsf(pData->SHframeTF[band][0][i]), 2.0f) +
                                    powf(cabsf(crdivf(pData->SHframeTF[band][1][i],sqrtf(3.0f))), 2.0f) +
                                    powf(cabsf(crdivf(pData->SHframeTF[band][2][i],sqrtf(3.0f))), 2.0f) +
                                    powf(cabsf(crdivf(pData->SHframeTF[band][3][i],sqrtf(3.0f))), 2.0f) + 2.23e-8f;
                    cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_SH_SIGNALS, 1, NUM_SH_SIGNALS, &calpha,
                                &(pars->M_rot[dir_max_idx[i]*NUM_SH_SIGNALS*NUM_SH_SIGNALS]), NUM_SH_SIGNALS,
                                inputFrame_s, 1, &cbeta,
                                inputFrame_rot, 1);
                    G = SAF_MAX(0.0f, 2.0f*crealf( ccmulf(conjf(inputFrame_rot[0]), crmulf(inputFrame_rot[3], 1.0f/sqrtf(3.0f))) ) /inputEnergy);
                    for(j=0; j<NUM_SH_SIGNALS; j++){
                        y[i][j] = pars->Y_grid_cmplx[j*(pars->grid_nDirs)+dir_max_idx[i]];
                        w[j] = crdivf(y[i][j], (float)NUM_SH_SIGNALS);
                    }
                    utility_cvvdot(w, inputFrame_s, NUM_SH_SIGNALS, NO_CONJ, &B);
                    GB[i] = crmulf(B,G);
                }

                /* interpolate HRTFs */
                hcropaclib_interpHRTFs(hCroPaC, band, azi, elev, hrtf_interp);

                /* Construct target covariance matrix, Cy */
                for(i=0; i<TIME_SLOTS; i++){
                    for(j=0; j<NUM_EARS; j++)
                        y_dir[i][j] = ccmulf(hrtf_interp[i][j], GB[i]);
                    for(j=0; j<NUM_SH_SIGNALS; j++){
                        a_diff[j] = ccmulf(y[i][j], GB[i]);
                        a_diff[j] = ccsubf(pData->SHframeTF[band][j][i], a_diff[j]);
                    }
                    cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_EARS, 1, NUM_SH_SIGNALS, &calpha,
                                pars->M_dec[band], NUM_SH_SIGNALS,
                                a_diff, 1, &cbeta,
                                y_diff[i], 1);
                }
                cblas_cgemm(CblasRowMajor, CblasConjTrans, CblasNoTrans, NUM_EARS, NUM_EARS, TIME_SLOTS, &calpha,
                            y_dir, NUM_EARS,
                            y_dir, NUM_EARS, &cbeta,
                            Cdir, NUM_EARS);
                cblas_cgemm(CblasRowMajor, CblasConjTrans, CblasNoTrans, NUM_EARS, NUM_EARS, TIME_SLOTS, &calpha,
                            y_diff, NUM_EARS,
                            y_diff, NUM_EARS, &cbeta,
                            Cdiff, NUM_EARS);
                
                /* adjust balance */
                for(i=0; i<NUM_EARS; i++){
                    for(j=0; j<NUM_EARS; j++){
                        if (balance[band] > 1)
                            Cdiff[i][j] = crmulf(Cdiff[i][j], 2.0f - balance[band]);
                        else
                            Cdir[i][j] = crmulf(Cdir[i][j], balance[band]);
                    }
                }
       
                /* Account for binaural diffuse coherence */
#ifdef ENABLE_BINAURAL_DIFF_COH
                U[0][0] = ccdivf(pData->Cambi[band][0][0], ccaddf(pData->Cambi[band][0][0], pData->Cambi[band][1][1]));
                U[0][1] = cmplxf(pars->binDiffuseCoh[band], 0.0f);
                U[1][0] = cmplxf(pars->binDiffuseCoh[band], 0.0f);
                U[1][1] = ccdivf(pData->Cambi[band][1][1], ccaddf(pData->Cambi[band][0][0], pData->Cambi[band][1][1]));
                cblas_cgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, NUM_EARS, NUM_EARS, NUM_EARS, &calpha,
                            U, NUM_EARS,
                            Cdiff, NUM_EARS, &cbeta,
                            U_Cdiff, NUM_EARS);
                memcpy(Cdiff, U_Cdiff, NUM_EARS*NUM_EARS*sizeof(float_complex));
#endif
                /* Average Cy over time */
                for(i=0; i<NUM_EARS; i++)
                    for(j=0; j<NUM_EARS; j++)
                        pData->Cy[band][i][j] = ccaddf(crmulf(pData->Cy[band][i][j], covAvgCoeff), crmulf(ccaddf(conjf(Cdir[i][j]), conjf(Cdiff[i][j])), 1.0f-covAvgCoeff)); //

                /* formulate optimal mixing matrix */
#ifdef ENABLE_RESIDUAL_STREAM
                float diag_Cambi[NUM_EARS][NUM_EARS] = {{0.0f}};
                diag_Cambi[0][0] = crealf(pData->Cambi[band][0][0]);
                diag_Cambi[1][1] = crealf(pData->Cambi[band][1][1]);
                formulate_M_and_Cr_cmplx(pData->hCdf, (float_complex*)pData->Cambi[band], (float_complex*)pData->Cy[band], (float_complex*)eye2,
                                            0, 0.2f, (float_complex*)pData->new_M[band], (float_complex*)Cr);
                /* Convert residual to real */
                for(i=0; i<NUM_EARS; i++)
                    for(j=0; j<NUM_EARS; j++)
                        Cr_real[i][j] = crealf(Cr[i][j]);

                /* Compute residual mixing matrix */
                formulate_M_and_Cr(pData->hCdf_res, (float*)diag_Cambi, (float*)Cr_real, (float*)real_eye2,
                                            0, 0.2f, (float*)pData->new_Mr[band], NULL);
#else
                formulate_M_and_Cr_cmplx(pData->hCdf, (float_complex*)pData->Cambi[band], (float_complex*)pData->Cy[band], (float_complex*)eye2,
                                            1, 0.2f, (float_complex*)pData->new_M[band], NULL);
#endif
            }
            else{
                Ex = Eambi = 0.0f;
                for(i=0; i<NUM_SH_SIGNALS; i++)
                    Ex += crealf(pData->Cx[band][i][i]);
                for(i=0; i<NUM_EARS; i++)
                    Eambi += crealf(pData->Cambi[band][i][i]);
                Ex /= (float)NUM_SH_SIGNALS;
                Eambi /= (float)NUM_EARS;
                Eambi += 2.23e-7f;
                memset(pData->new_M[band], 0, NUM_EARS*NUM_EARS*sizeof(float_complex));
                for(i=0; i<NUM_EARS; i++)
                    pData->new_M[band][i][i] = cmplxf(sqrtf(Ex/Eambi), 0.0f);
#ifdef ENABLE_RESIDUAL_STREAM
                memset(pData->new_Mr[band], 0, NUM_EARS*NUM_EARS*sizeof(float));
#endif
            }
        }
            
        /* extract onsets from decorrelation buffer */
#ifdef ENABLE_RESIDUAL_STREAM
        float alpha, beta, detectorEne, transientEQ;
        alpha = 0.95f;
        beta = 0.995f;
        for(band=0; band<HYBRID_BANDS; band++){
            for(i=0; i<NUM_EARS; i++){
                for(t=TIME_SLOTS*(NUM_DECOR_FRAMES-1); t<TIME_SLOTS*NUM_DECOR_FRAMES; t++){
                    detectorEne = powf(cabsf(pData->circBufferFrames[band][i][t]), 2.0f);
                    pData->transientDetector1[band][i] *= alpha;
                    if(pData->transientDetector1[band][i]<detectorEne)
                        pData->transientDetector1[band][i] = detectorEne;
                    pData->transientDetector2[band][i] = pData->transientDetector2[band][i]*beta + (1.0f-beta)*(pData->transientDetector1[band][i]);
                    if(pData->transientDetector2[band][i]>pData->transientDetector1[band][i])
                        pData->transientDetector2[band][i] = pData->transientDetector1[band][i];
                    transientEQ = SAF_MIN(1.0f, 4.0f*(pData->transientDetector2[band][i])/(pData->transientDetector1[band][i]+2.e-9f));
                    pData->circBufferFrames[band][i][t] = crmulf(pData->circBufferFrames[band][i][t], transientEQ);
                }
            }
        }
#endif

        /* Apply mixing matrices */
        for(band=0; band<HYBRID_BANDS; band++){
            for(t=0; t<TIME_SLOTS; t++){
                for(j=0; j<NUM_EARS; j++)
                    inFrame_t[j] = pData->ambiframeTF[band][j][t];
                for (i = 0; i < NUM_EARS; i++) {
                    for (j = 0; j < NUM_EARS; j++) {
#ifndef _MSC_VER
                        interp_M[i][j] = pData->interpolator[t]*pData->new_M[band][i][j] + (1.0f-pData->interpolator[t])*pData->current_M[band][i][j];
#else
                        interp_M[i][j] = ccaddf(crmulf(pData->new_M[band][i][j], pData->interpolator[t]), crmulf(pData->current_M[band][i][j], 1.0f - pData->interpolator[t]));
#endif
                    }
                }
                for(i=0; i<NUM_EARS; i++)
                    utility_cvvdot(interp_M[i], inFrame_t, NUM_EARS, NO_CONJ, &outFrame_t[i]);
                for(i=0; i<NUM_EARS; i++)
                    pData->binframeTF[band][i][t] = outFrame_t[i];
            }
                
#ifdef ENABLE_RESIDUAL_STREAM
            for(t=0; t<TIME_SLOTS; t++){
                for(j=0; j<NUM_EARS; j++)
                    inFrame_t[j] = pData->decorrelatedframeTF[band][j][t];
                    
                for (i = 0; i < NUM_EARS; i++) {
                    for (j = 0; j < NUM_EARS; j++) {
#ifndef _MSC_VER
                        interp_M[i][j] = pData->interpolator[t]*pData->new_Mr[band][i][j] + (1.0f-pData->interpolator[t])*pData->current_Mr[band][i][j];
#else
                        interp_M[i][j] = cmplxf(pData->new_Mr[band][i][j] * pData->interpolator[t] + pData->current_Mr[band][i][j] * (1.0f - pData->interpolator[t]), 0.0f);
#endif
                    }
                }
                for(i=0; i<NUM_EARS; i++)
                    utility_cvvdot(interp_M[i], inFrame_t, NUM_EARS, NO_CONJ, &outFrame_t[i]);
                for(i=0; i<NUM_EARS; i++)
                    pData->binframeTF[band][i][t] = ccaddf(pData->binframeTF[band][i][t], outFrame_t[i]);
            } 
#endif
        }
            
        /* for next frame */
        memcpy(pData->current_M, pData->new_M, HYBRID_BANDS*NUM_EARS*NUM_EARS*sizeof(float_complex));
#ifdef ENABLE_RESIDUAL_STREAM
        memcpy(pData->current_Mr, pData->new_Mr, HYBRID_BANDS*NUM_EARS*NUM_EARS*sizeof(float));
#endif
  
        /* inverse-TFT */
        if(enableCroPaC)
            afSTFT_backward(pData->hSTFT, pData->binframeTF, FRAME_SIZE, pData->binFrameTD);
        else
            afSTFT_backward(pData->hSTFT, pData->ambiframeTF, FRAME_SIZE, pData->binFrameTD);

        /* Copy to output */
        for (ch = 0; ch < SAF_MIN(NUM_EARS, nOutputs); ch++)
            utility_svvcopy(pData->binFrameTD[ch], FRAME_SIZE, outputs[ch]);
        for (; ch < nOutputs; ch++)
            memset(outputs[ch], 0, FRAME_SIZE*sizeof(float));
    }
    else
        for (ch=0; ch < nOutputs; ch++)
            memset(outputs[ch],0, FRAME_SIZE*sizeof(float));
    
    pData->procStatus = PROC_STATUS_NOT_ONGOING;
}


/* Set Functions */

void hcropaclib_refreshParams(void* const hCroPaC)
{
    hcropaclib_setCodecStatus(hCroPaC, CODEC_STATUS_NOT_INITIALISED);
}

void hcropaclib_setEnableCroPaC(void* const hCroPaC, int newState)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->enableCroPaC = newState;
}

void hcropaclib_setBalance(void* const hCroPaC, float newValue, int bandIdx)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->balance[bandIdx] = newValue;
}

void hcropaclib_setBalanceAllBands(void* const hCroPaC, float newValue)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    int band;
    
    for(band=0; band<HYBRID_BANDS; band++)
        pData->balance[band] = newValue;
}

void hcropaclib_setCovAvg(void* const hCroPaC, float newValue)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->covAvgCoeff = newValue;
}

void hcropaclib_setAnaLimit(void* const hCroPaC, float newValue)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->anaLimit_hz = SAF_CLAMP(newValue, HCROPAC_ANA_LIMIT_MIN_VALUE, HCROPAC_ANA_LIMIT_MAX_VALUE);
}

void hcropaclib_setUseDefaultHRIRsflag(void* const hCroPaC, int newState)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    
    if((!pData->useDefaultHRIRsFLAG) && (newState)){
        pData->useDefaultHRIRsFLAG = newState;
        hcropaclib_setCodecStatus(hCroPaC, CODEC_STATUS_NOT_INITIALISED);
    }
}

void hcropaclib_setSofaFilePath(void* const hCroPaC, const char* path)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    codecPars* pars = pData->pars;
    
    pars->sofa_filepath = malloc1d(strlen(path) + 1);
    strcpy(pars->sofa_filepath, path);
    pData->useDefaultHRIRsFLAG = 0;
    hcropaclib_setCodecStatus(hCroPaC, CODEC_STATUS_NOT_INITIALISED);
}

void hcropaclib_setChOrder(void* const hCroPaC, int newOrder)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->chOrdering = (HCROPAC_CH_ORDER)newOrder;
}

void hcropaclib_setNormType(void* const hCroPaC, int newType)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->norm = (HCROPAC_NORM_TYPES)newType;
}

void hcropaclib_setEnableDiffCorrection(void* const hCroPaC, int newState)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    if(pData->diffCorrection != newState){
        pData->diffCorrection = newState;
        hcropaclib_setCodecStatus(hCroPaC, CODEC_STATUS_NOT_INITIALISED);
    }
}

void hcropaclib_setHRIRsPreProc(void* const hCroPaC, HRIR_PREPROC_OPTIONS newState)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    if(pData->hrirProcMode != newState){
        pData->hrirProcMode = newState;
        hcropaclib_setCodecStatus(hCroPaC, CODEC_STATUS_NOT_INITIALISED);
    }
}

void hcropaclib_setEnableRotation(void* const hCroPaC, int newState)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->enableRotation = newState;
    pData->recalc_M_rotFLAG = 1;
}

void hcropaclib_setYaw(void  * const hCroPaC, float newYaw)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->yaw = pData->bFlipYaw == 1 ? -DEG2RAD(newYaw) : DEG2RAD(newYaw);
    pData->recalc_M_rotFLAG = 1;
}

void hcropaclib_setPitch(void* const hCroPaC, float newPitch)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->pitch = pData->bFlipPitch == 1 ? -DEG2RAD(newPitch) : DEG2RAD(newPitch);
    pData->recalc_M_rotFLAG = 1;
}

void hcropaclib_setRoll(void* const hCroPaC, float newRoll)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->roll = pData->bFlipRoll == 1 ? -DEG2RAD(newRoll) : DEG2RAD(newRoll);
    pData->recalc_M_rotFLAG = 1;
}

void hcropaclib_setFlipYaw(void* const hCroPaC, int newState)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    if(newState !=pData->bFlipYaw ){
        pData->bFlipYaw = newState;
        hcropaclib_setYaw(hCroPaC, -hcropaclib_getYaw(hCroPaC));
    }
}

void hcropaclib_setFlipPitch(void* const hCroPaC, int newState)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    if(newState !=pData->bFlipPitch ){
        pData->bFlipPitch = newState;
        hcropaclib_setPitch(hCroPaC, -hcropaclib_getPitch(hCroPaC));
    }
}

void hcropaclib_setFlipRoll(void* const hCroPaC, int newState)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    if(newState !=pData->bFlipRoll ){
        pData->bFlipRoll = newState;
        hcropaclib_setRoll(hCroPaC, -hcropaclib_getRoll(hCroPaC));
    }
}

void hcropaclib_setRPYflag(void* const hCroPaC, int newState)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    pData->useRollPitchYawFlag = newState;
}


/* Get Functions */

int hcropaclib_getFrameSize(void)
{
    return FRAME_SIZE;
}

HCROPAC_CODEC_STATUS hcropaclib_getCodecStatus(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->codecStatus;
}

float hcropaclib_getProgressBar0_1(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->progressBar0_1;
}

void hcropaclib_getProgressBarText(void* const hCroPaC, char* text)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    memcpy(text, pData->progressBarText, HCROPAC_PROGRESSBARTEXT_CHAR_LENGTH*sizeof(char));
}

int hcropaclib_getEnableCroPaC(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->enableCroPaC;
}

float hcropaclib_getBalance(void  * const hCroPaC, int bandIdx)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->balance[bandIdx];
}

float hcropaclib_getBalanceAllBands(void  * const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->balance[0];
}

void hcropaclib_getBalanceHandle
(
    void* const hCroPaC,
    float** pX_vector,
    float** pY_values,
    int* pNpoints
)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    (*pX_vector) = &pData->freqVector[0];
    (*pY_values) = &pData->balance[0];
    (*pNpoints) = HYBRID_BANDS;
} 

float hcropaclib_getCovAvg(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->covAvgCoeff;
}

float hcropaclib_getAnaLimit(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->anaLimit_hz;
}

int hcropaclib_getUseDefaultHRIRsflag(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->useDefaultHRIRsFLAG;
}

char* hcropaclib_getSofaFilePath(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    codecPars* pars = pData->pars;
    if(pars->sofa_filepath!=NULL)
        return pars->sofa_filepath;
    else
        return "no_file";
}

int hcropaclib_getChOrder(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return (int)pData->chOrdering;
}

int hcropaclib_getNormType(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return (int)pData->norm;
}

int hcropaclib_getEnableDiffCorrection(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->diffCorrection;
}

HRIR_PREPROC_OPTIONS hcropaclib_getHRIRsPreProc(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->hrirProcMode;
}

int hcropaclib_getNumEars()
{ 
    return NUM_EARS;
}

int hcropaclib_getNumberOfBands()
{
    return HYBRID_BANDS;
}

int hcropaclib_getNSHrequired()
{ 
    return NUM_SH_SIGNALS;
}

int hcropaclib_getEnableRotation(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->enableRotation;
}

float hcropaclib_getYaw(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->bFlipYaw == 1 ? -RAD2DEG(pData->yaw) : RAD2DEG(pData->yaw);
}

float hcropaclib_getPitch(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->bFlipPitch == 1 ? -RAD2DEG(pData->pitch) : RAD2DEG(pData->pitch);
}

float hcropaclib_getRoll(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->bFlipRoll == 1 ? -RAD2DEG(pData->roll) : RAD2DEG(pData->roll);
}

int hcropaclib_getFlipYaw(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->bFlipYaw;
}

int hcropaclib_getFlipPitch(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->bFlipPitch;
}

int hcropaclib_getFlipRoll(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->bFlipRoll;
}

int hcropaclib_getRPYflag(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->useRollPitchYawFlag;
}

int hcropaclib_getNDirs(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    codecPars* pars = pData->pars;
    return pars->N_hrir_dirs;
}

int hcropaclib_getNTriangles(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    codecPars* pars = pData->pars;
    return pars->N_Tri;
}

int hcropaclib_getHRIRlength(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    codecPars* pars = pData->pars;
    return pars->hrir_len;
}

int hcropaclib_getHRIRsamplerate(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    codecPars* pars = pData->pars;
    return pars->hrir_fs;
}

int hcropaclib_getDAWsamplerate(void* const hCroPaC)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    return pData->fs;
}

int hcropaclib_getProcessingDelay()
{
    return 12*HOP_SIZE;
}
