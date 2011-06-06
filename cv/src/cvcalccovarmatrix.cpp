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


/*F///////////////////////////////////////////////////////////////////////////////////////
//    Name: icvCalcCovarMatrixEx_8u32fR
//    Purpose: The function calculates a covariance matrix for a group of input objects
//             (images, vectors, etc.). ROI supported.
//    Context:
//    Parameters:  nObjects    - number of source objects
//                 objects     - array of pointers to ROIs of the source objects
//                 imgStep     - full width of each source object row in bytes
//                 avg         - pointer to averaged object
//                 avgStep     - full width of averaged object row in bytes
//                 size        - ROI size of each source and averaged objects
//                 covarMatrix - covariance matrix (output parameter; must be allocated
//                               before call)
//
//    Returns: CV_NO_ERR or error code
//
//    Notes:   
//F*/

IPCVAPI_IMPL( CvStatus, icvCalcCovarMatrixEx_8u32fR, (int nObjects,
                                                      void *input,
                                                      int objStep1,
                                                      int ioFlags,
                                                      int ioBufSize,
                                                      uchar * buffer,
                                                      void *userData,
                                                      float *avg,
                                                      int avgStep,
                                                      CvSize size, float *covarMatrix) )
{
    int objStep = objStep1;

    /* ---- TEST OF PARAMETERS ---- */

    if( nObjects < 2 )
        return CV_BADFACTOR_ERR;
    if( ioFlags < 0 || ioFlags > 3 )
        return CV_BADFACTOR_ERR;
    if( ioFlags && ioBufSize < 1024 )
        return CV_BADFACTOR_ERR;
    if( ioFlags && buffer == NULL )
        return CV_NULLPTR_ERR;
    if( input == NULL || avg == NULL || covarMatrix == NULL )
        return CV_NULLPTR_ERR;
    if( size.width > objStep || 4 * size.width > avgStep || size.height < 1 )
        return CV_BADSIZE_ERR;

    avgStep /= 4;

    if( ioFlags & CV_EIGOBJ_INPUT_CALLBACK )    /* ==== USE INPUT CALLBACK ==== */
    {
        int nio, ngr, igr, n = size.width * size.height, mm = 0;
        CvCallback read_callback = ((CvInput *) & input)->callback;
        uchar *buffer2;

        objStep = n;
        nio = ioBufSize / n;    /* number of objects in buffer */
        ngr = nObjects / nio;   /* number of io groups */
        if( nObjects % nio )
            mm = 1;
        ngr += mm;

        buffer2 = (uchar *) icvAlloc( sizeof( uchar ) * n );
        if( buffer2 == NULL )
            return CV_OUTOFMEM_ERR;

        for( igr = 0; igr < ngr; igr++ )
        {
            int k, l;
            int io, jo, imin = igr * nio, imax = imin + nio;
            uchar *bu1 = buffer, *bu2;

            if( imax > nObjects )
                imax = nObjects;

            /* read igr group */
            for( io = imin; io < imax; io++, bu1 += n )
            {
                CvStatus r;

                r = read_callback( io, (void *) bu1, userData );
                if( r )
                    return r;
            }

            /* diagonal square calc */
            bu1 = buffer;
            for( io = imin; io < imax; io++, bu1 += n )
            {
                bu2 = bu1;
                for( jo = io; jo < imax; jo++, bu2 += n )
                {
                    float w = 0.f;
                    float *fu = avg;
                    int ij = 0;

                    for( k = 0; k < size.height; k++, fu += avgStep )
                        for( l = 0; l < size.width; l++, ij++ )
                        {
                            float f = fu[l], u1 = bu1[ij], u2 = bu2[ij];

                            w += (u1 - f) * (u2 - f);
                        }
                    covarMatrix[io * nObjects + jo] = covarMatrix[jo * nObjects + io] = w;
                }
            }

            /* non-diagonal elements calc */
            for( jo = imax; jo < nObjects; jo++ )
            {
                CvStatus r;

                bu1 = buffer;
                bu2 = buffer2;

                /* read jo object */
                r = read_callback( jo, (void *) bu2, userData );
                if( r )
                    return r;

                for( io = imin; io < imax; io++, bu1 += n )
                {
                    float w = 0.f;
                    float *fu = avg;
                    int ij = 0;

                    for( k = 0; k < size.height; k++, fu += avgStep )
                    {
                        for( l = 0; l < size.width - 3; l += 4, ij += 4 )
                        {
                            float f = fu[l];
                            uchar u1 = bu1[ij];
                            uchar u2 = bu2[ij];

                            w += (u1 - f) * (u2 - f);
                            f = fu[l + 1];
                            u1 = bu1[ij + 1];
                            u2 = bu2[ij + 1];
                            w += (u1 - f) * (u2 - f);
                            f = fu[l + 2];
                            u1 = bu1[ij + 2];
                            u2 = bu2[ij + 2];
                            w += (u1 - f) * (u2 - f);
                            f = fu[l + 3];
                            u1 = bu1[ij + 3];
                            u2 = bu2[ij + 3];
                            w += (u1 - f) * (u2 - f);
                        }
                        for( ; l < size.width; l++, ij++ )
                        {
                            float f = fu[l], u1 = bu1[ij], u2 = bu2[ij];

                            w += (u1 - f) * (u2 - f);
                        }
                    }
                    covarMatrix[io * nObjects + jo] = covarMatrix[jo * nObjects + io] = w;
                }
            }
        }                       /* igr */

        icvFree( (void **) &buffer2 );
    }                           /* if() */

    else
        /* ==== NOT USE INPUT CALLBACK ==== */
    {
        int i, j;
        uchar **objects = (uchar **) (((CvInput *) & input)->data);

        for( i = 0; i < nObjects; i++ )
        {
            uchar *bu = objects[i];

            for( j = i; j < nObjects; j++ )
            {
                int k, l;
                float w = 0.f;
                float *a = avg;
                uchar *bu1 = bu;
                uchar *bu2 = objects[j];

                for( k = 0; k < size.height;
                     k++, bu1 += objStep, bu2 += objStep, a += avgStep )
                {
                    for( l = 0; l < size.width - 3; l += 4 )
                    {
                        float f = a[l];
                        uchar u1 = bu1[l];
                        uchar u2 = bu2[l];

                        w += (u1 - f) * (u2 - f);
                        f = a[l + 1];
                        u1 = bu1[l + 1];
                        u2 = bu2[l + 1];
                        w += (u1 - f) * (u2 - f);
                        f = a[l + 2];
                        u1 = bu1[l + 2];
                        u2 = bu2[l + 2];
                        w += (u1 - f) * (u2 - f);
                        f = a[l + 3];
                        u1 = bu1[l + 3];
                        u2 = bu2[l + 3];
                        w += (u1 - f) * (u2 - f);
                    }
                    for( ; l < size.width; l++ )
                    {
                        float f = a[l];
                        uchar u1 = bu1[l];
                        uchar u2 = bu2[l];

                        w += (u1 - f) * (u2 - f);
                    }
                }

                covarMatrix[i * nObjects + j] = covarMatrix[j * nObjects + i] = w;
            }
        }
    }                           /* else */

    return CV_NO_ERR;
}

/*======================== end of icvCalcCovarMatrixEx_8u32fR ===========================*/


#define ICV_DOT_PRODUCT_CASE( flavor, srctype, avgtype, load_macro )                    \
static CvStatus CV_STDCALL                                                              \
icvDotProductShifted_##flavor##_C1R( const srctype* vec1, int vecstep1,                 \
                                     const srctype* vec2, int vecstep2,                 \
                                     const avgtype* avg, int avgstep,                   \
                                     CvSize size, double* _result )                     \
{                                                                                       \
    double result = 0;                                                                  \
                                                                                        \
    for( ; size.height--; (char*&)vec1 += vecstep1,                                     \
                          (char*&)vec2 += vecstep2,                                     \
                          (char*&)avg += avgstep )                                      \
    {                                                                                   \
        int x;                                                                          \
        for( x = 0; x <= size.width - 4; x += 4 )                                       \
            result += (load_macro(vec1[x]) - avg[x])*(load_macro(vec2[x]) - avg[x]) +   \
                (load_macro(vec1[x+1]) - avg[x+1])*(load_macro(vec2[x+1]) - avg[x+1]) + \
                (load_macro(vec1[x+2]) - avg[x+2])*(load_macro(vec2[x+2]) - avg[x+2]) + \
                (load_macro(vec1[x+3]) - avg[x+3])*(load_macro(vec2[x+3]) - avg[x+3]);  \
        for( ; x < size.width; x++ )                                                    \
            result += (load_macro(vec1[x]) - avg[x])*(load_macro(vec2[x]) - avg[x]);    \
    }                                                                                   \
                                                                                        \
    *_result = result;                                                                  \
    return CV_OK;                                                                       \
}


ICV_DOT_PRODUCT_CASE( 8u32f, uchar, float, CV_8TO32F )
ICV_DOT_PRODUCT_CASE( 8u64f, uchar, double, CV_8TO32F )
ICV_DOT_PRODUCT_CASE( 8s32f, char, float, CV_8TO32F )
ICV_DOT_PRODUCT_CASE( 8s64f, char, double, CV_8TO32F )
ICV_DOT_PRODUCT_CASE( 16s32f, short, float, CV_NOP )
ICV_DOT_PRODUCT_CASE( 16s64f, short, double, CV_NOP )
ICV_DOT_PRODUCT_CASE( 32f, float, float, CV_NOP )
ICV_DOT_PRODUCT_CASE( 32f64f, float, double, CV_NOP )
ICV_DOT_PRODUCT_CASE( 64f, double, double, CV_NOP )


static void  icvInitDotProductShiftedTable( CvFuncTable* tabfl, CvFuncTable* tabdb )
{
    tabfl->fn_2d[CV_8U] = (void*)icvDotProductShifted_8u32f_C1R;
    tabfl->fn_2d[CV_8S] = (void*)icvDotProductShifted_8s32f_C1R;
    tabfl->fn_2d[CV_16S] = (void*)icvDotProductShifted_16s32f_C1R;
    tabfl->fn_2d[CV_32F] = (void*)icvDotProductShifted_32f_C1R;

    tabdb->fn_2d[CV_8U] = (void*)icvDotProductShifted_8u64f_C1R;
    tabdb->fn_2d[CV_8S] = (void*)icvDotProductShifted_8s64f_C1R;
    tabdb->fn_2d[CV_16S] = (void*)icvDotProductShifted_16s64f_C1R;
    tabdb->fn_2d[CV_32F] = (void*)icvDotProductShifted_32f64f_C1R;
    tabdb->fn_2d[CV_64F] = (void*)icvDotProductShifted_64f_C1R;
}


typedef struct vec_data
{
    void* ptr;
    int step;
}
vec_data;


CV_IMPL void
cvCalcCovarMatrix( const CvArr** vecarr, CvArr* covarr, CvArr* avgarr )
{
    static CvFuncTable acc_tab[2], dot_tab[2];
    static int inittab = 0;
    vec_data* vecdata = 0;
    
    CV_FUNCNAME( "cvCalcCovarMatrix" );

    __BEGIN__;

    CvMat covstub, *cov = (CvMat*)covarr;
    CvMat avgstub, *avg = (CvMat*)avgarr;
    CvSize srcsize, contsize;
    int srctype = 0, dsttype;
    int i, j, count;
    CvFunc2D_2A acc_func = 0;
    CvFunc2D_3A1P dot_func = 0;
    int cont_flag = 1;

    if( !inittab )
    {
        icvInitAccTable( acc_tab + 0, acc_tab + 1, 0 );
        icvInitDotProductShiftedTable( dot_tab + 0, dot_tab + 1 );
        inittab = 1;
    }

    if( !vecarr )
        CV_ERROR( CV_StsNullPtr, "NULL vec pointer" );

    CV_CALL( cov = cvGetMat( cov, &covstub ));
    CV_CALL( avg = cvGetMat( avg, &avgstub ));

    if( !CV_ARE_TYPES_EQ( cov, avg ))
        CV_ERROR( CV_StsUnmatchedFormats,
        "Covariation matrix and average vector should have the same types" );

    dsttype = CV_MAT_TYPE( cov->type );
    if( dsttype != CV_32FC1 && dsttype != CV_64FC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "Covariation matrix must be 32fC1 or 64fC1" );

    if( cov->rows != cov->cols )
        CV_ERROR( CV_StsBadSize, "Covariation matrix must be square" );

    count = cov->rows;
    CV_CALL( vecdata = (vec_data*)cvAlloc( count*sizeof(vecdata[0])));

    cvZero( avg );
    srcsize = icvGetMatSize( avg );

    contsize.width = srcsize.width * srcsize.height;
    contsize.height = 1;

    for( i = 0; i < count; i++ )
    {
        CvMat vecstub, *vec = (CvMat*)vecarr[i];
        if( !CV_IS_MAT(vec) )
        {
            CV_CALL( vec = cvGetMat( vec, &vecstub ));
        }

        if( !CV_ARE_SIZES_EQ( vec, avg ))
            CV_ERROR( CV_StsUnmatchedSizes,
            "All input vectors and average vector must have the same size" );

        vecdata[i].ptr = vec->data.ptr;
        vecdata[i].step = vec->step;

        if( i == 0 )
        {
            srctype = CV_MAT_TYPE( vec->type );
            if( CV_MAT_CN( srctype ) != 1 )
                CV_ERROR( CV_BadNumChannels, "All vectors must have a single channel" );
            acc_func = (CvFunc2D_2A)
                acc_tab[dsttype == CV_64FC1].fn_2d[CV_MAT_DEPTH(srctype)];
            dot_func = (CvFunc2D_3A1P)
                dot_tab[dsttype == CV_64FC1].fn_2d[CV_MAT_DEPTH(srctype)];
            if( !acc_func || !dot_func )
                CV_ERROR( CV_StsUnsupportedFormat,
                "The input vectors' type is unsupported" );
        }
        else if( CV_MAT_TYPE(vec->type) != srctype )
            CV_ERROR( CV_StsUnmatchedFormats,
            "All input vectors must have the same type" );

        if( CV_IS_MAT_CONT( avg->type & vec->type ))
        {
            acc_func( vec->data.ptr, CV_STUB_STEP,
                      avg->data.ptr, CV_STUB_STEP, contsize );
        }
        else
        {
            acc_func( vec->data.ptr, vec->step,
                      avg->data.ptr, avg->step, srcsize );
            cont_flag = 0;
        }
    }

    cvScale( avg, avg, 1./count );

    for( i = 0; i < count; i++ )
    {
        int a, b, delta;
        if( !(i & 1) )
            a = 0, b = i+1, delta = 1;
        else
            a = i, b = -1, delta = -1;

        for( j = a; j != b; j += delta )
        {
            double result = 0;
            if( cont_flag )
            {
                dot_func( vecdata[i].ptr, CV_STUB_STEP, vecdata[j].ptr, CV_STUB_STEP,
                          avg->data.ptr, CV_STUB_STEP, contsize, &result );
            }
            else
            {
                dot_func( vecdata[i].ptr, vecdata[i].step,
                          vecdata[j].ptr, vecdata[j].step,
                          avg->data.ptr, avg->step, srcsize, &result );
            }
            if( dsttype == CV_64FC1 )
            {
                ((double*)(cov->data.ptr + i*cov->step))[j] =
                ((double*)(cov->data.ptr + j*cov->step))[i] = result;
            }
            else
            {
                ((float*)(cov->data.ptr + i*cov->step))[j] =
                ((float*)(cov->data.ptr + j*cov->step))[i] = (float)result;
            }
        }
    }

    __END__;

    cvFree( (void**)&vecdata );
}

/* End of file. */

