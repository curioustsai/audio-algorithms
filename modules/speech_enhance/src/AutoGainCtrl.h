#ifndef __AUTOGAINCTRL_H__
#define __AUTOGAINCTRL_H__

#include <stdint.h>

typedef struct _AutoGainCtrl {
	uint32_t nframe;
	float tpka_fp;
	float g_min_fp;
	float g_max_fp;
	float lambda_pka_fp;
	float last_g_fp;
	float pka_fp;
} AutoGainCtrl;

int32_t AutoGainCtrl_Init(AutoGainCtrl* handle, float tpka_fp, uint32_t nframe, float g_min, float g_max);
int32_t AutoGainCtrl_Process(AutoGainCtrl* handle, float* input, uint32_t speech_frame, float spp, float* output);
int32_t AutoGainCtrl_Release(AutoGainCtrl* handle);

#endif
