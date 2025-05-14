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
    int max_iterations, int *output)
{
    // To avoid error caused by the floating number, use the following pseudo code
    //
    // float x = lowerX + thisX * stepX;
    // float y = lowerY + thisY * stepY;

    int i = blockIdx.x * blockDim.x + threadIdx.x;  // column
    int j = blockIdx.y * blockDim.y + threadIdx.y;  // row

    int end_row = start_row + total_rows;
    if (i < width && j >= start_row && j < end_row) {
        float x = x0 + i * dx;
        float y = y0 + j * dy;
        int index = j * width + i;
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

    int* host_output = new int[res_x * res_y];

    int *output_device;
    size_t total_size = res_x * res_y * sizeof(int);
    cudaMalloc(&output_device, total_size);

    dim3 blockSize(16, 16);
    dim3 gridSize((res_x + blockSize.x - 1) / blockSize.x,
                  (res_y + blockSize.y - 1) / blockSize.y);

    mandel_kernel<<<gridSize, blockSize>>>(
        lower_x, lower_y, step_x, step_y,
        res_x, res_y,
        0, res_y,
        max_iterations, output_device);

    cudaDeviceSynchronize();
    cudaMemcpy(host_output, output_device, total_size, cudaMemcpyDeviceToHost);
    memcpy(img, host_output, res_x * res_y * sizeof(int));

    delete[] host_output;
    cudaFree(output_device);
}
