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

#include <stdlib.h>
#include <assert.h>

static char* test_desc = "Comparing with IPL-based pipeline";

/* actual parameters */
static int min_img_size, max_img_size;
static int img_size_delta_type, img_size_delta;
static int base_iters;

static int ud_l = 0, ud_h = 1,
           dt_l = 0, dt_h = 2,
           ch_l = 0, ch_h = 1;

static const int img8u_range = 255;
static const int img8s_range = 128;
static const float img32f_range = 1.f;
static const int img32f_bits  = 23;

static int init_pyr_params = 0;

static char* funcs[] =
{
    "cvPyrUp",
    "cvPyrDown"
};

/* reflect bounds of ROI (i.e make appropriate "border mode") and apply gaussian */
static void apply_gaussian( IplImage* src, IplImage* dst )
{
    int   x0, y0, x1, y1;
    int   step = src->widthStep;
    char* data = src->imageData;
    int   bt_pix = ((src->depth & 255) >> 3) * src->nChannels;
    int   i, j;
    int   w, h;
    int   depth = src->depth;
    IplROI* roi = src->roi;

    /* 1. Create appropriate border mode. example: a b c I e c b (I-left boundary) */
    assert( roi != 0 && dst->roi == roi );

    x0 = roi->xOffset;
    y0 = roi->yOffset;
    x1 = x0 + roi->width;
    y1 = y0 + roi->height;

    data += y0*step;

    /* 1a. setup left & right border */
    for( i = y0; i < y1; i++, data += step )
    {
        for( j = 0; j < x0; j++ )
        {
            memcpy( data + (x0-j-1)*bt_pix, data + (x0+j+1)*bt_pix, bt_pix );
            memcpy( data + (x1+j)*bt_pix, data + (x1-j-2)*bt_pix, bt_pix );
        }
    }

    data = src->imageData;

    /* 1b. setup top & bottom */
    for( i = 0; i < y0; i++ )
    {
        memcpy( data + (y0-i-1)*step, data + (y0+i+1)*step, step );
        memcpy( data + (y1+i)*step,   data + (y1-i-2)*step, step );
    }

    w = roi->width;
    h = roi->height;

    roi->xOffset -= x0;
    roi->yOffset -= y0;
    roi->width += x0*2;
    roi->height += y0*2;

    /* 2. convolve */
    if( depth == IPL_DEPTH_8S )
    {
        src->depth = dst->depth = IPL_DEPTH_8U;
        cvXorS( src, cvScalarAll( 128 ), src );
    }
        
    cvSmooth( src, dst, CV_GAUSSIAN, 5, 5 );

    if( depth == IPL_DEPTH_8S )
    {
        cvXorS( src, cvScalarAll( 128 ), src );
        cvXorS( dst, cvScalarAll( 128 ), dst );
        src->depth = dst->depth = IPL_DEPTH_8S;
    }

    roi->width = w;
    roi->height = h;
    roi->xOffset = x0;
    roi->yOffset = y0;
}


static void clear_even_rows_and_cols( IplImage* img )
{
    char*   data;
    int     step;
    CvSize sz;
    int     depth, channels, bt_pix;
    int     i, j;

    atsGetImageInfo( img, (void**)&data, &step, &sz, &depth, &channels, &bt_pix );

    assert( sz.height % 2 == 0 );

    for( i = 0; i < sz.height; i+=2, data += step*2 )
    {
        for( j = 0; j < sz.width; j+=2 )
        {
            memset( data + (j + 1)*bt_pix, 0, bt_pix );
        }
        memset( data + step, 0, sz.width*bt_pix );
    }
}

#define N   5  /* linear size of kernel */

static void read_pyramid_params( void )
{
    if( !init_pyr_params )
    {
        int up_down, data_types, channels;

        /* Determine which tests are needed to run */
        trsCaseRead( &up_down, "/a/u/d", "a", "a - all, u - up_samle, d - down_sample" );
        if( up_down != 0 ) ud_l = ud_h = up_down - 1;

        trsCaseRead( &data_types,"/a/8u/8s/32f", "a",
            "a - all, 8u - unsigned char, 8s - signed char, 32f - float" );
        if( data_types != 0 ) dt_l = dt_h = data_types - 1;

        trsCaseRead( &channels, "/a/1/3", "a", "a - all, 1 - single channel, 3 - three channels" );
        if( channels != 0 ) ch_l = ch_h = channels - 1;

        /* read tests params */
        trsiRead( &min_img_size, "5", "Minimal width or height of down-sampled image" );
        trsiRead( &max_img_size, "640", "Maximal width or height of down-sampled image" );
        trsCaseRead( &img_size_delta_type,"/a/m", "m", "a - add, m - multiply" );
        trsiRead( &img_size_delta, "2", "Image size step(factor)" );
        trsiRead( &base_iters, "100", "Base number of iterations for 1x1 images" );

        init_pyr_params = 1;
    }
}


/******************************************************************\
*                      PyrDownCmpIPL                               *
\******************************************************************/
static int PyrDownCmpIPL( void* arg )
{
    const int   r = 2;
    int         code = TRS_OK;

    int         param = (int)arg;
    int         depth = param/2 == 2 ? IPL_DEPTH_32F :
                        param/2 == 1 ? IPL_DEPTH_8S  : IPL_DEPTH_8U;
    int         channels = param % 2 == 1 ? 3 : 1;

    int         seed = atsGetSeed();

    int         w = 0, h = 0, i = 0;
    int         merr_w = 0, merr_h = 0, merr_iter = 0; /* where was maximum error */
    double      max_err = 0.;
    double      success_error_level;

    IplROI      srcRoi, dstRoi;

    IplImage    *src, *src2, *dst, *dst2;
    AtsRandState rng_state;
    atsRandInit( &rng_state, 0, 1, seed );

    read_pyramid_params();

    if( !(ATS_RANGE( 1, ud_l, ud_h + 1)       &&
          ATS_RANGE( param/2, dt_l, dt_h + 1) &&
          ATS_RANGE( param%2, ch_l, ch_h + 1)))
          return TRS_UNDEF;

    src = atsCreateImage( max_img_size*2 + r*2, max_img_size*2 + r*2, depth, channels, 0);
    src2= cvCloneImage( src );
    dst = atsCreateImage( max_img_size,   max_img_size,   depth, channels, 0 );
    dst2= cvCloneImage( dst );

    srcRoi.coi = dstRoi.coi = 0;
    srcRoi.xOffset  = srcRoi.yOffset  = r;
    dstRoi.xOffset  = dstRoi.yOffset  = 0;

    src->roi = src2->roi = &srcRoi;
    dst->roi = dst2->roi = &dstRoi;

    success_error_level = depth == IPL_DEPTH_32F ? 1e-4 : 1;

    for( h = min_img_size; h <= max_img_size; )
    {
        for( w = min_img_size; w <= max_img_size; )
        {
            int     denom = (w - min_img_size + 1)*(h - min_img_size + 1)*4*channels;
            int     iters = (base_iters*2 + denom)/(2*denom);

            if( iters < 1 ) iters = 1;

            srcRoi.width   = w*2;
            srcRoi.height  = h*2;
            dstRoi.width   = w;
            dstRoi.height  = h;

            for( i = 0; i < iters; i++ )
            {
                double err0;

                switch( depth )
                {
                case IPL_DEPTH_8U:
                    atsRandSetBounds( &rng_state, 0, img8u_range );
                    break;
                case IPL_DEPTH_8S:
                    atsRandSetBounds( &rng_state, -img8s_range, img8s_range );
                    break;
                case IPL_DEPTH_32F:
                    atsRandSetBounds( &rng_state, -img32f_range, img32f_range );
                    atsRandSetFloatBits( &rng_state, img32f_bits );
                    break;
                default:
                    assert(0);
                    code = TRS_FAIL;
                    goto test_exit;
                }
                atsFillRandomImageEx( src, &rng_state );

                cvPyrDown( src, dst2, IPL_GAUSSIAN_5x5 );

                apply_gaussian( src, src2 );

                cvResize( src2, dst, CV_INTER_NN );
                err0 = cvNorm( dst, dst2, CV_C );

                if( err0 > max_err )
                {
                    max_err   = err0;
                    merr_w    = w;
                    merr_h    = h;
                    merr_iter = i;

                    if( max_err > success_error_level ) goto test_exit;
                }
            }
            ATS_INCREASE( w, img_size_delta_type, img_size_delta );
        }
        ATS_INCREASE( h, img_size_delta_type, img_size_delta );
    }

test_exit:

    src->roi = src2->roi = dst->roi = dst2->roi = 0;

    atsReleaseImage( src  );
    atsReleaseImage( src2 );
    atsReleaseImage( dst  );
    atsReleaseImage( dst2 );

    if( code == TRS_OK )
    {
        trsWrite( ATS_LST,
                  "Maximum error is %g at w = %d, h = %d, iteration = %d",
                   max_err, merr_w, merr_h, merr_iter );

        return max_err <= success_error_level ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad accuracy" );
    }
    else
    {
        trsWrite( ATS_LST,
                  "Fatal error (ROI ver) at w = %d, "
                  "h = %d, iteration = %d",
                  w, h, i );
        return trsResult( TRS_FAIL, "Function returns error code" );
    }
}


/******************************************************************\
*                        PyrUpCmpIPL                               *
\******************************************************************/
static int PyrUpCmpIPL( void* arg )
{
    const int   r = 2;
    int         code = TRS_OK;

    int         param = (int)arg;
    int         depth = param/2 == 2 ? IPL_DEPTH_32F :
                        param/2 == 1 ? IPL_DEPTH_8S  : IPL_DEPTH_8U;
    int         channels = param % 2 == 1 ? 3 : 1;

    int         w = 0, h = 0, i = 0;
    int         merr_w = 0, merr_h = 0, merr_iter = 0; /* where was maximum error */
    double      max_err = 0.;

    int         seed = -1; //atsGetSeed();

    double      success_error_level;
    IplROI      srcRoi, dstRoi;

    IplImage    *src, *dst, *dst2, *dst3, *tmpfl;
    AtsRandState rng_state;
    atsRandInit( &rng_state, 0, 1, seed );

    read_pyramid_params();

    if( !(ATS_RANGE( 0, ud_l, ud_h + 1)       &&
          ATS_RANGE( param/2, dt_l, dt_h + 1) &&
          ATS_RANGE( param%2, ch_l, ch_h + 1)))
          return TRS_UNDEF;

    src = atsCreateImage( max_img_size,   max_img_size, depth, channels, 0 );
    tmpfl = atsCreateImage( max_img_size*2 + r*2,  max_img_size*2 + r*2,
                            IPL_DEPTH_32F, channels, 0 );
    dst = atsCreateImage( max_img_size*2+r*2,max_img_size*2+r*2, depth, channels,0);
    dst2= atsCreateImage( max_img_size*2+r*2, max_img_size*2+r*2, depth, channels, 0 );
    dst3= atsCreateImage( max_img_size*2+r*2, max_img_size*2+r*2, depth, channels, 0 );

    srcRoi.coi = dstRoi.coi = 0;
    srcRoi.xOffset = srcRoi.yOffset  = 0;
    dstRoi.xOffset = dstRoi.yOffset  = r;

    src->roi = &srcRoi;
    dst->roi = dst2->roi = dst3->roi = tmpfl->roi = &dstRoi;

    success_error_level = depth == IPL_DEPTH_32F ? 1e-4 : 1;

    for( h = min_img_size; h <= max_img_size; )
    {
        for( w = min_img_size; w <= max_img_size; )
        {
            int  denom = (w - min_img_size + 1)*
                         (h - min_img_size + 1)*channels*4;
            int  iters = (base_iters*2 + denom)/(2*denom);

            if( iters < 1 ) iters = 1;

            srcRoi.width  = w;
            srcRoi.height = h;
            dstRoi.width  = w*2;
            dstRoi.height = h*2;

            for( i = 0; i < iters; i++ )
            {
                double err0;

                switch( depth )
                {
                case IPL_DEPTH_8U:
                    atsRandSetBounds( &rng_state, 0, img8u_range );
                    break;
                case IPL_DEPTH_8S:
                    atsRandSetBounds( &rng_state, -img8s_range, img8s_range );
                    break;
                case IPL_DEPTH_32F:
                    atsRandSetBounds( &rng_state, -img32f_range, img32f_range );
                    atsRandSetFloatBits( &rng_state, img32f_bits );
                    break;
                default:
                    assert(0);
                    code = TRS_FAIL;
                    goto test_exit;
                }
                atsFillRandomImageEx( src, &rng_state );

                cvPyrUp( src, dst2, IPL_GAUSSIAN_5x5 );

                cvResize( src, dst, CV_INTER_NN );

                clear_even_rows_and_cols( dst );
                cvCvtScale( dst, tmpfl );
                apply_gaussian( tmpfl, tmpfl );
                cvCvtScale( tmpfl, dst3, 4 );

                err0 = cvNorm( dst2, dst3, CV_C );

                if( err0 > max_err )
                {
                    max_err   = err0;
                    merr_w    = w;
                    merr_h    = h;
                    merr_iter = i;

                    if( max_err > success_error_level ) goto test_exit;
                }
            }
            ATS_INCREASE( w, img_size_delta_type, img_size_delta );
        }
        ATS_INCREASE( h, img_size_delta_type, img_size_delta );
    }

test_exit:

    src->roi = dst->roi = dst2->roi = dst3->roi = tmpfl->roi = 0;

    atsReleaseImage( src  );
    atsReleaseImage( dst  );
    atsReleaseImage( dst2 );
    atsReleaseImage( dst3 );
    atsReleaseImage( tmpfl );

    if( code == TRS_OK )
    {
        trsWrite( ATS_LST, " Maximum error is %g at w = %d, h = %d, iteration = %d",
                           max_err, merr_w, merr_h, merr_iter );

        return max_err <= success_error_level ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad accuracy" );
    }
    else
    {
        trsWrite( ATS_LST,
                  "Fatal error (ROI ver) at w = %d, "
                  "h = %d, iteration = %d",
                  w, h, i );
        return trsResult( TRS_FAIL, "Function returns error code" );
    }
}

#define _8U_C1    0
#define _8U_C3    1
#define _8S_C1    2
#define _8S_C3    3
#define _32F_C1   4
#define _32F_C3   5

void InitAPyramids( void )
{
    trsRegArg( funcs[0],  test_desc, atsAlgoClass, PyrUpCmpIPL, _8U_C1 );
    trsRegArg( funcs[0],  test_desc, atsAlgoClass, PyrUpCmpIPL, _8U_C3 );
    trsRegArg( funcs[0],  test_desc, atsAlgoClass, PyrUpCmpIPL, _8S_C1 );
    trsRegArg( funcs[0],  test_desc, atsAlgoClass, PyrUpCmpIPL, _8S_C3 );
    trsRegArg( funcs[0],  test_desc, atsAlgoClass, PyrUpCmpIPL, _32F_C1 );
    trsRegArg( funcs[0],  test_desc, atsAlgoClass, PyrUpCmpIPL, _32F_C3 );

    trsRegArg( funcs[1],  test_desc, atsAlgoClass, PyrDownCmpIPL, _8U_C1 );
    trsRegArg( funcs[1],  test_desc, atsAlgoClass, PyrDownCmpIPL, _8U_C3 );
    trsRegArg( funcs[1],  test_desc, atsAlgoClass, PyrDownCmpIPL, _8S_C1 );
    trsRegArg( funcs[1],  test_desc, atsAlgoClass, PyrDownCmpIPL, _8S_C3 );
    trsRegArg( funcs[1], test_desc, atsAlgoClass, PyrDownCmpIPL, _32F_C1 );
    trsRegArg( funcs[1], test_desc, atsAlgoClass, PyrDownCmpIPL, _32F_C3 );

} /* InitAPyramids */

/* End of file. */

/* End of file. */
