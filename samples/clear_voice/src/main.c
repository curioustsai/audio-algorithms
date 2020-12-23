#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sndfile.h>

#include "SpeechEnhance.h"
#include "SpeechEnhance_Internal.h"

#ifdef AUDIO_ALGO_DEBUG
#include "bmp.h"
#endif

#define LEN_FILENAME 256
#define NCH_OUTPUT 2
#define NCH_CEP 3
#define NCH_NR 5
#define NCH_AGC 3
#define NCH_DOA 6
#define NCH_CLUSTER 4

char* replace_subffix(char* str_in, char* str_out, int len_out, char* target, char* subword) {
    size_t len_subword;
    char* pch;

    strncpy(str_out, str_in, len_out);
    pch = strstr(str_out, target);

    memset(pch, 0, len_out - (pch - str_out));

    len_subword = strlen(subword);
    strncpy(pch, subword, len_subword);

    return str_out;
}

int32_t simulator(char* input_filename) {
    char filename[LEN_FILENAME];

    void* hSpeechEnhance;
    uint32_t sample_rate, nchannel, nframe, fftlen;
#ifdef AUDIO_ALGO_DEBUG
    uint32_t tfbmp_size, total_frame;
	uint32_t half_fftlen;
	uint32_t total_length;
#endif
    uint32_t frame_cnt;
    int16_t *input_q15, *output_q15, *out_buf;

    SNDFILE *infile, *outfile;
    SF_INFO sfinfo_in, sfinfo_out;
    memset(&sfinfo_in, 0, sizeof(SF_INFO));
    memset(&sfinfo_out, 0, sizeof(SF_INFO));

    if (!(infile = sf_open(input_filename, SFM_READ, &sfinfo_in))) {
        printf("Not able to open input file %s.\n", input_filename);
        return -1;
    }

    sample_rate = sfinfo_in.samplerate;
    nchannel = sfinfo_in.channels;
#ifdef AUDIO_ALGO_DEBUG
    total_length = sfinfo_in.frames;
#endif

	if (sample_rate == 16000) {
    	nframe = 256;
		fftlen = 512;
#ifdef AUDIO_ALGO_DEBUG
		half_fftlen = 256;
#endif
	} else if  (sample_rate == 48000) {
		nframe = 1024;
		fftlen = 2048;
#ifdef AUDIO_ALGO_DEBUG
		half_fftlen = 1024;
#endif
	} else {
		printf("not supported sample rate\n");
		return -1;
	}
    /* limit length */
    /* frame_stop = 5000; */
    /* total_length = min(total_length, frame_stop * nframe); */

#ifdef AUDIO_ALGO_DEBUG
    total_frame = total_length / nframe;
    tfbmp_size = bmp_size(total_frame, half_fftlen);
#endif

    memcpy(&sfinfo_out, &sfinfo_in, sizeof(SF_INFO));
    sfinfo_out.channels = NCH_OUTPUT;

    char* output_filename =
        replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_c_output.wav");
    if (!(outfile = sf_open(output_filename, SFM_WRITE, &sfinfo_out))) {
        printf("Not able to open outputfile file %s.\n", output_filename);
        return -1;
    }

    input_q15 = (int16_t*)malloc((sizeof(int16_t) * nframe * nchannel));
    output_q15 = (int16_t*)malloc((sizeof(int16_t) * nframe));
    out_buf = (int16_t*)malloc((sizeof(int16_t) * nframe * NCH_OUTPUT));

#ifdef AUDIO_ALGO_DEBUG
    /**
	 * sndfile
	 */
    SNDFILE *cep_file, *nr_file, *agc_file, *doa_file, *cluster_file;
    SF_INFO sfinfo_cep, sfinfo_nr, sfinfo_agc, sfinfo_doa, sfinfo_cluster;

    memcpy(&sfinfo_cep, &sfinfo_in, sizeof(SF_INFO));
    memcpy(&sfinfo_nr, &sfinfo_in, sizeof(SF_INFO));
    memcpy(&sfinfo_agc, &sfinfo_in, sizeof(SF_INFO));
    memcpy(&sfinfo_doa, &sfinfo_in, sizeof(SF_INFO));
    memcpy(&sfinfo_cluster, &sfinfo_in, sizeof(SF_INFO));

    sfinfo_cep.channels = NCH_CEP;
    sfinfo_nr.channels = NCH_NR;
    sfinfo_agc.channels = NCH_AGC;
    sfinfo_doa.channels = NCH_DOA;
    sfinfo_cluster.channels = NCH_CLUSTER;

    int16_t* cep_buf = (int16_t*)malloc((sizeof(int16_t) * nframe * sfinfo_cep.channels));
    int16_t* nr_buf = (int16_t*)malloc((sizeof(int16_t) * nframe * sfinfo_nr.channels));
    int16_t* agc_buf = (int16_t*)malloc((sizeof(int16_t) * nframe * sfinfo_agc.channels));
    int16_t* doa_buf = (int16_t*)malloc((sizeof(int16_t) * nframe * sfinfo_doa.channels));
    int16_t* cluster_buf = (int16_t*)malloc((sizeof(int16_t) * nframe * sfinfo_cluster.channels));

    char* cep_filename =
        replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_c_cep.wav");
    cep_file = sf_open(cep_filename, SFM_WRITE, &sfinfo_cep);

    char* nr_filename =
        replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_c_nr.wav");
    nr_file = sf_open(nr_filename, SFM_WRITE, &sfinfo_nr);

    char* agc_filename =
        replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_c_agc.wav");
    agc_file = sf_open(agc_filename, SFM_WRITE, &sfinfo_agc);

    char* doa_filename =
        replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_c_doa.wav");
    doa_file = sf_open(doa_filename, SFM_WRITE, &sfinfo_doa);

    char* cluster_filename =
        replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_c_cluster.wav");
    cluster_file = sf_open(cluster_filename, SFM_WRITE, &sfinfo_cluster);

    /**
	 * bmp
	 */
    char* tf_bin_bmp = (char*)malloc(sizeof(unsigned char) * tfbmp_size);
    char* noise_pow_bmp = (char*)malloc(sizeof(unsigned char) * tfbmp_size);
    char* spp_bmp = (char*)malloc(sizeof(unsigned char) * tfbmp_size);
    char* beamformed_bmp = (char*)malloc(sizeof(unsigned char) * tfbmp_size);

    bmp_init(tf_bin_bmp, total_frame, half_fftlen);
    bmp_init(noise_pow_bmp, total_frame, half_fftlen);
    bmp_init(spp_bmp, total_frame, half_fftlen);
    bmp_init(beamformed_bmp, total_frame, half_fftlen);
#endif

    if (NULL == input_q15 || NULL == output_q15) return -1;

    /* Internal Memory */
    SpeechEnhance_Init(&hSpeechEnhance, sample_rate, nchannel, fftlen, nframe);

    int readcount = nframe * nchannel;
    frame_cnt = 0;

	clock_t tick = clock();
    while ((readcount = sf_read_short(infile, input_q15, readcount)) == nframe*nchannel) {
        SpeechEnhance_Process(hSpeechEnhance, input_q15, output_q15);
		for (int idx_l = 0;  idx_l < nframe; ++idx_l) {
            out_buf[NCH_OUTPUT * idx_l + 0] = input_q15[idx_l*nchannel];
            out_buf[NCH_OUTPUT * idx_l + 1] = output_q15[idx_l];
		}
        sf_write_short(outfile, out_buf, nframe*NCH_OUTPUT);

#ifdef AUDIO_ALGO_DEBUG
		/**
		 * sndfile
		 */
        SpeechEnhance* ptr = (SpeechEnhance*)hSpeechEnhance;
        for (int idx_l = 0; idx_l < nframe; ++idx_l) {
            // DOA
            doa_buf[NCH_DOA * idx_l + 0] = input_q15[idx_l*nchannel];
            doa_buf[NCH_DOA * idx_l + 1] = (int16_t)(ptr->stSoundLocater.theta_pair_rad[0] / M_PI * 32768.0);
            doa_buf[NCH_DOA * idx_l + 2] = (int16_t)(ptr->stSoundLocater.theta_pair_rad[1] / M_PI * 32768.0);
            doa_buf[NCH_DOA * idx_l + 3] = (int16_t)(ptr->stSoundLocater.theta_deg);
            doa_buf[NCH_DOA * idx_l + 4] = (int16_t)(ptr->stSoundLocater.angleCluster);

			int curBeamIdx = ptr->stSoundLocater.currentBeamIndex;
			int angleRetain = ptr->stSoundLocater.angleRetain[curBeamIdx];
            doa_buf[NCH_DOA * idx_l + 5] = (int16_t)(angleRetain);

			// Cluster
			cluster_buf[NCH_CLUSTER * idx_l] = input_q15[idx_l*nchannel];
            cluster_buf[NCH_CLUSTER * idx_l + 1] = (int16_t)(ptr->stSoundLocater.angleNum);
            cluster_buf[NCH_CLUSTER * idx_l + 3] = (int16_t)(ptr->stSoundLocater.vadNum);
            cluster_buf[NCH_CLUSTER * idx_l + 2] = (int16_t)(ptr->stSoundLocater.weightMax*100.f);

            // Cepstrum VAD
            cep_buf[NCH_CEP * idx_l] = output_q15[idx_l];
            cep_buf[NCH_CEP * idx_l + 1] = (int16_t)(ptr->stCepstrumVAD.vad * 16384);
            cep_buf[NCH_CEP * idx_l + 2] = (int16_t)(ptr->stCepstrumVAD.pitch * 10);

            // Noise Estimation
            nr_buf[NCH_NR * idx_l] = output_q15[idx_l];
            nr_buf[NCH_NR * idx_l + 1] = (int16_t)(ptr->stSnrEst.speech_frame * 16384);
            nr_buf[NCH_NR * idx_l + 2] = (int16_t)(ptr->stSnrEst.noise_frame * 16384);
			nr_buf[NCH_NR * idx_l + 3] = (int16_t)(ptr->stSnrEst.snr_max * 100);
			nr_buf[NCH_NR * idx_l + 4] = (int16_t)(ptr->stPostFilt.snr_max * 100);

            // AutoGainCtrl
            agc_buf[NCH_AGC * idx_l] = output_q15[idx_l];
            agc_buf[NCH_AGC * idx_l + 1] = (int16_t)(ptr->stAGC.pka_fp);
            agc_buf[NCH_AGC * idx_l + 2] = (int16_t)(ptr->stAGC.last_g_fp * 3276.8f);
        }
        sf_write_short(doa_file, doa_buf, nframe * NCH_DOA);
        sf_write_short(cluster_file, cluster_buf, nframe * NCH_CLUSTER);
        sf_write_short(cep_file, cep_buf, nframe * NCH_CEP);
        sf_write_short(nr_file, nr_buf, nframe * NCH_NR);
        sf_write_short(agc_file, agc_buf, nframe * NCH_AGC);

		/**
		 * bmp
		 */
        float pow;
        float r, g, b;
        unsigned long color;
        for (int idx_l = 0; idx_l < half_fftlen; ++idx_l) {
            // TF bin
            pow = ptr->ref_power[idx_l];
            pow = 10 * log10f(pow + 1e-12f);
            if (pow > 132.0f) pow = 132.0f;
            pow /= 132.0f;
            pow = 1.0f - pow;
            bmp_set(tf_bin_bmp, frame_cnt, half_fftlen - 1 - idx_l, bmp_encode(pow, pow, pow));

            if (ptr->stCepstrumVAD.vad == 1) {
                color = bmp_get(tf_bin_bmp, frame_cnt, half_fftlen - 1 - idx_l);
                bmp_decode(color, &r, &g, &b);
                bmp_set(tf_bin_bmp, frame_cnt, half_fftlen - 1 - idx_l,
                        bmp_encode(r, (g + 1.f) / 2.f, b));
            }

            // Noise Power
            pow = ptr->stSnrEst.Npw[idx_l];
            pow = 10 * log10f(pow + 1e-12f);
            if (pow > 132.0f) pow = 132.0f;
            pow /= 132.0f;
            pow = 1.0f - pow;
            bmp_set(noise_pow_bmp, frame_cnt, half_fftlen - 1 - idx_l, bmp_encode(pow, pow, pow));

            // SPP
            pow = 1.0f - ptr->stSnrEst.spp[idx_l];
            bmp_set(spp_bmp, frame_cnt, half_fftlen - 1 - idx_l, bmp_encode(pow, pow, pow));

            // beamformed
            pow = ptr->beamformed_power[idx_l];
            pow = 10 * log10f(pow + 1e-12f);
            if (pow > 132.0f) pow = 132.0f;
            pow /= 132.0f;
            pow = 1.0f - pow;
            bmp_set(beamformed_bmp, frame_cnt, half_fftlen - 1 - idx_l, bmp_encode(pow, pow, pow));
        }
#endif // AUDIO_ALGO_DEBUG
        frame_cnt++;
    }
	tick = clock() - tick;
	printf("total tick: %ld, times: %fsec\n", tick, ((float)tick / CLOCKS_PER_SEC));

    SpeechEnhance_Release(hSpeechEnhance);
    free(input_q15);
    free(output_q15);
    free(out_buf);
    sf_close(infile);
    sf_close(outfile);

#ifdef AUDIO_ALGO_DEBUG
    /**
	 * sndfile
	 */
    free(doa_buf);
    free(cluster_buf);
    free(agc_buf);
    free(nr_buf);
    free(cep_buf);
    sf_close(doa_file);
    sf_close(cluster_file);
    sf_close(agc_file);
    sf_close(nr_file);
    sf_close(cep_file);

    /**
	 * bmp
	 */
    bmp_writefile(replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_tf_bin.bmp"),
                  tf_bin_bmp, tfbmp_size);
    free(tf_bin_bmp);

    bmp_writefile(replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_noise.bmp"),
                  noise_pow_bmp, tfbmp_size);
    free(noise_pow_bmp);

    bmp_writefile(replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_spp.bmp"),
                  spp_bmp, tfbmp_size);
    free(spp_bmp);

    bmp_writefile(
        replace_subffix(input_filename, filename, LEN_FILENAME, ".wav", "_beamformed.bmp"),
        beamformed_bmp, tfbmp_size);
    free(beamformed_bmp);
#endif

    return 0;
}

int32_t main(int argc, char* argv[]) {
    char *txt_file, filename[256];
    FILE* fptr;
    size_t len;

    if (argc != 2) return -1;

    txt_file = argv[1];
    fptr = fopen(txt_file, "r");

    if (fptr == NULL) {
        printf("cannot open file: %s", txt_file);
        return -1;
    }

    while (fgets(filename, 256, fptr) != NULL) {
        if (filename[0] == '*' || filename[0] == '\n') continue;

        len = strlen(filename);
        filename[len - 1] = 0;

        printf("processing: %s\n", filename);

        simulator(filename);
    }

    fclose(fptr);

    return 0;
}
