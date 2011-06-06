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
#include "_cv.h"
#include "cv.hpp"

/****************************************************************************************\
*                                CvImage implementation                                  *
\****************************************************************************************/
CvImage::CvImage()
{
    cvInitImageHeader( this, cvSize( 1, 1 ), 8, 1, 0, 4 );
}

CvImage::CvImage( CvSize _size, int _depth, int _channels )
{
    cvInitImageHeader( this, _size, _depth, _channels, 0, 4 );
}

CvImage::~CvImage()
{
    if( roi )
        cvResetImageROI( this );
    if( imageData )
        cvReleaseImageData( this );
}

CvImage & CvImage::copy( const CvImage & another )
{
    if( !imageData || width != another.width || height != another.height )
    {
        cvReleaseData( this );
        cvInitImageHeader( this, cvSize( another.width, another.height),
                           another.depth, another.nChannels, another.origin,
                           another.align );
        cvCreateImageData( this );
    }

    cvCopy( (IplImage *) & another, this );
    return *this;
}

int
CvImage::byte_per_pixel() const
{
    return ( (depth & 255) >> 3 ) * nChannels;
}

uchar *
CvImage::image_data()
{
    return ( uchar * ) (!roi ? imageData : imageData + roi->yOffset * widthStep +
                        roi->xOffset * byte_per_pixel());
}

const uchar *
CvImage::image_data() const
{
    return ( const uchar * ) (((CvImage *) this)->image_data());
}

CvSize
CvImage::image_roi_size() const
{
    return !roi ? cvSize( width, height ) : cvSize( roi->width, roi->height );
}
