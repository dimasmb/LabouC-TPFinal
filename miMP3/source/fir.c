#include <stdint.h>

//////////////////////////////////////////////////////////////
//  Filter Code Definitions
//////////////////////////////////////////////////////////////

// maximum number of inputs that can be handled
// in one function call
#define MAX_INPUT_LEN   80
// maximum length of filter than can be handled
#define MAX_FLT_LEN     63
// buffer to hold all of the input samples
#define BUFFER_LEN      (MAX_FLT_LEN - 1 + MAX_INPUT_LEN)

// array to hold input samples
float insamp[ BUFFER_LEN ];

// FIR init
void firFloatInit( void )
{
    memset( insamp, 0, sizeof( insamp ) );
}

// the FIR filter function
void firFloat( float *coeffs, float *input, float *output,
       int length, int filterLength )
{
    float acc;     // accumulator for MACs
    float *coeffp; // pointer to coefficients
    float *inputp; // pointer to input samples
    int n;
    int k;

    // put the new samples at the high end of the buffer
    memcpy( &insamp[filterLength - 1], input,
            length * sizeof(float) );

    // apply the filter to each input sample
    for ( n = 0; n < length; n++ ) {
        // calculate output n
        coeffp = coeffs;
        inputp = &insamp[filterLength - 1 + n];
        acc = 0;
        for ( k = 0; k < filterLength; k++ ) {
            acc += (*coeffp++) * (*inputp--);
        }
        output[n] = acc;
    }
    // shift input samples back in time for next time
    memmove( &insamp[0], &insamp[length],
            (filterLength - 1) * sizeof(float) );

}

void intToFloat( int16_t *input, float *output, int length, char stereo )
{
    int i;

    if(stereo == 0){
        for ( i = 0; i < length; i++ ) {
            output[i] = (float)input[i];
        }
    }
    else{
        for ( i = 0; i < length*2; i+=2 ) {
            output[i/2] = (float)((input[i]+input[i])/2);
        }
    }

}

void floatToInt( float *input, int16_t *output, int length )
{
    int i;

    for ( i = 0; i < length; i++ ) {
        if ( input[i] > 32767.0 ) {
            input[i] = 32767.0;
        } else if ( input[i] < -32768.0 ) {
            input[i] = -32768.0;
        }
        // convert
        output[i] = (int16_t)input[i];
    }
}
