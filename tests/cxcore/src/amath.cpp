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

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////// tests for matrix operations and math functions ///////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#include "cxcoretest.h"
#include <float.h>
#include <math.h>

/// !!! NOTE !!! These tests happily avoid overflow cases & out-of-range arguments
/// so that output arrays contain neigher Inf's nor Nan's.
/// Handling such cases would require special modification of check function
/// (validate_test_results) => TBD.
/// Also, need some logarithmic-scale generation of input data. Right now it is done (in some tests)
/// by generating min/max boundaries for random data in logarimithic scale, but
/// within the same test case all the input array elements are of the same order.

class CxCore_MathTest : public CvArrTest
{
public:
    CxCore_MathTest( const char* test_name, const char* test_funcs );
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    double get_success_error_level( int /*test_case_idx*/, int i, int j );
};


CxCore_MathTest::CxCore_MathTest( const char* test_name, const char* test_funcs )
    : CvArrTest( test_name, test_funcs, "" )
{
    test_array[INPUT].push(NULL);
    test_array[OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
}


double CxCore_MathTest::get_success_error_level( int /*test_case_idx*/, int i, int j )
{
    return CV_MAT_DEPTH(test_mat[i][j].type) == CV_32F ? FLT_EPSILON*128 : DBL_EPSILON*1024;
}


void CxCore_MathTest::get_test_array_types_and_sizes( int test_case_idx,
                                                      CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int depth = test_case_idx*2/test_case_count ? CV_64F : CV_32F;
    int cn = cvTsRandInt(rng) % 4 + 1, type = CV_MAKETYPE(depth, cn);
    int i, j;
    CvArrTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    for( i = 0; i < max_arr; i++ )
    {
        int count = test_array[i].size();
        for( j = 0; j < count; j++ )
            types[i][j] = type;
    }
}


////////// exp /////////////
class CxCore_ExpTest : public CxCore_MathTest
{
public:
    CxCore_ExpTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int i, int j, int type, CvScalar* low, CvScalar* high );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int out_type;
};


CxCore_ExpTest::CxCore_ExpTest()
    : CxCore_MathTest( "exp", "cvExp" )
{
    out_type = 0;
}


void CxCore_ExpTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CxCore_MathTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    out_type = types[OUTPUT][0];
    if( CV_MAT_DEPTH(types[INPUT][0]) == CV_32F && (cvRandInt(ts->get_rng()) & 3) == 0 )
        types[OUTPUT][0] = types[REF_OUTPUT][0] =
            out_type = (types[INPUT][0] & ~CV_MAT_DEPTH_MASK)|CV_64F;
}

void CxCore_ExpTest::get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high )
{
    double l = cvTsRandReal(ts->get_rng())*10+1;
    double u = cvTsRandReal(ts->get_rng())*10+1;
    l *= -l;
    u *= u;
    *low = cvScalarAll(l);
    *high = cvScalarAll(CV_MAT_DEPTH(out_type)==CV_64F? u : u*0.5);
}


void CxCore_ExpTest::run_func()
{
    cvExp( test_array[INPUT][0], test_array[OUTPUT][0] );
}


void CxCore_ExpTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* a = &test_mat[INPUT][0];
    CvMat* b = &test_mat[REF_OUTPUT][0];

    int a_depth = CV_MAT_DEPTH(a->type);
    int b_depth = CV_MAT_DEPTH(b->type);
    int ncols = test_mat[INPUT][0].cols*CV_MAT_CN(a->type);
    int i, j;

    for( i = 0; i < a->rows; i++ )
    {
        uchar* a_data = a->data.ptr + i*a->step;
        uchar* b_data = b->data.ptr + i*b->step;

        if( a_depth == CV_32F && b_depth == CV_32F )
        {
            for( j = 0; j < ncols; j++ )
                ((float*)b_data)[j] = (float)exp((double)((float*)a_data)[j]);
        }
        else if( a_depth == CV_32F && b_depth == CV_64F )
        {
            for( j = 0; j < ncols; j++ )
                ((double*)b_data)[j] = exp((double)((float*)a_data)[j]);
        }
        else
        {
            assert( a_depth == CV_64F && b_depth == CV_64F );
            for( j = 0; j < ncols; j++ )
                ((double*)b_data)[j] = exp(((double*)a_data)[j]);
        }
    }
}

CxCore_ExpTest exp_test;


////////// log /////////////
class CxCore_LogTest : public CxCore_MathTest
{
public:
    CxCore_LogTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int i, int j, int type, CvScalar* low, CvScalar* high );
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_LogTest::CxCore_LogTest()
    : CxCore_MathTest( "log", "cvLog" )
{
}


void CxCore_LogTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CxCore_MathTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    if( CV_MAT_DEPTH(types[INPUT][0]) == CV_32F && (cvRandInt(ts->get_rng()) & 3) == 0 )
        types[INPUT][0] = (types[INPUT][0] & ~CV_MAT_DEPTH_MASK)|CV_64F;
}


void CxCore_LogTest::get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high )
{
    double l = cvTsRandReal(ts->get_rng())*15-5;
    double u = cvTsRandReal(ts->get_rng())*15-5;
    double t;
    l = exp(l);
    u = exp(u);
    if( l > u )
        CV_SWAP( l, u, t );
    *low = cvScalarAll(l);
    *high = cvScalarAll(u);
}


void CxCore_LogTest::run_func()
{
    cvLog( test_array[INPUT][0], test_array[OUTPUT][0] );
}


void CxCore_LogTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* a = &test_mat[INPUT][0];
    CvMat* b = &test_mat[REF_OUTPUT][0];

    int a_depth = CV_MAT_DEPTH(a->type);
    int b_depth = CV_MAT_DEPTH(b->type);
    int ncols = test_mat[INPUT][0].cols*CV_MAT_CN(a->type);
    int i, j;

    for( i = 0; i < a->rows; i++ )
    {
        uchar* a_data = a->data.ptr + i*a->step;
        uchar* b_data = b->data.ptr + i*b->step;

        if( a_depth == CV_32F && b_depth == CV_32F )
        {
            for( j = 0; j < ncols; j++ )
                ((float*)b_data)[j] = (float)log((double)((float*)a_data)[j]);
        }
        else if( a_depth == CV_64F && b_depth == CV_32F )
        {
            for( j = 0; j < ncols; j++ )
                ((float*)b_data)[j] = (float)log(((double*)a_data)[j]);
        }
        else
        {
            assert( a_depth == CV_64F && b_depth == CV_64F );
            for( j = 0; j < ncols; j++ )
                ((double*)b_data)[j] = log(((double*)a_data)[j]);
        }
    }
}

CxCore_LogTest log_test;


////////// pow /////////////
class CxCore_PowTest : public CxCore_MathTest
{
public:
    CxCore_PowTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int i, int j, int type, CvScalar* low, CvScalar* high );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    double get_success_error_level( int test_case_idx, int i, int j );
    double power;
};


CxCore_PowTest::CxCore_PowTest()
    : CxCore_MathTest( "pow", "cvPow" )
{
    power = 0;
}


void CxCore_PowTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int depth = test_case_idx*5/test_case_count;
    int cn = cvTsRandInt(rng) % 4 + 1;
    int i, j;
    CvArrTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    depth += depth == CV_8S;

    if( depth < CV_32F || cvTsRandInt(rng)%8 == 0 )
        // integer power
        power = (int)(cvTsRandInt(rng)%21 - 10);
    else
    {
        i = cvTsRandInt(rng)%16;
        power = i == 15 ? 0.5 : i == 14 ? -0.5 : cvTsRandReal(rng)*10 - 5;
    }

    for( i = 0; i < max_arr; i++ )
    {
        int count = test_array[i].size();
        int type = CV_MAKETYPE(depth, cn);
        for( j = 0; j < count; j++ )
            types[i][j] = type;
    }
}


double CxCore_PowTest::get_success_error_level( int test_case_idx, int i, int j )
{
    int type = cvGetElemType( test_array[i][j] );
    if( CV_MAT_DEPTH(type) < CV_32F )
        return power == cvRound(power) && power >= 0 ? 0 : 1;
    else
        return CxCore_MathTest::get_success_error_level( test_case_idx, i, j );
}


void CxCore_PowTest::get_minmax_bounds( int /*i*/, int /*j*/, int type, CvScalar* low, CvScalar* high )
{
    double l, u = cvTsRandInt(ts->get_rng())%1000 + 1;
    if( power > 0 )
    {
        double mval = cvTsMaxVal(type);
        double u1 = pow(mval,1./power)*2;
        u = MIN(u,u1);
    }

    l = power == cvRound(power) ? -u : FLT_EPSILON;
    *low = cvScalarAll(l);
    *high = cvScalarAll(u);
}


void CxCore_PowTest::run_func()
{
    cvPow( test_array[INPUT][0], test_array[OUTPUT][0], power );
}


inline static int ipow( int a, int power )
{
    int b = 1;
    while( power > 0 )
    {
        if( power&1 )
            b *= a, power--;
        else
            a *= a, power >>= 1;
    }
    return b;
}


inline static double ipow( double a, int power )
{
    double b = 1.;
    while( power > 0 )
    {
        if( power&1 )
            b *= a, power--;
        else
            a *= a, power >>= 1;
    }
    return b;
}


void CxCore_PowTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* a = &test_mat[INPUT][0];
    CvMat* b = &test_mat[REF_OUTPUT][0];

    int depth = CV_MAT_DEPTH(a->type);
    int ncols = test_mat[INPUT][0].cols*CV_MAT_CN(a->type);
    int ipower = cvRound(power), apower = abs(ipower);
    int i, j;

    for( i = 0; i < a->rows; i++ )
    {
        uchar* a_data = a->data.ptr + i*a->step;
        uchar* b_data = b->data.ptr + i*b->step;

        switch( depth )
        {
        case CV_8U:
            if( ipower < 0 )
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((uchar*)a_data)[j];
                    ((uchar*)b_data)[j] = (uchar)(val <= 1 ? val :
                                        val == 2 && ipower == -1 ? 1 : 0);
                }
            else
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((uchar*)a_data)[j];
                    val = ipow( val, ipower );
                    ((uchar*)b_data)[j] = CV_CAST_8U(val);
                }
            break;
        case CV_8S:
            if( ipower < 0 )
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((char*)a_data)[j];
                    ((char*)b_data)[j] = (char)((val&~1)==0 ? val :
                                          val ==-1 ? 1-2*(ipower&1) :
                                          val == 2 && ipower == -1 ? 1 : 0);
                }
            else
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((char*)a_data)[j];
                    val = ipow( val, ipower );
                    ((char*)b_data)[j] = CV_CAST_8S(val);
                }
            break;
        case CV_16U:
            if( ipower < 0 )
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((ushort*)a_data)[j];
                    ((ushort*)b_data)[j] = (ushort)((val&~1)==0 ? val :
                                          val ==-1 ? 1-2*(ipower&1) :
                                          val == 2 && ipower == -1 ? 1 : 0);
                }
            else
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((ushort*)a_data)[j];
                    val = ipow( val, ipower );
                    ((ushort*)b_data)[j] = CV_CAST_16U(val);
                }
            break;
        case CV_16S:
            if( ipower < 0 )
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((short*)a_data)[j];
                    ((short*)b_data)[j] = (short)((val&~1)==0 ? val :
                                          val ==-1 ? 1-2*(ipower&1) :
                                          val == 2 && ipower == -1 ? 1 : 0);
                }
            else
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((short*)a_data)[j];
                    val = ipow( val, ipower );
                    ((short*)b_data)[j] = CV_CAST_16S(val);
                }
            break;
        case CV_32S:
            if( ipower < 0 )
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((int*)a_data)[j];
                    ((int*)b_data)[j] = (val&~1)==0 ? val :
                                        val ==-1 ? 1-2*(ipower&1) :
                                        val == 2 && ipower == -1 ? 1 : 0;
                }
            else
                for( j = 0; j < ncols; j++ )
                {
                    int val = ((int*)a_data)[j];
                    val = ipow( val, ipower );
                    ((int*)b_data)[j] = val;
                }
            break;
        case CV_32F:
            if( power != ipower )
                for( j = 0; j < ncols; j++ )
                {
                    double val = ((float*)a_data)[j];
                    val = pow( fabs(val), power );
                    ((float*)b_data)[j] = CV_CAST_32F(val);
                }
            else
                for( j = 0; j < ncols; j++ )
                {
                    double val = ((float*)a_data)[j];
                    if( ipower < 0 )
                        val = 1./val;
                    val = ipow( val, apower );
                    ((float*)b_data)[j] = (float)val;
                }
            break;
        case CV_64F:
            if( power != ipower )
                for( j = 0; j < ncols; j++ )
                {
                    double val = ((double*)a_data)[j];
                    val = pow( fabs(val), power );
                    ((double*)b_data)[j] = CV_CAST_64F(val);
                }
            else
                for( j = 0; j < ncols; j++ )
                {
                    double val = ((double*)a_data)[j];
                    if( ipower < 0 )
                        val = 1./val;
                    val = ipow( val, apower );
                    ((double*)b_data)[j] = (double)val;
                }
            break;
        }
    }
}

CxCore_PowTest pow_test;



////////// cart2polar /////////////
class CxCore_CartToPolarTest : public CxCore_MathTest
{
public:
    CxCore_CartToPolarTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int use_degrees;
};


CxCore_CartToPolarTest::CxCore_CartToPolarTest()
    : CxCore_MathTest( "cart2polar", "cvCartToPolar" )
{
    use_degrees = 0;
    test_array[INPUT].push(NULL);
    test_array[OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
}


void CxCore_CartToPolarTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    CxCore_MathTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    use_degrees = cvTsRandInt(rng) & 1;
    if( cvTsRandInt(rng) % 4 == 0 ) // check missing magnitude/angle cases
    {
        int idx = cvTsRandInt(rng) & 1;
        sizes[OUTPUT][idx] = sizes[REF_OUTPUT][idx] = cvSize(0,0);
    }
}


void CxCore_CartToPolarTest::run_func()
{
    cvCartToPolar( test_array[INPUT][0], test_array[INPUT][1],
                   test_array[OUTPUT][0], test_array[OUTPUT][1], use_degrees );
}


double CxCore_CartToPolarTest::get_success_error_level( int test_case_idx, int i, int j )
{
    return j == 1 ? 0.5*(use_degrees ? 1 : CV_PI/180.) :
        CxCore_MathTest::get_success_error_level( test_case_idx, i, j );
}


void CxCore_CartToPolarTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* x = &test_mat[INPUT][0];
    CvMat* y = &test_mat[INPUT][1];
    CvMat* mag = test_array[REF_OUTPUT][0] ? &test_mat[REF_OUTPUT][0] : 0;
    CvMat* angle = test_array[REF_OUTPUT][1] ? &test_mat[REF_OUTPUT][1] : 0;
    double C = use_degrees ? 180./CV_PI : 1.;

    int depth = CV_MAT_DEPTH(x->type);
    int ncols = x->cols*CV_MAT_CN(x->type);
    int i, j;

    for( i = 0; i < x->rows; i++ )
    {
        uchar* x_data = x->data.ptr + i*x->step;
        uchar* y_data = y->data.ptr + i*y->step;
        uchar* mag_data = mag ? mag->data.ptr + i*mag->step : 0;
        uchar* angle_data = angle ? angle->data.ptr + i*angle->step : 0;

        if( depth == CV_32F )
        {
            for( j = 0; j < ncols; j++ )
            {
                double xval = ((float*)x_data)[j];
                double yval = ((float*)y_data)[j];

                if( mag_data )
                    ((float*)mag_data)[j] = (float)sqrt(xval*xval + yval*yval);
                if( angle_data )
                {
                    double a = atan2( yval, xval );
                    if( a < 0 )
                        a += CV_PI*2;
                    a *= C;
                    ((float*)angle_data)[j] = (float)a;
                }
            }
        }
        else
        {
            assert( depth == CV_64F );
            for( j = 0; j < ncols; j++ )
            {
                double xval = ((double*)x_data)[j];
                double yval = ((double*)y_data)[j];

                if( mag_data )
                    ((double*)mag_data)[j] = sqrt(xval*xval + yval*yval);
                if( angle_data )
                {
                    double a = atan2( yval, xval );
                    if( a < 0 )
                        a += CV_PI*2;
                    a *= C;
                    ((double*)angle_data)[j] = a;
                }
            }
        }
    }

    if( angle )
    {
        // hack: increase angle value by 1 (so that alpha becomes 1+alpha)
        // to hide large relative errors in case of very small angles
        cvTsAdd( &test_mat[OUTPUT][1], cvScalarAll(1.), 0, cvScalarAll(0.),
                 cvScalarAll(1.), &test_mat[OUTPUT][1], 0 );
        cvTsAdd( &test_mat[REF_OUTPUT][1], cvScalarAll(1.), 0, cvScalarAll(0.),
                 cvScalarAll(1.), &test_mat[REF_OUTPUT][1], 0 );
    }
}

CxCore_CartToPolarTest cart2polar_test;



////////// polar2cart /////////////
class CxCore_PolarToCartTest : public CxCore_MathTest
{
public:
    CxCore_PolarToCartTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int use_degrees;
};


CxCore_PolarToCartTest::CxCore_PolarToCartTest()
    : CxCore_MathTest( "polar2cart", "cvPolarToCart" )
{
    use_degrees = 0;
    test_array[INPUT].push(NULL);
    test_array[OUTPUT].push(NULL);
    test_array[REF_OUTPUT].push(NULL);
}


void CxCore_PolarToCartTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    CxCore_MathTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    use_degrees = cvTsRandInt(rng) & 1;
    if( cvTsRandInt(rng) % 4 == 0 ) // check missing magnitude case
        sizes[INPUT][1] = cvSize(0,0);

    if( cvTsRandInt(rng) % 4 == 0 ) // check missing x/y cases
    {
        int idx = cvTsRandInt(rng) & 1;
        sizes[OUTPUT][idx] = sizes[REF_OUTPUT][idx] = cvSize(0,0);
    }
}


void CxCore_PolarToCartTest::run_func()
{
    cvPolarToCart( test_array[INPUT][1], test_array[INPUT][0],
                   test_array[OUTPUT][0], test_array[OUTPUT][1], use_degrees );
}


double CxCore_PolarToCartTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return FLT_EPSILON*100;
}


void CxCore_PolarToCartTest::prepare_to_validation( int /*test_case_idx*/ )
{
    CvMat* x = test_array[REF_OUTPUT][0] ? &test_mat[REF_OUTPUT][0] : 0;
    CvMat* y = test_array[REF_OUTPUT][1] ? &test_mat[REF_OUTPUT][1] : 0;
    CvMat* angle = &test_mat[INPUT][0];
    CvMat* mag = test_array[INPUT][1] ? &test_mat[INPUT][1] : 0;
    double C = use_degrees ? CV_PI/180. : 1.;

    int depth = CV_MAT_DEPTH(angle->type);
    int ncols = angle->cols*CV_MAT_CN(angle->type);
    int i, j;

    for( i = 0; i < angle->rows; i++ )
    {
        uchar* x_data = x ? x->data.ptr + i*x->step : 0;
        uchar* y_data = y ? y->data.ptr + i*y->step : 0;
        uchar* mag_data = mag ? mag->data.ptr + i*mag->step : 0;
        uchar* angle_data = angle->data.ptr + i*angle->step;

        if( depth == CV_32F )
        {
            for( j = 0; j < ncols; j++ )
            {
                double a = ((float*)angle_data)[j]*C;
                double m = mag_data ? ((float*)mag_data)[j] : 1.;

                if( x_data )
                    ((float*)x_data)[j] = (float)(m*cos(a));
                if( y_data )
                    ((float*)y_data)[j] = (float)(m*sin(a));
            }
        }
        else
        {
            assert( depth == CV_64F );
            for( j = 0; j < ncols; j++ )
            {
                double a = ((double*)angle_data)[j]*C;
                double m = mag_data ? ((double*)mag_data)[j] : 1.;

                if( x_data )
                    ((double*)x_data)[j] = m*cos(a);
                if( y_data )
                    ((double*)y_data)[j] = m*sin(a);
            }
        }
    }
}

CxCore_PolarToCartTest polar2cart_test;

///////////////////////////////////////// matrix tests ////////////////////////////////////////////

class CxCore_MatrixTest : public CvArrTest
{
public:
    CxCore_MatrixTest( const char* test_name, const char* test_funcs, int in_count, int out_count,
                       bool allow_int, bool scalar_output, int max_cn );
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    bool allow_int;
    bool scalar_output;
    int max_cn;
};


CxCore_MatrixTest::CxCore_MatrixTest( const char* test_name, const char* test_funcs,
                                      int in_count, int out_count,
                                      bool _allow_int, bool _scalar_output, int _max_cn )
    : CvArrTest( test_name, test_funcs, "" ),
    allow_int(_allow_int), scalar_output(_scalar_output), max_cn(_max_cn)
{
    int i;
    for( i = 0; i < in_count; i++ )
        test_array[INPUT].push(NULL);

    for( i = 0; i < out_count; i++ )
    {
        test_array[OUTPUT].push(NULL);
        test_array[REF_OUTPUT].push(NULL);
    }

    element_wise_relative_error = false;
}


void CxCore_MatrixTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int depth = test_case_idx*(allow_int ? CV_64F : 2)/test_case_count;
    int cn = cvTsRandInt(rng) % max_cn + 1;
    int i, j;

    if( allow_int )
        depth += depth == CV_8S;
    else
        depth += CV_32F;

    CvArrTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    for( i = 0; i < max_arr; i++ )
    {
        int count = test_array[i].size();
        int flag = (i == OUTPUT || i == REF_OUTPUT) && scalar_output;
        int type = !flag ? CV_MAKETYPE(depth, cn) : CV_64FC1;

        for( j = 0; j < count; j++ )
        {
            types[i][j] = type;
            if( flag )
                sizes[i][j] = cvSize( 4, 1 );
        }
    }
}


double CxCore_MatrixTest::get_success_error_level( int test_case_idx, int i, int j )
{
    int input_depth = CV_MAT_DEPTH(cvGetElemType( test_array[INPUT][0] ));
    double input_precision = input_depth < CV_32F ? 0 : input_depth == CV_32F ?
                            FLT_EPSILON*128 : DBL_EPSILON*1024;
    double output_precision = CvArrTest::get_success_error_level( test_case_idx, i, j );
    return MAX(input_precision, output_precision);
}

///////// dotproduct //////////

class CxCore_DotProductTest : public CxCore_MatrixTest
{
public:
    CxCore_DotProductTest();
protected:
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_DotProductTest::CxCore_DotProductTest() :
    CxCore_MatrixTest( "dotproduct", "cvDotProduct", 2, 1, true, true, 4 )
{
}


void CxCore_DotProductTest::run_func()
{
    *((CvScalar*)(test_mat[OUTPUT][0].data.ptr)) =
        cvRealScalar(cvDotProduct( test_array[INPUT][0], test_array[INPUT][1] ));
}


void CxCore_DotProductTest::prepare_to_validation( int )
{
    *((CvScalar*)(test_mat[REF_OUTPUT][0].data.ptr)) =
        cvRealScalar(cvTsCrossCorr( &test_mat[INPUT][0], &test_mat[INPUT][1] ));
}

CxCore_DotProductTest dotproduct_test;


///////// crossproduct //////////

class CxCore_CrossProductTest : public CxCore_MatrixTest
{
public:
    CxCore_CrossProductTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_CrossProductTest::CxCore_CrossProductTest() :
    CxCore_MatrixTest( "crossproduct", "cvCrossProduct", 2, 1, false, false, 1 )
{
}


void CxCore_CrossProductTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int depth = test_case_idx*2/test_case_count ? CV_64F : CV_32F;
    int cn = cvTsRandInt(rng) & 1 ? 3 : 1, type = CV_MAKETYPE(depth, cn);
    CvSize sz;

    types[INPUT][0] = types[INPUT][1] = types[OUTPUT][0] = types[REF_OUTPUT][0] = type;

    if( cn == 3 )
        sz = cvSize(1,1);
    else if( cvTsRandInt(rng) & 1 )
        sz = cvSize(3,1);
    else
        sz = cvSize(1,3);

    sizes[INPUT][0] = sizes[INPUT][1] = sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = sz;
}


void CxCore_CrossProductTest::run_func()
{
    cvCrossProduct( test_array[INPUT][0], test_array[INPUT][1], test_array[OUTPUT][0] );
}


void CxCore_CrossProductTest::prepare_to_validation( int )
{
    CvScalar a = {0,0,0,0}, b = {0,0,0,0}, c = {0,0,0,0};

    if( test_mat[INPUT][0].rows > 1 )
    {
        a.val[0] = cvGetReal2D( &test_mat[INPUT][0], 0, 0 );
        a.val[1] = cvGetReal2D( &test_mat[INPUT][0], 1, 0 );
        a.val[2] = cvGetReal2D( &test_mat[INPUT][0], 2, 0 );

        b.val[0] = cvGetReal2D( &test_mat[INPUT][1], 0, 0 );
        b.val[1] = cvGetReal2D( &test_mat[INPUT][1], 1, 0 );
        b.val[2] = cvGetReal2D( &test_mat[INPUT][1], 2, 0 );
    }
    else if( test_mat[INPUT][0].cols > 1 )
    {
        a.val[0] = cvGetReal1D( &test_mat[INPUT][0], 0 );
        a.val[1] = cvGetReal1D( &test_mat[INPUT][0], 1 );
        a.val[2] = cvGetReal1D( &test_mat[INPUT][0], 2 );

        b.val[0] = cvGetReal1D( &test_mat[INPUT][1], 0 );
        b.val[1] = cvGetReal1D( &test_mat[INPUT][1], 1 );
        b.val[2] = cvGetReal1D( &test_mat[INPUT][1], 2 );
    }
    else
    {
        a = cvGet1D( &test_mat[INPUT][0], 0 );
        b = cvGet1D( &test_mat[INPUT][1], 0 );
    }

    c.val[2] = a.val[0]*b.val[1] - a.val[1]*b.val[0];
    c.val[1] = -a.val[0]*b.val[2] + a.val[2]*b.val[0];
    c.val[0] = a.val[1]*b.val[2] - a.val[2]*b.val[1];

    if( test_mat[REF_OUTPUT][0].rows > 1 )
    {
        cvSetReal2D( &test_mat[REF_OUTPUT][0], 0, 0, c.val[0] );
        cvSetReal2D( &test_mat[REF_OUTPUT][0], 1, 0, c.val[1] );
        cvSetReal2D( &test_mat[REF_OUTPUT][0], 2, 0, c.val[2] );
    }
    else if( test_mat[REF_OUTPUT][0].cols > 1 )
    {
        cvSetReal1D( &test_mat[REF_OUTPUT][0], 0, c.val[0] );
        cvSetReal1D( &test_mat[REF_OUTPUT][0], 1, c.val[1] );
        cvSetReal1D( &test_mat[REF_OUTPUT][0], 2, c.val[2] );
    }
    else
    {
        cvSet1D( &test_mat[REF_OUTPUT][0], 0, c );
    }
}

CxCore_CrossProductTest crossproduct_test;


///////////////// scaleadd /////////////////////

class CxCore_ScaleAddTest : public CxCore_MatrixTest
{
public:
    CxCore_ScaleAddTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void run_func();
    void prepare_to_validation( int test_case_idx );
};

CxCore_ScaleAddTest::CxCore_ScaleAddTest() :
    CxCore_MatrixTest( "scaleadd", "cvScaleAdd", 3, 1, false, false, 2 )
{
}


void CxCore_ScaleAddTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    sizes[INPUT][2] = cvSize(1,1);
}


void CxCore_ScaleAddTest::run_func()
{
    CvScalar s = cvGet1D( &test_mat[INPUT][2], 0 );
    cvScaleAdd( test_array[INPUT][0], s, test_array[INPUT][1], test_array[OUTPUT][0] );
}


void CxCore_ScaleAddTest::prepare_to_validation( int )
{
    CvScalar s = cvGet1D( &test_mat[INPUT][2], 0 );
    int rows = test_mat[INPUT][0].rows;
    int type = CV_MAT_TYPE(test_mat[INPUT][0].type);
    int cn = CV_MAT_CN(type);
    int ncols = test_mat[INPUT][0].cols*cn;
    int i, j;

    for( i = 0; i < rows; i++ )
    {
        uchar* src1 = test_mat[INPUT][0].data.ptr + test_mat[INPUT][0].step*i;
        uchar* src2 = test_mat[INPUT][1].data.ptr + test_mat[INPUT][1].step*i;
        uchar* dst = test_mat[REF_OUTPUT][0].data.ptr + test_mat[REF_OUTPUT][0].step*i;

        switch( type )
        {
        case CV_32FC1:
            for( j = 0; j < ncols; j++ )
                ((float*)dst)[j] = (float)(((float*)src1)[j]*s.val[0] + ((float*)src2)[j]);
            break;
        case CV_32FC2:
            for( j = 0; j < ncols; j += 2 )
            {
                double re = ((float*)src1)[j];
                double im = ((float*)src1)[j+1];
                ((float*)dst)[j] = (float)(re*s.val[0] - im*s.val[1] + ((float*)src2)[j]);
                ((float*)dst)[j+1] = (float)(re*s.val[1] + im*s.val[0] + ((float*)src2)[j+1]);
            }
            break;
        case CV_64FC1:
            for( j = 0; j < ncols; j++ )
                ((double*)dst)[j] = ((double*)src1)[j]*s.val[0] + ((double*)src2)[j];
            break;
        case CV_64FC2:
            for( j = 0; j < ncols; j += 2 )
            {
                double re = ((double*)src1)[j];
                double im = ((double*)src1)[j+1];
                ((double*)dst)[j] = (double)(re*s.val[0] - im*s.val[1] + ((double*)src2)[j]);
                ((double*)dst)[j+1] = (double)(re*s.val[1] + im*s.val[0] + ((double*)src2)[j+1]);
            }
            break;
        default:
            assert(0);
        }
    }
}

CxCore_ScaleAddTest scaleadd_test;


///////////////// gemm /////////////////////

class CxCore_GEMMTest : public CxCore_MatrixTest
{
public:
    CxCore_GEMMTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int tabc_flag;
};

CxCore_GEMMTest::CxCore_GEMMTest() :
    CxCore_MatrixTest( "gemm", "cvGEMM", 5, 1, false, false, 2 )
{
    test_case_count = 100;
}


void CxCore_GEMMTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    CvSize sizeA;
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    sizeA = sizes[INPUT][0];
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    sizes[INPUT][0] = sizeA;
    sizes[INPUT][2] = sizes[INPUT][3] = cvSize(1,1);
    types[INPUT][2] = types[INPUT][3] &= ~CV_MAT_CN_MASK;

    tabc_flag = cvTsRandInt(rng) & 7;

    switch( tabc_flag & (CV_GEMM_A_T|CV_GEMM_B_T) )
    {
    case 0:
        sizes[INPUT][1].height = sizes[INPUT][0].width;
        sizes[OUTPUT][0].height = sizes[INPUT][0].height;
        sizes[OUTPUT][0].width = sizes[INPUT][1].width;
        break;
    case CV_GEMM_B_T:
        sizes[INPUT][1].width = sizes[INPUT][0].width;
        sizes[OUTPUT][0].height = sizes[INPUT][0].height;
        sizes[OUTPUT][0].width = sizes[INPUT][1].height;
        break;
    case CV_GEMM_A_T:
        sizes[INPUT][1].height = sizes[INPUT][0].height;
        sizes[OUTPUT][0].height = sizes[INPUT][0].width;
        sizes[OUTPUT][0].width = sizes[INPUT][1].width;
        break;
    case CV_GEMM_A_T | CV_GEMM_B_T:
        sizes[INPUT][1].width = sizes[INPUT][0].height;
        sizes[OUTPUT][0].height = sizes[INPUT][0].width;
        sizes[OUTPUT][0].width = sizes[INPUT][1].height;
        break;
    }

    sizes[REF_OUTPUT][0] = sizes[OUTPUT][0];

    if( cvTsRandInt(rng) & 1 )
        sizes[INPUT][4] = cvSize(0,0);
    else if( !(tabc_flag & CV_GEMM_C_T) )
        sizes[INPUT][4] = sizes[OUTPUT][0];
    else
    {
        sizes[INPUT][4].width = sizes[OUTPUT][0].height;
        sizes[INPUT][4].height = sizes[OUTPUT][0].width;
    }
}


void CxCore_GEMMTest::get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high )
{
    *low = cvScalarAll(-10.);
    *high = cvScalarAll(10.);
}


void CxCore_GEMMTest::run_func()
{
    double alpha = cvmGet( &test_mat[INPUT][2], 0, 0 );
    double beta = cvmGet( &test_mat[INPUT][3], 0, 0 );
    cvGEMM( test_array[INPUT][0], test_array[INPUT][1], alpha,
            test_array[INPUT][4], beta, test_array[OUTPUT][0], tabc_flag );
}


void CxCore_GEMMTest::prepare_to_validation( int )
{
    double alpha = cvmGet( &test_mat[INPUT][2], 0, 0 );
    double beta = cvmGet( &test_mat[INPUT][3], 0, 0 );
    cvTsGEMM( &test_mat[INPUT][0], &test_mat[INPUT][1], alpha,
        test_array[INPUT][4] ? &test_mat[INPUT][4] : 0,
        beta, &test_mat[REF_OUTPUT][0], tabc_flag );
}

CxCore_GEMMTest gemm_test;


///////////////// multransposed /////////////////////

class CxCore_MulTransposedTest : public CxCore_MatrixTest
{
public:
    CxCore_MulTransposedTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int order;
};


CxCore_MulTransposedTest::CxCore_MulTransposedTest() :
    CxCore_MatrixTest( "multransposed", "cvMulTransposed, cvRepeat", 2, 1, false, false, 1 )
{
    test_case_count = 100;
    order = 0;
    test_array[TEMP].push(NULL);
}


void CxCore_MulTransposedTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int bits = cvTsRandInt(rng);
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    if( bits & 1 )
        sizes[INPUT][1] = sizes[TEMP][0] = cvSize(0,0);
    else
    {
        sizes[INPUT][1] = sizes[TEMP][0] = sizes[INPUT][0];
        if( bits & 2 )
            sizes[INPUT][1].height = 1;
    }

    types[TEMP][0] = types[INPUT][0];

    order = (bits & 4) != 0;
    sizes[OUTPUT][0].width = sizes[OUTPUT][0].height = order == 0 ?
        sizes[INPUT][0].height : sizes[INPUT][0].width;
    sizes[REF_OUTPUT][0] = sizes[OUTPUT][0];
}


void CxCore_MulTransposedTest::get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high )
{
    *low = cvScalarAll(-10.);
    *high = cvScalarAll(10.);
}


void CxCore_MulTransposedTest::run_func()
{
    cvMulTransposed( test_array[INPUT][0], test_array[OUTPUT][0],
                     order, test_array[INPUT][1] );
}


void CxCore_MulTransposedTest::prepare_to_validation( int )
{
    CvMat* delta = test_array[INPUT][1] ? &test_mat[INPUT][1] : 0;
    if( delta )
    {
        if( test_mat[INPUT][1].rows < test_mat[INPUT][0].rows )
        {
            cvRepeat( delta, &test_mat[TEMP][0] );
            delta = &test_mat[TEMP][0];
        }
        cvTsAdd( &test_mat[INPUT][0], cvScalarAll(1.), delta, cvScalarAll(-1.),
                 cvScalarAll(0.), &test_mat[TEMP][0], 0 );
        delta = &test_mat[TEMP][0];
    }
    else
        delta = &test_mat[INPUT][0];

    cvTsGEMM( delta, delta, 1., 0, 0, &test_mat[REF_OUTPUT][0], order == 0 ? CV_GEMM_B_T : CV_GEMM_A_T );
}

CxCore_MulTransposedTest multransposed_test;


///////////////// Transform /////////////////////

class CxCore_TransformTest : public CxCore_MatrixTest
{
public:
    CxCore_TransformTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_TransformTest::CxCore_TransformTest() :
    CxCore_MatrixTest( "transform", "cvTransform", 3, 1, true, false, 4 )
{
}


void CxCore_TransformTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int bits = cvTsRandInt(rng);
    int depth, cn, dst_cn, mat_cols, mattype;
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    mat_cols = cn = CV_MAT_CN(types[INPUT][0]);
    depth = CV_MAT_DEPTH(types[INPUT][0]);
    dst_cn = cvTsRandInt(rng) % 4 + 1;
    types[OUTPUT][0] = types[REF_OUTPUT][0] = CV_MAKETYPE(depth, dst_cn);

    mattype = depth < CV_32S ? CV_32F : depth == CV_64F ? CV_64F : bits & 1 ? CV_32F : CV_64F;
    types[INPUT][1] = mattype;
    types[INPUT][2] = CV_MAKETYPE(mattype, dst_cn);

    if( bits & 2 )
    {
        sizes[INPUT][2] = cvSize(0,0);
        mat_cols += (bits & 4) != 0;
    }
    else if( bits & 4 )
        sizes[INPUT][2] = cvSize(1,1);
    else
    {
        if( bits & 8 )
            sizes[INPUT][2] = cvSize(dst_cn,1);
        else
            sizes[INPUT][2] = cvSize(1,dst_cn);
        types[INPUT][2] &= ~CV_MAT_CN_MASK;
    }

    sizes[INPUT][1] = cvSize(mat_cols,dst_cn);
}


double CxCore_TransformTest::get_success_error_level( int test_case_idx, int i, int j )
{
    return CV_MAT_DEPTH(test_mat[INPUT][0].type) < CV_32F ? 1 :
        CxCore_MatrixTest::get_success_error_level( test_case_idx, i, j );
}

void CxCore_TransformTest::run_func()
{
    cvTransform( test_array[INPUT][0], test_array[OUTPUT][0], &test_mat[INPUT][1],
                 test_array[INPUT][2] ? &test_mat[INPUT][2] : 0);
}


void CxCore_TransformTest::prepare_to_validation( int )
{
    CvMat* transmat = &test_mat[INPUT][1];
    CvMat* shift = test_array[INPUT][2] ? &test_mat[INPUT][2] : 0;

    cvTsTransform( &test_mat[INPUT][0], &test_mat[REF_OUTPUT][0], transmat, shift );
}

CxCore_TransformTest transform_test;


///////////////// PerspectiveTransform /////////////////////

class CxCore_PerspectiveTransformTest : public CxCore_MatrixTest
{
public:
    CxCore_PerspectiveTransformTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_PerspectiveTransformTest::CxCore_PerspectiveTransformTest() :
    CxCore_MatrixTest( "perspective", "cvPerspectiveTransform", 2, 1, false, false, 2 )
{
}


void CxCore_PerspectiveTransformTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int bits = cvTsRandInt(rng);
    int depth, cn, mattype;
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    cn = CV_MAT_CN(types[INPUT][0]) + 1;
    depth = CV_MAT_DEPTH(types[INPUT][0]);
    types[INPUT][0] = types[OUTPUT][0] = types[REF_OUTPUT][0] = CV_MAKETYPE(depth, cn);

    mattype = depth == CV_64F ? CV_64F : bits & 1 ? CV_32F : CV_64F;
    types[INPUT][1] = mattype;
    sizes[INPUT][1] = cvSize(cn + 1, cn + 1);
}


void CxCore_PerspectiveTransformTest::run_func()
{
    cvPerspectiveTransform( test_array[INPUT][0], test_array[OUTPUT][0], &test_mat[INPUT][1] );
}


static void cvTsPerspectiveTransform( const CvArr* _src, CvArr* _dst, const CvMat* transmat )
{
    int i, j, cols;
    int cn, depth, mat_depth;
    CvMat astub, bstub, *a, *b;
    double mat[16], *buf;

    a = cvGetMat( _src, &astub, 0, 0 );
    b = cvGetMat( _dst, &bstub, 0, 0 );

    cn = CV_MAT_CN(a->type);
    depth = CV_MAT_DEPTH(a->type);
    mat_depth = CV_MAT_DEPTH(transmat->type);
    cols = transmat->cols;

    // prepare cn x (cn + 1) transform matrix
    if( mat_depth == CV_32F )
    {
        for( i = 0; i < transmat->rows; i++ )
            for( j = 0; j < cols; j++ )
                mat[i*cols + j] = ((float*)(transmat->data.ptr + transmat->step*i))[j];
    }
    else
    {
        assert( mat_depth == CV_64F );
        for( i = 0; i < transmat->rows; i++ )
            for( j = 0; j < cols; j++ )
                mat[i*cols + j] = ((double*)(transmat->data.ptr + transmat->step*i))[j];
    }

    // transform data
    cols = a->cols * cn;
    buf = (double*)cvStackAlloc( cols * sizeof(double) );

    for( i = 0; i < a->rows; i++ )
    {
        uchar* src = a->data.ptr + i*a->step;
        uchar* dst = b->data.ptr + i*b->step;

        switch( depth )
        {
        case CV_32F:
            for( j = 0; j < cols; j++ )
                buf[j] = ((float*)src)[j];
            break;
        case CV_64F:
            for( j = 0; j < cols; j++ )
                buf[j] = ((double*)src)[j];
            break;
        default:
            assert(0);
        }

        switch( cn )
        {
        case 2:
            for( j = 0; j < cols; j += 2 )
            {
                double t0 = buf[j]*mat[0] + buf[j+1]*mat[1] + mat[2];
                double t1 = buf[j]*mat[3] + buf[j+1]*mat[4] + mat[5];
                double w = buf[j]*mat[6] + buf[j+1]*mat[7] + mat[8];
                w = w ? 1./w : 0;
                buf[j] = t0*w;
                buf[j+1] = t1*w;
            }
            break;
        case 3:
            for( j = 0; j < cols; j += 3 )
            {
                double t0 = buf[j]*mat[0] + buf[j+1]*mat[1] + buf[j+2]*mat[2] + mat[3];
                double t1 = buf[j]*mat[4] + buf[j+1]*mat[5] + buf[j+2]*mat[6] + mat[7];
                double t2 = buf[j]*mat[8] + buf[j+1]*mat[9] + buf[j+2]*mat[10] + mat[11];
                double w = buf[j]*mat[12] + buf[j+1]*mat[13] + buf[j+2]*mat[14] + mat[15];
                w = w ? 1./w : 0;
                buf[j] = t0*w;
                buf[j+1] = t1*w;
                buf[j+2] = t2*w;
            }
            break;
        default:
            assert(0);
        }

        switch( depth )
        {
        case CV_32F:
            for( j = 0; j < cols; j++ )
                ((float*)dst)[j] = (float)buf[j];
            break;
        case CV_64F:
            for( j = 0; j < cols; j++ )
                ((double*)dst)[j] = buf[j];
            break;
        default:
            assert(0);
        }
    }
}


void CxCore_PerspectiveTransformTest::prepare_to_validation( int )
{
    CvMat* transmat = &test_mat[INPUT][1];
    cvTsPerspectiveTransform( test_array[INPUT][0], test_array[REF_OUTPUT][0], transmat );
}

CxCore_PerspectiveTransformTest perspective_test;


///////////////// Mahalanobis /////////////////////

class CxCore_MahalanobisTest : public CxCore_MatrixTest
{
public:
    CxCore_MahalanobisTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    int prepare_test_case( int test_case_idx );
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_MahalanobisTest::CxCore_MahalanobisTest() :
    CxCore_MatrixTest( "mahalanobis", "cvMahalanobis", 3, 1, false, true, 1 )
{
    test_case_count = 100;
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
}


void CxCore_MahalanobisTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    if( cvTsRandInt(rng) & 1 )
        sizes[INPUT][0].width = sizes[INPUT][1].width = 1;
    else
        sizes[INPUT][0].height = sizes[INPUT][1].height = 1;

    sizes[TEMP][0] = sizes[TEMP][1] = sizes[INPUT][0];
    sizes[INPUT][2].width = sizes[INPUT][2].height = sizes[INPUT][0].width + sizes[INPUT][0].height - 1;
    sizes[TEMP][2] = sizes[INPUT][2];
    types[TEMP][0] = types[TEMP][1] = types[TEMP][2] = types[INPUT][0];
}


int CxCore_MahalanobisTest::prepare_test_case( int test_case_idx )
{
    int result = CxCore_MatrixTest::prepare_test_case( test_case_idx );

    // make sure that the inverted covariation matrix is positively defined.
    cvTsGEMM( &test_mat[INPUT][2], &test_mat[INPUT][2], 1., 0, 0., &test_mat[TEMP][2], CV_GEMM_B_T );
    cvTsCopy( &test_mat[TEMP][2], &test_mat[INPUT][2] );

    return result;
}


void CxCore_MahalanobisTest::run_func()
{
    *((CvScalar*)(test_mat[OUTPUT][0].data.db)) =
        cvRealScalar(cvMahalanobis(test_array[INPUT][0], test_array[INPUT][1], test_array[INPUT][2]));
}

void CxCore_MahalanobisTest::prepare_to_validation( int )
{
    cvTsAdd( &test_mat[INPUT][0], cvScalarAll(1.),
             &test_mat[INPUT][1], cvScalarAll(-1.),
             cvScalarAll(0.), &test_mat[TEMP][0], 0 );
    if( test_mat[INPUT][0].rows == 1 )
        cvTsGEMM( &test_mat[TEMP][0], &test_mat[INPUT][2], 1.,
                  0, 0., &test_mat[TEMP][1], 0 );
    else
        cvTsGEMM( &test_mat[INPUT][2], &test_mat[TEMP][0], 1.,
                  0, 0., &test_mat[TEMP][1], 0 );

    *((CvScalar*)(test_mat[REF_OUTPUT][0].data.db)) =
        cvRealScalar(sqrt(cvTsCrossCorr(&test_mat[TEMP][0], &test_mat[TEMP][1])));
}

CxCore_MahalanobisTest mahalanobis_test;


///////////////// covarmatrix /////////////////////

class CxCore_CovarMatrixTest : public CxCore_MatrixTest
{
public:
    CxCore_CovarMatrixTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    int prepare_test_case( int test_case_idx );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    CvTestPtrVec temp_hdrs;
    uchar* hdr_data;
    int flags, t_flag;
    bool are_images;
};


CxCore_CovarMatrixTest::CxCore_CovarMatrixTest() :
    CxCore_MatrixTest( "covarmatrix", "cvCalcCovarMatrix", 1, 1, true, false, 1 ),
        flags(0), t_flag(0), are_images(false)
{
    test_case_count = 100;
    test_array[INPUT_OUTPUT].push(NULL);
    test_array[REF_INPUT_OUTPUT].push(NULL);
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
}


void CxCore_CovarMatrixTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int bits = cvTsRandInt(rng);
    int i, len, count;
    CvSize sz;
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    flags = bits & (CV_COVAR_NORMAL | CV_COVAR_USE_AVG | CV_COVAR_SCALE);
    t_flag = (bits & 256) != 0;

    if( !t_flag )
        len = sizes[INPUT][0].width, count = sizes[INPUT][0].height;
    else
        len = sizes[INPUT][0].height, count = sizes[INPUT][0].width;

    if( CV_MAT_DEPTH(types[INPUT][0]) == CV_32S )
        types[INPUT][0] = (types[INPUT][0] & ~CV_MAT_DEPTH_MASK) | CV_32F;

    sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = flags & CV_COVAR_NORMAL ? cvSize(len,len) : cvSize(count,count);
    sizes[INPUT_OUTPUT][0] = sizes[REF_INPUT_OUTPUT][0] = sz = !t_flag ? cvSize(len,1) : cvSize(1,len);
    sizes[TEMP][0] = sizes[INPUT][0];

    types[INPUT_OUTPUT][0] = types[REF_INPUT_OUTPUT][0] =
    types[OUTPUT][0] = types[REF_OUTPUT][0] = types[TEMP][0] =
        CV_MAT_DEPTH(types[INPUT][0]) == CV_64F || (bits & 512) ? CV_64F : CV_32F;

    are_images = (bits & 1024) != 0;
    for( i = 0; i < count; i++ )
        temp_hdrs.push(NULL);
}


int CxCore_CovarMatrixTest::prepare_test_case( int test_case_idx )
{
    int result = CxCore_MatrixTest::prepare_test_case( test_case_idx );
    int i, count = temp_hdrs.size();
    int hdr_size = are_images ? sizeof(IplImage) : sizeof(CvMat);

    hdr_data = (uchar*)cvAlloc( count*hdr_size );
    for( i = 0; i < count; i++ )
    {
        CvMat part;
        void* ptr = hdr_data + i*hdr_size;

        if( !t_flag )
            cvGetRow( &test_mat[INPUT][0], &part, i );
        else
            cvGetCol( &test_mat[INPUT][0], &part, i );

        if( !are_images )
            *((CvMat*)ptr) = part;
        else
            cvGetImage( &part, (IplImage*)ptr );

        temp_hdrs[i] = ptr;
    }

    return result;
}


void CxCore_CovarMatrixTest::run_func()
{
    cvCalcCovarMatrix( (const void**)&temp_hdrs[0], temp_hdrs.size(),
                       test_array[OUTPUT][0], test_array[INPUT_OUTPUT][0], flags );
}


void CxCore_CovarMatrixTest::prepare_to_validation( int )
{
    CvMat* avg = &test_mat[REF_INPUT_OUTPUT][0];
    int count = temp_hdrs.size();
    double scale = 1.;

    if( !(flags & CV_COVAR_USE_AVG) )
    {
        int i;
        cvTsZero( avg );

        for( i = 0; i < count; i++ )
        {
            CvMat stub;
            cvTsAdd( avg, cvScalarAll(1.), cvGetMat( temp_hdrs[i], &stub ),
                     cvScalarAll(1.), cvScalarAll(0.), avg, 0 );
        }

        cvTsAdd( avg, cvScalarAll(1./count), 0,
                 cvScalarAll(0.), cvScalarAll(0.), avg, 0 );
    }

    if( flags & CV_COVAR_SCALE )
    {
        scale = flags & CV_COVAR_NORMAL ? 1./count :
            1./(test_mat[REF_INPUT_OUTPUT][0].rows*test_mat[REF_INPUT_OUTPUT][0].cols);
    }

    cvRepeat( avg, &test_mat[TEMP][0] );
    cvTsAdd( &test_mat[INPUT][0], cvScalarAll(1.),
             &test_mat[TEMP][0], cvScalarAll(-1.),
             cvScalarAll(0.), &test_mat[TEMP][0], 0 );

    cvTsGEMM( &test_mat[TEMP][0], &test_mat[TEMP][0],
              scale, 0, 0., &test_mat[REF_OUTPUT][0],
              t_flag ^ ((flags & CV_COVAR_NORMAL) != 0) ?
              CV_GEMM_A_T : CV_GEMM_B_T );

    cvFree( (void**)&hdr_data );
    temp_hdrs.clear();
}

CxCore_CovarMatrixTest covarmatrix_test;


static void cvTsFloodWithZeros( CvMat* mat, CvRNG* rng )
{
    int k, total = mat->rows*mat->cols;
    int zero_total = cvTsRandInt(rng) % total;
    assert( CV_MAT_TYPE(mat->type) == CV_32FC1 ||
            CV_MAT_TYPE(mat->type) == CV_64FC1 );

    for( k = 0; k < zero_total; k++ )
    {
        int i = cvTsRandInt(rng) % mat->rows;
        int j = cvTsRandInt(rng) % mat->cols;
        uchar* row = mat->data.ptr + mat->step*i;

        if( CV_MAT_DEPTH(mat->type) == CV_32FC1 )
            ((float*)row)[j] = 0.f;
        else
            ((double*)row)[j] = 0.;
    }
}


///////////////// determinant /////////////////////

class CxCore_DetTest : public CxCore_MatrixTest
{
public:
    CxCore_DetTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    void get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high );
    int prepare_test_case( int test_case_idx );
    void run_func();
    void prepare_to_validation( int test_case_idx );
};


CxCore_DetTest::CxCore_DetTest() :
    CxCore_MatrixTest( "det", "cvDet", 1, 1, false, true, 1 )
{
    test_case_count = 100;
    max_log_array_size = 7;
    test_array[TEMP].push(NULL);
}


void CxCore_DetTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );

    sizes[INPUT][0].width = sizes[INPUT][0].height = sizes[INPUT][0].height;
    sizes[TEMP][0] = sizes[INPUT][0];
    types[TEMP][0] = CV_64FC1;
}


void CxCore_DetTest::get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high )
{
    *low = cvScalarAll(-2.);
    *high = cvScalarAll(2.);
}


double CxCore_DetTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return CV_MAT_DEPTH(cvGetElemType(test_array[INPUT][0])) == CV_32F ? 1e-2 : 1e-5;
}


int CxCore_DetTest::prepare_test_case( int test_case_idx )
{
    int ok = CxCore_MatrixTest::prepare_test_case( test_case_idx );
    cvTsFloodWithZeros( &test_mat[INPUT][0], ts->get_rng() );

    return ok;
}


void CxCore_DetTest::run_func()
{
    *((CvScalar*)(test_mat[OUTPUT][0].data.db)) = cvRealScalar(cvDet(test_array[INPUT][0]));
}


// LU method that chooses the optimal in a column pivot element
static double cvTsLU( CvMat* a, CvMat* b=NULL, CvMat* x=NULL, int* rank=0 )
{
    int i, j, k, N = a->rows, N1 = a->cols, Nm = MIN(N, N1), step = a->step/sizeof(double);
    int M = b ? b->cols : 0, b_step = b ? b->step/sizeof(double) : 0;
    int x_step = x ? x->step/sizeof(double) : 0;
    double *a0 = a->data.db, *b0 = b ? b->data.db : 0;
    double *x0 = x ? x->data.db : 0;
    double t, det = 1.;
    assert( CV_MAT_TYPE(a->type) == CV_64FC1 &&
            (!b || CV_ARE_TYPES_EQ(a,b)) && (!x || CV_ARE_TYPES_EQ(a,x)));

    for( i = 0; i < Nm; i++ )
    {
        double max_val = fabs(a0[i*step + i]);
        double *a1, *a2, *b1 = 0, *b2 = 0;
        k = i;

        for( j = i+1; j < N; j++ )
        {
            t = fabs(a0[j*step + i]);
            if( max_val < t )
            {
                max_val = t;
                k = j;
            }
        }

        if( k != i )
        {
            for( j = i; j < N1; j++ )
                CV_SWAP( a0[i*step + j], a0[k*step + j], t );

            for( j = 0; j < M; j++ )
                CV_SWAP( b0[i*b_step + j], b0[k*b_step + j], t );
            det = -det;
        }

        if( max_val == 0 )
        {
            if( rank )
                *rank = i;
            return 0.;
        }

        a1 = a0 + i*step;
        a2 = a1 + step;
        b1 = b0 + i*b_step;
        b2 = b1 + b_step;

        for( j = i+1; j < N; j++, a2 += step, b2 += b_step )
        {
            t = a2[i]/a1[i];
            for( k = i+1; k < N1; k++ )
                a2[k] -= t*a1[k];

            for( k = 0; k < M; k++ )
                b2[k] -= t*b1[k];
        }

        det *= a1[i];
    }

    if( x )
    {
        assert( b );

        for( i = N-1; i >= 0; i-- )
        {
            double* a1 = a0 + i*step;
            double* b1 = b0 + i*b_step;
            for( j = 0; j < M; j++ )
            {
                t = b1[j];
                for( k = i+1; k < N1; k++ )
                    t -= a1[k]*x0[k*x_step + j];
                x0[i*x_step + j] = t/a1[i];
            }
        }
    }

    if( rank )
        *rank = i;
    return det;
}


void CxCore_DetTest::prepare_to_validation( int )
{
    if( !CV_ARE_TYPES_EQ( &test_mat[INPUT][0], &test_mat[TEMP][0] ))
        cvTsConvert( &test_mat[INPUT][0], &test_mat[TEMP][0] );
    else
        cvTsCopy( &test_mat[INPUT][0], &test_mat[TEMP][0], 0 );

    *((CvScalar*)(test_mat[REF_OUTPUT][0].data.db)) = cvRealScalar(cvTsLU(&test_mat[TEMP][0], 0, 0));
}

CxCore_DetTest det_test;



///////////////// invert /////////////////////

class CxCore_InvertTest : public CxCore_MatrixTest
{
public:
    CxCore_InvertTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high );
    double get_success_error_level( int test_case_idx, int i, int j );
    int prepare_test_case( int test_case_idx );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int method, rank;
    double result;
};


CxCore_InvertTest::CxCore_InvertTest() :
    CxCore_MatrixTest( "invert", "cvInvert, cvSVD, cvSVBkSb", 1, 1, false, false, 1 ), method(0), result(0.), rank(0)
{
    test_case_count = 100;
    max_log_array_size = 7;
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
}


void CxCore_InvertTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int bits = cvTsRandInt(rng);
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    int min_size = MIN( sizes[INPUT][0].width, sizes[INPUT][0].height );

    if( (bits & 3) == 0 )
    {
        method = CV_SVD;
        if( bits & 4 )
        {
            sizes[INPUT][0] = cvSize(min_size, min_size);
            if( bits & 8 )
                method = CV_SVD_SYM;
        }
    }
    else
    {
        method = CV_LU;
        sizes[INPUT][0] = cvSize(min_size, min_size);
    }

    sizes[TEMP][0].width = sizes[INPUT][0].height;
    sizes[TEMP][0].height = sizes[INPUT][0].width;
    sizes[TEMP][1] = sizes[INPUT][0];
    types[TEMP][0] = types[INPUT][0];
    types[TEMP][1] = CV_64FC1;
    sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = cvSize(min_size, min_size);
}


double CxCore_InvertTest::get_success_error_level( int /*test_case_idx*/, int, int )
{
    return CV_MAT_DEPTH(cvGetElemType(test_array[OUTPUT][0])) == CV_32F ? 1e-2 : 1e-9;
}

int CxCore_InvertTest::prepare_test_case( int test_case_idx )
{
    int ok = CxCore_MatrixTest::prepare_test_case( test_case_idx );

    cvTsFloodWithZeros( &test_mat[INPUT][0], ts->get_rng() );

    if( method == CV_SVD_SYM )
    {
        cvTsGEMM( &test_mat[INPUT][0], &test_mat[INPUT][0], 1.,
                  0, 0., &test_mat[TEMP][0], CV_GEMM_B_T );
        cvTsCopy( &test_mat[TEMP][0], &test_mat[INPUT][0] );
    }

    return ok;
}



void CxCore_InvertTest::get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high )
{
    *low = cvScalarAll(-2.);
    *high = cvScalarAll(2.);
}


void CxCore_InvertTest::run_func()
{
    result = cvInvert(test_array[INPUT][0], test_array[TEMP][0], method);
}


static double cvTsSVDet( CvMat* mat )
{
    int type = CV_MAT_TYPE(mat->type);
    int i, nm = CV_MIN( mat->rows, mat->cols );
    CvMat* w = cvCreateMat( nm, 1, type );
    double det = 1.;

    cvSVD( mat, w, 0, 0, 0 );

    if( type == CV_32FC1 )
    {
        for( i = 0; i < nm; i++ )
            det *= w->data.fl[i];
    }
    else
    {
        for( i = 0; i < nm; i++ )
            det *= w->data.db[i];
    }

    cvReleaseMat( &w );
    return det;
}

void CxCore_InvertTest::prepare_to_validation( int )
{
    CvMat* input = &test_mat[INPUT][0];
    double det = method != CV_LU ? cvTsSVDet( input ) : 0;
    double threshold = (CV_MAT_DEPTH(input->type) == CV_32F ? FLT_EPSILON : DBL_EPSILON)*100;

    if( CV_MAT_TYPE(input->type) == CV_32FC1 )
        cvTsConvert( input, &test_mat[TEMP][1] );
    else
        cvTsCopy( input, &test_mat[TEMP][1], 0 );

    if( method == CV_LU && result == 0 || method != CV_LU &&
        det < threshold || result < threshold )
    {
        cvTsZero( &test_mat[OUTPUT][0] );
        cvTsZero( &test_mat[REF_OUTPUT][0] );
        //cvTsAdd( 0, cvScalarAll(0.), 0, cvScalarAll(0.), cvScalarAll(fabs(det)>1e-3),
        //         &test_mat[REF_OUTPUT][0], 0 );
        return;
    }

    if( input->rows >= input->cols )
        cvTsGEMM( &test_mat[TEMP][0], input, 1., 0, 0., &test_mat[OUTPUT][0], 0 );
    else
        cvTsGEMM( input, &test_mat[TEMP][0], 1., 0, 0., &test_mat[OUTPUT][0], 0 );

    cvTsSetIdentity( &test_mat[REF_OUTPUT][0], cvScalarAll(1.) );
}

CxCore_InvertTest invert_test;


///////////////// solve /////////////////////

class CxCore_SolveTest : public CxCore_MatrixTest
{
public:
    CxCore_SolveTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high );
    double get_success_error_level( int test_case_idx, int i, int j );
    int prepare_test_case( int test_case_idx );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int method, rank;
    double result;
};


CxCore_SolveTest::CxCore_SolveTest() :
    CxCore_MatrixTest( "solve", "cvSolve, cvSVD, cvSVBkSb", 2, 1, false, false, 1 ), method(0), result(0.), rank(0)
{
    test_case_count = 100;
    max_log_array_size = 7;
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
}


void CxCore_SolveTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int bits = cvTsRandInt(rng);
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    CvSize in_sz = sizes[INPUT][0];
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    sizes[INPUT][0] = in_sz;
    int min_size = MIN( sizes[INPUT][0].width, sizes[INPUT][0].height );

    if( (bits & 3) == 0 )
    {
        method = CV_SVD;
        if( bits & 4 )
        {
            sizes[INPUT][0] = cvSize(min_size, min_size);
            /*if( bits & 8 )
                method = CV_SVD_SYM;*/
        }
    }
    else
    {
        method = CV_LU;
        sizes[INPUT][0] = cvSize(min_size, min_size);
    }

    sizes[INPUT][1].height = sizes[INPUT][0].height;
    sizes[TEMP][0].width = sizes[INPUT][1].width;
    sizes[TEMP][0].height = sizes[INPUT][0].width;
    sizes[TEMP][1] = sizes[INPUT][0];
    types[TEMP][0] = types[INPUT][0];
    types[TEMP][1] = CV_64FC1;
    sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = cvSize(sizes[INPUT][1].width, min_size);
}


int CxCore_SolveTest::prepare_test_case( int test_case_idx )
{
    int ok = CxCore_MatrixTest::prepare_test_case( test_case_idx );

    /*if( method == CV_SVD_SYM )
    {
        cvTsGEMM( test_array[INPUT][0], test_array[INPUT][0], 1.,
                  0, 0., test_array[TEMP][0], CV_GEMM_B_T );
        cvTsCopy( test_array[TEMP][0], test_array[INPUT][0] );
    }*/

    return ok;
}


void CxCore_SolveTest::get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high )
{
    *low = cvScalarAll(-2.);
    *high = cvScalarAll(2.);
}


double CxCore_SolveTest::get_success_error_level( int /*test_case_idx*/, int, int )
{
    return CV_MAT_DEPTH(cvGetElemType(test_array[OUTPUT][0])) == CV_32F ? 1e-2 : 1e-8;
}


void CxCore_SolveTest::run_func()
{
    result = cvSolve(test_array[INPUT][0], test_array[INPUT][1], test_array[TEMP][0], method);
}

void CxCore_SolveTest::prepare_to_validation( int )
{
    //int rank = test_mat[REF_OUTPUT][0].rows;
    CvMat* dst;

    if( method == CV_LU && result == 0 )
    {
        if( CV_MAT_TYPE(test_mat[INPUT][0].type) == CV_32FC1 )
            cvTsConvert( &test_mat[INPUT][0], &test_mat[TEMP][1] );
        else
            cvTsCopy( &test_mat[INPUT][0], &test_mat[TEMP][1], 0 );

        cvTsZero( &test_mat[OUTPUT][0] );
        double det = cvTsLU( &test_mat[TEMP][1], 0, 0 );
        cvTsAdd( 0, cvScalarAll(0.), 0, cvScalarAll(0.), cvScalarAll(det != 0),
                 &test_mat[REF_OUTPUT][0], 0 );
        return;
    }

    dst = test_mat[INPUT][0].rows <= test_mat[INPUT][0].cols ? &test_mat[OUTPUT][0] : &test_mat[INPUT][1];

    cvTsGEMM( &test_mat[INPUT][0], &test_mat[TEMP][0], 1., &test_mat[INPUT][1], -1., dst, 0 );
    if( dst != &test_mat[OUTPUT][0] )
        cvTsGEMM( &test_mat[INPUT][0], dst, 1., 0, 0., &test_mat[OUTPUT][0], CV_GEMM_A_T );
    cvTsZero( &test_mat[REF_OUTPUT][0] );
}

CxCore_SolveTest solve_test;


///////////////// SVD /////////////////////

class CxCore_SVDTest : public CxCore_MatrixTest
{
public:
    CxCore_SVDTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    void get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high );
    int prepare_test_case( int test_case_idx );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int flags;
    bool have_u, have_v, symmetric, compact, vector_w;
};


CxCore_SVDTest::CxCore_SVDTest() :
    CxCore_MatrixTest( "svd", "cvSVD", 1, 4, false, false, 1 ),
        flags(0), have_u(false), have_v(false), symmetric(false), compact(false), vector_w(false)
{
    test_case_count = 100;
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
}


void CxCore_SVDTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int bits = cvTsRandInt(rng);
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    int min_size, i, m, n;

    min_size = MIN( sizes[INPUT][0].width, sizes[INPUT][0].height );

    flags = bits & (CV_SVD_MODIFY_A+CV_SVD_U_T+CV_SVD_V_T);
    have_u = (bits & 8) != 0;
    have_v = (bits & 16) != 0;
    symmetric = (bits & 32) != 0;
    compact = (bits & 64) != 0;
    vector_w = (bits & 128) != 0;

    if( symmetric )
        sizes[INPUT][0] = cvSize(min_size, min_size);

    m = sizes[INPUT][0].height;
    n = sizes[INPUT][0].width;

    if( compact )
        sizes[TEMP][0] = cvSize(min_size, min_size);
    else
        sizes[TEMP][0] = sizes[INPUT][0];
    sizes[TEMP][3] = cvSize(0,0);

    if( vector_w )
    {
        sizes[TEMP][3] = sizes[TEMP][0];
        if( bits & 256 )
            sizes[TEMP][0] = cvSize(1, min_size);
        else
            sizes[TEMP][0] = cvSize(min_size, 1);
    }

    if( have_u )
    {
        sizes[TEMP][1] = compact ? cvSize(min_size, m) : cvSize(m, m);

        if( flags & CV_SVD_U_T )
            CV_SWAP( sizes[TEMP][1].width, sizes[TEMP][1].height, i );
    }
    else
        sizes[TEMP][1] = cvSize(0,0);

    if( have_v )
    {
        sizes[TEMP][2] = compact ? cvSize(n, min_size) : cvSize(n, n);

        if( !(flags & CV_SVD_V_T) )
            CV_SWAP( sizes[TEMP][2].width, sizes[TEMP][2].height, i );
    }
    else
        sizes[TEMP][2] = cvSize(0,0);

    types[TEMP][0] = types[TEMP][1] = types[TEMP][2] = types[TEMP][3] = types[INPUT][0];
    types[OUTPUT][0] = types[OUTPUT][1] = types[OUTPUT][2] = types[INPUT][0];
    types[OUTPUT][3] = CV_8UC1;
    sizes[OUTPUT][0] = !have_u || !have_v ? cvSize(0,0) : sizes[INPUT][0];
    sizes[OUTPUT][1] = !have_u ? cvSize(0,0) : compact ? cvSize(min_size,min_size) : cvSize(m,m);
    sizes[OUTPUT][2] = !have_v ? cvSize(0,0) : compact ? cvSize(min_size,min_size) : cvSize(n,n);
    sizes[OUTPUT][3] = cvSize(min_size,1);

    for( i = 0; i < 4; i++ )
    {
        sizes[REF_OUTPUT][i] = sizes[OUTPUT][i];
        types[REF_OUTPUT][i] = types[OUTPUT][i];
    }
}


int CxCore_SVDTest::prepare_test_case( int test_case_idx )
{
    int ok = CxCore_MatrixTest::prepare_test_case( test_case_idx );
    CvMat* input = &test_mat[INPUT][0];
    cvTsFloodWithZeros( input, ts->get_rng() );

    if( symmetric && (have_u || have_v) )
    {
        CvMat* temp = &test_mat[TEMP][have_u ? 1 : 2];
        cvTsGEMM( input, input, 1.,
                  0, 0., temp, CV_GEMM_B_T );
        cvTsCopy( temp, input );
    }

    if( (flags & CV_SVD_MODIFY_A) && test_array[OUTPUT][0] )
        cvTsCopy( input, &test_mat[OUTPUT][0] );

    return ok;
}


void CxCore_SVDTest::get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high )
{
    *low = cvScalarAll(-2.);
    *high = cvScalarAll(2.);
}


void CxCore_SVDTest::run_func()
{
    CvArr* src = test_array[!(flags & CV_SVD_MODIFY_A) ? INPUT : OUTPUT][0];
    if( !src )
        src = test_array[INPUT][0];
    cvSVD( src, test_array[TEMP][0], test_array[TEMP][1], test_array[TEMP][2], flags );
}


void CxCore_SVDTest::prepare_to_validation( int )
{
    CvMat* input = &test_mat[INPUT][0];
    int m = input->rows, n = input->cols, min_size = MIN(m, n);
    CvMat *src, *dst, *w;
    double prev = 0, threshold = CV_MAT_TYPE(input->type) == CV_32FC1 ? FLT_EPSILON : DBL_EPSILON;
    int i, j = 0, step;

    if( have_u )
    {
        src = &test_mat[TEMP][1];
        dst = &test_mat[OUTPUT][1];
        cvTsGEMM( src, src, 1., 0, 0., dst, src->rows == dst->rows ? CV_GEMM_B_T : CV_GEMM_A_T );
        cvTsSetIdentity( &test_mat[REF_OUTPUT][1], cvScalarAll(1.) );
    }

    if( have_v )
    {
        src = &test_mat[TEMP][2];
        dst = &test_mat[OUTPUT][2];
        cvTsGEMM( src, src, 1., 0, 0., dst, src->rows == dst->rows ? CV_GEMM_B_T : CV_GEMM_A_T );
        cvTsSetIdentity( &test_mat[REF_OUTPUT][2], cvScalarAll(1.) );
    }

    w = &test_mat[TEMP][0];
    step = w->rows == 1 ? 1 : w->step/CV_ELEM_SIZE(w->type);
    for( i = 0; i < min_size; i++ )
    {
        double norm = 0, aii;
        uchar* row_ptr;
        if( w->rows > 1 && w->cols > 1 )
        {
            CvMat row;
            cvGetRow( w, &row, i );
            norm = cvNorm( &row, 0, CV_L1 );
            j = i;
            row_ptr = row.data.ptr;
        }
        else
        {
            row_ptr = w->data.ptr;
            j = i*step;
        }

        aii = CV_MAT_TYPE(w->type) == CV_32FC1 ?
            (double)((float*)row_ptr)[j] : ((double*)row_ptr)[j];
        if( w->rows == 1 || w->cols == 1 )
            norm = aii;
        norm = fabs(norm - aii);
        test_mat[OUTPUT][3].data.ptr[i] = aii >= 0 && norm < threshold && (i == 0 || aii <= prev);
        prev = aii;
    }

    cvTsAdd( 0, cvScalarAll(0.), 0, cvScalarAll(0.),
             cvScalarAll(1.), &test_mat[REF_OUTPUT][3], 0 );

    if( have_u && have_v )
    {
        if( vector_w )
        {
            cvTsZero( &test_mat[TEMP][3] );
            for( i = 0; i < min_size; i++ )
            {
                double val = cvGetReal1D( w, i );
                cvSetReal2D( &test_mat[TEMP][3], i, i, val );
            }
            w = &test_mat[TEMP][3];
        }

        if( m >= n )
        {
            cvTsGEMM( &test_mat[TEMP][1], w, 1., 0, 0., &test_mat[REF_OUTPUT][0],
                      flags & CV_SVD_U_T ? CV_GEMM_A_T : 0 );
            cvTsGEMM( &test_mat[REF_OUTPUT][0], &test_mat[TEMP][2], 1., 0, 0.,
                      &test_mat[OUTPUT][0], flags & CV_SVD_V_T ? 0 : CV_GEMM_B_T );
        }
        else
        {
            cvTsGEMM( w, &test_mat[TEMP][2], 1., 0, 0., &test_mat[REF_OUTPUT][0],
                      flags & CV_SVD_V_T ? 0 : CV_GEMM_B_T );
            cvTsGEMM( &test_mat[TEMP][1], &test_mat[REF_OUTPUT][0], 1., 0, 0.,
                      &test_mat[OUTPUT][0], flags & CV_SVD_U_T ? CV_GEMM_A_T : 0 );
        }

        cvTsCopy( &test_mat[INPUT][0], &test_mat[REF_OUTPUT][0], 0 );
    }
}


CxCore_SVDTest svd_test;


///////////////// SVBkSb /////////////////////

class CxCore_SVBkSbTest : public CxCore_MatrixTest
{
public:
    CxCore_SVBkSbTest();
protected:
    void get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types );
    double get_success_error_level( int test_case_idx, int i, int j );
    void get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high );
    int prepare_test_case( int test_case_idx );
    void run_func();
    void prepare_to_validation( int test_case_idx );
    int flags;
    bool have_b, symmetric, compact, vector_w;
};


CxCore_SVBkSbTest::CxCore_SVBkSbTest() :
    CxCore_MatrixTest( "svbksb", "cvSVBkSb", 2, 1, false, false, 1 ),
        flags(0), have_b(false), symmetric(false), compact(false), vector_w(false)
{
    test_case_count = 100;
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
    test_array[TEMP].push(NULL);
}


void CxCore_SVBkSbTest::get_test_array_types_and_sizes( int test_case_idx, CvSize** sizes, int** types )
{
    CvRNG* rng = ts->get_rng();
    int bits = cvTsRandInt(rng);
    CxCore_MatrixTest::get_test_array_types_and_sizes( test_case_idx, sizes, types );
    int min_size, i, m, n;
    CvSize b_size;

    min_size = MIN( sizes[INPUT][0].width, sizes[INPUT][0].height );

    flags = bits & (CV_SVD_MODIFY_A+CV_SVD_U_T+CV_SVD_V_T);
    have_b = (bits & 16) != 0;
    symmetric = (bits & 32) != 0;
    compact = (bits & 64) != 0;
    vector_w = (bits & 128) != 0;

    if( symmetric )
        sizes[INPUT][0] = cvSize(min_size, min_size);

    m = sizes[INPUT][0].height;
    n = sizes[INPUT][0].width;

    sizes[INPUT][1] = cvSize(0,0);
    b_size = cvSize(m,m);
    if( have_b )
    {
        sizes[INPUT][1].height = sizes[INPUT][0].height;
        sizes[INPUT][1].width = cvTsRandInt(rng) % 100 + 1;
        b_size = sizes[INPUT][1];
    }

    if( compact )
        sizes[TEMP][0] = cvSize(min_size, min_size);
    else
        sizes[TEMP][0] = sizes[INPUT][0];

    if( vector_w )
    {
        if( bits & 256 )
            sizes[TEMP][0] = cvSize(1, min_size);
        else
            sizes[TEMP][0] = cvSize(min_size, 1);
    }

    sizes[TEMP][1] = compact ? cvSize(min_size, m) : cvSize(m, m);

    if( flags & CV_SVD_U_T )
        CV_SWAP( sizes[TEMP][1].width, sizes[TEMP][1].height, i );

    sizes[TEMP][2] = compact ? cvSize(n, min_size) : cvSize(n, n);

    if( !(flags & CV_SVD_V_T) )
        CV_SWAP( sizes[TEMP][2].width, sizes[TEMP][2].height, i );

    types[TEMP][0] = types[TEMP][1] = types[TEMP][2] = types[INPUT][0];
    types[OUTPUT][0] = types[REF_OUTPUT][0] = types[INPUT][0];
    sizes[OUTPUT][0] = sizes[REF_OUTPUT][0] = cvSize( b_size.width, n );
}


int CxCore_SVBkSbTest::prepare_test_case( int test_case_idx )
{
    int ok = CxCore_MatrixTest::prepare_test_case( test_case_idx );
    CvMat* input = &test_mat[INPUT][0];
    cvTsFloodWithZeros( input, ts->get_rng() );

    if( symmetric )
    {
        CvMat* temp = &test_mat[TEMP][1];
        cvTsGEMM( input, input, 1., 0, 0., temp, CV_GEMM_B_T );
        cvTsCopy( temp, input );
    }

    cvSVD( input, test_array[TEMP][0], test_array[TEMP][1], test_array[TEMP][2], flags );

    return ok;
}


void CxCore_SVBkSbTest::get_minmax_bounds( int /*i*/, int /*j*/, int /*type*/, CvScalar* low, CvScalar* high )
{
    *low = cvScalarAll(-2.);
    *high = cvScalarAll(2.);
}


double CxCore_SVBkSbTest::get_success_error_level( int /*test_case_idx*/, int /*i*/, int /*j*/ )
{
    return CV_MAT_DEPTH(cvGetElemType(test_array[INPUT][0])) == CV_32F ? 1e-3 : 1e-7;
}


void CxCore_SVBkSbTest::run_func()
{
    cvSVBkSb( test_array[TEMP][0], test_array[TEMP][1], test_array[TEMP][2],
              test_array[INPUT][1], test_array[OUTPUT][0], flags );
}


void CxCore_SVBkSbTest::prepare_to_validation( int )
{
    CvMat* input = &test_mat[INPUT][0];
    int i, m = input->rows, n = input->cols, min_size = MIN(m, n), nb;
    bool is_float = CV_MAT_DEPTH(input->type) == CV_32F;
    CvSize w_size = compact ? cvSize(min_size,min_size) : cvSize(m,n);
    CvMat* w = &test_mat[TEMP][0];
    CvMat* wdb = cvCreateMat( w_size.height, w_size.width, CV_64FC1 );
    // use exactly the same threshold as in icvSVD... ,
    // so the changes in the library and here should be synchronized.
    double threshold = cvSum( w ).val[0]*2*(is_float ? FLT_EPSILON : DBL_EPSILON);
    CvMat *u, *v, *b, *t0, *t1;

    cvTsZero(wdb);
    for( i = 0; i < min_size; i++ )
    {
        double wii = vector_w ? cvGetReal1D(w,i) : cvGetReal2D(w,i,i);
        cvSetReal2D( wdb, i, i, wii > threshold ? 1./wii : 0. );
    }

    u = &test_mat[TEMP][1];
    v = &test_mat[TEMP][2];
    b = 0;
    nb = m;

    if( test_array[INPUT][1] )
    {
        b = &test_mat[INPUT][1];
        nb = b->cols;
    }

    if( is_float )
    {
        u = cvCreateMat( u->rows, u->cols, CV_64F );
        cvTsConvert( &test_mat[TEMP][1], u );
        if( b )
        {
            b = cvCreateMat( b->rows, b->cols, CV_64F );
            cvTsConvert( &test_mat[INPUT][1], b );
        }
    }

    t0 = cvCreateMat( wdb->cols, nb, CV_64F );

    if( b )
        cvTsGEMM( u, b, 1., 0, 0., t0, !(flags & CV_SVD_U_T) ? CV_GEMM_A_T : 0 );
    else if( flags & CV_SVD_U_T )
        cvTsCopy( u, t0 );
    else
        cvTsTranspose( u, t0 );

    if( is_float )
    {
        cvReleaseMat( &b );

        if( !symmetric )
        {
            cvReleaseMat( &u );
            v = cvCreateMat( v->rows, v->cols, CV_64F );
        }
        else
        {
            v = u;
            u = 0;
        }
        cvTsConvert( &test_mat[TEMP][2], v );
    }

    t1 = cvCreateMat( wdb->rows, nb, CV_64F );
    cvTsGEMM( wdb, t0, 1, 0, 0, t1, 0 );

    if( !is_float || !symmetric )
    {
        cvReleaseMat( &t0 );
        t0 = !is_float ? &test_mat[REF_OUTPUT][0] : cvCreateMat( test_mat[REF_OUTPUT][0].rows, nb, CV_64F );
    }

    cvTsGEMM( v, t1, 1, 0, 0, t0, flags & CV_SVD_V_T ? CV_GEMM_A_T : 0 );
    cvReleaseMat( &t1 );

    if( t0 != &test_mat[REF_OUTPUT][0] )
    {
        cvTsConvert( t0, &test_mat[REF_OUTPUT][0] );
        cvReleaseMat( &t0 );
    }

    if( v != &test_mat[TEMP][2] )
        cvReleaseMat( &v );

    cvReleaseMat( &wdb );
}


CxCore_SVBkSbTest svbksb_test;


// TODO: eigenvv, invsqrt, cbrt, fastarctan, (round, floor, ceil(?)),
// svd, svbksb (the latter two are somewhat tested in invert & solve,
// though some special cases are not considered)

/* End of file. */
