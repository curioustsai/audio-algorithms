#ifndef __SOUNDLOCATOR_H__
#define __SOUNDLOCATOR_H__

#include "basic_def.h"
#include "basic_op.h"
#include <complex.h>

typedef struct _SoundLocater {
    float sound_speed;
    uint32_t fn;
    uint32_t fs;
    uint32_t nchannel;
    uint32_t fftlen;
    uint32_t half_fftlen;

    uint32_t free_tracking_mode; // 0: disable, 1: enable
    int target_angle;            // only available when free tracking disabled

    uint32_t num_pair;

    /**
	 * GCC-Phat parameters
	 */
    uint32_t num_interpolation;
    uint32_t num_angle_samples;
    uint32_t fftlen_interpl;
    uint32_t half_fftlen_interpl;
    float *candidate_angle; // length: num_angle_samples
    uint32_t *mapper;       // length: num_angle_samples * num_pair
    complex float *xcorr;   // length: half_fftlen * num_pair
    float *theta_pair_rad;
    float *theta_pair_deg;
    void *fft_lookup;

    // local use buffer, can be replace with scratch memory
    complex float *xcorr_interpl;
    float *gphat;
    float *ifft;

    /**
	 * Projection
	 */
    float (*micPos)[3]; // 0: x, 1: y, 2: z
    float (*basis)[3];  // 0: x, 1: y, 2: z
    float theta_rad;    // [-pi, pi]
    float theta_deg;    // [ 0, 180]
    float phi_rad;      // [-pi, pi]
    float phi_deg;      // [-90, 90]
    float energy;
    float *project_matrix;
    float trace_matrix;

    /**
	 * Cluster
	 */
    uint32_t angleBufLen;
    uint32_t angleStep;
    int32_t *vadBuf; // length: angleBufLen

    int32_t angleCluster;
    int32_t *angleBuf;  // length: angleBufLen
    float *gccValueBuf; // length: angleBufLen

    uint32_t currentBeamIndex;
    uint32_t angleRetainNum;
    uint32_t *angleReset;     //length: angleRetainNum
    int32_t *angleRetain;     //length: angleRetainNum
    int32_t *sameSourceCount; //length: angleRetainNum
    int32_t *diffSourceCount; //length: angleRetainNum
    int32_t *angleDiff;
    uint32_t angleRetainMaxTime; // 625 stands for 10 sec in 0.016 frame length

    uint32_t angleNumThreshold;
    uint32_t vadNumThreshold;
    float gccValThreshold;
    float weightThreshold;

    /**
	 * if current frame is inbeam or not
	 */
    uint32_t inbeam;
    /**
	 * if current frame is  outbeam or not
	 */
    uint32_t outbeam;

    // local use buffer, can be replace with scratch memory
    float *weightBuf;     // length: (angle_range + 2 * step)
    uint32_t *numBuf;     // length: (angle_range + 2 * step)
    uint32_t *numBuf_vad; // length: (angle_range + 2 * step)

} SoundLocater;

void SoundLocater_Init(SoundLocater *handle, uint32_t fs, uint32_t fftlen, uint32_t nchannel);
void SoundLocater_FindDoa(SoundLocater *handle, complex float *X, uint32_t *angle_deg,
                          float *energy);
void SoundLocater_Cluster(SoundLocater *handle, uint32_t angle_deg, float gccVal, uint32_t vad,
                          int *inbeam, int *outbeam);
void SoundLocater_Release(SoundLocater *handle);

/*static function */
// void SoundLocater_GccPhat();
// void SoundLocater_ProjectAngle();
// void SoundLocater_ClusterAngle();
// void SoundLocater_ReplaceAngle();
// void SoundLocater_RetainAngle();
// void SoundLocater_InBeamDet();
// void SoundLocater_OutBeamDet();

#endif // __SOUNDLOCATOR_H__
