__kernel void convolution(  __global const float* input_image,
                            __global float* output_image,
                            __global const float* filter,
                            int image_width,
                            int image_height,
                            int filter_width
                        ){
    int i = get_global_id(1); // row
    int j = get_global_id(0); // col

    int half_filter = filter_width / 2;
    float sum = 0.0f;

    for (int k = -half_filter; k <= half_filter; ++k)
    {
        for (int l = -half_filter; l <= half_filter; ++l)
        {
            int ni = i + k;
            int nj = j + l;

            if (ni >= 0 && ni < image_height && nj >= 0 && nj < image_width)
            {
                float img_val = input_image[ni * image_width + nj];
                float filt_val = filter[(k + half_filter) * filter_width + (l + half_filter)];
                sum += img_val * filt_val;
            }
        }
    }

    output_image[i * image_width + j] = sum;
}