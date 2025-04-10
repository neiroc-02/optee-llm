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

#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>
#include <optee_llm_ta.h>

#define INPUT_NUM_ELEMENTS (MAX_BATCH_SIZE * MAX_SEQ_LENGTH * IN_CHANNELS)
#define INPUT_SIZE_BYTES (INPUT_NUM_ELEMENTS * sizeof(float))
#define OUTPUT_NUM_ELEMENTS (MAX_BATCH_SIZE * OUT_CHANNELS)
#define OUTPUT_SIZE_BYTES (OUTPUT_NUM_ELEMENTS * sizeof(float))

int main(void)
{
    TEEC_Result res;
    TEEC_Context context;
    TEEC_Session session;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_OPTEE_LLM_UUID;
    TEEC_SharedMemory input_shm = {0};
    TEEC_SharedMemory output_shm = {0};
    TEEC_SharedMemory dims_shm = {0};
    uint32_t err_origin;

    /* Initialize a context connecting us to the TEE */
    res = TEEC_InitializeContext(NULL, &context);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    /*
     * Open a session to the "hello world" TA, the TA will print "hello
     * world!" in the log when the session is created.
     */
    res = TEEC_OpenSession(&context, &session, &uuid,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
             res, err_origin);

    /*
     * Execute a function in the TA by invoking it, in this case
     * we're incrementing a number.
     *
     * The value of command ID part and how the parameters are
     * interpreted is part of the interface provided by the TA.
     */

    // 3. Allocate shared memory for the input tensor.
    input_shm.size = INPUT_SIZE_BYTES;
    input_shm.flags = TEEC_MEM_INPUT;
    res = TEEC_AllocateSharedMemory(&context, &input_shm);
    if (res != TEEC_SUCCESS)
    {
        printf("TEEC_AllocateSharedMemory (input) failed: 0x%x\n", res);
        goto cleanup_session;
    }

    // 4. Allocate shared memory for the output tensor.
    output_shm.size = OUTPUT_SIZE_BYTES;
    output_shm.flags = TEEC_MEM_OUTPUT;
    res = TEEC_AllocateSharedMemory(&context, &output_shm);
    if (res != TEEC_SUCCESS)
    {
        printf("TEEC_AllocateSharedMemory (output) failed: 0x%x\n", res);
        goto cleanup_input;
    }

    // 5. Allocate shared memory for the tensor dimensions structure.
    dims_shm.size = sizeof(tensor_dims_t);
    dims_shm.flags = TEEC_MEM_INPUT;
    res = TEEC_AllocateSharedMemory(&context, &dims_shm);
    if (res != TEEC_SUCCESS)
    {
        printf("TEEC_AllocateSharedMemory (dims) failed: 0x%x\n", res);
        goto cleanup_output;
    }

    // 6. Initialize the input tensor.
    // The input tensor is very large, so we fill it here using the shared memory.
    {
        float *input_data = (float *)input_shm.buffer;
        for (size_t i = 0; i < INPUT_NUM_ELEMENTS; i++)
        {
            input_data[i] = 0.01f; // Test data for latency testing.
        }
    }

    // 7. Prepare the tensor dimensions.
    {
        tensor_dims_t *dims = (tensor_dims_t *)dims_shm.buffer;
        dims->batch_size = MAX_BATCH_SIZE;
        dims->seq_length = MAX_SEQ_LENGTH;
        dims->in_channels = IN_CHANNELS;
    }

    // 8. Set up the operation parameters.
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_MEMREF_WHOLE, // Param0: Input tensor
        TEEC_MEMREF_WHOLE, // Param1: Output tensor
        TEEC_MEMREF_WHOLE, // Param2: Tensor dimensions
        TEEC_NONE);
    op.params[0].memref.parent = &input_shm;
    op.params[1].memref.parent = &output_shm;
    op.params[2].memref.parent = &dims_shm;

    // 9. Invoke the TA command.
    res = TEEC_InvokeCommand(&session, TA_OPTEE_LLM_CMD_LORA, &op, &err_origin);
    if (res != TEEC_SUCCESS)
    {
        printf("TEEC_InvokeCommand failed: 0x%x, origin: 0x%x\n", res, err_origin);
        goto cleanup_dims;
    }

    // 10. Process and print the output tensor.
    {
        float *output_data = (float *)output_shm.buffer;
        printf("Output Tensor:\n");
        for (uint32_t sample = 0; sample < MAX_BATCH_SIZE; sample++)
        {
            printf("Sample %u: ", sample);
            for (uint32_t c = 0; c < OUT_CHANNELS; c++)
            {
                printf("%f ", output_data[sample * OUT_CHANNELS + c]);
            }
            printf("\n");
        }
    }

cleanup_dims:
    TEEC_ReleaseSharedMemory(&dims_shm);
cleanup_output:
    TEEC_ReleaseSharedMemory(&output_shm);
cleanup_input:
    TEEC_ReleaseSharedMemory(&input_shm);
cleanup_session:
    TEEC_CloseSession(&session);
    TEEC_FinalizeContext(&context);
    return (res == TEEC_SUCCESS ? 0 : 1);
}
