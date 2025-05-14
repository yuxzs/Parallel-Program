#include <cstdio>
#include <cstdlib>
#include <cuda.h>

__device__ int mandel(float c_re, float c_im, int max_iterations) {
    float z_re = c_re, z_im = c_im;
    int i;
    for (i = 0; i < max_iterations; ++i) {
        if (z_re * z_re + z_im * z_im > 4.f)
            break;

        float new_re = (z_re * z_re) - (z_im * z_im);
        float new_im = 2.f * z_re * z_im;
        z_re = c_re + new_re;
        z_im = c_im + new_im;
    }
    return i;
}

__global__ void mandel_kernel(float x0, float y0, float dx, float dy,
    int width, int height,
    int start_row, int total_rows,
    int max_iterations, int *output, size_t pitch_in_ints)
{
    // To avoid error caused by the floating number, use the following pseudo code
    //
    // float x = lowerX + thisX * stepX;
    // float y = lowerY + thisY * stepY;

    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;

    int end_row = start_row + total_rows;
    if (i < width && j >= start_row && j < end_row) {
        float x = x0 + i * dx;
        float y = y0 + j * dy;
        int index = j * pitch_in_ints + i;
        output[index] = mandel(x, y, max_iterations);
    }
}

// Host front-end function that allocates the memory and launches the GPU kernel
void host_fe(float upper_x,
             float upper_y,
             float lower_x,
             float lower_y,
             int *img,
             int res_x,
             int res_y,
             int max_iterations)
{
    float step_x = (upper_x - lower_x) / (float)res_x;
    float step_y = (upper_y - lower_y) / (float)res_y;

    int *host_output;
    size_t total_bytes = res_x * res_y * sizeof(int);
    cudaHostAlloc((void**)&host_output, total_bytes, cudaHostAllocDefault);

    int *output_device;
    size_t pitch_bytes;
    cudaMallocPitch((void**)&output_device, &pitch_bytes, res_x * sizeof(int), res_y);
    size_t pitch_in_ints = pitch_bytes / sizeof(int);

    dim3 blockSize(16, 16);
    dim3 gridSize((res_x + blockSize.x - 1) / blockSize.x,
                  (res_y + blockSize.y - 1) / blockSize.y);

    mandel_kernel<<<gridSize, blockSize>>>(
        lower_x, lower_y, step_x, step_y,
        res_x, res_y,
        0, res_y,
        max_iterations, output_device, pitch_in_ints);

    cudaDeviceSynchronize();

    cudaMemcpy2D(host_output,             
        res_x * sizeof(int),      
        output_device,            
        pitch_bytes,              
        res_x * sizeof(int),      
        res_y,                    
        cudaMemcpyDeviceToHost); 
    // for (int row = 0; row < res_y; ++row) {
    //     cudaMemcpy(host_output + row * res_x,
    //                 (char*)output_device + row * pitch_bytes,
    //                 res_x * sizeof(int),
    //                 cudaMemcpyDeviceToHost);
    // }

    // cudaEvent_t start, stop;
    // float memcpyTime = 0.0f;

    // cudaEventCreate(&start);
    // cudaEventCreate(&stop);
    // cudaEventRecord(start, 0);

    memcpy(img, host_output, total_bytes);

    // cudaEventRecord(stop, 0);
    // cudaEventSynchronize(stop);
    // cudaEventElapsedTime(&memcpyTime, start, stop);

    // printf("[INFO] memcpy host_output → img took %.3f ms\n", memcpyTime);

    // cudaEventDestroy(start);
    // cudaEventDestroy(stop);

    cudaFreeHost(host_output);
    cudaFree(output_device);
}
