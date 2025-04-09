/*
 * Copyright (c) 2016-2017, Linaro Limited
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

#ifndef TA_OPTEE_LLM_H	//CHANGED HEADER GUARDS
#define TA_OPTEE_LLM_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */

//NOTE: CHANGED BINARY HERE!!!
#include <stdint.h>

typedef struct {
    uint32_t batch_size;
    uint32_t seq_length;
    uint32_t in_channels;
} tensor_dims_t;

// Fixed dimensions for the LoRA branch (expected input channels)
#define IN_CHANNELS 2048
#define RANK 4
#define OUT_CHANNELS 3
#define MAX_BATCH_SIZE 8
#define MAX_SEQ_LENGTH 128

#define TA_OPTEE_LLM_UUID \
	{ 0x522fa39d, 0xb734, 0x4b30, \
		{ 0x9c, 0x5a, 0x57, 0x41, 0xdb, 0x20, 0x84, 0xae} }

/* The function IDs implemented in this TA */
#define TA_OPTEE_LLM_CMD_INC_VALUE		0			// CHANGED THE NAME OF THE FUNCTIONS
#define TA_OPTEE_LLM_CMD_DEC_VALUE		1
#define TA_OPTEE_LLM_CMD_LORA		        2

#endif /*TA_OPTEE_LLM_H*/
