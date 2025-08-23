#include "cordic.hpp"
#include "stm32g4xx_ll_cordic.h"

static constexpr float M_1_Q31   = 4.656612873077392578125e-10f; // 1/2^31
static constexpr float M_Q31_PI  = 6.8356527557643158978229477811035e8f;  // 2^31/pi

void cordic_init()
{
	LL_CORDIC_Config(CORDIC, 
		LL_CORDIC_FUNCTION_COSINE, 
		LL_CORDIC_PRECISION_1CYCLE, // 4 iterations per cycle for maximum 24 iteration precision
		LL_CORDIC_SCALE_0,           // Result scaling not applicable
		LL_CORDIC_NBWRITE_1,         // One 32 bit input for angle. No modulus argument for sin and cos.
		LL_CORDIC_NBREAD_2,            // Two output data - sine then cosine
		LL_CORDIC_INSIZE_32BITS,      // q1.31 format for input data
		LL_CORDIC_OUTSIZE_32BITS);     // q1.31 format for input data
}

void cordic_sincos(volatile float angle, float *sin, float *cos)
{
  int input = (int)(angle * M_Q31_PI);
  LL_CORDIC_WriteData(CORDIC, input);
  *cos = (float)((int)LL_CORDIC_ReadData(CORDIC)) * M_1_Q31;
  *sin = (float)((int)LL_CORDIC_ReadData(CORDIC)) * M_1_Q31;
}
