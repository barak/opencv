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
#include <limits.h>
#include <float.h>

static char* funcs[] =
{
    "cvIntegrateImage"
};

static char *test_desc[] =
{
    "Comparison with simple etalon functions"
};

/* actual parameters */
static int min_img_size, max_img_size;
static int max_dct_size;
static int base_iters;

static int init_haar_params = 0;

static const int img8u_range = 256;

static void read_haar_params( void )
{
    if( !init_haar_params )
    {
        /* read tests params */
        trsiRead( &min_img_size, "10", "Minimal width or height of image" );
        trsiRead( &max_img_size, "300", "Maximal width or height of image" );
        trsiRead( &base_iters, "50", "Base number of iterations" );

        init_haar_params = 1;
    }
}

static void intergate_etalon( IplImage* img, IplImage* sum,
                              IplImage* sqsum, IplImage* tilted )
{
    int x, y;
    int step = img->widthStep/sizeof(uchar);
    int sstep = sum->widthStep/sizeof(int);
    int sqstep = sqsum->widthStep/sizeof(double);
    int tstep = tilted->widthStep/sizeof(int);
    CvSize size = cvGetSize( img );

    memset( sum->imageData, 0, sum->widthStep );
    memset( sqsum->imageData, 0, sqsum->widthStep );
    memset( tilted->imageData, 0, tilted->widthStep );

    for( y = 0; y < size.height; y++ )
    {
        uchar* data = (uchar*)img->imageData + y*step;
        int* sdata = (int*)sum->imageData + (y+1)*sstep;
        int* tdata = (int*)tilted->imageData + (y+1)*tstep;
        double* sqdata = (double*)sqsum->imageData + (y+1)*sqstep;
        int  s = 0, sq = 0;

        for( x = 0; x <= size.width; x++ )
        {
            int t = x > 0 ? data[x-1] : 0, ts = t;
            s += t;
            sq += t*t;
            
            sdata[x] = s + sdata[x - sstep];
            sqdata[x] = sq + sqdata[x - sqstep];
            
            if( x == 0 )
                ts += tdata[-tstep+1];
            else
            {
                ts += tdata[x-tstep-1];
                if( y > 0 )
                {
                    ts += data[x-step-1];
                    if( x < size.width )
                        ts += tdata[x-tstep+1] - tdata[x-tstep*2];
                }
            }

            tdata[x] = ts;
        }
    }
}


static int haar_test( void )
{
    const double success_error_level = 0;

    int   seed = atsGetSeed();
    int   code = TRS_OK;

    /* position where the maximum error occured */
    int   i, merr_iter = 0;

    /* test parameters */
    double  max_err = 0.;

    IplImage *img = 0;
    IplImage *sum1 = 0, *sum2 = 0;
    IplImage *sqsum1 = 0, *sqsum2 = 0;
    IplImage *tsum1 = 0, *tsum2 = 0;
    AtsRandState rng_state;

    atsRandInit( &rng_state, 0, img8u_range, seed );

    read_haar_params();

    for( i = 0; i < base_iters; i++ )
    {
        CvSize size;
        CvSize sumsize;
        double err = 0, err1 = 0, err2 = 0;
        int64 t;
        IplImage* sqsumptr = 0, *tiltedptr = 0;

        size.width = atsRandPlain32s( &rng_state ) %
                     (max_img_size - min_img_size + 1) + min_img_size;
        size.height = atsRandPlain32s( &rng_state ) %
                     (max_img_size - min_img_size + 1) + min_img_size;
        //size.width = 3;
        //size.height = 3;
        sumsize.width = size.width + 1;
        sumsize.height = size.height + 1;

        img = cvCreateImage( size, IPL_DEPTH_8U, 1 );
        sum1 = cvCreateImage( sumsize, IPL_DEPTH_32S, 1 );
        tiltedptr = tsum1 = cvCloneImage( sum1 );
        sum2 = cvCloneImage( sum1 );
        tsum2 = cvCloneImage( sum1 );
        sqsumptr = sqsum1 = cvCreateImage( sumsize, IPL_DEPTH_64F, 1 );
        sqsum2 = cvCloneImage( sqsum1 );

        atsFillRandomImageEx( img, &rng_state );

        switch( atsRandPlain32s( &rng_state ) % 10 )
        {
        case 0:
        case 1:
             sqsumptr = tiltedptr = 0;
             break;
        case 2:
        case 3:
             tiltedptr = 0;
             break;
        }

        intergate_etalon( img, sum2, sqsum2, tsum2 );
        t = atsGetTickCount();
        OPENCV_CALL( cvIntegral( img, sum1, sqsumptr, tiltedptr ));
        t = atsGetTickCount() - t;

        trsWrite( ATS_LST, "cpe: %g\n", ((double)t)/(size.width*size.height));

        err = cvNorm( sum1, sum2, CV_C );
        if( tiltedptr )
        {
            err1 = cvNorm( tsum1, tsum2, CV_C );
            err = MAX( err, err1 );
        }
        if( sqsumptr )
        {
            err2 = cvNorm( sqsum1, sqsum2, CV_C );
            err = MAX( err, err2 );
        }

        cvReleaseImage( &img );
        cvReleaseImage( &sum1 );
        cvReleaseImage( &sum2 );
        cvReleaseImage( &tsum1 );
        cvReleaseImage( &tsum2 );
        cvReleaseImage( &sqsum1 );
        cvReleaseImage( &sqsum2 );

        if( err > max_err )
        {
            merr_iter = i;
            max_err = err;
            if( max_err > success_error_level )
                goto test_exit;
        }
    }

test_exit:

    cvReleaseImage( &img );
    cvReleaseImage( &sum1 );
    cvReleaseImage( &sum2 );
    cvReleaseImage( &tsum1 );
    cvReleaseImage( &tsum2 );
    cvReleaseImage( &sqsum1 );
    cvReleaseImage( &sqsum2 );

    if( code == TRS_OK )
    {
        trsWrite( ATS_LST, "Max err is %g at iter = %d, seed = %08x",
                           max_err, merr_iter, seed );

        return max_err <= success_error_level ?
            trsResult( TRS_OK, "No errors" ) :
            trsResult( TRS_FAIL, "Bad accuracy" );
    }
}


void InitAHaar( void )
{
    /* Register test functions */
    trsReg( funcs[0], test_desc[0], atsAlgoClass, haar_test );

} /* InitAMoments */

/* End of file. */

