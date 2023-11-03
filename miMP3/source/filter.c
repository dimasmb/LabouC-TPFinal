#include "filter.h"

void convolve (double *p_coeffs, int p_coeffs_n,
			uint16_t *p_in, uint16_t *p_out, int n)
{
  int i, j, k;
  double tmp;

  for (k = 0; k < n; k++)  //  position in output
  {
    tmp = 0;

    for (i = 0; i < p_coeffs_n; i++)  //  position in coefficients array
    {
      j = k - i;  //  position in input

      if (j >= 0)  //  bounds check for input buffer
      {
        tmp += p_coeffs [i] * p_in [j];
      }
    }

    p_out [k] = (uint16_t) tmp;
  }
}
