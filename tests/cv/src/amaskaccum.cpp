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
static char* FuncName[] = 
{
    "cvAccMask",
    "cvSquareAccMask",
    "cvMultiplyAccMask",
    "cvRunningAvgMask"
};
static char* TestName[]    = 
{
    "Linear Accumulating with Mask",
    "Accumulating of Squares with Mask",
    "Accumulating of Products with Mask",
    "Running Average with Mask"
};
static char TestClass[]   = "Algorithm";


static long lImageWidth;
static long lImageHeight;

#define EPSILON 0.00001

static int fcaLinAcc( void )
{
    /* Some Variables */
    
    AtsRandState      state;
    IplImage*         pSrc8u;
    IplImage*         pSrc32f;
    
    IplImage*         pDst;
    IplImage*         pTest;
    IplImage*         pTemp; 
    IplImage*         Mask;
    
    double Error;
    
    static int  read_param = 0;
    
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Reading test-parameters */
        trslRead( &lImageHeight, "4", "Image Height" );
        trslRead( &lImageWidth, "4", "Image width" );
    }
    atsRandInit(&state,0,255,127);
    pSrc8u         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
    pSrc32f        = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pDst           = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTest          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTemp          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    Mask           = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U,  1);
    atsFillRandomImageEx(pSrc8u, &state );
    atsFillRandomImageEx(pSrc32f, &state );
    atsFillRandomImageEx(pDst, &state );
    atsFillRandomImageEx(Mask, &state );
    cvCopy( pDst, pTest );
    cvThreshold( Mask, Mask, 125, 0, CV_THRESH_TOZERO );
    
    cvAccMask(pSrc8u,pTest,Mask);
    atsConvert(pSrc8u,pTemp);
    cvAdd(pDst,pTemp,pDst,Mask);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    cvAccMask(pSrc32f,pTest,Mask);
    cvAdd(pDst,pSrc32f,pDst,Mask);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    
    trsWrite(ATS_SUM, "\nAccuracy   %e\n",EPSILON);
    /*************************************************************************************/
    /*    check 8u                                                                       */
    /*************************************************************************************/
    
    cvReleaseImage( &pSrc8u );
    cvReleaseImage( &pSrc32f );
    cvReleaseImage( &pDst );
    cvReleaseImage( &pTest);
    cvReleaseImage( &pTemp );
    cvReleaseImage( &Mask );
    
    
    if( Error < EPSILON  ) return trsResult( TRS_OK, "No errors fixed for this text" );
    else return trsResult( TRS_FAIL,"Total fixed %d errors", 1);
} /* fmaAcc */

static int fcaSqrAcc( void )
{
    AtsRandState   	  state;
    IplImage*         pSrc8u;
    IplImage*         pSrc32f;
    
    IplImage*         pDst;
    IplImage*         pTest;
    IplImage*         pTemp; 
    IplImage*         Mask;
    
    double Error;
    
    static int  read_param = 0;
    
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Reading test-parameters */
        trslRead( &lImageHeight, "80", "Image Height" );
        trslRead( &lImageWidth, "80", "Image width" );
    }
    atsRandInit(&state,0,255,127);
    pSrc8u   = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
    pSrc32f  = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pDst     = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTemp    = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTest    = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    Mask     = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U,  1);
    atsFillRandomImageEx(pSrc8u, &state );
    atsFillRandomImageEx(pSrc32f, &state );
    atsFillRandomImageEx(pDst, &state );
    atsFillRandomImageEx(Mask, &state );
    cvCopy( pDst, pTest );
    cvThreshold( Mask, Mask, 125, 0, CV_THRESH_TOZERO );
    
    cvSquareAccMask(pSrc8u,pTest,Mask);
    atsConvert(pSrc8u,pTemp);
    cvMul(pTemp,pTemp,pTemp);
    cvAdd(pDst,pTemp,pDst,Mask);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    cvSquareAccMask(pSrc32f,pTest,Mask);
    cvMul(pSrc32f,pSrc32f,pSrc32f);
    cvAdd(pDst,pSrc32f,pDst,Mask);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    
    trsWrite(ATS_SUM, "\nAccuracy   %e\n",EPSILON);
    /*************************************************************************************/
    /*    check 8u                                                                       */
    /*************************************************************************************/
    
    cvReleaseImage( &pSrc8u );
    cvReleaseImage( &pSrc32f );
    cvReleaseImage( &pDst );
    cvReleaseImage( &pTest);
    cvReleaseImage( &pTemp );
    cvReleaseImage( &Mask );
        
    if( Error < EPSILON  ) return trsResult( TRS_OK, "No errors fixed for this text" );
    else return trsResult( TRS_FAIL,"Total fixed %d errors", 1);
} /* fmaAcc */

static int fcaMultAcc( void )
{
    /* Some Variables */
    
    AtsRandState      state;
    IplImage*         pSrcA8u;
    IplImage*         pSrcA32f;
    IplImage*         pSrcB8u;
    IplImage*         pSrcB32f;
    
    IplImage*         pDst;
    IplImage*         pTest;
    IplImage*         pTempA;
    IplImage*         pTempB;
    IplImage*         Mask;
    
    double Error;
    
    static int  read_param = 0;
    
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Reading test-parameters */
        trslRead( &lImageHeight, "80", "Image Height" );
        trslRead( &lImageWidth, "80", "Image width" );
    }
    atsRandInit(&state,0,255,127);
    pSrcA8u         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
    pSrcA32f        = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pSrcB8u         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
    pSrcB32f        = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pDst            = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTempA          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTempB          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    Mask            = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U,  1);
    
    atsFillRandomImageEx(pSrcA8u, &state );
    atsFillRandomImageEx(pSrcA32f, &state );
    atsFillRandomImageEx(pSrcB8u, &state );
    atsFillRandomImageEx(pSrcB32f, &state );
    atsFillRandomImageEx(pDst, &state );
    atsFillRandomImageEx(Mask,&state);
    pTest = cvCloneImage(pDst);
    cvThreshold( Mask, Mask, 125, 0, CV_THRESH_TOZERO );
    
    cvMultiplyAccMask(pSrcA8u,pSrcB8u,pTest,Mask);
    atsConvert(pSrcA8u,pTempA);
    atsConvert(pSrcB8u,pTempB);
    cvMul(pTempA,pTempB,pTempA);
    cvAdd(pDst,pTempA,pDst,Mask);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    cvMultiplyAccMask(pSrcA32f,pSrcB32f,pTest,Mask);
    cvMul(pSrcA32f,pSrcB32f,pTempA);
    cvAdd(pDst,pTempA,pDst,Mask);
    Error = (long)cvNorm(pTest,pDst,CV_C);
    
    trsWrite(ATS_SUM, "\nAccuracy   %e\n",EPSILON);
    /*************************************************************************************/
    /*    check 8u                                                                       */
    /*************************************************************************************/
    
    
        
    cvReleaseImage( &pSrcA8u );
    cvReleaseImage( &pSrcA32f );
    cvReleaseImage( &pSrcB8u );
    cvReleaseImage( &pSrcB32f );
    cvReleaseImage( &pDst );
    cvReleaseImage( &pTest);
    cvReleaseImage( &pTempA );
    cvReleaseImage( &pTempB );
    cvReleaseImage( &Mask );
        
    if( Error < EPSILON  ) return trsResult( TRS_OK, "No errors fixed for this text" );
    else return trsResult( TRS_FAIL,"Total fixed %d errors", 1);
} /* fmaAcc */

static int fcaRunAvg( void )
{
    /* Some Variables */
    
    AtsRandState   	  state;
    IplImage*         pSrc8u;
    IplImage*         pSrc32f;
    
    IplImage*         pDst;
    IplImage*         pDst0;
    IplImage*         pTest;
    IplImage*		  pTemp;
    IplImage*		  Mask;
    
    float alpha =0.05f; 
    double Error;
    
    static int  read_param = 0;
    
    /* Initialization global parameters */
    if( !read_param )
    {
        read_param = 1;
        /* Reading test-parameters */
        trslRead( &lImageHeight, "80", "Image Height" );
        trslRead( &lImageWidth, "80", "Image width" );
    }
    atsRandInit(&state,0,255,127);
    pSrc8u         = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U, 1);
    pSrc32f        = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pDst           = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pDst0          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    pTemp          = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_32F, 1);
    Mask           = cvCreateImage(cvSize(lImageWidth, lImageHeight), IPL_DEPTH_8U,  1);
    atsFillRandomImageEx(pSrc8u, &state );
    atsFillRandomImageEx(pSrc32f, &state );
    atsFillRandomImageEx(pDst, &state );
    atsFillRandomImageEx(Mask, &state );
    
    pTest = cvCloneImage(pDst);
    cvThreshold( Mask, Mask, 125, 0, CV_THRESH_TOZERO );
    cvRunningAvgMask(pSrc8u,pTest,Mask,alpha);
    atsConvert(pSrc8u,pTemp);
    cvCopy( pDst, pDst0 );
    cvScale(pDst,pDst,(1.f-alpha));
    cvScale(pTemp,pTemp,alpha);
    cvAdd(pDst,pTemp,pDst);
    cvCopy(pDst,pDst0,Mask); 
    Error = (long)cvNorm(pTest,pDst0,CV_C);
    cvRunningAvgMask(pSrc32f,pTest,Mask,alpha);
    cvCopy( pDst0, pDst );
    cvScale(pDst,pDst,(1.f-alpha));
    cvScale(pSrc32f,pTemp,alpha);
    cvAdd(pDst,pTemp,pDst);
    cvCopy(pDst,pDst0,Mask);
    Error = (long)cvNorm(pTest,pDst0,CV_C);
    
    trsWrite(ATS_SUM, "\nAccuracy   %e\n",EPSILON);
    /*************************************************************************************/
    /*    check 8u                                                                       */
    /*************************************************************************************/
    
    pTemp->maskROI = 0;
    pDst->maskROI = 0;
    
    cvReleaseImage( &pSrc8u );
    cvReleaseImage( &pSrc32f );
    cvReleaseImage( &pDst );
    cvReleaseImage( &pDst0 );
    cvReleaseImage( &pTest);
    cvReleaseImage( &pTemp );
    cvReleaseImage( &Mask );
    
    if( Error < EPSILON  ) return trsResult( TRS_OK, "No errors fixed for this text" );
    else return trsResult( TRS_FAIL,"Total fixed %d errors", 1);
} /* fmaAcc */





void InitAMaskAcc( void )
{
    /* Registering test function */
    trsReg( FuncName[0], TestName[0], TestClass, fcaLinAcc );
    trsReg( FuncName[1], TestName[1], TestClass, fcaSqrAcc );
    trsReg( FuncName[2], TestName[2], TestClass, fcaMultAcc );
    trsReg( FuncName[3], TestName[3], TestClass, fcaRunAvg );
} /* InitAAcc */


/* End of file. */
