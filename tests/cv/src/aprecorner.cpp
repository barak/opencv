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


/* Testing parameters */
static char TestName[] = "Calculating the Convolution with Sobel operators";
static char TestClass[] = "Algorithm";
static int lImageWidth;
static int lImageHeight;
static int  kerSize;
static int  lBlockSize;

static int  read_param = 0;
static int  data_types = 0;
static double EPSILON = 0.01;

static int preCorner(IplImage* src, IplImage* dst, int ker)
{
    float GaussKer[15];
    float DiffKer1[15];
    float DiffKer2[15];
	float denom = 1;

    CvSize kerSize;
    IplImage* fldstX;
    IplImage* fldstXX;
    IplImage* fldstY;
    IplImage* fldstXY;
    IplImage* fldstYY;
    IplImage* tmp;
	
    IplConvKernelFP* GaussKerX;
    IplConvKernelFP* GaussKerY;
    IplConvKernelFP* DiffKernelX1;
    IplConvKernelFP* DiffKernelY1;
    IplConvKernelFP* DiffKernelX2;
    IplConvKernelFP* DiffKernelY2;

	kerSize.width=kerSize.height=ker;
   
    /* Creating images for testing */
   
    for(int i = 0; i < ker-1;i++)denom *= 2;
    denom = denom*denom * denom *255*255;
    denom=1.0f/denom;

    tmp = atsCreateImage(lImageWidth, lImageHeight, IPL_DEPTH_32F, 1, 0);
    fldstX = atsCreateImage(lImageWidth, lImageHeight, IPL_DEPTH_32F,1,0);
    fldstXX = atsCreateImage(lImageWidth, lImageHeight, IPL_DEPTH_32F,1,0);
    fldstY = atsCreateImage(lImageWidth, lImageHeight, IPL_DEPTH_32F,1,0);
    fldstXY = atsCreateImage(lImageWidth, lImageHeight, IPL_DEPTH_32F,1,0);
    fldstYY = atsCreateImage(lImageWidth, lImageHeight, IPL_DEPTH_32F,1,0);

    /* Calculating the kernels */
    atsCalcKernel(IPL_DEPTH_32F,0,1,ker,(char*)GaussKer,(char*)DiffKer1,&kerSize,CV_ORIGIN_TL);
	
    GaussKerX = atsCreateConvKernelFP(kerSize.width,1,kerSize.width/2,0,GaussKer);
    GaussKerY = atsCreateConvKernelFP(1,kerSize.width,0,kerSize.width/2,GaussKer);
    DiffKernelX1 = atsCreateConvKernelFP(kerSize.height,1,kerSize.height/2,0,DiffKer1);
    DiffKernelY1 = atsCreateConvKernelFP(1,kerSize.height,0,kerSize.height/2,DiffKer1);

    atsCalcKernel(IPL_DEPTH_32F,0,2,ker,(char*)GaussKer,(char*)DiffKer2,&kerSize,CV_ORIGIN_TL);
    DiffKernelX2 = atsCreateConvKernelFP(kerSize.height,1,kerSize.height/2,0,DiffKer2);
    DiffKernelY2 = atsCreateConvKernelFP(1,kerSize.height,0,kerSize.height/2,DiffKer2);

    atsConvolveSep2D(src,fldstX,DiffKernelX1,GaussKerY);
    atsConvolveSep2D(src,fldstY,GaussKerX,DiffKernelY1);
    atsConvolveSep2D(src,fldstXX,DiffKernelX2,GaussKerY);
    atsConvolveSep2D(src,fldstYY,GaussKerX,DiffKernelY2);
    atsConvolveSep2D(src,fldstXY,DiffKernelX1,DiffKernelY1);


    cvMul( fldstX, fldstX, tmp);
    cvMul( tmp, fldstYY, dst);

    cvMul( fldstY, fldstY, tmp);
    cvMul( tmp, fldstXX, tmp );
    cvAdd(dst, tmp, dst);

    cvMul(fldstY, fldstX, tmp);
    cvScale(tmp, tmp, 2);
    cvMul (tmp, fldstXY, tmp);
    cvSub(dst, tmp, dst);
	cvScale(dst,dst,denom);

   atsDeleteConvKernelFP(GaussKerY);
   atsDeleteConvKernelFP(GaussKerX);
   atsDeleteConvKernelFP(DiffKernelX1);
   atsDeleteConvKernelFP(DiffKernelY1);
   atsDeleteConvKernelFP(DiffKernelX2);
   atsDeleteConvKernelFP(DiffKernelY2);

   cvReleaseImage( &tmp );
   cvReleaseImage( &fldstX );
   cvReleaseImage( &fldstXX );
   cvReleaseImage( &fldstY );
   cvReleaseImage( &fldstXY );
   cvReleaseImage( &fldstYY );

   return 0;

}



static int fcaPreCorner( void )
{
    long lErrors = 0;
    IplImage* src8u;
    IplImage* src32f;
    IplImage* tmpsrc;
    IplImage* eigenv32f;
    IplImage* testdst;

    
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Determining which test are needed to run */
        trsCaseRead( &data_types,"/u/s/f/a", "a",
                    "u - unsigned char, s - signed char, f - float, a - all" );
        /* Reading test-parameters */
        trsiRead( &lImageHeight, "100", "Image height");
        trsiRead( &lImageWidth, "100", "Image width");
        trsiRead( &kerSize,"7","Size of operator");
        
    }
    if( data_types != 3 && data_types != 0 ) return TRS_UNDEF;

    
    /* Creating images for testing */
    src8u = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
	src32f = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    tmpsrc = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
	eigenv32f = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    testdst = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);

    atsFillRandomImage(src8u,0, 64);
	atsFillRandomImage(src32f,-128, 127);

    atsConvert(src8u,tmpsrc);
    preCorner(tmpsrc,eigenv32f,kerSize);
    cvPreCornerDetect(src8u,testdst,kerSize);
    lErrors += (long)cvNorm(eigenv32f,testdst,CV_C);
	
    preCorner(src32f,eigenv32f,kerSize);
    cvPreCornerDetect(src32f,testdst,kerSize);
	lErrors += (long)cvNorm(eigenv32f,testdst,CV_C);


   if(lErrors)return TRS_FAIL;
   return TRS_OK;
} /* fcaSobel8uC1R */

void InitAPreCorner(void)
{
    trsReg( "cvPreCornerDetect", TestName, TestClass, fcaPreCorner );

} /* InitASobel */
