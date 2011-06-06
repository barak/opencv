/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "cvtest.h"
#include "time.h"

char* atsTimingClass = "Timing"; 
char* atsAlgoClass = "Algorithm";
int   atsCPUFreq = 0;

int atsIsNaN( double x )
{
    int highword = (int)(*((int64*)&x)>>32);
    return (highword & 0x7fffffff) > 0x7ff00000;
}

//////////////////////// comparison functions /////////////////////////////

long atsCompSingle(float flFirst,
                   float flSecond,
                   double dbAcc)
{
        if(fabs( flFirst-flSecond )>
            dbAcc * ( fabs(flFirst) + fabs(flSecond))+dbAcc)
            return 1;      
                   
        else return 0;
        
}

long atsCompSinglePrec(float* flFirstArray,
                       float* flSecondArray,
                       long lLen,
                       double dbAcc)
{
    long lErrors = 0;
    int i;
    for(i = 0; i < lLen; i++)
    {
        if(fabs( flFirstArray[i]-flSecondArray[i] )>
            dbAcc * ( fabs(flFirstArray[i]) + fabs(flSecondArray[i]))+dbAcc){
            lErrors++;
        }
        
    }
        return lErrors;
}

long atsCompDoublePrec(double* flFirstArray,
                       double* flSecondArray,
                       long lLen,
                       double dbAcc)
{
    long lErrors = 0;
    int i;
    for(i = 0; i < lLen; i++)
    {
        if(fabs( flFirstArray[i]-flSecondArray[i] )>  
           dbAcc * ( fabs(flFirstArray[i]) + fabs(flSecondArray[i]))+dbAcc)
           lErrors++;
        
      
    }
     return lErrors;
}

    

long atsCompare1Dfl( float* flArrayAct, float* flArrayExp, long lLen, double dbTol )
{
    int   i;
    long  lErrors = 0;
    
    for( i = 0; i < lLen; i++ )
    {
        if( fabs( flArrayAct[i] - flArrayExp[i] ) > dbTol )
        {
            lErrors = 153;
            trsWrite( ATS_LST,
                "Error: x = %d   act  %f    exp %f\n",
                i,
                flArrayAct[i],
                flArrayExp[i] );
            break;
        } /* if */
    } /* for */
    
    if( lErrors == 0 ) trsWrite( ATS_LST,
        "No errors detected for this test\n" );
    else trsWrite( ATS_SUM | ATS_LST, "Total fixed %d errors :(\n", lErrors );
    return lErrors;
} /* atsCompare1Dfl */


long atsCompare2Dfl( float* ArrayAct, float* ArrayExp, CvSize size, int stride, double Tol )
{
    int   x, y;
    long  lErrors = 0;
    
    for( y = 0; y < size.height; y++, ArrayAct = (float*)((long)ArrayAct + stride), 
                                      ArrayExp = (float*)((long)ArrayExp + stride) )
        for( x = 0; x < size.width; x++ )
            if( fabs( ArrayAct[x] - ArrayExp[x] ) > Tol )
            {
                lErrors = 153;
                trsWrite( ATS_LST,
                    "Error: x=%d  y=%d   act  %f    exp %f\n",
                    x, y,
                    (float)ArrayAct[x],
                    (float)ArrayExp[x] );
                break;
            } /* if */
            
            if( lErrors == 0 ) trsWrite( ATS_LST,
                "No errors detected for this test\n" );
            else trsWrite( ATS_SUM | ATS_LST, "Total fixed %d errors :(\n", lErrors );
            return lErrors;
} /* atsCompare1Db */


long atsCompare1Db( uchar* ArrayAct, uchar* ArrayExp, long lLen, int Tol )
{
    int   i;
    long  lErrors = 0;
    
    for( i = 0; i < lLen; i++ )
    {
        if( abs( ArrayAct[i] - ArrayExp[i] ) > Tol )
        {
            lErrors = 153;
            trsWrite( ATS_LST,
                "Error: x = %d   act  %d    exp %d\n",
                i,
                (int)ArrayAct[i],
                (int)ArrayExp[i] );
            break;
        } /* if */
    } /* for */
    
    if( lErrors == 0 ) trsWrite( ATS_LST,
        "No errors detected for this test\n" );
    else trsWrite( ATS_SUM | ATS_LST, "Total fixed %d errors :(\n", lErrors );
    return lErrors;
} /* atsCompare1Db */


long atsCompare2Db( uchar* ArrayAct, uchar* ArrayExp, CvSize size, int stride, int Tol )
{
    int   x, y;
    long  lErrors = 0;
    
    for( y = 0; y < size.height; y++, ArrayAct += stride, ArrayExp += stride )
        for( x = 0; x < size.width; x++ )
            if( abs( ArrayAct[x] - ArrayExp[x] ) > Tol )
            {
                lErrors = 153;
                trsWrite( ATS_LST,
                    "Error: x=%d  y=%d   act  %d    exp %d\n",
                    x, y,
                    (int)ArrayAct[x],
                    (int)ArrayExp[x] );
                break;
            } /* if */
    
    if( lErrors ) trsWrite( ATS_SUM | ATS_LST, "Total fixed %d errors :(\n", lErrors );
    return lErrors;
}


long atsCompare1Dc( char* ArrayAct, char* ArrayExp, long lLen, int Tol )
{
    int   i;
    long  lErrors = 0;
    
    for( i = 0; i < lLen; i++ )
    {
        if( abs( ArrayAct[i] - ArrayExp[i] ) > Tol )
        {
            lErrors = 153;
            trsWrite( ATS_LST,
                "Error: x = %d   act  %d    exp %d\n",
                i,
                (int)ArrayAct[i],
                (int)ArrayExp[i] );
            break;
        } /* if */
    } /* for */
    
    if( lErrors ) trsWrite( ATS_SUM | ATS_LST, "Total fixed %d errors :(\n", lErrors );
    return lErrors;
} /* atsCompare1Dc */


long atsCompare2Dc( char* ArrayAct, char* ArrayExp, CvSize size, int stride, int Tol )
{
    int   x, y;
    long  lErrors = 0;
    
    for( y = 0; y < size.height; y++, ArrayAct += stride, ArrayExp += stride )
        for( x = 0; x < size.width; x++ )
            if( abs( ArrayAct[x] - ArrayExp[x] ) > Tol )
            {
                lErrors = 153;
                trsWrite( ATS_LST,
                    "Error: x=%d  y=%d   act  %d    exp %d\n",
                    x, y,
                    (int)ArrayAct[x],
                    (int)ArrayExp[x] );
                break;
            } /* if */
            
            if( lErrors ) trsWrite( ATS_SUM | ATS_LST, "Total fixed %d errors :(\n", lErrors );
            return lErrors;
}


//////////////////////////// ellipse initialization ////////////////////////

void atsbInitEllipse( uchar* Src,
                      int    width,
                      int    height,
                      int    step,
                      int    x,
                      int    y,
                      int    major,
                      int    minor,
                      float  orientation,
                      uchar  value )
{
    /* Some variables */
    int   i, j;
    float cx, cy;

    /* Filling */
    for( i = 0; i < height; i++ )
    {
        for( j = 0; j < width; j++ )
        {
            cx =  (float)((j - x) * cos( orientation ) + (i - y) * sin( orientation ));
            cy = (float)(-(j - x) * sin( orientation ) + (i - y) * cos( orientation ));
            if( (cx * cx) / (major * major) + (cy * cy) / (minor * minor) <= 1.0f )
                Src[j] = value;
            else Src[j] = 0;
        }
        Src += step;
    }
} /* atsbInitEllipse */


void atsfInitEllipse( float* Src,
                      int    width,
                      int    height,
                      int    step,
                      int    x,
                      int    y,
                      int    major,
                      int    minor,
                      float  orientation,
                      float  value )
{
    /* Some variables */
    int   i, j;
    float cx, cy;

    /* Filling */
    for( i = 0; i < height; i++ )
    {
        for( j = 0; j < width; j++ )
        {
            cx =  (float)((j - x) * cos( orientation ) + (i - y) * sin( orientation ));
            cy = (float)(-(j - x) * sin( orientation ) + (i - y) * cos( orientation ));
            if( (cx * cx) / (major * major) + (cy * cy) / (minor * minor) <= 1.0f )
                Src[j] = value;
            else Src[j] = 0;
        }
        Src = (float*)((long)Src + step);
    }
} /* atsfInitEllipse */


void ats1flInitGrad( double Min, double Max, float* pDst, long lLen )
{
    /* Some variables */
    int   i;
    float flStep;

    assert( pDst != 0 );
    assert( lLen > 1 );

    flStep = (float)(Max - Min) / (lLen - 1);

    /* Init */
    pDst[0] = (float)Min;
    pDst[lLen - 1] = (float)Max;
    for( i = 1; i < lLen - 1; i++ )
    {
        pDst[i] = (float)(flStep * i + Min);
        assert( pDst[i - 1] < pDst[i] );
    } /* for */
    assert( pDst[lLen - 2] < pDst[lLen - 1] );

} /* ats1flInitGrad */


///////////////////////// random number generation (I) ///////////////////////

double atsInitRandom( double Min, double Max )
{
  static int flag = 0;
  if( flag == 0 )
  {
    srand( (unsigned)time( 0 ) );
    flag = 1;
  } /* if */

  return (double)rand() / RAND_MAX * (Max - Min) + Min;
} /* atsInitRandom */


void ats1bInitRandom( double Min, double Max, unsigned char* pDst, long lLen )
{
  /* Some variables */
  int i;

  assert( pDst != 0 );
  assert( lLen > 0 );
  /* Init */
  for( i = 0; i < lLen; i++ ) pDst[i] = (unsigned char)atsInitRandom( Min,Max );

} /* ats1bInitRandom */


void ats1cInitRandom( double Min, double Max, char* pDst, long lLen )
{
  /* Some variables */
  int i;

  assert( pDst != 0 );
  assert( lLen > 0 );
  /* Init */
  for( i = 0; i < lLen; i++ ) pDst[i] = (unsigned char)atsInitRandom( Min,Max );

} /* ats1cInitRandom */


void ats1iInitRandom( double Min, double Max, int* pDst, long lLen )
{
  /* Some variables */
  int i;

  assert( pDst != 0 );
  assert( lLen > 0 );
  /* Init */
  for( i = 0; i < lLen; i++ ) pDst[i] = (int)atsInitRandom( Min,Max );

} /* ats1iInitRandom */


void ats1flInitRandom( double Min, double Max, float* pDst, long lLen )
{
  /* Some variables */
  int   i;

  assert( pDst != 0 );
  assert( lLen > 1 );

  /* Init */
  for( i = 0; i < lLen; i++ )
  {
    pDst[i] = (float)atsInitRandom( Min, Max );
  } /* for */

} /* ats1flInitGradRandom */


void ats1flInitGradRandom( double Min, double Max, float* pDst, long lLen )
{
  /* Some variables */
  int   i;
  float flStep;

  assert( pDst != 0 );
  assert( lLen > 1 );

  flStep = (float)(Max - Min) / (lLen - 1);

  /* Init */
  pDst[0] = (float)Min;
  pDst[lLen - 1] = (float)Max;
  for( i = 1; i < lLen - 1; i++ )
  {
    pDst[i] = (float)atsInitRandom( pDst[i - 1] + 0.1 * flStep,
                             flStep * (i + 1) - 0.1 * flStep );
    assert( pDst[i] > pDst[i - 1] );
  } /* for */

} /* ats1flInitGradRandom */



////////////////////////// random number generation (II) ////////////////////

/* get seed using processor tick counter */
unsigned  atsGetSeed( void )
{
    int64 seed64 = atsGetTickCount();
    return ((unsigned)(seed64 >> 32))|((unsigned)seed64);
}

static int calc_shift( unsigned val )
{
    int i = 0;
    for( i = 0; i <= 31 && !(val & (0x80000000 >> i)); i++ );
    return i+1;
}

/* will generate random numbers in [lower,upper) */
void  atsRandSetBounds( AtsRandState* state, double lower, double upper )
{
    state->fb = (float)(lower - (state->fa = (float)(upper - lower)));
    state->ia = cvRound( upper - lower );
    state->ib = cvRound( lower );
    /* shift = <ia is power of 2> ? calc_shift( ia ) : 0; */
    state->shift = (state->ia & (state->ia - 1)) == 0 ? calc_shift(state->ia) : 0;
    if( state->shift == 32 ) state->shift = 0;
    state->mask = -1;
}


/* will generate random numbers in [lower,upper) */
void  atsRandInit( AtsRandState* state, double lower, double upper, unsigned seed )
{
    state->seed = seed;
    atsRandSetBounds( state, lower, upper );
}

void atsRandSetFloatBits( AtsRandState* state, int bits )
{
    state->mask = -(1 << (23 - bits));
}

/* Super-Duper generator */
#define ATS_LCG_A   69069
#define ATS_LCG_B   1

#define ATS_RNG_NEXT( seed )   ((seed)*ATS_LCG_A + ATS_LCG_B)

float  atsRand32f( AtsRandState* state )
{
    float res = 0.f;
    if( state )
    {
        unsigned seed = ATS_RNG_NEXT( state->seed );
        *((unsigned*)&res) = ((seed >> 9) | 0x3f800000) & state->mask;
        state->seed = seed;
        res = res*state->fa + state->fb;
    }
    return res;
}


void  atsbRand32f( AtsRandState* state, float* vect, int len )
{
    int i, mask;
    unsigned seed;
    float fa, fb;

    seed = state->seed;
    fa = state->fa;
    fb = state->fb;
    mask = state->mask;

    for( i = 0; i < len; i++ )
    {
        float res;
        seed = ATS_RNG_NEXT( seed );
        *((unsigned*)&res) = ((seed >> 9) | 0x3f800000) & mask;
        vect[i] = res*fa + fb;
    }

    state->seed = seed;
}

void  atsbRand64d( AtsRandState* state, double* vect, int len )
{
    int i, mask;
    unsigned seed;
    float fa, fb;

    seed = state->seed;
    fa = state->fa;
    fb = state->fb;
    mask = state->mask;

    for( i = 0; i < len; i++ )
    {
        float res;
        seed = ATS_RNG_NEXT( seed );
        *((unsigned*)&res) = ((seed >> 9) | 0x3f800000) & mask;
        vect[i] =(double)(res*fa + fb);
    }

    state->seed = seed;
}


int  atsRand32s( AtsRandState* state )
{
    int res = 0;
    if( state )
    {
        unsigned seed = ATS_RNG_NEXT( state->seed );
        int shift = state->shift;
        if( !shift )
            res = (seed >> 3) % state->ia + state->ib;
        else
            res = (seed >> state->shift) + state->ib;

        state->seed = seed;
    }
    return res;
}


int  atsRandPlain32s( AtsRandState* state )
{
    assert( state );
    state->seed = ATS_RNG_NEXT( state->seed );
    return state->seed >> 1;
}


float  atsRandPlane32f( AtsRandState* state )
{
    int temp;
    assert( state );
    temp = ((state->seed = ATS_RNG_NEXT( state->seed )) >> 9) | 0x3f800000;
    return *((float*)&temp) - 1.f;
}


void  atsbRand32s( AtsRandState* state, int* vect, int len )
{
    int i;
    unsigned seed;
    int ia, ib, shift;

    seed = state->seed;
    ia = state->ia;
    ib = state->ib;
    shift = state->shift;

    if( !shift )
        for( i = 0; i < len; i++ )
        {
            seed = ATS_RNG_NEXT( seed );
            vect[i] = ((seed >> 3) % ia) + ib;
        }
    else
        for( i = 0; i < len; i++ )
        {
            seed = ATS_RNG_NEXT( seed );
            vect[i] = (seed >> shift) + ib;
        }

    state->seed = seed;
}


void  atsbRand16s( AtsRandState* state, short* vect, int len )
{
    int i;
    unsigned seed;
    int ia, ib, shift;

    seed = state->seed;
    ia = state->ia;
    ib = state->ib;
    shift = state->shift;

    if( !shift )
        for( i = 0; i < len; i++ )
        {
            seed = ATS_RNG_NEXT( seed );
            vect[i] = (short)(((seed >> 3) % ia) + ib);
        }
    else
        for( i = 0; i < len; i++ )
        {
            seed = ATS_RNG_NEXT( seed );
            vect[i] = (short)((seed >> shift) + ib);
        }

    state->seed = seed;
}


void  atsbRand8u( AtsRandState* state, uchar* vect, int len )
{
    int i;
    unsigned seed;
    int ia, ib, shift;

    seed = state->seed;
    ia = state->ia;
    ib = state->ib;
    shift = state->shift;

    if( !shift )
        for( i = 0; i < len; i++ )
        {
            seed = ATS_RNG_NEXT( seed );
            vect[i] = (uchar)(((seed >> 3) % ia) + ib);
        }
    else
        for( i = 0; i < len; i++ )
        {
            seed = ATS_RNG_NEXT( seed );
            vect[i] = (uchar)((seed >> shift) + ib);
        }

    state->seed = seed;
}


void  atsbRand8s( AtsRandState* state, char* vect, int len )
{
    atsbRand8u( state, (uchar*)vect, len );
}

////////////////////////// reading/writing matrices /////////////////////////

float*  atsReadMatrix( const char* filename, int* _m, int* _n )
{
    FILE* f = fopen( filename, "rt" );
    float* data = 0;
    double NaN = sqrt(-1.);

    if( _m ) *_m = 0;
    if( _n ) *_n = 0;

    if( f )
    {
        int m = 0, n = 0, i;
        
        if( fscanf( f, "%d%d", &m, &n ) == 2 && m > 0 && n > 0 )
        {
            data = (float*)malloc( m*n*sizeof(data[0]));
            if( data )
            {
                for( i = 0; i < m*n; i++ )
                {
                    data[i] = (float)NaN;
                    if( fscanf( f, "%g", data + i ) != 1 )
                    {
                        fscanf( f, "%*s" );
                    }
                }
            }

            if( data )
            {
                if( _m ) *_m = m;
                if( _n ) *_n = n;
            }
        }
        fclose(f);
    }
    return data;
}


void  atsWriteMatrix( const char* filename, int m, int n, float* data )
{
    FILE* f = fopen( filename, "wt" );

    if( f )
    {
        int i, j;

        fprintf( f, "%5d%5d\n", m, n );

        for( i = 0; i < m; i++ )
        {
            for( j = 0; j < n; j++ )
            {
                fprintf( f, "%20.7f", data[i*n + j] ); 
            }
            fprintf( f, "\n" );
        }

        fclose(f);
    }
}


/////////////////////// accessing to test data ///////////////////////////////

static char test_data_path[1000];

void atsInitModuleTestData( char* module, char* path_from_module )
{
    int i;
    for( i = strlen( module ); i >= 0 && module[i] != '/' && module[i] != '\\'; i-- );
    strcpy( test_data_path, module );
    strcpy( test_data_path + i + 1, path_from_module );
    strcat( test_data_path, "/" );
    //strcpy( test_data_path, "F:/Users/Neo/opencv/tests/cv/testdata/" );
}

char* atsGetTestDataPath( char* buffer, char* folder, char* filename, char* extention )
{
    assert( buffer );

    strcpy( buffer, test_data_path );
    if( folder )
    {
        strcat( buffer, folder );
        strcat( buffer, "/" );
    }
    if( filename )
    {
        strcat( buffer, filename );
        if( extention )
        {
            if( *extention != '.' )
                strcat( buffer, "." );
            strcat( buffer, extention );
        }
    }
    return buffer;
}

#define ARG_STR "-lib"

void atsLoadPrimitives( int flag )
{
    cvUseOptimized( flag );
}



