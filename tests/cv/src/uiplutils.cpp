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
#include <math.h>
#include <assert.h>
#include <float.h>

#define  clip(x,a,b)  ((x)<(a) ? (a) : (x) > (b) ? (b) : (x))

void  atsGetImageInfo( IplImage* img, void** pData, int* pStep,
                       CvSize*  pSz, int*  pDepth, int* pChannels,
                       int* pBtPix )
{
    if( !img )
    {
        assert(0);
    }
    else
    {
        int channels = img->nChannels;
        int depth    = img->depth;
        int step     = img->widthStep;
        int bt_pix;

        if( img->origin != IPL_ORIGIN_TL ||
            img->dataOrder != IPL_DATA_ORDER_PIXEL )
        {
            assert(0);
            return;
        }

        bt_pix = ((depth & 255) >> 3) * channels;

        if( pDepth )    *pDepth    = depth;
        if( pChannels ) *pChannels = channels;
        if( pStep )     *pStep     = step;
        if( pBtPix )    *pBtPix    = bt_pix;

        if( pSz )
        {
            pSz->width = img->roi ? img->roi->width : img->width;
            pSz->height= img->roi ? img->roi->height : img->height;
        }

        if( pData )
        {
            *pData = img->imageData + (!img->roi ? 0 :
                     img->roi->yOffset*step + img->roi->xOffset*bt_pix );
        }
    }
}


/*
   Fills the whole image or selected ROI by random numbers.
   Supports only 8u, 8s and 32f formats.
*/
void atsFillRandomImage( IplImage* img, double low, double high )
{
    int      depth, channels, step;
    CvSize  sz;
    void*    data;

    atsGetImageInfo( img, (void**)&data, &step, &sz, &depth, &channels, 0 );
    sz.width *= channels;

    switch( depth )
    {
    case IPL_DEPTH_32F:
    {
        float *fdata = (float*)data;
        int    x, y;

        step /= sizeof(float);
        high = (high-low)/RAND_MAX;
        for( y = 0; y < sz.height; y++, fdata += step )
            for( x = 0; x < sz.width; x++ )
            {
                fdata[x] = (float)(rand()*high + low);
            }
        break;
    }
    case IPL_DEPTH_8U:
    case IPL_DEPTH_8S:
    {
        int l = (int)(low + (low > 0 ? .5 : low == 0 ? 0 : -.5));
        int h = (int)floor(high+(high > 0 ? .5 : high == 0 ? 0 : -.5));
        int x, y;
        uchar *udata = (uchar*)data;

        if( img->depth == IPL_DEPTH_8U )
        {
            l = clip( l, 0, 255 );
            h = clip( h, 0, 255 );
        }
        else
        {
            l = clip( l, -128, 127 );
            h = clip( h, -128, 127 );
        }
        if( l > h ) x = l, l = h, h = x;
        h -= l - 1;

        if( h == 256 ) /* speedup */
        {
            int w1 = sz.width&1;
            for( y = 0; y < sz.height; y++, udata += step )
            {
                for( x = 0; x+1 < sz.width; x += 2 )
                {
                    *((short*)(udata + x)) = (short)rand();
                }
                if( w1 ) udata[sz.width-1] = (uchar)(rand()&255);
            }
        }
        else
        {
            for( y = 0; y < sz.height; y++, udata += step )
                for( x = 0; x < sz.width; x++ )
                {
                    udata[x] = (uchar)(rand()%h + l);
                }
        }
        break;
    }
    default: assert(0);
    }
}


/*
   Fills the whole image or selected ROI by random numbers using 32-bit RNG.
   Supports only 8u, 8s and 32f formats.
*/
void atsFillRandomImageEx( IplImage* img, AtsRandState* state )
{
    int      depth, channels, step;
    CvSize  sz;
    void*    _data;

    atsGetImageInfo( img, (void**)&_data, &step, &sz, &depth, &channels, 0 );
    sz.width *= channels;

    switch( depth )
    {
    case IPL_DEPTH_32F:
    {
        float *data = (float*)_data;
        int    y;

        step /= sizeof(data[0]);

        for( y = 0; y < sz.height; y++, data += step )
        {
            atsbRand32f( state, data, sz.width );
        }
        break;
    }
    case IPL_DEPTH_8U:
    case IPL_DEPTH_8S:
    {
        uchar *data = (uchar*)_data;
        int    y;

        step /= sizeof(data[0]);

        for( y = 0; y < sz.height; y++, data += step )
        {
            atsbRand8u( state, data, sz.width );
        }
        break;
    }
    case IPL_DEPTH_16S:
    {
        short *data = (short*)_data;
        int    y;

        step /= sizeof(data[0]);

        for( y = 0; y < sz.height; y++, data += step )
        {
            atsbRand16s( state, data, sz.width );
        }
        break;
    }
    case IPL_DEPTH_32S:
    {
        int  *data = (int*)_data;
        int   y;

        step /= sizeof(data[0]);

        for( y = 0; y < sz.height; y++, data += step )
        {
            atsbRand32s( state, data, sz.width );
        }
        break;
    }
    default: assert(0);
    }
}


/* Allocates the IPL image and (may be) clears it */
IplImage*  atsCreateImage( int w, int h, int depth, int nch, int clear_flag )
{
    IplImage *img = cvCreateImage( cvSize( w, h ), depth, nch );
    if( clear_flag )
        cvZero( img );
    return img;
}


void atsReleaseImage( IplImage* img )
{
    assert( img->roi == 0 && img->maskROI == 0 );
    cvReleaseImage( &img );
}


/* extracts ROI data from the image and writes it in a single row */
void atsGetDataFromImage( IplImage* img, void* data )
{
    char*    src;
    char*    dst = (char*)data;
    int      bt_pix, step;
    CvSize  sz;
    int      y;

    atsGetImageInfo( img, (void**)&src, &step, &sz, 0, 0, &bt_pix );
    sz.width *= bt_pix;

    for( y = 0; y < sz.height; y++, src += step, dst += sz.width )
    {
        memcpy( dst, src, sz.width );
    }
}

/* writes linear data to the image ROI */
void atsPutDataToImage( IplImage *img, void *data )
{
    char    *src = (char*)data, *dst;
    int      bt_pix, step;
    CvSize  sz;
    int      y;

    atsGetImageInfo( img, (void**)&dst, &step, &sz, 0, 0, &bt_pix );
    sz.width *= bt_pix;

    for( y = 0; y < sz.height; y++, dst += step, src += sz.width )
    {
        memcpy( dst, src, sz.width );
    }
}


void atsConvert( IplImage* src, IplImage* dst )
{
    cvConvert( src, dst );
}


/* The function draws line in 8uC1/C3 image */
void  atsDrawLine( IplImage* img, float x1, float y1, float x2, float y2, int color )
{
    float    dx = x2 - x1;
    float    dy = y2 - y1;
    float    adx = (float)fabs(dx);
    float    ady = (float)fabs(dy);
    float    steps = 0;
    uchar*   img_data;
    int      img_step;
    CvSize  sz;
    int      depth = 0, ch = 0, bt_pix = 0;

    uchar    b = (uchar)(color & 0xff);
    uchar    g = (uchar)((color >> 8) & 0xff);
    uchar    r = (uchar)((color >> 16) & 0xff);

    atsGetImageInfo( img, (void**)&img_data, &img_step, &sz, &depth, &ch, &bt_pix );

    assert( depth == IPL_DEPTH_8U );
    assert( ch == 1 || ch == 3);

    if( adx > ady )
    {
        dy /= adx;
        dx = dx > 0 ? 1.f : -1.f;
        steps = adx;
    }
    else if( ady != 0 )
    {
        dx /= ady;
        dy = dy > 0 ? 1.f : -1.f;
        steps = ady;
    }
    else
    {
        dx = dy = 0;
    }

    do
    {
        int x = cvRound(x1);
        int y = cvRound(y1);

        if( (unsigned)x < (unsigned)sz.width &&
            (unsigned)y < (unsigned)sz.height )
        {
            uchar* data = img_data + y*img_step + x*bt_pix;
            data[0] = b;
            if( ch == 3 )
            {
                data[1] = g;
                data[2] = r;
            }
        }

        x1 += dx;
        y1 += dy;
        steps--;
    }
    while( steps >= 0 );
}


/* The function draws ellipse arc in 8uC1/C3 image */
void  atsDrawEllipse( IplImage* img, float xc, float yc, float a, float b,
                      float angle, float arc0, float arc1, int color )
{
    assert( a >= b );
    if( a == 0 ) a = 0.1f;
    if( b == 0 ) b = 0.1f;
    {
    double  ba = b/a;
    double  e = sqrt( 1. - ba*ba);
    double  mag = b*ba;
    double  alpha, beta;
    double  c = a*e;
    double  x, y;
    double  a0 = arc0, a1 = arc1;
    double  ang;
    int     is_pt = arc0 == arc1;

    ang  = angle*CV_PI/180.;
    alpha = cos(ang);
    beta  = sin(ang);

    if( a0 > a1 )
    {
        double temp = a0;
        a0 = a1;
        a1 = temp;
    }

    if( a1 - a0 >= 360 )
    {
        a0 = a1 = 0;
    }

    a0 *= CV_PI/180.;
    a1 *= CV_PI/180.;

    x = c + a*cos(a0);
    y = b*sin(a0);
    a0 = atan2( y, x )*180/CV_PI;
    if( a0 < 0 ) a0 += 360.;

    if( is_pt )
        a0 = a1;
    else
    {
        x = c + a*cos(a1);
        y = b*sin(a1);
        a1 = atan2( y, x )*180/CV_PI;
        if( a1 < 0 ) a1 += 360.f;
        if( a0 > a1 - 0.1 ) a0 -= 360;
    }

    xc = (float)( xc - c*alpha);
    yc = (float)( yc + c*beta);

    atsDrawConic( img, xc, yc, (float)mag, (float)e, angle, (float)a0, (float)a1, color );
    }
}


/* The function draws conic arc in 8uC1/C3 image */
void  atsDrawConic( IplImage* img, float xc, float yc, float mag, float e,
                    float angle, float arc0, float arc1, int color )
{
    int delta = 1;
    double  dang = CV_PI*delta/180;
    double  alpha, beta;
    double  da = cos(dang), db = sin(dang);
    double  a, b;
    float   x1 = 0.f, y1 = 0.f;
    int     i, n, fl = 0;

    assert( mag > 0 && e >= 0 );

    angle *= (float)(CV_PI/180.);
    alpha = cos(angle);
    beta  = sin(angle);

    if( arc0 > arc1 )
    {
        float temp = arc0;
        arc0 = arc1;
        arc1 = temp;
    }

    n = cvRound( arc1 - arc0 );
    if( n > 360 ) n = 360;

    arc0 *= (float)(CV_PI/180.);

    a = cos( arc0 );
    b = sin( arc0 );

    for( i = 0; i < n + delta; i += delta )
    {
        double d;

        if( i > n )
        {
            arc1 *= (float)(CV_PI/180.);
            a = cos(arc1);
            b = sin(arc1);
        }

        d = 1 - e*a;
        if( d != 0 )
        {
            double r = mag/d;
            double x = r*a;
            double y = r*b;
            float  x2 = (float)(xc + x*alpha - y*beta);
            float  y2 = (float)(yc - x*beta - y*alpha);
            if( fl ) atsDrawLine( img, x1, y1, x2, y2, color );
            x1 = x2;
            y1 = y2;
            fl = 1;
        }
        else
        {
            fl = 0;
        }
        d = a*da - b*db;
        b = a*db + b*da;
        a = d;
    }
}


static void _atsCalcConicPoint( double xc, double yc,
                                double mag, double e, double alpha,
                                double beta, double ang, CvPoint* pt )
{
    double a = cos( ang );
    double b = sin( ang );

    double d = 1 - e*a;
    if( d == 0 )
    {
        pt->x = pt->y = -10000;
    }
    else
    {
        d = mag/d;
        a *= d;
        b *= d;
        pt->x = cvRound( xc + alpha*a - beta*b );
        pt->y = cvRound( yc - beta*a - alpha*b );
    }
}


int  atsCalcQuadricCoeffs( double xc, double yc, double mag, double e,
                           double angle, double arc0, double arc1,
                           double* _A, double* _B, double* _C, double* _D, double* _E,
                           CvPoint* pt1, CvPoint* pt2 )
{
    double ang = angle*CV_PI/180;
    double alpha = cos( ang );
    double beta = sin( ang );
    double alal = alpha*alpha;
    double bebe = beta*beta;
    double albe = alpha*beta;
    double a = 0, c = 0;
    double A = 0, B = 0, C = 0, D = 0, E = 0;
    double dx;
    int    code = 0;
    double cf_max = 0;
    /*double arcm; */
    /*int    oct1, oct2; */
    /*CvPoint ptm; */

    assert( mag > 0 && e >= 0 );

    if( arc0 > arc1 )
    {
        double temp = arc0;
        arc0 = arc1;
        arc1 = temp;
    }

    if( arc1 - arc0 > 360 )
    {
        arc0 = 0;
        arc1 = 360;
    }

    if( e != 1 ) /* non-parabolic case */
    {
        a = mag/fabs(e*e - 1);
        c = sqrt( mag*a );
        dx = a*e;
        a = 1./(a*a);
        c = 1./(c*c);
        D = -2*dx*a;
        if( e > 1 )
        {
            c = -c;
            D = -D;
        }
    }
    else
    {
        c = 1;
        dx = mag*0.5;
        D = -mag*2;
    }

    E = -beta*D;
    D *= alpha;

    /* rotate matrix */
    A = alal*a + bebe*c;
    B = albe*(c - a)*2;
    C = bebe*a + alal*c;

    /* move to (xc, yc) */
    D -= (2*A*xc + B*yc);
    E -= (B*xc + 2*C*yc);

    arc0 *= CV_PI/180;
    arc1 *= CV_PI/180;

    _atsCalcConicPoint( xc, yc, mag, e, alpha, beta, arc0, pt1 );
    if( pt1->x == -10000 ) code--;

    _atsCalcConicPoint( xc, yc, mag, e, alpha, beta, arc1, pt2 );
    if( pt2->x == -10000 ) code--;

    if( pt1->x == pt2->x && pt1->y == pt2->y )
    {
        if( arc1 - arc0 < 1.f ) code--;
    }

    if( fabs(A) > cf_max ) cf_max = fabs(A);
    if( fabs(B) > cf_max ) cf_max = fabs(B);
    if( fabs(C) > cf_max ) cf_max = fabs(C);
    if( fabs(D) > cf_max ) cf_max = fabs(D);
    if( fabs(E) > cf_max ) cf_max = fabs(E);

    cf_max = (1 << 20)/cf_max;

    *_A = A*cf_max;
    *_B = B*cf_max;
    *_C = C*cf_max;
    *_D = D*cf_max;
    *_E = E*cf_max;

    return code;
}


void  atsClearBorder( IplImage* img )
{
    uchar* data = 0;
    int    i, step = 0;
    CvSize size;
    
    assert( img->depth == IPL_DEPTH_8U && img->nChannels == 1 );
    atsGetImageInfo( img, (void**)&data, &step, &size, 0, 0, 0 );
    
    /* clear bounds */
    for( i = 0; i < size.width; i++ )
        data[i] = data[(size.height-1)*step + i] = 0;

    for( i = 0; i < size.height; i++ )
        data[i*step] = data[i*step + size.width - 1] = 0;
}


void  atsGenerateBlobImage( IplImage* img, int min_blob_size, int max_blob_size,
                            int blob_count, int min_brightness, int max_brightness,
                            AtsRandState* rng_state )
{
    uchar* data;
    int    i, step;
    CvSize size;

    assert( img->depth == IPL_DEPTH_8U && img->nChannels == 1 );
    atsGetImageInfo( img, (void**)&data, &step, &size, 0, 0, 0 );
    cvZero( img );

    for( i = 0; i < blob_count; i++ )
    {
        CvPoint center;
        CvSize  axes;
        int angle = atsRandPlain32s( rng_state ) % 180;
        int brightness = atsRandPlain32s( rng_state ) %
                         (max_brightness - min_brightness) + min_brightness;
        center.x = atsRandPlain32s( rng_state ) % size.width;
        center.y = atsRandPlain32s( rng_state ) % size.height;
        axes.width = (atsRandPlain32s( rng_state ) %
                     (max_blob_size - min_blob_size) + min_blob_size + 1)/2;
        axes.height = (atsRandPlain32s( rng_state ) %
                      (max_blob_size - min_blob_size) + min_blob_size + 1)/2;
        
        cvEllipse( img, center, axes, angle, 0, 360, cvScalar(brightness), CV_FILLED ); 
    }
}


/* patch */
void* icvAlloc_( int lSize )
{
    size_t pPointer = (size_t)malloc( lSize + 64 + 4);
    
    if( pPointer == 0 ) return 0;
    *(size_t*)(((pPointer + 32 + 4) & 0xffffffe0) - 4) = pPointer;
    return (void*)((pPointer + 32 + 4) & 0xffffffe0);
} /* icvAlloc */


void  icvFree_( void** ppPointer )
{
    void* pPointer;
    
    /* Checking for bad pointer */
    if( ppPointer && *ppPointer )
    {
        pPointer = *ppPointer;
        *ppPointer = 0;

        /* Pointer must be align by 32 */
        assert( ((size_t)pPointer & 0x1f) == 0 );
    
        free( (void*)*(size_t*)((size_t)pPointer - 4) );
    }
}


CvPoint  atsFindFirstErr( IplImage* imgA, IplImage* imgB, double eps )
{
    uchar* imgA_data = 0;
    uchar* imgB_data = 0;
    int imgA_step = 0;
    int imgB_step = 0;
    CvSize imgA_size;
    CvSize imgB_size;
    int x = 0, y = 0;

    cvGetImageRawData( imgA, &imgA_data, &imgA_step, &imgA_size );
    cvGetImageRawData( imgB, &imgB_data, &imgB_step, &imgB_size );

    assert( imgA_size.width == imgB_size.width && imgA_size.height == imgB_size.height );
    assert( imgA->depth == imgB->depth && imgA->nChannels == imgB->nChannels );

    imgA_size.width *= imgA->nChannels;

    for( y = 0; y < imgA_size.height; y++, imgA_data += imgA_step, imgB_data += imgB_step )
    {
        switch( imgA->depth )
        {
        case IPL_DEPTH_8U:
            for( x = 0; x < imgA_size.width; x++ )
            {
                int t = imgA_data[x] - imgB_data[x];
                if( abs(t) > eps )
                    goto exit_func;
            }
            break;
        case IPL_DEPTH_8S:
            for( x = 0; x < imgA_size.width; x++ )
            {
                int t = ((char*)imgA_data)[x] - ((char*)imgB_data)[x];
                if( abs(t) > eps )
                    goto exit_func;
            }
            break;
        case IPL_DEPTH_16S:
            for( x = 0; x < imgA_size.width; x++ )
            {
                int t = ((short*)imgA_data)[x] - ((short*)imgB_data)[x];
                if( abs(t) > eps )
                    goto exit_func;
            }
            break;
        case IPL_DEPTH_32S:
            for( x = 0; x < imgA_size.width; x++ )
            {
                int t = ((int*)imgA_data)[x] - ((int*)imgB_data)[x];
                if( abs(t) > eps )
                    goto exit_func;
            }
            break;
        case IPL_DEPTH_32F:
            for( x = 0; x < imgA_size.width; x++ )
            {
                float t = ((float*)imgA_data)[x] - ((float*)imgB_data)[x];
                if( fabs(t) > eps )
                    goto exit_func;
            }
            break;
        default:
            assert(0);
        }
    }

exit_func:

    return x < imgA_size.width && y < imgA_size.height ?
           cvPoint( x / imgA->nChannels, y ) : cvPoint( -1, -1 );
}


CvPoint atsRandPoint( AtsRandState* rng_state, CvSize size )
{
    CvPoint pt;

    pt.x = atsRandPlain32s( rng_state ) % size.width;
    pt.y = atsRandPlain32s( rng_state ) % size.height;

    return pt;
}


CvPoint2D32f atsRandPoint2D32f( AtsRandState* rng_state, CvSize size )
{
    CvPoint2D32f pt;

    pt.x = (float)(atsRandPlain32s( rng_state ) % size.width);
    pt.y = (float)(atsRandPlain32s( rng_state ) % size.height);

    return pt;
}

/* general-purpose saturation macros */ 
#define ATS_CAST_8U(t)    (uchar)( !((t) & ~255) ? (t) : (t) > 0 ? 255 : 0)
#define ATS_CAST_8S(t)    (char)( !(((t)+128) & ~255) ? (t) : (t) > 0 ? 127 : -128 )
#define ATS_CAST_16S(t)   (short)( !(((t)+32768) & ~65535) ? (t) : (t) > 0 ? 32767 : -32768 )
#define ATS_CAST_32S(t)   (int)(t)
#define ATS_CAST_64S(t)   (int64)(t)
#define ATS_CAST_32F(t)   (float)(t)
#define ATS_CAST_64F(t)   (double)(t)


/* End of file. */
