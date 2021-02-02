#include "AutoGainCtrl.h"
#include "basic_op.h"

int32_t AutoGainCtrl_Init(AutoGainCtrl* handle, float tpka_fp, uint32_t nframe, float g_min, float g_max)
{
	handle->tpka_fp = tpka_fp;
	handle->g_min_fp = g_min;
	handle->g_max_fp = g_max;
	handle->lambda_pka_fp = 7.0f / 8.0f;
	handle->nframe = nframe;

	handle->last_g_fp = g_max; // 1.0;
	handle->pka_fp = 1024.0f;

	return 0;
}

int32_t AutoGainCtrl_Release(AutoGainCtrl* handle)
{
	return 0;
}

int32_t AutoGainCtrl_Process(AutoGainCtrl* handle, float* input, uint32_t speech_frame, float spp, float* output)
{
	float max_abs_y;
	float mu;
	float g, g_fp;
	uint16_t idx;
	uint16_t nframe = handle->nframe;
	uint16_t eps = 64;

	max_abs_y = fabsf(input[0]);
	for (idx = 1; idx < nframe; ++idx)
		if (max_abs_y < fabsf(input[idx]))
			max_abs_y = fabsf(input[idx]);

	if (max_abs_y > handle->pka_fp)
	{
		handle->pka_fp = 0.75f * handle->pka_fp + 0.25f * max_abs_y;
	}
	else
	{
		if ((speech_frame == 1) && (spp > 0.5f) && (max_abs_y > eps))
		{
			mu = spp * (1 - handle->lambda_pka_fp);
			handle->pka_fp = (1 - mu) * handle->pka_fp + mu * max_abs_y;
		}
		else // TODO: do something under noise case
		{
			handle->pka_fp = 0.98f * handle->pka_fp + 0.02f * max_abs_y;
		}
	}

	g = handle->tpka_fp / (handle->pka_fp + 1e-12f);
	g = max(g, handle->g_min_fp);
	g = min(g, handle->g_max_fp);

	g_fp = 0.75f * handle->last_g_fp + 0.25f * g;
	if (g_fp * max_abs_y > 32767.0f)
		g_fp = 32767.f / (max_abs_y + 1e-12f);

	for (idx = 0; idx < nframe; ++idx)
		output[idx] = g_fp * input[idx];

	handle->last_g_fp = g_fp;

	return 0;
}
