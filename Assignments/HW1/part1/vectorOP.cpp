#include "PPintrin.h"

// implementation of absSerial(), but it is vectorized using PP intrinsics
void absVector(float *values, float *output, int N)
{
  __pp_vec_float x;
  __pp_vec_float result;
  __pp_vec_float zero = _pp_vset_float(0.f);
  __pp_mask maskAll, maskIsNegative, maskIsNotNegative;

  //  Note: Take a careful look at this loop indexing.  This example
  //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
  //  Why is that the case?
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {

    // All ones
    maskAll = _pp_init_ones();

    // All zeros
    maskIsNegative = _pp_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _pp_vload_float(x, values + i, maskAll); // x = values[i];

    // Set mask according to predicate
    _pp_vlt_float(maskIsNegative, x, zero, maskAll); // if (x < 0) {

    // Execute instruction using mask ("if" clause)
    _pp_vsub_float(result, zero, x, maskIsNegative); //   output[i] = -x;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotNegative = _pp_mask_not(maskIsNegative); // } else {

    // Execute instruction ("else" clause)
    _pp_vload_float(result, values + i, maskIsNotNegative); //   output[i] = x; }

    // Write results back to memory
    _pp_vstore_float(output + i, result, maskAll);
  }
}

void clampedExpVector(float *values, int *exponents, float *output, int N)
{
  //
  // PP STUDENTS TODO: Implement your vectorized version of
  // clampedExpSerial() here.
  //
  // Your solution should work for any value of
  // N and VECTOR_WIDTH, not just when VECTOR_WIDTH divides N
  //

  // __pp_vec_float zero = _pp_vset_float(0.f);
  __pp_vec_float ten = _pp_vset_float(9.999999f);
  __pp_vec_float one = _pp_vset_float(1.f);
  __pp_vec_int int_one = _pp_vset_int(1);
  __pp_vec_int int_zero = _pp_vset_int(0);
  __pp_mask maskCool, maskAll, maskIsNegative, maskIsNotNegative, maskGtNegative, maskGtNotNegative;

  __pp_vec_float x;
  __pp_vec_int y;
  __pp_vec_float result;

  int cool = N % VECTOR_WIDTH;
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
        if ((N - i) < VECTOR_WIDTH){
          maskCool = _pp_init_ones(cool);
        }else{
          maskCool = _pp_init_ones();
        }
        // All ones
        maskAll = _pp_init_ones();
        maskAll = _pp_mask_and(maskAll, maskCool);
        // All zeros
        maskIsNegative = _pp_init_ones(0);

        // y = exponents[i];
        _pp_vload_int(y, exponents + i, maskAll);
        // if (y == 0)
        _pp_veq_int(maskIsNegative, y, int_zero, maskAll);
        // output[i] = 1.f
        _pp_vmove_float(result, one, maskIsNegative);

        //else
        maskIsNotNegative = _pp_mask_not(maskIsNegative);
        maskIsNotNegative = _pp_mask_and(maskIsNotNegative, maskCool);
        //result = x;
        _pp_vload_float(result, values + i, maskIsNotNegative);
        
        //int count = y - 1;
        _pp_vsub_int(y, y, int_one, maskIsNotNegative);
        _pp_vgt_int(maskIsNotNegative, y, int_zero, maskIsNotNegative);
        int count = _pp_cntbits(maskIsNotNegative);
        // char buffer[64];
        // sprintf(buffer, "count = %d", count);
        // addUserLog(buffer);
        _pp_vload_float(x, values + i, maskIsNotNegative);
        // while ( count > 0 )
        while( count > 0){
          _pp_vmult_float(result, result, x, maskIsNotNegative);

          // _pp_vgt_float(maskIsNegative, result, ten, maskIsNegative);

          _pp_vsub_int(y, y, int_one, maskIsNotNegative);
          _pp_vgt_int(maskIsNotNegative, y, int_zero, maskIsNotNegative);
          count = _pp_cntbits(maskIsNotNegative);
        }

        // if (result > 9.99999f)
        maskGtNegative = _pp_init_ones();
        maskGtNegative = _pp_mask_and(maskGtNegative, maskCool);
        maskGtNotNegative = _pp_init_ones(0);
        _pp_vgt_float(maskGtNotNegative, result, ten, maskGtNegative);
        _pp_vmove_float(result, ten, maskGtNotNegative);

        _pp_vstore_float(output + i, result, maskAll);
  }
}

// returns the sum of all elements in values
// You can assume N is a multiple of VECTOR_WIDTH
// You can assume VECTOR_WIDTH is a power of 2
float arraySumVector(float *values, int N)
{

  //
  // PP STUDENTS TODO: Implement your vectorized version of arraySumSerial here
  //
  __pp_mask maskCool, maskAll;
  float sum = 0;
  __pp_vec_float x, result;

  int cool = N % VECTOR_WIDTH;

  for (int i = 0; i < N && VECTOR_WIDTH == 1; i++)
  {
    maskAll = _pp_init_ones();
    int count = _pp_cntbits(maskAll);
    _pp_vload_float(x, values + i, maskAll);
    // sum += result.value[0];
    // // for(int j = 0; j < count; j++){
    // //   sum += result.value[j];
    // // }
    sum += values[i];
  }

  for (int i = 0; i < N && VECTOR_WIDTH != 1; i += VECTOR_WIDTH)
  {
    if ((N - i) < VECTOR_WIDTH){
      maskCool = _pp_init_ones(cool);
    }else{
      maskCool = _pp_init_ones();
    }

    maskAll = _pp_init_ones();
    maskAll = _pp_mask_and(maskAll, maskCool);

    int count = _pp_cntbits(maskAll);
    _pp_vload_float(x, values + i, maskAll);
    _pp_hadd_float(x, x);
    _pp_interleave_float(result, x);
    count = (count%2) ? count + 1 : count;
    count /= 2;
    for(int j = 0; j < count; j++){
      sum += result.value[j];
    }
    // char buffer[64];
    // sprintf(buffer, "count = %d", count);
    // addUserLog(buffer);

  }

  return sum;
}