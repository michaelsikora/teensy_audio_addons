/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Pete (El Supremo)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef effect_sikora_antinoise_h
#define effect_sikora_antinoise_h

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"

#define NP_MAX_COEFFS 300

const short AudioNoiseProfile1[257] = {
 -8938,   5191, -15630,   4717,   1335,   1178,  10355,  -2805, -10240, -13326,
 -6445,  -2834,  -9618,   6772,   -597, -18220,  22479,   1372, -13890, -10656,
  2178, -14869,   3333,  -5780,  -5973,    115,  14710,  -8010,  -8925,   -762,
  1867,   5971,  -8188,  -9623,  -4721, -13699,   1533,  -1702,  -7578,  -3912,
 -3234,     67,  -3660,  -3203,   8944,   7128, -15253,  -2626,   8194,  -1746,
 -8194,   4718,    781,  -4296, -10242,  -1795,   3512,  -3782, -11375,  -1532,
  2613,    256,   6979,  -4648, -10907,  11779,    315,  -2039,  -2623,   2439,
-21549,  10615, -20252, -15663,   1331,   6493,  -5651,  -4129,  13780, -16484,
  8701,  -2962,   1415, -13783, -32768,  -8597, -16827,    370,  -1603,    847,
 16863,  -4075,   2637,  -2264,   3962,   3494,  14475,   4576,  -4504,   5850,
 -8053, -11862,  10595, -11797,   6348,  -4478,    662,   7605,  15795, -14539,
 -7552,  -5121,  -2700,   1112, -11477, -13086,  -2974, -12428,  -1396,  -5408,
  2386, -12134,  -2005, -10918,  -1727,    889,   7959, -17049,   4660,   9829,
  6465,  -5989,  -2745,  -5466,  -4970,   1761,  24506, -14376,   5125,  -5060,
   487,    804,   1304,  -1669, -10593,  -8123,  26971,    688,  -3863,  11908,
 -7534, -15558,   -362,    844,  -3419, -11059,   7241,  -7502,    601,  11250,
-15270, -12133,  -6181,   3085,  -4749, -11463,   8632,   9866,  -3378, -12424,
  1095,  -7090, -14275,   3771,  -5905,   1044,   -435,  -2328,   1103,   6885,
 -6600,  -4681,   2992,   6731,  11818,   5489,  -3090, -12946,  -4806,  -6223,
-23622, -15499,  -2828, -11312,  -2666,  -1093,  10271,   5628,  -6157,  -3045,
 -8220,  -3037,  15140,   1599,  -3248,  -7474,  12909,  10352,  -6960,   9464,
 -4295, -11145,  10809,  12978,  -8157,  -3605,  -6729, -10527,   9549,  -7342,
  8494,  21200,    -27,   1297,   8992,    133,   3198,  10082,   4966,    420,
 13549,   5599,   1269,  15362,    402,   4644,   6287,  -5036,  11634,  -8125,
 -5437, -14962,  11137,  -5764,  -1112, -16070,   7706, -11907,  19706,  -8046,
  5216,  -1435,  -9051,  -9133,  17142,   3767,   2851
};

#define NS_PASSTHRU ((const short *) 1)
#define MULTI_UNITYGAIN 65536

class AudioSikoraAntiNoise : public AudioStream
{
public:
	AudioSikoraAntiNoise(void): AudioStream(1,inputQueueArray), 
								max_ncorr(malloc(sizeof(q15_t))), 
								multiplier(MULTI_UNITYGAIN),
								noise_prof(NULL), 
								b_corr(NULL),
								previous_gains(NULL),
								idx_max_ncorr(NULL){
	}
	
	void begin(const short *np, int n_coeffs) {
		noise_prof = np;
		if (noise_prof && (noise_prof != NS_PASSTHRU) && n_coeffs <= NP_MAX_COEFFS) {
			}
//		max_ncorr = malloc(sizeof(q15_t));
		previous_gains = malloc(sizeof(q15_t));
		previous_gains[0] = MULTI_UNITYGAIN;
		idx_max_ncorr = malloc(sizeof(uint32_t));
		alpha = 0.0;
		state = 0;
		gain_threshold = 2000;
	}
	
	void end(void) {
		noise_prof = NULL;
		max_ncorr = NULL;
		free(previous_gains);
		free(max_ncorr);
		free(idx_max_ncorr);
	}
	
	virtual void update(void);
	
	void gain(float n) {
		if (n > 32767.0f) n = 32767.0f;
		else if (n < -32767.0f) n = -32767.0f;
		multiplier = n * 65536.0f;
	}
	
	void update_val(q15_t value){ (*max_ncorr) = value; }
	q15_t get_val(void){ return (*max_ncorr); }
	
	void set_alpha(float value){ alpha = value; }
	float get_alpha(void){ return alpha; }
	
	q15_t *max_ncorr;
	int32_t multiplier;
	const short *noise_prof;
	short int state;
	
private:
	audio_block_t *inputQueueArray[1];
	q15_t *b_corr; //, *previous_gains;
	float alpha;
	q15_t *previous_gains;
	uint32_t gain_threshold;
	
	audio_block_t *noise_profile;

	// pointer to current noise profile or NULL or NS_PASSTHRU
	uint32_t *idx_max_ncorr;
	
};

#endif
