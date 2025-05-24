#include "host_fe.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>

void host_fe(int filter_width,
             float *filter,
             int image_height,
             int image_width,
             float *input_image,
             float *output_image,
             cl_device_id *device,
             cl_context *context,
             cl_program *program)
{
    cl_int status;
    int filter_size = filter_width * filter_width;
    int image_size = image_height * image_width;

    // === 1. Create command queue ===
    cl_command_queue queue = clCreateCommandQueue(*context, *device, 0, &status);
    CHECK(status, "clCreateCommandQueue");

    // === 2. Create buffers ===
    cl_mem d_input = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    sizeof(float) * image_size, input_image, &status);
    CHECK(status, "clCreateBuffer input");

    cl_mem d_filter = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                     sizeof(float) * filter_size, filter, &status);
    CHECK(status, "clCreateBuffer filter");

    cl_mem d_output = clCreateBuffer(*context, CL_MEM_WRITE_ONLY,
                                     sizeof(float) * image_size, NULL, &status);
    CHECK(status, "clCreateBuffer output");

    // === 3. Create kernel ===
    cl_kernel kernel = clCreateKernel(*program, "convolution", &status);
    CHECK(status, "clCreateKernel");

    // === 4. Set kernel arguments ===
    status |= clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_input);
    status |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_output);
    status |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_filter);
    status |= clSetKernelArg(kernel, 3, sizeof(int), &image_width);
    status |= clSetKernelArg(kernel, 4, sizeof(int), &image_height);
    status |= clSetKernelArg(kernel, 5, sizeof(int), &filter_width);
    CHECK(status, "clSetKernelArg");

    // === 5. Define global work size ===
    size_t global_size[2] = { (size_t)image_width, (size_t)image_height };

    // === 6. Enqueue kernel execution ===
    status = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);
    CHECK(status, "clEnqueueNDRangeKernel");

    // === 7. Read result back ===
    status = clEnqueueReadBuffer(queue, d_output, CL_TRUE, 0,
                                 sizeof(float) * image_size, output_image, 0, NULL, NULL);
    CHECK(status, "clEnqueueReadBuffer");

    // === 8. Cleanup ===
    clReleaseMemObject(d_input);
    clReleaseMemObject(d_filter);
    clReleaseMemObject(d_output);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
}
