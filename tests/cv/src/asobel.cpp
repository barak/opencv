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
#include <stdint.h>

static char* funcs[] =
{
    "cvLaplace",
    "cvSobel",
    "cvScharr"
};

static char *test_desc = "Test for absolute difference functions";

/* actual parameters */
static int min_img_size, max_img_size;
static int base_iters;
static int min_aperture = 3, max_aperture = 7;

static int init_derv_params = 0;

static const int img8u_range = 256;
static const int img8s_range = 128;
static const float img32f_range = 1.f;
static const int img32f_bits = 23;

static void read_derv_params( void )
{
    if( !init_derv_params )
    {
        /* read tests params */
        trsiRead( &min_img_size, "1", "Minimal width or height of image" );
        trsiRead( &max_img_size, "300", "Maximal width or height of image" );
        trsiRead( &base_iters, "100", "Base number of iterations" );

        init_derv_params = 1;
    }
}

static int derv_test( void* arg )
{
    const double success_error_level = 1e-3;

    int   param = (int)(intptr_t)arg;
    int   depth = param % 2, dst_depth;
    int   func = param / 2;
    int   max_order = func != 2 ? 3 : 2;

    int   seed  = atsGetSeed();

    /* position where the maximum error occured */
    int   i, merr_iter = 0;

    /* test parameters */
    double  max_err = 0.;
    int     code = TRS_OK;

    IplROI      roi;
    IplImage    *src_img, *dst_img, *dst2_img;
    IplImage    *srcfl_img, *dstfl_img, *dstfl2_img;
    AtsRandState rng_state;

    float KerX[32];
    float KerY[32];
	CvSize KerLens;
	IplConvKernelFP* KX = 0;
	IplConvKernelFP* KY = 0;
    
    const char* msg = "no errors";

    atsRandInit( &rng_state, 0, 1, seed );

    read_derv_params();

    depth = depth == 0 ? IPL_DEPTH_8U : IPL_DEPTH_32F;
    dst_depth = depth == IPL_DEPTH_32F ? IPL_DEPTH_32F : IPL_DEPTH_16S;

    src_img = atsCreateImage( max_img_size, max_img_size, depth, 1, 0 );
    dst_img = atsCreateImage( max_img_size, max_img_size, dst_depth, 1, 0 );
    dst2_img = atsCreateImage( max_img_size, max_img_size, dst_depth, 1, 0 );

    srcfl_img = atsCreateImage( max_img_size, max_img_size, IPL_DEPTH_32F, 1, 0 );
    dstfl_img = atsCreateImage( max_img_size, max_img_size, IPL_DEPTH_32F, 1, 0 );
    dstfl2_img = atsCreateImage( max_img_size, max_img_size, IPL_DEPTH_32F, 1, 0 );

    src_img->roi = dst_img->roi = dst2_img->roi = 
    srcfl_img->roi = dstfl_img->roi = dstfl2_img->roi = &roi;

    roi.coi = 0;
    roi.xOffset = roi.yOffset = 0;

    for( i = 0; i < base_iters; i++ )
    {
        int w = atsRandPlain32s( &rng_state ) % (max_img_size - min_img_size + 1) + min_img_size;
        int h = atsRandPlain32s( &rng_state ) % (max_img_size - min_img_size + 1) + min_img_size;
        //int aperture = atsRandPlain32s( &rng_state ) % (max_aperture - min_aperture + 1) + min_aperture;
        int aperture = 7;
        int dx = 0, dy = 0;
        double err;

        aperture |= 1;

        roi.width = w;
        roi.height = h;

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
        }

        atsFillRandomImageEx( src_img, &rng_state );
        atsConvert( src_img, srcfl_img );

        switch( func )
        {
        case 0:
            dx = 2;
            dy = 0;
            cvLaplace( src_img, dst_img, aperture );
            break;
        case 1:
            dx = atsRandPlain32s( &rng_state ) % max_order;
            dy = atsRandPlain32s( &rng_state ) % (3 - dx);
            cvSobel( src_img, dst_img, dx, dy, aperture );
            break;
        case 2:
            aperture = CV_SCHARR;
            if( atsRandPlain32s( &rng_state ) & 128 )
                dx = 1, dy = 0;
            else
                dx = 0, dy = 1;
            cvSobel( src_img, dst_img, dx, dy, aperture );
            break;
        default:
            assert(0);
            msg = "Invalid function";
            code = TRS_FAIL;
            goto test_exit;
        }

        atsCalcKernel(IPL_DEPTH_32F,dx,dy,aperture,(char*)KerX,(char*)KerY,&KerLens,CV_ORIGIN_TL);
	    KX = atsCreateConvKernelFP(KerLens.width,1,KerLens.width/2,0,KerX);
        KY = atsCreateConvKernelFP(1,KerLens.width,0,KerLens.width/2,KerY);

        atsConvolveSep2D( srcfl_img, dstfl_img, KX, KY );

        if( func == 0 )
        {
            int t;
            CV_SWAP( KX->nRows, KX->nCols, t );
            CV_SWAP( KX->anchorX, KX->anchorY, t );

            CV_SWAP( KY->nRows, KY->nCols, t );
            CV_SWAP( KY->anchorX, KY->anchorY, t );

            atsConvolveSep2D( srcfl_img, dstfl2_img, KY, KX );
            cvAdd( dstfl_img, dstfl2_img, dstfl_img );
        }

        atsConvert( dstfl_img, dst2_img );

        err = cvNorm( dst2_img, dst_img, CV_C );

        if( err > max_err )
        {
            merr_iter = i;
            max_err = err;
            if( max_err > success_error_level )
                goto test_exit;
        }
    }

test_exit:

    src_img->roi = dst_img->roi = dst2_img->roi =
    srcfl_img->roi = dstfl_img->roi = dstfl2_img->roi = 0;
    
    cvReleaseImage( &src_img );
    cvReleaseImage( &dst_img );
    cvReleaseImage( &dst2_img );

    cvReleaseImage( &srcfl_img );
    cvReleaseImage( &dstfl_img );
    cvReleaseImage( &dstfl2_img );

    trsWrite( ATS_LST, "Max err is %g at "
                       "iter = %d, seed = %08x",
                       max_err, merr_iter, seed );

    return code == TRS_FAIL ? trsResult( TRS_FAIL, "Test failed" ) :
        max_err <= success_error_level ? trsResult( TRS_OK, "No errors" ) :
                                         trsResult( TRS_FAIL, "Bad accuracy" );
}


#define LAPLACE_8UC1    0
#define LAPLACE_32FC1   1

#define SOBEL_8UC1      2
#define SOBEL_32FC1     3

#define SCHARR_8UC1     4
#define SCHARR_32FC1    5

void InitADerv( void )
{
    /* Registering test functions */
    trsRegArg( funcs[0], test_desc, atsAlgoClass, derv_test, LAPLACE_8UC1 );
    trsRegArg( funcs[0], test_desc, atsAlgoClass, derv_test, LAPLACE_32FC1 );

    trsRegArg( funcs[1], test_desc, atsAlgoClass, derv_test, SOBEL_8UC1 );
    trsRegArg( funcs[1], test_desc, atsAlgoClass, derv_test, SOBEL_32FC1 );

    trsRegArg( funcs[2], test_desc, atsAlgoClass, derv_test, SCHARR_8UC1 );
    trsRegArg( funcs[2], test_desc, atsAlgoClass, derv_test, SCHARR_32FC1 );

} /* InitADerv */

/* End of file. */
