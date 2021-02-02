#include "MicArray_Internal.h"
#include <stdlib.h>

int32_t MicArray_Init(void** pHandle, uint32_t sample_rate, uint32_t nchannel, uint32_t fftlen,
                           uint32_t nframe) {
    uint32_t idx;
    uint32_t nshift = fftlen - nframe;
    uint32_t half_fftlen = uiv_half_fftlen(fftlen);
    MicArray* handle;

    handle = calloc(1, sizeof(MicArray));

    handle->sample_rate = sample_rate;
    handle->fftlen = fftlen;
    handle->nframe = nframe;
    handle->nchannel = nchannel;
    handle->nshift = nshift;
    handle->ref_ch = 0;
    handle->frame_cnt = 0;
    handle->fftwin = (float*)calloc(fftlen, sizeof(float));
    for (int i = 0; i < fftlen; i++) {
        handle->fftwin[i] = sqrtf(0.5f - 0.5f * cosf(2.f * M_PI * i / fftlen));
    }
    handle->fft_lookup = uiv_fft_init(fftlen);

    handle->alpha_dc = 0.98f;
    for (idx = 0; idx < MAX_NCHANNEL; ++idx) {
        handle->state_y[idx] = 0;
        handle->state_x[idx] = 0;
    }
    handle->inputs_last = (float*)malloc(nshift * nchannel * sizeof(float));
    handle->overlap = (float*)malloc(nshift * sizeof(float));

    uiv_fill_f32(0, handle->inputs_last, nshift * nchannel);
    uiv_fill_f32(0, handle->overlap, nshift);

    /* Can be optimized as scratch memory */
    handle->dc_remove = (float*)malloc(nframe * nchannel * sizeof(float));
    handle->inputs_t = (float*)malloc(fftlen * sizeof(float));
    handle->inputs_f = (float*)malloc(half_fftlen * 2 * nchannel * sizeof(float));
    handle->ref_power = (float*)malloc(half_fftlen * sizeof(float));
    handle->X_itr = (float*)malloc(half_fftlen * 2 * nchannel * sizeof(float));
    handle->beamformed = (float*)malloc(half_fftlen * 2 * sizeof(float));
    handle->beamformed_power = (float*)malloc(half_fftlen * sizeof(float));
    handle->output_ifft = (float*)malloc(fftlen * sizeof(float));
#ifdef _AGC_ENABLE
    handle->agc_out = (float*)malloc(nframe * sizeof(float));
#endif

    CepstrumVAD_Init(&handle->stCepstrumVAD, fftlen, sample_rate);
    SoundLocater_Init(&handle->stSoundLocater, sample_rate, fftlen, nchannel);
    NoiseReduce_Init(&handle->stSnrEst, sample_rate, fftlen, 0);
    Beamformer_Init(&handle->stBeamformer, fftlen, nchannel);
    Biquad_Init(&handle->stBiquad);

#ifdef _NS_ENABLE
    NoiseReduce_Init(&handle->stPostFilt, sample_rate, fftlen, 1);
#endif
#ifdef _AGC_ENABLE
    AutoGainCtrl_Init(&handle->stAGC, 32767.0f / 2.0f, nframe, 1.0f / 8.0f, 2.0f);
#endif
    *pHandle = handle;

    return 0;
}

int32_t MicArray_Process(void* hSpeechEnance, int16_t* mic_inputs, int16_t* output) {
    uint32_t idx_l, idx_c;
    uint32_t vad;
    uint32_t speech_status, noise_status;
    uint32_t update_speech, update_noise;
#ifdef _AGC_ENABLE
    float spp_mean;
#endif

    MicArray* const handle = hSpeechEnance;
    const uint32_t nchannel = handle->nchannel;
    const uint32_t nframe = handle->nframe;
    const uint32_t nshift = handle->nshift;
    const uint32_t fftlen = handle->fftlen;
    const uint32_t half_fftlen = uiv_half_fftlen(fftlen);
    const uint32_t ref_ch = handle->ref_ch;

    float* const dc_remove = handle->dc_remove;
    float* const inputs_t = handle->inputs_t;
    float* const inputs_f = handle->inputs_f;
    float* const ref_power = handle->ref_power;
    float* const X_itr = handle->X_itr;
    float* const beamformed = handle->beamformed;
    float* const output_ifft = handle->output_ifft;

    float* ref_input;
    float* ref_output;

    ++handle->frame_cnt;

#ifdef _DC_REMOVAL
    /* DC removal, IIR notch filter, interleaved input, serial output */
    for (idx_l = 0; idx_l < nframe; ++idx_l) {
        for (idx_c = 0; idx_c < nchannel; ++idx_c) {
            dc_remove[idx_c * nframe + idx_l] = handle->alpha_dc * handle->state_y[idx_c] +
                                                (float)mic_inputs[idx_l * nchannel + idx_c] -
                                                handle->state_x[idx_c];
            handle->state_x[idx_c] = (float)mic_inputs[idx_l * nchannel + idx_c];
            handle->state_y[idx_c] = dc_remove[idx_c * nframe + idx_l];
        }
    }
#else
    for (idx_l = 0; idx_l < nframe; ++idx_l) {
        for (idx_c = 0; idx_c < nchannel; ++idx_c) {
            dc_remove[idx_c * nframe + idx_l] = (float)mic_inputs[idx_l * nchannel + idx_c];
        }
    }
#endif

    for (idx_c = 0; idx_c < nchannel; ++idx_c) {
        uiv_copy_f32(&handle->inputs_last[idx_c * nshift], &inputs_t[0], nshift);
        uiv_copy_f32(&dc_remove[idx_c * nframe], &inputs_t[nshift], nframe);
        uiv_copy_f32(&inputs_t[nframe], &handle->inputs_last[idx_c * nshift], nshift);

        // windowing & fft
        uiv_mult_f32(&inputs_t[0], handle->fftwin, &inputs_t[0], fftlen);
        uiv_fft(handle->fft_lookup, &inputs_t[0], &inputs_f[idx_c * half_fftlen * 2U]);
        uiv_fft_shift(handle->fft_lookup, &inputs_f[idx_c * half_fftlen * 2U]);
    }
#ifdef _FFT_ONLY
    goto DEBUG_LOOP; // for debug
#endif

    /* Keep interleaved data for beamforming */
    for (idx_c = 0; idx_c < nchannel; ++idx_c) {
        for (idx_l = 0; idx_l < half_fftlen; ++idx_l) {
            X_itr[2 * idx_l * nchannel + 2 * idx_c] = inputs_f[2 * idx_c * half_fftlen + 2 * idx_l];
            X_itr[2 * idx_l * nchannel + 2 * idx_c + 1] =
                inputs_f[2 * idx_c * half_fftlen + 2 * idx_l + 1];
        }
    }

    /* Calculate Ref-ch Power*/
    ref_input = inputs_f + ref_ch * fftlen;
    uiv_cmplx_mag_squared_f32(ref_input, ref_power, half_fftlen);

    /* Cepstrum VAD */
    vad = CepstrumVAD_Process(&handle->stCepstrumVAD, ref_power);

    /* Noise Estimation */
    NoiseReduce_EstimateNoise(&handle->stSnrEst, ref_power, handle->frame_cnt, vad);
    NoiseReduce_SnrVAD(&handle->stSnrEst);

    /* SoundLocater Doa */
    uint32_t angle_deg;
    float energy;
    int inbeam, outbeam;

    SoundLocater_FindDoa(&handle->stSoundLocater, (complex float*)inputs_f, &angle_deg, &energy);
    SoundLocater_Cluster(&handle->stSoundLocater, angle_deg, energy, handle->stSnrEst.speech_frame,
                         &inbeam, &outbeam);

    noise_status = ((handle->stSnrEst.noise_frame == 1) && (vad == 0));
    speech_status = ((handle->stSnrEst.speech_frame == 1) && (vad == 1) && inbeam);

    /* Beamforming */
    update_noise = Beamformer_UpdateNoiseMatrix(&handle->stBeamformer, (complex float*)X_itr,
                                                noise_status, handle->stSnrEst.spp);
    update_speech =
        Beamformer_UpdateSpeechMatrix(&handle->stBeamformer, (complex float*)X_itr, speech_status);

    Beamformer_UpdateSteeringVector(&handle->stBeamformer, update_speech, update_noise);
    Beamformer_UpdateMvdrFilter(&handle->stBeamformer, update_speech, update_noise);
    Beamformer_DoFilter(&handle->stBeamformer, (complex float*)X_itr, (complex float*)beamformed);

    uiv_cmplx_mag_squared_f32(beamformed, handle->beamformed_power, half_fftlen);
    ref_input = beamformed;

#ifdef _NS_ENABLE
    NoiseReduce_EstimateNoise(&handle->stPostFilt, handle->beamformed_power, handle->frame_cnt,
                              speech_status);
    NoiseReduce_SnrVAD(&handle->stPostFilt);
    NoiseReduce_WienerFilter(&handle->stPostFilt, ref_input, ref_input);
#endif
    /* ifft */
    uiv_ifft_shift(handle->fft_lookup, ref_input);
    uiv_ifft(handle->fft_lookup, ref_input, output_ifft);

#ifdef _FFT_ONLY
DEBUG_LOOP:
    uiv_ifft_shift(handle->fft_lookup, &inputs_f[ref_ch * half_fftlen * 2U]);
    uiv_ifft(handle->fft_lookup, &inputs_f[ref_ch * half_fftlen * 2U], output_ifft);
#endif

    /* overlap and add */
    uiv_mult_f32(output_ifft, handle->fftwin, output_ifft, fftlen);
    uiv_add_f32(output_ifft, handle->overlap, output_ifft, nshift);
    uiv_copy_f32(&output_ifft[nframe], handle->overlap, nshift);
    ref_output = output_ifft;

    /* AGC */
#if defined(_AGC_ENABLE) && defined(_NS_ENABLE)
    uiv_mean_f32(handle->stPostFilt.spp, half_fftlen, &spp_mean);
#elif defined(_AGC_ENABLE) && !defined(_NS_ENABLE)
    uiv_mean_f32(handle->stSnrEst.spp, half_fftlen, &spp_mean);
#endif

#ifdef _AGC_ENABLE
    AutoGainCtrl_Process(&handle->stAGC, output_ifft, speech_status, spp_mean, agc_out);
    ref_output = agc_out;
#endif // _AGC_ENABLE

    // HPF biquad filter
    Biquad_Process(&handle->stBiquad, ref_output, ref_output, nframe);
    for (idx_l = 0; idx_l < nframe; ++idx_l) output[idx_l] = (int16_t)roundf(ref_output[idx_l]);

    return 0;
}

int32_t MicArray_Release(void* hMicArray) {
    MicArray* handle = (MicArray*)hMicArray;

    uiv_fft_destroy(handle->fft_lookup);
    free(handle->fftwin);
    free(handle->inputs_last);
    free(handle->overlap);

    free(handle->dc_remove);
    free(handle->inputs_t);
    free(handle->inputs_f);
    free(handle->ref_power);
    free(handle->X_itr);
    free(handle->beamformed);
    free(handle->beamformed_power);
    free(handle->output_ifft);
#ifdef _AGC_ENABLE
    free(handle->agc_out);
#endif

    CepstrumVAD_Release(&handle->stCepstrumVAD);
    NoiseReduce_Release(&handle->stSnrEst);
    Beamformer_Release(&handle->stBeamformer);
    SoundLocater_Release(&handle->stSoundLocater);
#ifdef _NS_ENABLE
    NoiseReduce_Release(&handle->stPostFilt);
#endif
#ifdef _AGC_ENABLE
    AutoGainCtrl_Release(&handle->stAGC);
#endif

    return 0;
}
