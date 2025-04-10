/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <optee_llm_ta.h> // CHANGED TA FILE HEADER

// Populate these with random data, eventually the right weights
float lora_B[RANK][IN_CHANNELS];
float lora_A[OUT_CHANNELS][RANK];

// Helper function to initialize the random data
static void populate_random_data(void)
{
    TEE_GenerateRandom(lora_B, sizeof(lora_B));
    TEE_GenerateRandom(lora_A, sizeof(lora_A));
    
    // Scale the random values into the [0, 1) float range without math.h.
    // Cast each float value to an int, apply modulo, then convert to float.
    for (int r = 0; r < RANK; r++) {
        for (int c = 0; c < IN_CHANNELS; c++) {
            int temp = ((int)lora_B[r][c]) % 100;
            if (temp < 0)
                temp += 100;
            lora_B[r][c] = temp / 100.0f;
        }
    }
    for (int o = 0; o < OUT_CHANNELS; o++) {
        for (int r = 0; r < RANK; r++) {
            int temp = ((int)lora_A[o][r]) % 100;
            if (temp < 0)
                temp += 100;
            lora_A[o][r] = temp / 100.0f;
        }
    }
}

// Forward pass functions as defined earlier.
static void matmul_B(const float x[IN_CHANNELS],
	      const float B[RANK][IN_CHANNELS],
	      float intermediate[RANK])
{
	for (int r = 0; r < RANK; r++)
	{
		float sum = 0.0f;
		for (int c = 0; c < IN_CHANNELS; c++)
		{
			sum += x[c] * B[r][c];
		}
		intermediate[r] = sum;
	}
}

static void matmul_A(const float intermediate[RANK],
	      const float A[OUT_CHANNELS][RANK],
	      float output[OUT_CHANNELS])
{
	for (int out = 0; out < OUT_CHANNELS; out++)
	{
		float sum = 0.0f;
		for (int r = 0; r < RANK; r++)
		{
			sum += intermediate[r] * A[out][r];
		}
		output[out] = sum;
	}
}

static void lora_forward_token(const float x[IN_CHANNELS],
			const float B[RANK][IN_CHANNELS],
			const float A[OUT_CHANNELS][RANK],
			float scale,
			float output[OUT_CHANNELS])
{
	float intermediate[RANK];
	matmul_B(x, B, intermediate);
	matmul_A(intermediate, A, output);
	for (int i = 0; i < OUT_CHANNELS; i++)
	{
		output[i] *= scale;
	}
}

// Runs LoRA inference for a single sample with a variable sequence length.
static void lora_forward_sample(const float *input_sample,
				uint32_t seq_length,
				const float B[RANK][IN_CHANNELS],
				const float A[OUT_CHANNELS][RANK],
				float scale,
				float output_sample[OUT_CHANNELS])
{
	// Temporary buffer for token-level output.
	float token_output[OUT_CHANNELS];
	// Initialize the sample output.
	for (int i = 0; i < OUT_CHANNELS; i++)
	{
		output_sample[i] = 0.0f;
	}
	// Process each token.
	for (uint32_t token = 0; token < seq_length; token++)
	{
		// Calculate pointer offset for this token.
		const float *x = input_sample + token * IN_CHANNELS;
		lora_forward_token(x, B, A, scale, token_output);
		for (int i = 0; i < OUT_CHANNELS; i++)
		{
			output_sample[i] += token_output[i];
		}
	}
	// Compute the average over the sequence tokens.
	for (int i = 0; i < OUT_CHANNELS; i++)
	{
		output_sample[i] /= seq_length;
	}
}


// Main inference function
static TEE_Result run_lora_inference(uint32_t param_types, TEE_Param params[4])
{
	// Expected parameter types:
	// Param0: Input tensor MEMREF
	// Param1: Output tensor MEMREF
	// Param2: Tensor dimensions passed as MEMREF
	const float scale = 1.0f;
	
	populate_random_data();
	
	const uint32_t expected_types =
	    TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
			    TEE_PARAM_TYPE_MEMREF_OUTPUT,
			    TEE_PARAM_TYPE_MEMREF_INPUT,
			    TEE_PARAM_TYPE_NONE);
	if (param_types != expected_types)
		return TEE_ERROR_BAD_PARAMETERS;

	// Get pointers to the buffers.
	float *input = (float *)params[0].memref.buffer;
	float *output = (float *)params[1].memref.buffer;

	// Parse dimensions from param2.
	if (params[2].memref.size < sizeof(tensor_dims_t))
		return TEE_ERROR_BAD_PARAMETERS;
	tensor_dims_t *dims = (tensor_dims_t *)params[2].memref.buffer;

	// Validate dimensions.
	if (dims->in_channels != IN_CHANNELS ||
	    dims->batch_size > MAX_BATCH_SIZE ||
	    dims->seq_length > MAX_SEQ_LENGTH)
		return TEE_ERROR_BAD_PARAMETERS;

	// Run the forward pass for each sample.
	// The input tensor is assumed to be flattened in row-major order:
	// [batch_size * seq_length * IN_CHANNELS]
	for (uint32_t sample = 0; sample < dims->batch_size; sample++)
	{
		float sample_output[OUT_CHANNELS] = {0};
		// Offset into the input for this sample.
		const float *sample_input = input + sample * dims->seq_length * IN_CHANNELS;
		lora_forward_sample(sample_input, dims->seq_length, lora_B, lora_A, scale, sample_output);
		// Write sample output to the flat output buffer: [batch_size * OUT_CHANNELS]
		for (int i = 0; i < OUT_CHANNELS; i++)
		{
			output[sample * OUT_CHANNELS + i] = sample_output[i];
		}
	}

	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");
	// FIXME: this may be causing lag???
	// populate_random_data();
	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
				    TEE_Param __maybe_unused params[4],
				    void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

static TEE_Result inc_value(uint32_t param_types,
			    TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a++;
	IMSG("Increase value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}

static TEE_Result dec_value(uint32_t param_types,
			    TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a--;
	IMSG("Decrease value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}
/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
				      uint32_t cmd_id,
				      uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id)
	{
	case TA_OPTEE_LLM_CMD_INC_VALUE: // CHANGED THE COMMAND NAMES IN THE CASES
		return inc_value(param_types, params);
	case TA_OPTEE_LLM_CMD_DEC_VALUE:
		return dec_value(param_types, params);
	case TA_OPTEE_LLM_CMD_LORA:
		return run_lora_inference(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
