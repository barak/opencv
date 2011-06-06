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

/* ////////////////////////////////////////////////////////////////////
//
//  CvMat comparison functions: range checking, min, max
//
// */

#include "_cv.h"

/****************************************************************************************\
*                                      InRange[S]                                        *
\****************************************************************************************/

#define ICV_DEF_IN_RANGE_CASE_C1( worktype, _toggle_macro_ )    \
for( x = 0; x < size.width; x++ )                               \
{                                                               \
    worktype a1 = _toggle_macro_(src1[x]),                      \
             a2 = src2[x], a3 = src3[x];                        \
    dst[x] = (uchar)-(_toggle_macro_(a2) <= a1 &&               \
                     a1 < _toggle_macro_(a3));                  \
}


#define ICV_DEF_IN_RANGE_CASE_C2( worktype, _toggle_macro_ )        \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*2]),                        \
             a2 = src2[x*2], a3 = src3[x*2];                        \
    int f = _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);    \
    a1 = _toggle_macro_(src1[x*2+1]);                               \
    a2 = src2[x*2+1];                                               \
    a3 = src3[x*2+1];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    dst[x] = (uchar)-f;                                             \
}


#define ICV_DEF_IN_RANGE_CASE_C3( worktype, _toggle_macro_ )        \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*3]),                        \
             a2 = src2[x*3], a3 = src3[x*3];                        \
    int f = _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);    \
    a1 = _toggle_macro_(src1[x*3+1]);                               \
    a2 = src2[x*3+1];                                               \
    a3 = src3[x*3+1];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    a1 = _toggle_macro_(src1[x*3+2]);                               \
    a2 = src2[x*3+2];                                               \
    a3 = src3[x*3+2];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    dst[x] = (uchar)-f;                                             \
}


#define ICV_DEF_IN_RANGE_CASE_C4( worktype, _toggle_macro_ )        \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*4]),                        \
             a2 = src2[x*4], a3 = src3[x*4];                        \
    int f = _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);    \
    a1 = _toggle_macro_(src1[x*4+1]);                               \
    a2 = src2[x*4+1];                                               \
    a3 = src3[x*4+1];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    a1 = _toggle_macro_(src1[x*4+2]);                               \
    a2 = src2[x*4+2];                                               \
    a3 = src3[x*4+2];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    a1 = _toggle_macro_(src1[x*4+3]);                               \
    a2 = src2[x*4+3];                                               \
    a3 = src3[x*4+3];                                               \
    f &= _toggle_macro_(a2) <= a1 && a1 < _toggle_macro_(a3);       \
    dst[x] = (uchar)-f;                                             \
}


#define ICV_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype,               \
                               _toggle_macro_, cn )                     \
IPCVAPI( CvStatus,                                                      \
icvInRange_##flavor##_C##cn##R,( const arrtype* src1, int step1,        \
                                 const arrtype* src2, int step2,        \
                                 const arrtype* src3, int step3,        \
                                 uchar* dst, int step, CvSize size ));  \
IPCVAPI_IMPL( CvStatus,                                                 \
icvInRange_##flavor##_C##cn##R,( const arrtype* src1, int step1,        \
                                 const arrtype* src2, int step2,        \
                                 const arrtype* src3, int step3,        \
                                 uchar* dst, int step, CvSize size ))   \
{                                                                       \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2, \
                          (char*&)src3 += step3, (char*&)dst += step )  \
    {                                                                   \
        int x;                                                          \
        ICV_DEF_IN_RANGE_CASE_C##cn( worktype, _toggle_macro_ )         \
    }                                                                   \
                                                                        \
    return CV_OK;                                                       \
}


#define ICV_DEF_IN_RANGE_CASE_CONST_C1( worktype, _toggle_macro_ )  \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x]);                          \
    dst[x] = (uchar)-(scalar[0] <= a1 && a1 < scalar[1]);           \
}


#define ICV_DEF_IN_RANGE_CASE_CONST_C2( worktype, _toggle_macro_ )  \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*2]);                        \
    int f = scalar[0] <= a1 && a1 < scalar[2];                      \
    a1 = _toggle_macro_(src1[x*2+1]);                               \
    f &= scalar[1] <= a1 && a1 < scalar[3];                         \
    dst[x] = (uchar)-f;                                             \
}


#define ICV_DEF_IN_RANGE_CASE_CONST_C3( worktype, _toggle_macro_ )  \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*3]);                        \
    int f = scalar[0] <= a1 && a1 < scalar[3];                      \
    a1 = _toggle_macro_(src1[x*3+1]);                               \
    f &= scalar[1] <= a1 && a1 < scalar[4];                         \
    a1 = _toggle_macro_(src1[x*3+2]);                               \
    f &= scalar[2] <= a1 && a1 < scalar[5];                         \
    dst[x] = (uchar)-f;                                             \
}


#define ICV_DEF_IN_RANGE_CASE_CONST_C4( worktype, _toggle_macro_ )  \
for( x = 0; x < size.width; x++ )                                   \
{                                                                   \
    worktype a1 = _toggle_macro_(src1[x*4]);                        \
    int f = scalar[0] <= a1 && a1 < scalar[4];                      \
    a1 = _toggle_macro_(src1[x*4+1]);                               \
    f &= scalar[1] <= a1 && a1 < scalar[5];                         \
    a1 = _toggle_macro_(src1[x*4+2]);                               \
    f &= scalar[2] <= a1 && a1 < scalar[6];                         \
    a1 = _toggle_macro_(src1[x*4+3]);                               \
    f &= scalar[3] <= a1 && a1 < scalar[7];                         \
    dst[x] = (uchar)-f;                                             \
}


#define ICV_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype,         \
                                     _toggle_macro_, cn )               \
IPCVAPI( CvStatus,                                                      \
icvInRangeC_##flavor##_C##cn##R,( const arrtype* src1, int step1,       \
                                  uchar* dst, int step, CvSize size,    \
                                  const worktype* scalar ));            \
IPCVAPI_IMPL( CvStatus,                                                 \
icvInRangeC_##flavor##_C##cn##R,( const arrtype* src1, int step1,       \
                                  uchar* dst, int step, CvSize size,    \
                                  const worktype* scalar ))             \
{                                                                       \
    for( ; size.height--; (char*&)src1 += step1, (char*&)dst += step )  \
    {                                                                   \
        int x;                                                          \
        ICV_DEF_IN_RANGE_CASE_CONST_C##cn( worktype, _toggle_macro_ )   \
    }                                                                   \
                                                                        \
    return CV_OK;                                                       \
}


#define ICV_DEF_IN_RANGE_ALL( flavor, arrtype, worktype, _toggle_macro_ )   \
ICV_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype, _toggle_macro_, 1 )       \
ICV_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype, _toggle_macro_, 2 )       \
ICV_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype, _toggle_macro_, 3 )       \
ICV_DEF_IN_RANGE_FUNC( flavor, arrtype, worktype, _toggle_macro_, 4 )       \
                                                                            \
ICV_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype, _toggle_macro_, 1 ) \
ICV_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype, _toggle_macro_, 2 ) \
ICV_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype, _toggle_macro_, 3 ) \
ICV_DEF_IN_RANGE_CONST_FUNC( flavor, arrtype, worktype, _toggle_macro_, 4 )

ICV_DEF_IN_RANGE_ALL( 8u, uchar, int, CV_NOP )
ICV_DEF_IN_RANGE_ALL( 16s, short, int, CV_NOP )
ICV_DEF_IN_RANGE_ALL( 32s, int, int, CV_NOP )
ICV_DEF_IN_RANGE_ALL( 32f, float, float, CV_NOP )
ICV_DEF_IN_RANGE_ALL( 64f, double, double, CV_NOP )

#define icvInRange_8s_C1R 0
#define icvInRange_8s_C2R 0
#define icvInRange_8s_C3R 0
#define icvInRange_8s_C4R 0

#define icvInRangeC_8s_C1R 0
#define icvInRangeC_8s_C2R 0
#define icvInRangeC_8s_C3R 0
#define icvInRangeC_8s_C4R 0

CV_DEF_INIT_BIG_FUNC_TAB_2D( InRange, R )
CV_DEF_INIT_BIG_FUNC_TAB_2D( InRangeC, R )

typedef CvStatus (CV_STDCALL * CvInRangeCFunc)( const void* src, int srcstep,
                                                uchar* dst, int dststep,
                                                CvSize size, const void* scalar );

/*************************************** InRange ****************************************/

CV_IMPL void
cvInRange( const void* srcarr1, const void* srcarr2,
           const void* srcarr3, void* dstarr )
{
    static CvBigFuncTable inrange_tab;
    static int inittab = 0;

    CV_FUNCNAME( "cvInRange" );

    __BEGIN__;

    int type, coi = 0;
    int src1_step, src2_step, src3_step, dst_step;
    CvMat srcstub1, *src1 = (CvMat*)srcarr1;
    CvMat srcstub2, *src2 = (CvMat*)srcarr2;
    CvMat srcstub3, *src3 = (CvMat*)srcarr3;
    CvMat dststub,  *dst = (CvMat*)dstarr;
    CvSize size;
    CvFunc2D_4A func;

    if( !inittab )
    {
        icvInitInRangeRTable( &inrange_tab );
        inittab = 1;
    }

    if( !CV_IS_MAT(src1) )
    {
        CV_CALL( src1 = cvGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(src2) )
    {
        CV_CALL( src2 = cvGetMat( src2, &srcstub2, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(src3) )
    {
        CV_CALL( src3 = cvGetMat( src3, &srcstub3, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(dst) )
    {
        CV_CALL( dst = cvGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_ARE_TYPES_EQ( src1, src2 ) ||
        !CV_ARE_TYPES_EQ( src1, src3 ) )
        CV_ERROR_FROM_CODE( CV_StsUnmatchedFormats );

    if( !CV_IS_MASK_ARR( dst ))
        CV_ERROR( CV_StsUnsupportedFormat, "Destination image should be 8uC1 or 8sC1");

    if( !CV_ARE_SIZES_EQ( src1, src2 ) ||
        !CV_ARE_SIZES_EQ( src1, src3 ) ||
        !CV_ARE_SIZES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedSizes );

    type = CV_MAT_TYPE(src1->type);
    size = icvGetMatSize( src1 );

    if( CV_IS_MAT_CONT( src1->type & src2->type & src3->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = src2_step = src3_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        src2_step = src2->step;
        src3_step = src3->step;
        dst_step = dst->step;
    }

    func = (CvFunc2D_4A)(inrange_tab.fn_2d[type]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                     src3->data.ptr, src3_step, dst->data.ptr, dst_step, size ));

    __END__;
}


/************************************** InRangeS ****************************************/

CV_IMPL void
cvInRangeS( const void* srcarr, CvScalar lower,
            CvScalar upper, void* dstarr )
{
    static CvBigFuncTable inrange_tab;
    static int inittab = 0;

    CV_FUNCNAME( "cvInRangeS" );

    __BEGIN__;

    int sctype, type, coi = 0;
    int src1_step, dst_step;
    CvMat srcstub1, *src1 = (CvMat*)srcarr;
    CvMat dststub,  *dst = (CvMat*)dstarr;
    CvSize size;
    CvInRangeCFunc func;
    double buf[8];

    if( !inittab )
    {
        icvInitInRangeCRTable( &inrange_tab );
        inittab = 1;
    }

    if( !CV_IS_MAT(src1) )
    {
        CV_CALL( src1 = cvGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(dst) )
    {
        CV_CALL( dst = cvGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MASK_ARR( dst ))
        CV_ERROR( CV_StsUnsupportedFormat, "Destination image should be 8uC1 or 8sC1");

    if( !CV_ARE_SIZES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedSizes );

    sctype = type = CV_MAT_TYPE(src1->type);
    if( CV_MAT_DEPTH(sctype) < CV_32S )
        sctype = (type & CV_MAT_CN_MASK) | CV_32SC1;

    size = icvGetMatSize( src1 );

    if( CV_IS_MAT_CONT( src1->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        dst_step = dst->step;
    }

    func = (CvInRangeCFunc)(inrange_tab.fn_2d[type]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    cvScalarToRawData( &lower, buf, sctype, 0 );
    cvScalarToRawData( &upper, (char*)buf + icvPixSize[sctype], sctype, 0 );

    IPPI_CALL( func( src1->data.ptr, src1_step, dst->data.ptr,
                     dst_step, size, buf ));

    __END__;
}


/****************************************************************************************\
*                                         Cmp                                            *
\****************************************************************************************/

#define ICV_DEF_CMP_CASE_C1( __op__, _toggle_macro_ )                   \
for( x = 0; x <= size.width - 4; x += 4 )                               \
{                                                                       \
    int f0 = __op__( _toggle_macro_(src1[x]), _toggle_macro_(src2[x])); \
    int f1 = __op__( _toggle_macro_(src1[x+1]), _toggle_macro_(src2[x+1])); \
    dst[x] = (uchar)-f0;                                                \
    dst[x+1] = (uchar)-f1;                                              \
    f0 = __op__( _toggle_macro_(src1[x+2]), _toggle_macro_(src2[x+2])); \
    f1 = __op__( _toggle_macro_(src1[x+3]), _toggle_macro_(src2[x+3])); \
    dst[x+2] = (uchar)-f0;                                              \
    dst[x+3] = (uchar)-f1;                                              \
}                                                                       \
                                                                        \
for( ; x < size.width; x++ )                                            \
{                                                                       \
    int f0 = __op__( _toggle_macro_(src1[x]), _toggle_macro_(src2[x])); \
    dst[x] = (uchar)-f0;                                                \
}


#define ICV_DEF_CMP_FUNC( __op__, name, flavor, arrtype,                \
                          worktype, _toggle_macro_ )                    \
IPCVAPI( CvStatus,                                                      \
icv##name##_##flavor##_C1R,( const arrtype* src1, int step1,            \
                             const arrtype* src2, int step2,            \
                             uchar* dst, int step, CvSize size ));      \
IPCVAPI_IMPL( CvStatus,                                                 \
icv##name##_##flavor##_C1R,( const arrtype* src1, int step1,            \
                             const arrtype* src2, int step2,            \
                             uchar* dst, int step, CvSize size ))       \
{                                                                       \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2, \
                          (char*&)dst += step )                         \
    {                                                                   \
        int x;                                                          \
        ICV_DEF_CMP_CASE_C1( __op__, _toggle_macro_ )                   \
    }                                                                   \
                                                                        \
    return CV_OK;                                                       \
}


#define ICV_DEF_CMP_CONST_CASE_C1( __op__, _toggle_macro_ )             \
for( x = 0; x <= size.width - 4; x += 4 )                               \
{                                                                       \
    int f0 = __op__( _toggle_macro_(src1[x]), scalar );                 \
    int f1 = __op__( _toggle_macro_(src1[x+1]), scalar );               \
    dst[x] = (uchar)-f0;                                                \
    dst[x+1] = (uchar)-f1;                                              \
    f0 = __op__( _toggle_macro_(src1[x+2]), scalar );                   \
    f1 = __op__( _toggle_macro_(src1[x+3]), scalar );                   \
    dst[x+2] = (uchar)-f0;                                              \
    dst[x+3] = (uchar)-f1;                                              \
}                                                                       \
                                                                        \
for( ; x < size.width; x++ )                                            \
{                                                                       \
    int f0 = __op__( _toggle_macro_(src1[x]), scalar );                 \
    dst[x] = (uchar)-f0;                                                \
}


#define ICV_DEF_CMP_CONST_FUNC( __op__, name, flavor, arrtype,          \
                                worktype, _toggle_macro_)               \
IPCVAPI( CvStatus,                                                      \
icv##name##C_##flavor##_C1R,( const arrtype* src1, int step1,           \
                             uchar* dst, int step,                      \
                             CvSize size, worktype* pScalar ));         \
IPCVAPI_IMPL( CvStatus,                                                 \
icv##name##C_##flavor##_C1R,( const arrtype* src1, int step1,           \
                             uchar* dst, int step,                      \
                             CvSize size, worktype* pScalar ))          \
{                                                                       \
    worktype scalar = *pScalar;                                         \
    for( ; size.height--; (char*&)src1 += step1, (char*&)dst += step )  \
    {                                                                   \
        int x;                                                          \
        ICV_DEF_CMP_CONST_CASE_C1( __op__, _toggle_macro_ )             \
    }                                                                   \
                                                                        \
    return CV_OK;                                                       \
}


#define ICV_DEF_CMP_ALL( flavor, arrtype, worktype, _toggle_macro_ )            \
ICV_DEF_CMP_FUNC( CV_GT, CmpGT, flavor, arrtype, worktype, _toggle_macro_ )     \
ICV_DEF_CMP_FUNC( CV_EQ, CmpEQ, flavor, arrtype, worktype, _toggle_macro_ )     \
ICV_DEF_CMP_CONST_FUNC( CV_GT, CmpGT, flavor, arrtype, worktype, _toggle_macro_)\
ICV_DEF_CMP_CONST_FUNC( CV_GE, CmpGE, flavor, arrtype, worktype, _toggle_macro_)\
ICV_DEF_CMP_CONST_FUNC( CV_EQ, CmpEQ, flavor, arrtype, worktype, _toggle_macro_)

ICV_DEF_CMP_ALL( 8u, uchar, int, CV_NOP )
ICV_DEF_CMP_ALL( 16s, short, int, CV_NOP )
ICV_DEF_CMP_ALL( 32s, int, int, CV_NOP )
ICV_DEF_CMP_ALL( 32f, float, double, CV_NOP )
ICV_DEF_CMP_ALL( 64f, double, double, CV_NOP )

#define icvCmpGT_8s_C1R     0
#define icvCmpEQ_8s_C1R     0
#define icvCmpGTC_8s_C1R    0
#define icvCmpGEC_8s_C1R    0
#define icvCmpEQC_8s_C1R    0

CV_DEF_INIT_FUNC_TAB_2D( CmpGT, C1R )
CV_DEF_INIT_FUNC_TAB_2D( CmpEQ, C1R )
CV_DEF_INIT_FUNC_TAB_2D( CmpGTC, C1R )
CV_DEF_INIT_FUNC_TAB_2D( CmpGEC, C1R )
CV_DEF_INIT_FUNC_TAB_2D( CmpEQC, C1R )

/***************************************** cvCmp ****************************************/

CV_IMPL void
cvCmp( const void* srcarr1, const void* srcarr2,
       void* dstarr, int cmp_op )
{
    static CvFuncTable cmp_tab[2];
    static int inittab = 0;

    CV_FUNCNAME( "cvCmp" );

    __BEGIN__;

    int type, coi = 0;
    int invflag = 0;
    int src1_step, src2_step, dst_step;
    CvMat srcstub1, *src1 = (CvMat*)srcarr1;
    CvMat srcstub2, *src2 = (CvMat*)srcarr2;
    CvMat dststub,  *dst = (CvMat*)dstarr;
    CvMat *temp;
    CvSize size;
    CvFunc2D_3A func;

    if( !inittab )
    {
        icvInitCmpGTC1RTable( &cmp_tab[0] );
        icvInitCmpEQC1RTable( &cmp_tab[1] );
        inittab = 1;
    }

    switch( cmp_op )
    {
    case CV_CMP_GT:
    case CV_CMP_EQ:
        break;
    case CV_CMP_GE:
        CV_SWAP( src1, src2, temp );
        invflag = 1;
        break;
    case CV_CMP_LT:
        CV_SWAP( src1, src2, temp );
        break;
    case CV_CMP_LE:
        invflag = 1;
        break;
    case CV_CMP_NE:
        cmp_op = CV_CMP_EQ;
        invflag = 1;
        break;
    default:
        CV_ERROR( CV_StsBadArg, "Unknown comparison operation" );
    }

    if( !CV_IS_MAT(src1) )
    {
        CV_CALL( src1 = cvGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(src2) )
    {
        CV_CALL( src2 = cvGetMat( src2, &srcstub2, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(dst) )
    {
        CV_CALL( dst = cvGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_ARE_TYPES_EQ( src1, src2 ) )
        CV_ERROR_FROM_CODE( CV_StsUnmatchedFormats );

    if( CV_MAT_CN( src1->type ) != 1 )
        CV_ERROR( CV_StsUnsupportedFormat, "Input arrays must be single-channel");

    if( !CV_IS_MASK_ARR( dst ))
        CV_ERROR( CV_StsUnsupportedFormat, "Destination array should be 8uC1 or 8sC1");

    if( !CV_ARE_SIZES_EQ( src1, src2 ) ||
        !CV_ARE_SIZES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedSizes );

    type = CV_MAT_TYPE(src1->type);
    size = icvGetMatSize( src1 );

    if( CV_IS_MAT_CONT( src1->type & src2->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = src2_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        src2_step = src2->step;
        dst_step = dst->step;
    }

    func = (CvFunc2D_3A)(cmp_tab[cmp_op == CV_CMP_EQ].fn_2d[type]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                     dst->data.ptr, dst_step, size ));

    if( invflag )
        IPPI_CALL( icvNot_8u_C1R( dst->data.ptr, dst_step,
                           dst->data.ptr, dst_step, size ));

    __END__;
}


/*************************************** cvCmpS *****************************************/

CV_IMPL void
cvCmpS( const void* srcarr, double value, void* dstarr, int cmp_op )
{
    static CvFuncTable cmps_tab[3];
    static int inittab = 0;

    CV_FUNCNAME( "cvCmpS" );

    __BEGIN__;

    int type, coi = 0;
    int invflag = 0;
    int src1_step, dst_step;
    CvMat srcstub1, *src1 = (CvMat*)srcarr;
    CvMat dststub,  *dst = (CvMat*)dstarr;
    CvSize size;
    CvFunc2D_2A1P func;
    double buf;

    if( !inittab )
    {
        icvInitCmpEQCC1RTable( &cmps_tab[CV_CMP_EQ] );
        icvInitCmpGTCC1RTable( &cmps_tab[CV_CMP_GT] );
        icvInitCmpGECC1RTable( &cmps_tab[CV_CMP_GE] );
        inittab = 1;
    }

    switch( cmp_op )
    {
    case CV_CMP_GT:
    case CV_CMP_EQ:
    case CV_CMP_GE:
        break;
    case CV_CMP_LT:
        invflag = 1;
        cmp_op = CV_CMP_GE;
        break;
    case CV_CMP_LE:
        invflag = 1;
        cmp_op = CV_CMP_GT;
        break;
    case CV_CMP_NE:
        invflag = 1;
        cmp_op = CV_CMP_EQ;
        break;
    default:
        CV_ERROR( CV_StsBadArg, "Unknown comparison operation" );
    }

    if( !CV_IS_MAT(src1) )
    {
        CV_CALL( src1 = cvGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(dst) )
    {
        CV_CALL( dst = cvGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MASK_ARR( dst ))
        CV_ERROR( CV_StsUnsupportedFormat, "Destination array should be 8uC1 or 8sC1");

    if( CV_MAT_CN( src1->type ) != 1 )
        CV_ERROR( CV_StsUnsupportedFormat, "Input array must be single-channel");

    if( !CV_ARE_SIZES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedSizes );

    type = CV_MAT_TYPE(src1->type);

    if( CV_MAT_DEPTH(type) <= CV_32S )
        (int&)buf = cvRound(value);
    else
        buf = value;

    size = icvGetMatSize( src1 );

    if( CV_IS_MAT_CONT( src1->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        dst_step = dst->step;
    }

    func = (CvFunc2D_2A1P)(cmps_tab[cmp_op].fn_2d[type]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, dst->data.ptr,
                     dst_step, size, &buf ));

    if( invflag )
        IPPI_CALL( icvNot_8u_C1R( dst->data.ptr, dst_step,
                           dst->data.ptr, dst_step, size ));

    __END__;
}


/****************************************************************************************\
*                                       Min/Max                                          *
\****************************************************************************************/


#define ICV_DEF_MINMAX_CASE_C1( __op__, arrtype, worktype, _toggle_macro_ )\
for( x = 0; x <= size.width - 4; x += 4 )                               \
{                                                                       \
    worktype a0 = _toggle_macro_(src1[x]), b0 = _toggle_macro_(src2[x]);\
    worktype a1 = _toggle_macro_(src1[x+1]), b1 = _toggle_macro_(src2[x+1]);\
    a0 = __op__( a0, b0 );                                              \
    a1 = __op__( a1, b1 );                                              \
    dst[x] = (arrtype)_toggle_macro_(a0);                               \
    dst[x+1] = (arrtype)_toggle_macro_(a1);                             \
    a0 = _toggle_macro_(src1[x+2]), b0 = _toggle_macro_(src2[x+2]);     \
    a1 = _toggle_macro_(src1[x+3]), b1 = _toggle_macro_(src2[x+3]);     \
    a0 = __op__( a0, b0 );                                              \
    a1 = __op__( a1, b1 );                                              \
    a0 = _toggle_macro_(a0);                                            \
    a1 = _toggle_macro_(a1);                                            \
    dst[x+2] = (arrtype)_toggle_macro_(a0);                             \
    dst[x+3] = (arrtype)_toggle_macro_(a1);                             \
}                                                                       \
                                                                        \
for( ; x < size.width; x++ )                                            \
{                                                                       \
    worktype a0 = _toggle_macro_(src1[x]), b0 = _toggle_macro_(src2[x]);\
    a0 = __op__( a0, b0 );                                              \
    dst[x] = (arrtype)_toggle_macro_(a0);                               \
}


#define ICV_DEF_MINMAX_FUNC( __op__, name, flavor, arrtype,             \
                             worktype, _toggle_macro_ )                 \
IPCVAPI( CvStatus,                                                      \
icv##name##_##flavor##_C1R,( const arrtype* src1, int step1,            \
                             const arrtype* src2, int step2,            \
                             arrtype* dst, int step, CvSize size ));    \
IPCVAPI_IMPL( CvStatus,                                                 \
icv##name##_##flavor##_C1R,( const arrtype* src1, int step1,            \
                             const arrtype* src2, int step2,            \
                             arrtype* dst, int step, CvSize size ))     \
{                                                                       \
    for( ; size.height--; (char*&)src1 += step1, (char*&)src2 += step2, \
                          (char*&)dst += step )                         \
    {                                                                   \
        int x;                                                          \
        ICV_DEF_MINMAX_CASE_C1( __op__, arrtype,                        \
                                worktype, _toggle_macro_ )              \
    }                                                                   \
                                                                        \
    return CV_OK;                                                       \
}


#define ICV_DEF_MINMAX_CONST_CASE_C1( __op__, arrtype, worktype, _toggle_macro_ )\
for( x = 0; x <= size.width - 4; x += 4 )                       \
{                                                               \
    worktype a0 = _toggle_macro_(src1[x]);                      \
    worktype a1 = _toggle_macro_(src1[x+1]);                    \
    a0 = __op__( a0, scalar );                                  \
    a1 = __op__( a1, scalar );                                  \
    dst[x] = (arrtype)_toggle_macro_(a0);                       \
    dst[x+1] = (arrtype)_toggle_macro_(a1);                     \
    a0 = _toggle_macro_(src1[x+2]);                             \
    a1 = _toggle_macro_(src1[x+3]);                             \
    a0 = __op__( a0, scalar );                                  \
    a1 = __op__( a1, scalar );                                  \
    dst[x+2] = (arrtype)_toggle_macro_(a0);                     \
    dst[x+3] = (arrtype)_toggle_macro_(a1);                     \
}                                                               \
                                                                \
for( ; x < size.width; x++ )                                    \
{                                                               \
    worktype a0 = _toggle_macro_(src1[x]);                      \
    a0 = __op__( a0, scalar );                                  \
    dst[x] = (arrtype)_toggle_macro_(a0);                       \
}


#define ICV_DEF_MINMAX_CONST_FUNC( __op__, name, flavor, arrtype,       \
                                   worktype, _toggle_macro_)            \
IPCVAPI( CvStatus,                                                      \
icv##name##C_##flavor##_C1R,( const arrtype* src1, int step1,           \
                             arrtype* dst, int step,                    \
                             CvSize size, worktype* pScalar ));         \
IPCVAPI_IMPL( CvStatus,                                                 \
icv##name##C_##flavor##_C1R,( const arrtype* src1, int step1,           \
                             arrtype* dst, int step,                    \
                             CvSize size, worktype* pScalar ))          \
{                                                                       \
    worktype scalar = _toggle_macro_(*pScalar);                         \
    for( ; size.height--; (char*&)src1 += step1, (char*&)dst += step )  \
    {                                                                   \
        int x;                                                          \
        ICV_DEF_MINMAX_CONST_CASE_C1( __op__, arrtype,                  \
                                      worktype, _toggle_macro_ )        \
    }                                                                   \
                                                                        \
    return CV_OK;                                                       \
}


#define ICV_DEF_MINMAX_ALL( flavor, arrtype, worktype,                             \
                            _toggle_macro_, _min_op_, _max_op_ )                   \
ICV_DEF_MINMAX_FUNC( _min_op_, Min, flavor, arrtype, worktype, _toggle_macro_ )    \
ICV_DEF_MINMAX_FUNC( _max_op_, Max, flavor, arrtype, worktype, _toggle_macro_ )    \
ICV_DEF_MINMAX_CONST_FUNC(_min_op_, Min, flavor, arrtype, worktype, _toggle_macro_)\
ICV_DEF_MINMAX_CONST_FUNC(_max_op_, Max, flavor, arrtype, worktype, _toggle_macro_)


ICV_DEF_MINMAX_ALL( 8u, uchar, int, CV_NOP, CV_IMIN, CV_IMAX )
ICV_DEF_MINMAX_ALL( 16s, short, int, CV_NOP, CV_IMIN, CV_IMAX )
ICV_DEF_MINMAX_ALL( 32s, int, int, CV_NOP, CV_IMIN, CV_IMAX )
ICV_DEF_MINMAX_ALL( 32f, int, int, CV_TOGGLE_FLT, CV_IMIN, CV_IMAX )
ICV_DEF_MINMAX_ALL( 64f, double, double, CV_NOP, CV_MIN, CV_MAX )

#define icvMin_8s_C1R     0
#define icvMax_8s_C1R     0
#define icvMinC_8s_C1R    0
#define icvMaxC_8s_C1R    0

CV_DEF_INIT_FUNC_TAB_2D( Min, C1R )
CV_DEF_INIT_FUNC_TAB_2D( Max, C1R )
CV_DEF_INIT_FUNC_TAB_2D( MinC, C1R )
CV_DEF_INIT_FUNC_TAB_2D( MaxC, C1R )

/*********************************** cvMin & cvMax **************************************/

static void
icvMinMax( const void* srcarr1, const void* srcarr2,
           void* dstarr, int is_max )
{
    static CvFuncTable minmax_tab[2];
    static int inittab = 0;

    CV_FUNCNAME( "icvMinMax" );

    __BEGIN__;

    int type, coi = 0;
    int src1_step, src2_step, dst_step;
    CvMat srcstub1, *src1 = (CvMat*)srcarr1;
    CvMat srcstub2, *src2 = (CvMat*)srcarr2;
    CvMat dststub,  *dst = (CvMat*)dstarr;
    CvSize size;
    CvFunc2D_3A func;

    if( !inittab )
    {
        icvInitMinC1RTable( &minmax_tab[0] );
        icvInitMaxC1RTable( &minmax_tab[1] );
        inittab = 1;
    }

    if( !CV_IS_MAT(src1) )
    {
        CV_CALL( src1 = cvGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(src2) )
    {
        CV_CALL( src2 = cvGetMat( src2, &srcstub2, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(dst) )
    {
        CV_CALL( dst = cvGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_ARE_TYPES_EQ( src1, src2 ) ||
        !CV_ARE_TYPES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedFormats );

    if( CV_MAT_CN( src1->type ) != 1 )
        CV_ERROR( CV_StsUnsupportedFormat, "Input arrays must be single-channel");

    if( !CV_ARE_SIZES_EQ( src1, src2 ) ||
        !CV_ARE_SIZES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedSizes );

    type = CV_MAT_TYPE(src1->type);
    size = icvGetMatSize( src1 );

    if( CV_IS_MAT_CONT( src1->type & src2->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = src2_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        src2_step = src2->step;
        dst_step = dst->step;
    }

    func = (CvFunc2D_3A)(minmax_tab[is_max != 0].fn_2d[type]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, src2->data.ptr, src2_step,
                     dst->data.ptr, dst_step, size ));

    __END__;
}


CV_IMPL void
cvMin( const void* srcarr1, const void* srcarr2, void* dstarr )
{
    icvMinMax( srcarr1, srcarr2, dstarr, 0 );
}


CV_IMPL void
cvMax( const void* srcarr1, const void* srcarr2, void* dstarr )
{
    icvMinMax( srcarr1, srcarr2, dstarr, 1 );
}


/********************************* cvMinS / cvMaxS **************************************/

static void
icvMinMaxS( const void* srcarr, double value, void* dstarr, int is_max )
{
    static CvFuncTable minmaxs_tab[2];
    static int inittab = 0;

    CV_FUNCNAME( "icvMinMaxS" );

    __BEGIN__;

    int type, coi = 0;
    int src1_step, dst_step;
    CvMat srcstub1, *src1 = (CvMat*)srcarr;
    CvMat dststub,  *dst = (CvMat*)dstarr;
    CvSize size;
    CvFunc2D_2A1P func;
    union
    {
        int i;
        float f;
        double d;
    }
    buf;

    if( !inittab )
    {
        icvInitMinCC1RTable( &minmaxs_tab[0] );
        icvInitMaxCC1RTable( &minmaxs_tab[1] );
        inittab = 1;
    }

    if( !CV_IS_MAT(src1) )
    {
        CV_CALL( src1 = cvGetMat( src1, &srcstub1, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT(dst) )
    {
        CV_CALL( dst = cvGetMat( dst, &dststub, &coi ));
        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_ARE_TYPES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedFormats );

    if( CV_MAT_CN( src1->type ) != 1 )
        CV_ERROR( CV_StsUnsupportedFormat, "Input array must be single-channel");

    if( !CV_ARE_SIZES_EQ( src1, dst ))
        CV_ERROR_FROM_CODE( CV_StsUnmatchedSizes );

    type = CV_MAT_TYPE(src1->type);
    
    if( CV_MAT_DEPTH(type) <= CV_32S )
        buf.i = cvRound(value);
    else if( CV_MAT_DEPTH(type) == CV_32F )
        buf.f = (float)value;
    else
        buf.d = value;

    size = icvGetMatSize( src1 );

    if( CV_IS_MAT_CONT( src1->type & dst->type ))
    {
        size.width *= size.height;
        src1_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }
    else
    {
        src1_step = src1->step;
        dst_step = dst->step;
    }

    func = (CvFunc2D_2A1P)(minmaxs_tab[is_max].fn_2d[type]);

    if( !func )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    IPPI_CALL( func( src1->data.ptr, src1_step, dst->data.ptr,
                     dst_step, size, &buf ));

    __END__;
}


CV_IMPL void
cvMinS( const void* srcarr, double value, void* dstarr )
{
    icvMinMaxS( srcarr, value, dstarr, 0 );
}


CV_IMPL void
cvMaxS( const void* srcarr, double value, void* dstarr )
{
    icvMinMaxS( srcarr, value, dstarr, 1 );
}


/* End of file. */
