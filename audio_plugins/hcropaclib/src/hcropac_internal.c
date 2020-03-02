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
 * Filename: hcropac_internal.c
 * ----------------------------
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

#include "hcropac_internal.h"

void hcropaclib_setCodecStatus(void* const hCroPaC, CODEC_STATUS newStatus)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    if(newStatus==CODEC_STATUS_NOT_INITIALISED){
        /* Pause until current initialisation is complete */
        while(pData->codecStatus == CODEC_STATUS_INITIALISING)
            SAF_SLEEP(10);
    }
    pData->codecStatus = newStatus;
}
 
void hcropaclib_interpHRTFs
(
    void* const hCroPaC,
    int band,
    float secAzi[TIME_SLOTS],
    float secElev[TIME_SLOTS],
    float_complex h_intrp[TIME_SLOTS][NUM_EARS]
)
{
    hcropaclib_data *pData = (hcropaclib_data*)(hCroPaC);
    codecPars* pars = pData->pars;
    int i,t;
    int aziIndex, elevIndex, gridIndex, N_azi, idx3[TIME_SLOTS][3];
    float_complex ipd;
    float aziRes, elevRes, weights[TIME_SLOTS][3];
    float magnitudes3[3][NUM_EARS], magInterp[TIME_SLOTS][NUM_EARS], itds3[3], itdInterp[TIME_SLOTS];
    
    /* find closest pre-computed Amplitude-norm VBAP direction */
    aziRes = (float)pars->az_res;
    elevRes = (float)pars->el_res;
    N_azi = (int)(360.0f / aziRes + 0.5f) + 1;
    for(t=0; t<TIME_SLOTS; t++){
        aziIndex = (int)(matlab_fmodf(secAzi[t] + 180.0f, 360.0f) / aziRes + 0.5f);
        elevIndex = (int)((secElev[t] + 90.0f) / elevRes + 0.5f);
        gridIndex = elevIndex * N_azi + aziIndex;
        memcpy(idx3[t], &(pars->vbap_gtableIdx[gridIndex*3]), 3*sizeof(int));
        memcpy(weights[t], &(pars->vbap_gtableComp[gridIndex*3]), 3*sizeof(float));
    }
    for(t=0; t<TIME_SLOTS; t++){
        for(i=0; i<3; i++){
            magnitudes3[i][0] = pars->hrtf_fb_mag[band*NUM_EARS*(pars->N_hrir_dirs) + 0*(pars->N_hrir_dirs) + idx3[t][i]];
            magnitudes3[i][1] = pars->hrtf_fb_mag[band*NUM_EARS*(pars->N_hrir_dirs) + 1*(pars->N_hrir_dirs) + idx3[t][i]];
            itds3[i] = pars->itds_s[idx3[t][i]];
        }
        /* interpolate hrtf magnitudes and itd */
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 1, 1, 3, 1,
                    (float*)weights[t], 3,
                    (float*)itds3, 1, 0,
                    &itdInterp[t], 1);
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 1, 2, 3, 1,
                    (float*)weights[t], 3,
                    (float*)magnitudes3, 2, 0,
                    (float*)magInterp[t], 2);
        
        /* introduce interaural phase difference */
        ipd = cmplxf(0.0f, (matlab_fmodf(2.0f*PI* (pData->freqVector[band]) * itdInterp[t] + PI, 2.0f*PI) - PI) / 2.0f);
        h_intrp[t][0] = ccmulf(cmplxf(magInterp[t][0], 0.0f), cexpf(ipd));
        h_intrp[t][1] = ccmulf(cmplxf(magInterp[t][1], 0.0f), conjf(cexpf(ipd)));
    }
}


















