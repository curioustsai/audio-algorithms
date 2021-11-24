// (c) Copyright 2016, Sean Connelly (@velipso), https://sean.cm
// MIT License
// Project Home: https://github.com/velipso/sndfilter

#include "compressor.h"
#include <math.h>
#include <string.h>

// core algorithm extracted from Chromium source, DynamicsCompressorKernel.cpp, here:
//   https://git.io/v1uSK
//
// changed a few things though in an attempt to simplify the curves and algorithm, and also included
// a pregain so that samples can be scaled up then compressed

void sf_defaultcomp(sf_compressor_state_st *state, int rate) {
    // sane defaults
    sf_advancecomp(state, rate,
                   0.000f,   // pregain
                   0.000f,   // postgain
                   1.000f,   // knee
                   -24.000f, // threshold
                   2.000f,   // ratio
                   -12.000f, // threshold_agg
                   4.000f,   // ratio_agg
                   -60.000f, // threshold_expander
                   1.00f,    // ratio_expander
                   0.003f,   // attack
                   0.250f,   // release
                   0.006f,   // predelay
                   0.090f,   // releasezone1
                   0.160f,   // releasezone2
                   0.420f,   // releasezone3
                   0.980f,   // releasezone4
                   1.000f    // wet
    );
}

void sf_simplecomp(sf_compressor_state_st *state, int rate, float pregain, float postgain,
                   float knee, float threshold, float ratio, float threshold_agg, float ratio_agg,
                   float threshold_expander, float ratio_expander, float attack, float release) {
    // set defaults
    sf_advancecomp(state, rate, pregain, postgain, knee, threshold, ratio, threshold_agg, ratio_agg,
                   threshold_expander, ratio_expander, attack, release,
                   0.006f, // predelay
                   0.090f, // releasezone1
                   0.160f, // releasezone2
                   0.420f, // releasezone3
                   0.980f, // releasezone4
                   1.000f  // wet
    );
}

static inline float db2lin(float db) { // dB to linear
    return powf(10.0f, 0.05f * db);
}

static inline float lin2db(float lin) { // linear to dB
    return 20.0f * log10f(lin);
}

// for more information on the knee curve, check out the compressor-curve.html demo + source code
// included in this repo
static inline float kneecurve(float x, float k, float linearthreshold) {
    return linearthreshold + (1.0f - expf(-k * (x - linearthreshold))) / k;
}

static inline float kneeslope(float x, float k, float linearthreshold) {
    return k * x / ((k * linearthreshold + 1.0f) * expf(k * (x - linearthreshold)) - 1);
}

static inline float compcurve(float x, float knee, float linearthreshold, float linearthresholdknee,
                              float k, float slope, float threshold, float kneedboffset,
                              float linearthreshold_agg, float linearthresholdknee_agg, float k_agg,
                              float slope_agg, float threshold_agg, float kneedboffset_agg,
                              float offset_agg, float linearthreshold_expander,
                              float slope_expander, float threshold_expander) {

    if (x < linearthreshold) {
        if (x > linearthreshold_expander) // linear piece
            return x;
        else
            return db2lin(fmaxf(
                threshold_expander - (slope_expander * (threshold_expander - lin2db(x))), -93));
    } else {
        if (x < linearthreshold_agg) { // compressor 1
            if (x > linearthresholdknee)
                return db2lin(kneedboffset + slope * (lin2db(x) - threshold - knee));
            else
                return kneecurve(x, k, linearthreshold);
        } else { // compressor 2
            if (x > linearthresholdknee_agg)
                return db2lin(kneedboffset_agg + slope_agg * (lin2db(x) - threshold_agg - knee) -
                              offset_agg);
            else
                return kneecurve(x, k_agg, linearthreshold_agg) / db2lin(offset_agg);
        }
    }
}

static inline float calculate_k(float linearthreshold, float linearthresholdknee, float slope) {
    float k = 5.0f; // initial guess
    float xknee = linearthresholdknee;
    float mink = 0.1f;
    float maxk = 10000.0f;
    // search by comparing the knee slope at the current k guess, to the ideal slope
    for (int i = 0; i < 15; i++) {
        if (kneeslope(xknee, k, linearthreshold) < slope)
            maxk = k;
        else
            mink = k;
        k = sqrtf(mink * maxk);
    }

    return k;
}

// this is the main initialization function
// it does a bunch of pre-calculation so that the inner loop of signal processing is fast
void sf_advancecomp(sf_compressor_state_st *state, int rate, float pregain, float postgain,
                    float knee, float threshold, float ratio, float threshold_agg, float ratio_agg,
                    float threshold_expander, float ratio_expander, float attack, float release,
                    float predelay, float releasezone1, float releasezone2, float releasezone3,
                    float releasezone4, float wet) {

    // setup the predelay buffer
    int delaybufsize = rate * predelay;
    if (delaybufsize < 1)
        delaybufsize = 1;
    else if (delaybufsize > SF_COMPRESSOR_MAXDELAY)
        delaybufsize = SF_COMPRESSOR_MAXDELAY;
    memset(state->delaybuf, 0, sizeof(float) * delaybufsize);

    // useful values
    float linearpregain = db2lin(pregain);
    float attacksamples = rate * attack;
    float attacksamplesinv = 1.0f / attacksamples;
    float releasesamples = rate * release;
    float satrelease = 0.0025f; // seconds
    float satreleasesamplesinv = 1.0f / ((float)rate * satrelease);
    float dry = 1.0f - wet;

    // compressor 1 (normal)
    float linearthreshold = db2lin(threshold);
    float linearthresholdknee = db2lin(threshold + knee);
    float slope = 1.0f / ratio;
    float kneedboffset = 0.0f;
    float k = 5.0f;    // initial guess
    if (knee > 0.0f) { // if a knee exists, search for a good k value
        k = calculate_k(linearthreshold, linearthresholdknee, slope);
        kneedboffset = lin2db(kneecurve(linearthresholdknee, k, linearthreshold));
    }

    // compressor 2 (aggressive)
    float linearthreshold_agg = db2lin(threshold_agg);
    float linearthresholdknee_agg = db2lin(threshold_agg + knee);
    float slope_agg = 1.0f / ratio_agg;
    float kneedboffset_agg = 0.0f;
    float k_agg = 5.0f; // initial guess
    if (knee > 0.0f) {  // if a knee exists, search for a good k value
        k_agg = calculate_k(linearthreshold_agg, linearthresholdknee_agg, slope_agg);
        kneedboffset_agg = lin2db(kneecurve(linearthresholdknee_agg, k_agg, linearthreshold_agg));
    }
    float knot_threshold_agg =
        kneedboffset + slope * (lin2db(linearthreshold_agg) - threshold - knee);
    float offset_agg = threshold_agg - knot_threshold_agg;

    // expander
    float linearthreshold_expander = db2lin(threshold_expander);
    float slope_expander = 1.0f / ratio_expander;

    float mastergain = db2lin(postgain);

    // calculate the adaptive release curve parameters
    // solve a,b,c,d in `y = a*x^3 + b*x^2 + c*x + d`
    // interescting points (0, y1), (1, y2), (2, y3), (3, y4)
    float y1 = releasesamples * releasezone1;
    float y2 = releasesamples * releasezone2;
    float y3 = releasesamples * releasezone3;
    float y4 = releasesamples * releasezone4;
    float a = (-y1 + 3.0f * y2 - 3.0f * y3 + y4) / 6.0f;
    float b = y1 - 2.5f * y2 + 2.0f * y3 - 0.5f * y4;
    float c = (-11.0f * y1 + 18.0f * y2 - 9.0f * y3 + 2.0f * y4) / 6.0f;
    float d = y1;

    // save everything
    state->knee = knee;
    state->wet = wet;
    state->linearpregain = linearpregain;

    // compressor 1 (normal)
    state->threshold = threshold;
    state->linearthreshold = linearthreshold;
    state->linearthresholdknee = linearthresholdknee;
    state->slope = slope;
    state->k = k;
    state->kneedboffset = kneedboffset;

    // compressor 2 (aggressive)
    state->threshold_agg = threshold_agg;
    state->linearthreshold_agg = linearthreshold_agg;
    state->linearthresholdknee_agg = linearthresholdknee_agg;
    state->slope_agg = slope_agg;
    state->k_agg = k_agg;
    state->kneedboffset_agg = kneedboffset_agg;
    state->offset_agg = offset_agg;

    // expander
    state->threshold_expander = threshold_expander;
    state->linearthreshold_expander = linearthreshold_expander;
    state->slope_expander = slope_expander;

    state->attacksamplesinv = attacksamplesinv;
    state->satreleasesamplesinv = satreleasesamplesinv;
    state->dry = dry;
    state->mastergain = mastergain;
    state->a = a;
    state->b = b;
    state->c = c;
    state->d = d;
    state->detectoravg = 0.0f;
    state->compgain = 1.0f;
    state->maxcompdiffdb = -1.0f;
    state->delaybufsize = delaybufsize;
    state->delaywritepos = 0;
    state->delayreadpos = delaybufsize > 1 ? 1 : 0;
}

// for more information on the adaptive release curve, check out adaptive-release-curve.html demo +
// source code included in this repo
static inline float adaptivereleasecurve(float x, float a, float b, float c, float d) {
    // a*x^3 + b*x^2 + c*x + d
    float x2 = x * x;
    return a * x2 * x + b * x2 + c * x + d;
}

static inline float clampf(float v, float min, float max) {
    return v < min ? min : (v > max ? max : v);
}

static inline float absf(float v) { return v < 0.0f ? -v : v; }

static inline float fixf(float v, float def) {
    // fix NaN and infinity values that sneak in... not sure why this is needed, but it is
    if (isnan(v) || isinf(v)) return def;
    return v;
}

void sf_compressor_process(sf_compressor_state_st *state, int size, float *input, float *output) {

    // pull out the state into local variables
    float threshold = state->threshold;
    float knee = state->knee;
    float linearpregain = state->linearpregain;
    float linearthreshold = state->linearthreshold;
    float slope = state->slope;
    float attacksamplesinv = state->attacksamplesinv;
    float satreleasesamplesinv = state->satreleasesamplesinv;
    float wet = state->wet;
    float dry = state->dry;
    float k = state->k;
    float kneedboffset = state->kneedboffset;
    float linearthresholdknee = state->linearthresholdknee;
    float mastergain = state->mastergain;
    float a = state->a;
    float b = state->b;
    float c = state->c;
    float d = state->d;
    float detectoravg = state->detectoravg;
    float compgain = state->compgain;
    float maxcompdiffdb = state->maxcompdiffdb;
    int delaybufsize = state->delaybufsize;
    int delaywritepos = state->delaywritepos;
    int delayreadpos = state->delayreadpos;
    float *delaybuf = state->delaybuf;

    float linearthreshold_agg = state->linearthreshold_agg;
    float linearthresholdknee_agg = state->linearthresholdknee_agg;
    float k_agg = state->k_agg;
    float slope_agg = state->slope_agg;
    float threshold_agg = state->threshold_agg;
    float kneedboffset_agg = state->kneedboffset_agg;
    float offset_agg = state->offset_agg;
    float linearthreshold_expander = state->linearthreshold_expander;
    float threshold_expander = state->threshold_expander;
    float slope_expander = state->slope_expander;

    int samplesperchunk = SF_COMPRESSOR_SPU;
    int chunks = size / samplesperchunk;
    int samplepos = 0;
    float spacingdb = SF_COMPRESSOR_SPACINGDB;
    float eps = 1e-10;

    int ch = 0;
    for (ch = 0; ch < chunks; ch++) {
        float desiredgain = detectoravg;
        float compdiffdb = lin2db(compgain / (desiredgain + eps));

        // calculate envelope rate based on whether we're attacking or releasing
        float enveloperate;
        if (compdiffdb < 0.0f) { // compgain < scaleddesiredgain, so we're releasing
            compdiffdb = fixf(compdiffdb, -1.0f);
            maxcompdiffdb = -1; // reset for a future attack mode
            // apply the adaptive release curve
            // scale compdiffdb between 0-3
            float x = (clampf(compdiffdb, -12.0f, 0.0f) + 12.0f) * 0.25f;
            float releasesamples = adaptivereleasecurve(x, a, b, c, d);
            enveloperate = db2lin(spacingdb / (releasesamples + eps));
        } else { // compresorgain > scaleddesiredgain, so we're attacking
            compdiffdb = fixf(compdiffdb, 1.0f);
            if (maxcompdiffdb == -1 || maxcompdiffdb < compdiffdb) maxcompdiffdb = compdiffdb;
            float attenuate = maxcompdiffdb;
            if (attenuate < 0.5f) attenuate = 0.5f;
            enveloperate = 1.0f - powf(0.25f / (attenuate + eps), attacksamplesinv);
        }

        // process the chunk
        int chi = 0;
        for (chi = 0; chi < samplesperchunk; chi++, samplepos++,
            delayreadpos = (delayreadpos + 1) % delaybufsize,
            delaywritepos = (delaywritepos + 1) % delaybufsize) {

            float input_pregain = input[samplepos] * linearpregain;
            delaybuf[delaywritepos] = input_pregain;
            float inputmax = absf(input_pregain);

            float inputcomp =
                compcurve(inputmax, knee, linearthreshold, linearthresholdknee, k, slope, threshold,
                          kneedboffset, linearthreshold_agg, linearthresholdknee_agg, k_agg,
                          slope_agg, threshold_agg, kneedboffset_agg, offset_agg,
                          linearthreshold_expander, slope_expander, threshold_expander);
            float attenuation = inputcomp / (inputmax + eps);

            float rate;
            if (attenuation > detectoravg) { // if releasing
                float attenuationdb = -lin2db(attenuation);
                if (attenuationdb < 2.0f) attenuationdb = 2.0f;
                float dbpersample = attenuationdb * satreleasesamplesinv;
                rate = db2lin(dbpersample) - 1.0f;
            } else
                rate = 1.0f;

            detectoravg += (attenuation - detectoravg) * rate;

            if (enveloperate < 1) // attack, reduce gain
                compgain += (desiredgain - compgain) * enveloperate;
            else { // release, increase gain
                compgain *= enveloperate;
            }

            // the final gain value!
            float gain = dry + wet * mastergain * compgain;

            // apply the gain
            output[samplepos] = delaybuf[delayreadpos] * gain;
        }
    }

    state->detectoravg = detectoravg;
    state->compgain = compgain;
    state->maxcompdiffdb = maxcompdiffdb;
    state->delaywritepos = delaywritepos;
    state->delayreadpos = delayreadpos;
}
