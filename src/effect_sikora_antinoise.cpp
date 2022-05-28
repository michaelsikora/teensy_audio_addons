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

#include <Arduino.h>
#include "effect_sikora_antinoise.h"
#include "utility/dspinst.h"

static void applyGain(int16_t *data, int32_t mult)
{
	uint32_t *p = (uint32_t *)data;
	const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);

	do {
		uint32_t tmp32 = *p; // read 2 samples from *data
		int32_t val1 = signed_multiply_32x16b(mult, tmp32);
		int32_t val2 = signed_multiply_32x16t(mult, tmp32);
		val1 = signed_saturate_rshift(val1, 16, 0);
		val2 = signed_saturate_rshift(val2, 16, 0);
		*p++ = pack_16b_16b(val2, val1);
	} while (p < end);
}

void AudioSikoraAntiNoise::update(void)
{
	audio_block_t *block; 
	int32_t mult = multiplier;

	previous_gains[0] = multiplier;


	block = receiveReadOnly();
	if (!block) return;
	
	// do passthru
	if (noise_prof == NS_PASSTHRU) {
		// Just passthrough
		transmit(block);
		release(block);
		return;
	}
			
	b_corr = malloc(sizeof(q15_t)*(2*AUDIO_BLOCK_SAMPLES-1));
	
	arm_correlate_fast_q15(	(q15_t *)block->data, 
							AUDIO_BLOCK_SAMPLES,
							(short *)noise_prof,
							AUDIO_BLOCK_SAMPLES,
							(q15_t *)b_corr);	
							
	arm_max_q15(  (q15_t *)b_corr,
				  (uint32_t) 2*AUDIO_BLOCK_SAMPLES - 1,
				  (q15_t *)max_ncorr,
				  (uint32_t *)idx_max_ncorr);							

	free(b_corr);

	mult = (long int)(alpha*(*max_ncorr) + (1-alpha)*previous_gains[0]);
	multiplier = mult;
	
	if (mult > gain_threshold) {
		if (state == 0) state = 1;
		mult *= 2;
	} else {
		if (state == 1) state = 0;
	}

	if (mult == 0) {
		// zero gain, discard any input and transmit nothing
		release(block);
	} else if (mult == MULTI_UNITYGAIN) {
		// unity gain, pass input to output without any change
		transmit(block);
		release(block);
		return;
	} else {
		// apply gain to signal
		applyGain(block->data, mult);
		transmit(block); // send the block output
		release(block);
		return;
	}
	release(block);
}


