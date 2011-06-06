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

/* Haar features calculation */

#include "_cvaux.h"

#include <float.h>
#include <stdio.h>
#include "_cvutils.h"

/* these settings affect the quality of detection: change with care */
#define CV_ADJUST_FEATURES 1
#define CV_ADJUST_WEIGHTS  0

typedef int sumtype;
typedef double sqsumtype;

typedef struct CvHidHaarFeature
{
    struct
    {
        sumtype *p0, *p1, *p2, *p3;
        float weight;
    }
    rect[CV_HAAR_FEATURE_MAX];
}
CvHidHaarFeature;


typedef struct CvHidHaarClassifier
{
    int count;
    CvHaarFeature* origFeature;
    CvHidHaarFeature* feature;
    float* threshold;
    int* left;
    int* right;
    float* alpha;
}
CvHidHaarClassifier;


typedef struct CvHidHaarStageClassifier
{
    int  count;
    float threshold;
    CvHidHaarClassifier* classifier;
    int two_rects;
}
CvHidHaarStageClassifier;


struct CvHidHaarClassifierCascade
{
    int  headerSize;
    int  count;
    int  is_stump_based;
    int  has_tilted_features;
    CvSize origWindowSize;
    CvSize realWindowSize;
    double scale, invWindowArea;
    CvHidHaarStageClassifier* stageClassifier;
    CvMat sum;
    CvMat tiltedSum;
    CvMat sqsum;
    sqsumtype *pq0, *pq1, *pq2, *pq3;
    sumtype *p0, *p1, *p2, *p3;
};


static const char* numSuffix( int i )
{
    switch( i % 10 )
    {
    case 1: return "st";
    case 2: return "nd";
    case 3: return "rd";
    default: return "th";
    }
}


static void
updateRealWindowSize( CvHidHaarClassifierCascade* cascade, double scale )
{
    assert( cascade != 0 && cascade->headerSize == sizeof(*cascade));
    
    cascade->scale = scale;
    cascade->realWindowSize =
        cvSize( cvRound( cascade->origWindowSize.width * scale ),
                cvRound( cascade->origWindowSize.height * scale ));
}


/* create faster internal representation of haar classifier cascade */
CV_IMPL CvHidHaarClassifierCascade*
cvCreateHidHaarClassifierCascade( CvHaarClassifierCascade* cascade,
                                  const CvArr* sumImage,
                                  const CvArr* sqSumImage,
                                  const CvArr* tiltedSumImage,
                                  double scale )
{
    CvHidHaarClassifierCascade* out = 0;

    CV_FUNCNAME("cvCreateHidHaarClassifierCascade");

    __BEGIN__;

    int i, j, k, l;
    int pos;
    int datasize = sizeof(CvHidHaarClassifierCascade);
    int total = 0;
    int nodes = 0;
    int nodepos = 0;
    int alphapos = 0;
    int nodecount = 0;
    char errorstr[100];
    CvHaarFeature* haarFeature0;
    CvHidHaarFeature* hidHaarFeature0;
    float* threshold0;
    int* left0;
    int* right0;
    float* alpha0;
    CvHidHaarClassifier* classifier0;
    CvSize origWindowSize;
    int has_tilted_features = 0;

    if( !cascade || !cascade->stageClassifier )
        CV_ERROR( CV_StsNullPtr, "" );

    if( cascade->count <= 0 )
        CV_ERROR( CV_StsOutOfRange, "Negative classifier counter" );
    
    origWindowSize = cascade->origWindowSize;

    if( origWindowSize.width <= 0 || origWindowSize.height <= 0 )
        CV_ERROR( CV_StsBadSize, "Negative original window width or height" );

    if( scale <= 0 )
        CV_ERROR( CV_StsOutOfRange, "Scale must be positive" );

    /* check input structure correctness and calculate total memory size needed for
       internal representation of the classifier cascade */
    for( i = 0; i < cascade->count; i++ )
    {
        CvHaarStageClassifier* stageClassifier = cascade->stageClassifier + i;

        if( !stageClassifier->classifier ||
            stageClassifier->count <= 0 )
        {
            sprintf( errorstr, "%d-%s stage classifier header is invalid "
                     "(has null pointers or non-positive classfier count)",
                     i, numSuffix(i));
            CV_ERROR( CV_StsNullPtr, errorstr );
        }

        total += stageClassifier->count;

        for( j = 0; j < stageClassifier->count; j++ )
        {
            CvHaarClassifier* classifier = stageClassifier->classifier + j;

            nodes += classifier->count;
            for( l = 0; l < classifier->count; l++ )
            {
                for( k = 0; k < CV_HAAR_FEATURE_MAX; k++ )
                {
                    if( classifier->haarFeature[l].rect[k].weight )
                    {
                        CvRect r = classifier->haarFeature[l].rect[k].r;
                        int tilted = classifier->haarFeature[l].tilted;
                        has_tilted_features |= tilted != 0;
                        if( r.width < 0 || r.height < 0 || r.y < 0 ||
                            r.x + r.width > origWindowSize.width
                            ||
                            (!tilted &&
                            (r.x < 0 || r.y + r.height > origWindowSize.height))
                            ||
                            (tilted && (r.x - r.height < 0 ||
                            r.y + r.width + r.height > origWindowSize.height)))
                        {
                            sprintf( errorstr, "%d-%s rectangle of %d-%s classifier of "
                                     "%d-%s stage classifier is invalid or is outside "
                                     "the original window",
                                     k, numSuffix(k), j, numSuffix(j), i, numSuffix(i) );
                            CV_ERROR( CV_StsNullPtr, errorstr );
                        }
                    }
                }
            }
        }
    }

    datasize = sizeof(CvHidHaarClassifierCascade) +
               sizeof(CvHidHaarStageClassifier)*cascade->count +
               sizeof(CvHidHaarClassifier) * total +
               ( sizeof( CvHidHaarFeature ) + sizeof( CvHaarFeature ) +
                 sizeof( float ) + 2 * sizeof( int ) ) * nodes +
               sizeof( float ) * (nodes + total);

    CV_CALL( out = (CvHidHaarClassifierCascade*)cvAlloc( datasize ));

    /* init header */
    out->headerSize = sizeof(*out);
    out->count = cascade->count;
    out->origWindowSize = cascade->origWindowSize;
    updateRealWindowSize( out, scale );
    out->sum.data.ptr = out->sqsum.data.ptr = out->tiltedSum.data.ptr = 0;
    
    out->stageClassifier = (CvHidHaarStageClassifier*)(out + 1);
    classifier0 = (CvHidHaarClassifier*)(out->stageClassifier + out->count);
    haarFeature0 = (CvHaarFeature*)(classifier0 + total);
    hidHaarFeature0 = (CvHidHaarFeature*) (haarFeature0 + nodes);
    threshold0 = (float*) (hidHaarFeature0 + nodes);
    left0 = (int*) (threshold0 + nodes);
    right0 = (int*) (left0 + nodes);
    alpha0 = (float*) (right0 + nodes);

    out->is_stump_based = 1;
    out->has_tilted_features = has_tilted_features;

    /* initialize internal representation */
    for( i = 0, pos = 0, nodepos = 0, alphapos = 0; i < cascade->count; i++ )
    {
        CvHaarStageClassifier* stageClassifier = cascade->stageClassifier + i;
        CvHidHaarStageClassifier* hidStageClassifier = out->stageClassifier + i;

        hidStageClassifier->count = stageClassifier->count;
        hidStageClassifier->threshold = stageClassifier->threshold;
        hidStageClassifier->classifier = classifier0 + pos;
        hidStageClassifier->two_rects = 1;
        pos += stageClassifier->count;

        for( j = 0; j < stageClassifier->count; j++ )
        {
            nodecount = stageClassifier->classifier[j].count;
            hidStageClassifier->classifier[j].count = nodecount;
            
            hidStageClassifier->classifier[j].origFeature = haarFeature0 + nodepos;
            hidStageClassifier->classifier[j].feature = hidHaarFeature0 + nodepos;
            hidStageClassifier->classifier[j].threshold = threshold0 + nodepos;
            hidStageClassifier->classifier[j].left = left0 + nodepos;
            hidStageClassifier->classifier[j].right = right0 + nodepos;
            hidStageClassifier->classifier[j].alpha = alpha0 + alphapos;

            nodepos += hidStageClassifier->classifier[j].count;
            alphapos += hidStageClassifier->classifier[j].count + 1;
            for( l = 0; l < nodecount; l++ )
            {
                /* original haar feature */
                hidStageClassifier->classifier[j].origFeature[l] =
                    stageClassifier->classifier[j].haarFeature[l];
                if( hidStageClassifier->classifier[j].origFeature[l].rect[2].weight == 0 ||
                    hidStageClassifier->classifier[j].origFeature[l].rect[2].r.width == 0 ||
                    hidStageClassifier->classifier[j].origFeature[l].rect[2].r.height == 0 )
                {
                    memset( &(hidStageClassifier->classifier[j].origFeature[l].rect[2]), 0,
                            sizeof(hidStageClassifier->classifier[j].origFeature[l].rect[2]) );
                }
                else
                    hidStageClassifier->two_rects = 0;

                /* threshold */
                hidStageClassifier->classifier[j].threshold[l] =
                    stageClassifier->classifier[j].threshold[l];
                /* left */
                hidStageClassifier->classifier[j].left[l] =
                    stageClassifier->classifier[j].left[l];
                /* right */
                hidStageClassifier->classifier[j].right[l] =
                    stageClassifier->classifier[j].right[l];
                /* alpha */
                hidStageClassifier->classifier[j].alpha[l] =
                    stageClassifier->classifier[j].alpha[l];
            }

            out->is_stump_based &= nodecount == 1;

            hidStageClassifier->classifier[j].alpha[nodecount] =
                stageClassifier->classifier[j].alpha[nodecount];
        }
    }

    if( sumImage )
        CV_CALL( cvSetImagesForHaarClassifierCascade( out, sumImage, sqSumImage,
                                                      tiltedSumImage, scale ));

    __END__;

    if( cvGetErrStatus() < 0 )
        cvFree( (void**)&out );

    return out;
}


CV_IMPL CvSize
cvGetHaarClassifierCascadeWindowSize( CvHidHaarClassifierCascade* cascade )
{
    CvSize realWindowSize = { -1, -1 };

    CV_FUNCNAME("cvGetHaarClassifierCascadeWindowSize");

    __BEGIN__;

    if( !cascade )
        CV_ERROR( CV_StsNullPtr, "" );

    if( cascade->headerSize != sizeof(*cascade))
        CV_ERROR( CV_StsNullPtr, "Invalid CvHidHaarClassifierCascade header" );

    realWindowSize = cascade->realWindowSize;

    __END__;

    return realWindowSize;
}


CV_IMPL double
cvGetHaarClassifierCascadeWindowScale( CvHidHaarClassifierCascade* cascade )
{
    double scale = -1;
    
    CV_FUNCNAME("cvGetHaarClassifierCascadeWindowSize");

    __BEGIN__;

    if( !cascade )
        CV_ERROR( CV_StsNullPtr, "" );

    if( cascade->headerSize != sizeof(*cascade))
        CV_ERROR( CV_StsNullPtr, "Invalid CvHidHaarClassifierCascade header" );

    scale = cascade->scale;

    __END__;

    return scale;
}


#define sum_elem_ptr(sum,row,col)  \
    ((sumtype*)CV_MAT_ELEM_PTR_FAST((sum),(row),(col),sizeof(sumtype)))

#define sqsum_elem_ptr(sqsum,row,col)  \
    ((sqsumtype*)CV_MAT_ELEM_PTR_FAST((sqsum),(row),(col),sizeof(sqsumtype)))

#define calc_sum(rect,offset) \
    ((rect).p0[offset] - (rect).p1[offset] - (rect).p2[offset] + (rect).p3[offset])


CV_IMPL void
cvSetImagesForHaarClassifierCascade( CvHidHaarClassifierCascade* cascade,
                                     const CvArr* sumImage,
                                     const CvArr* sqSumImage,
                                     const CvArr* tiltedSumImage,
                                     double scale )
{
    CV_FUNCNAME("cvSetImagesForHaarClassifierCascade");

    __BEGIN__;

    CvMat sum_stub, *sum = (CvMat*)sumImage;
    CvMat sqsum_stub, *sqsum = (CvMat*)sqSumImage;
    CvMat tilted_stub, *tilted = (CvMat*)tiltedSumImage;
    int coi0 = 0, coi1 = 0;
    int i, j, k, l;
    int pixSize;
    CvSize winSize;
    CvRect equ_rect;
    double weight_scale;

    if( !cascade )
        CV_ERROR( CV_StsNullPtr, "" );

    if( cascade->headerSize != sizeof(*cascade))
        CV_ERROR( CV_StsNullPtr, "Invalid CvHidHaarClassifierCascade header" );

    if( scale <= 0 )
        CV_ERROR( CV_StsOutOfRange, "Scale must be positive" );

    CV_CALL( sum = cvGetMat( sum, &sum_stub, &coi0 ));
    CV_CALL( sqsum = cvGetMat( sqsum, &sqsum_stub, &coi1 ));

    if( !CV_ARE_SIZES_EQ( sum, sqsum ))
        CV_ERROR( CV_StsUnmatchedSizes, "All integral images must have the same size" );

    if( CV_MAT_TYPE(sqsum->type) != CV_64FC1 ||
        CV_MAT_TYPE(sum->type) != CV_32SC1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "Only (32s, 64f, 32s) combination of (sum,sqsum,tiltedsum) formats is allowed" );

    if( cascade->has_tilted_features )
    {
        CV_CALL( tilted = cvGetMat( tilted, &tilted_stub, &coi1 ));

        if( CV_MAT_TYPE(tilted->type) != CV_32SC1 )
            CV_ERROR( CV_StsUnsupportedFormat,
            "Only (32s, 64f, 32s) combination of (sum,sqsum,tiltedsum) formats is allowed" );

        if( sum->step != tilted->step )
            CV_ERROR( CV_StsUnmatchedSizes,
            "Sum and tiltedSum must have the same stride (step, widthStep)" );

        if( !CV_ARE_SIZES_EQ( sum, tilted ))
            CV_ERROR( CV_StsUnmatchedSizes, "All integral images must have the same size" );
        cascade->tiltedSum = *tilted;
    }
    else
    {
        cascade->tiltedSum = *sum;
    }

    cascade->sum = *sum;
    cascade->sqsum = *sqsum;
    
    pixSize = CV_ELEM_SIZE( CV_MAT_TYPE( sum->type ));
    updateRealWindowSize( cascade, scale );
    winSize = cascade->realWindowSize;
    
    equ_rect.x = equ_rect.y = cvRound(scale);
    equ_rect.width = cvRound((cascade->origWindowSize.width-2)*scale);
    equ_rect.height = cvRound((cascade->origWindowSize.height-2)*scale);
    weight_scale = 1./(equ_rect.width*equ_rect.height);
    cascade->invWindowArea = weight_scale;

    cascade->p0 = sum_elem_ptr(*sum, equ_rect.y, equ_rect.x);
    cascade->p1 = sum_elem_ptr(*sum, equ_rect.y, equ_rect.x + equ_rect.width );
    cascade->p2 = sum_elem_ptr(*sum, equ_rect.y + equ_rect.height, equ_rect.x );
    cascade->p3 = sum_elem_ptr(*sum, equ_rect.y + equ_rect.height,
                                     equ_rect.x + equ_rect.width );

    cascade->pq0 = sqsum_elem_ptr(*sqsum, equ_rect.y, equ_rect.x);
    cascade->pq1 = sqsum_elem_ptr(*sqsum, equ_rect.y, equ_rect.x + equ_rect.width );
    cascade->pq2 = sqsum_elem_ptr(*sqsum, equ_rect.y + equ_rect.height, equ_rect.x );
    cascade->pq3 = sqsum_elem_ptr(*sqsum, equ_rect.y + equ_rect.height,
                                          equ_rect.x + equ_rect.width );

    /* init pointers in haar features according to real window size and
       given image pointers */
    for( i = 0; i < cascade->count; i++ )
    {
        for( j = 0; j < cascade->stageClassifier[i].count; j++ )
        {
            for( l = 0; l < cascade->stageClassifier[i].classifier[j].count; l++ )
            {
                CvHaarFeature* feature = 
                    cascade->stageClassifier[i].classifier[j].origFeature + l;
                /* CvHidHaarClassifier* classifier =
                    cascade->stageClassifier[i].classifier + j; */
                CvHidHaarFeature* hidfeature = 
                    cascade->stageClassifier[i].classifier[j].feature + l;
                double sum0 = 0, area0 = 0;
                CvRect r[3];
#if CV_ADJUST_FEATURES
                int base_w = -1, base_h = -1;
                int new_base_w = 0, new_base_h = 0;
                int kx, ky;
                int flagx = 0, flagy = 0;
                int x0 = 0, y0 = 0;
#endif
                int nr;

                /* align blocks */
                for( k = 0; k < CV_HAAR_FEATURE_MAX; k++ )
                {
                    r[k] = feature->rect[k].r;
                    if( r[k].width == 0 )
                    {
                        memset( hidfeature->rect + k, 0, sizeof(hidfeature->rect[k]));
                        break;
                    }
#if CV_ADJUST_FEATURES
                    base_w = (int)CV_IMIN( (unsigned)base_w, (unsigned)(r[k].width-1) );
                    base_w = (int)CV_IMIN( (unsigned)base_w, (unsigned)(r[k].x - r[0].x-1) );
                    base_h = (int)CV_IMIN( (unsigned)base_h, (unsigned)(r[k].height-1) );
                    base_h = (int)CV_IMIN( (unsigned)base_h, (unsigned)(r[k].y - r[0].y-1) );
#endif
                }

                base_w += 1;
                base_h += 1;
                nr = k;

#if CV_ADJUST_FEATURES
                kx = r[0].width / base_w;
                ky = r[0].height / base_h;

                if( kx <= 0 )
                {
                    flagx = 1;
                    new_base_w = cvRound( r[0].width * scale ) / kx;
                    x0 = cvRound( r[0].x * scale );
                }

                if( ky <= 0 )
                {
                    flagy = 1;
                    new_base_h = cvRound( r[0].height * scale ) / ky;
                    y0 = cvRound( r[0].y * scale );
                }
#endif
            
                for( k = 0; k < CV_HAAR_FEATURE_MAX; k++ )
                {
                    CvRect tr;
                    double correctionRatio;
                
                    if( k < nr )
                    {
#if CV_ADJUST_FEATURES
                        if( flagx )
                        {
                            tr.x = (r[k].x - r[0].x) * new_base_w / base_w + x0;
                            tr.width = r[k].width * new_base_w / base_w;
                        }
                        else
#endif
                        {
                            tr.x = cvRound( r[k].x * scale );
                            tr.width = cvRound( r[k].width * scale );
                        }

#if CV_ADJUST_FEATURES
                        if( flagy )
                        {
                            tr.y = (r[k].y - r[0].y) * new_base_h / base_h + y0;
                            tr.height = r[k].height * new_base_h / base_h;
                        }
                        else
#endif
                        {
                            tr.y = cvRound( r[k].y * scale );
                            tr.height = cvRound( r[k].height * scale );
                        }

#if CV_ADJUST_WEIGHTS
                        {
                        // RAINER START
                        const float origFeatureSize =  (float)(feature->rect[k].r.width)*feature->rect[k].r.height; 
                        const float origNormSize = (float)(cascade->origWindowSize.width)*(cascade->origWindowSize.height);
                        const float featureSize = float(tr.width*tr.height);
                        //const float normSize    = float(equ_rect.width*equ_rect.height);
                        float targetRatio = origFeatureSize / origNormSize;
                        //float isRatio = featureSize / normSize;
                        //correctionRatio = targetRatio / isRatio / normSize;
                        correctionRatio = targetRatio / featureSize;
                        // RAINER END
                        }
#else
                        correctionRatio = weight_scale * (!feature->tilted ? 1 : 0.5);
#endif

                        if( !feature->tilted )
                        {
                            hidfeature->rect[k].p0 = sum_elem_ptr(*sum, tr.y, tr.x);
                            hidfeature->rect[k].p1 = sum_elem_ptr(*sum, tr.y, tr.x + tr.width);
                            hidfeature->rect[k].p2 = sum_elem_ptr(*sum, tr.y + tr.height, tr.x);
                            hidfeature->rect[k].p3 = sum_elem_ptr(*sum, tr.y + tr.height, tr.x + tr.width);
                        }
                        else
                        {
                            hidfeature->rect[k].p2 = sum_elem_ptr(*tilted, tr.y + tr.width, tr.x + tr.width);
                            hidfeature->rect[k].p3 = sum_elem_ptr(*tilted, tr.y + tr.width + tr.height,
                                                                  tr.x + tr.width - tr.height);
                            hidfeature->rect[k].p0 = sum_elem_ptr(*tilted, tr.y, tr.x);
                            hidfeature->rect[k].p1 = sum_elem_ptr(*tilted, tr.y + tr.height, tr.x - tr.height);
                        }

                        hidfeature->rect[k].weight = (float)(feature->rect[k].weight * correctionRatio);

                        if( k == 0 )
                            area0 = tr.width * tr.height;
                        else
                            sum0 += hidfeature->rect[k].weight * tr.width * tr.height;
                    }
                    else
                    {
                        memset( hidfeature->rect + k, 0, sizeof(hidfeature->rect[k]));
                        hidfeature->rect[k].weight = 0;
                    }
                }

                hidfeature->rect[0].weight = (float)(-sum0/area0);
            } /* l */
        } /* j */
    }

    __END__;
}


CV_IMPL int
cvRunHaarClassifierCascade( CvHidHaarClassifierCascade* cascade,
                            CvPoint pt, int start_stage )
{
    int result = -1;
    CV_FUNCNAME("cvRunHaarClassifierCascade");

    __BEGIN__;

    int p_offset, pq_offset;
    int i, j;
    double mean, variance_norm_factor;

    if( !cascade )
        CV_ERROR( CV_StsNullPtr, "" );

    if( cascade->headerSize != sizeof(*cascade))
        CV_ERROR( CV_StsNullPtr, "Invalid CvHidHaarClassifierCascade header" );

    if( pt.x < 0 || pt.y < 0 ||
        pt.x + cascade->realWindowSize.width >= cascade->sum.width-2 ||
        pt.y + cascade->realWindowSize.height >= cascade->sum.height-2 )
        EXIT;

    p_offset = pt.y * (cascade->sum.step/sizeof(sumtype)) + pt.x;
    pq_offset = pt.y * (cascade->sqsum.step/sizeof(sqsumtype)) + pt.x;
    mean = calc_sum(*cascade,p_offset)*cascade->invWindowArea;
    variance_norm_factor = cascade->pq0[pq_offset] - cascade->pq1[pq_offset] -
                           cascade->pq2[pq_offset] + cascade->pq3[pq_offset];
    variance_norm_factor = 
        /*1.*/sqrt(variance_norm_factor*cascade->invWindowArea - mean*mean);

    if( cascade->is_stump_based )
    {
        for( i = start_stage; i < cascade->count; i++ )
        {
            double stage_sum = 0;

            if( cascade->stageClassifier[i].two_rects )
            {
                for( j = 0; j < cascade->stageClassifier[i].count; j++ )
                {
                    CvHidHaarClassifier* classifier = cascade->stageClassifier[i].classifier + j;
                    CvHidHaarFeature* feature = classifier->feature;
                    double sum, t = classifier->threshold[0]*variance_norm_factor, a, b;

                    sum = calc_sum(feature->rect[0],p_offset) * feature->rect[0].weight;
                    sum += calc_sum(feature->rect[1],p_offset) * feature->rect[1].weight;

                    a = classifier->alpha[0];
                    b = classifier->alpha[1];
                    stage_sum += sum < t ? a : b;
                }
            }
            else
            {
                for( j = 0; j < cascade->stageClassifier[i].count; j++ )
                {
                    CvHidHaarClassifier* classifier = cascade->stageClassifier[i].classifier + j;
                    CvHidHaarFeature* feature = classifier->feature;
                    double sum, t = classifier->threshold[0]*variance_norm_factor;

                    sum = calc_sum(feature->rect[0],p_offset) * feature->rect[0].weight;
                    sum += calc_sum(feature->rect[1],p_offset) * feature->rect[1].weight;

                    if( feature->rect[2].p0 )
                        sum += calc_sum(feature->rect[2],p_offset) * feature->rect[2].weight;

                    stage_sum += classifier->alpha[sum >= t];
                }
            }

            if( stage_sum < cascade->stageClassifier[i].threshold - 0.0001 )
            {
                result = -i;
                EXIT;
            }
        }
    }
    else
    {
        for( i = start_stage; i < cascade->count; i++ )
        {
            double stage_sum = 0;

            for( j = 0; j < cascade->stageClassifier[i].count; j++ )
            {
                CvHidHaarClassifier* classifier = cascade->stageClassifier[i].classifier + j;
                double sum, t;// t = classifier->threshold*variance_norm_factor;
                int idx;

                idx = 0;
                do 
                {
                    t = classifier->threshold[idx] * variance_norm_factor;

                    sum = calc_sum(classifier->feature[idx].rect[0],p_offset)
                        * classifier->feature[idx].rect[0].weight;
                    sum += calc_sum(classifier->feature[idx].rect[1],p_offset)
                        * classifier->feature[idx].rect[1].weight;

                    if( classifier->feature[idx].rect[2].p0 )
                        sum += calc_sum(classifier->feature[idx].rect[2],p_offset)
                            * classifier->feature[idx].rect[2].weight;

                    idx = ( sum < t ) ? classifier->left[idx] : classifier->right[idx];

                }
                while( idx > 0 );

                stage_sum += classifier->alpha[-idx];
            }

            if( stage_sum < cascade->stageClassifier[i].threshold - 0.0001 )
            {
                result = -i;
                EXIT;
            }
        }
    }

    result = 1;

    __END__;

    return result;
}


static int is_equal( const void* _r1, const void* _r2, void* )
{
    const CvRect* r1 = (const CvRect*)_r1;
    const CvRect* r2 = (const CvRect*)_r2;
    int distance = cvRound(r1->width*0.2);

    return r2->x <= r1->x + distance &&
           r2->x >= r1->x - distance &&
           r2->y <= r1->y + distance &&
           r2->y >= r1->y - distance &&
           r2->width <= cvRound( r1->width * 1.2 ) &&
           cvRound( r2->width * 1.2 ) >= r1->width;
}


CV_IMPL CvSeq*
cvHaarDetectObjects( const IplImage* img,
                     CvHidHaarClassifierCascade* hid_cascade,
                     CvMemStorage* storage, double scale_factor,
                     int min_neighbors, int flags )
{
    int split_stage = 2;
    CvSize size = cvGetSize( img ), sumsize = { size.width + 1, size.height + 1 };
    IplImage *temp = 0, *sum = 0, *tilted = 0, *sqsum = 0;
    IplImage *sumcanny = 0;
    CvSeq* seq = 0;
    CvSeq* seq2 = 0;
    CvSeq* idx_seq = 0;
    CvSeq* result_seq = 0;
    CvMemStorage* temp_storage = 0;
    CvAvgComp* comps = 0;
    int ncomp;
    int do_canny_pruning = flags & CV_HAAR_DO_CANNY_PRUNING;
    
    CV_FUNCNAME( "cvDetectFaces" );

    __BEGIN__;

    double factor;
    int i, npass = 2;
    int hid_cascade_count0;

    if( !CV_IS_IMAGE(img) )
        CV_ERROR( CV_StsBadArg, "Invalid source image" );

    if( !hid_cascade || hid_cascade->headerSize != sizeof(*hid_cascade))
        CV_ERROR( CV_StsBadArg, "Invalid classifier cascade" );

    if( !storage )
        CV_ERROR( CV_StsNullPtr, "Null storage pointer" );

    temp = cvCreateImage( size, 8, 1 );
    sum = cvCreateImage( sumsize, IPL_DEPTH_32S, 1 );
    sqsum = cvCreateImage( sumsize, IPL_DEPTH_64F, 1 );
    temp_storage = cvCreateChildMemStorage( storage );

    if( hid_cascade->has_tilted_features )
        tilted = cvCreateImage( sumsize, IPL_DEPTH_32S, 1 );

    seq = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvRect), temp_storage );
    seq2 = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvAvgComp), temp_storage );
    result_seq = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvAvgComp), storage );

    if( min_neighbors == 0 )
        seq = result_seq;

    if( img->nChannels == 3 )
    {
        cvCvtColor( img, temp, CV_BGR2GRAY );
        img = temp;
    }
    
    cvIntegral( img, sum, sqsum, tilted );
    
    if( do_canny_pruning )
    {
        sumcanny = cvCreateImage( sumsize, IPL_DEPTH_32S, 1 );
        cvCanny( img, temp, 0, 50, 3 );
        cvIntegral( temp, sumcanny );
    }
    
    hid_cascade_count0 = hid_cascade->count;

    if( split_stage >= hid_cascade_count0 || split_stage == 0 )
    {
        split_stage = hid_cascade_count0;
        npass = 1;
    }

    for( factor = 1; factor*hid_cascade->origWindowSize.width <
         MIN( size.width - 10, size.height - 10 ); factor *= scale_factor )
    {
        double const ystep = MAX( 2, factor );
        
        CvSize winSize = cvSize( cvRound( hid_cascade->origWindowSize.width * factor ),
                                 cvRound( hid_cascade->origWindowSize.height * factor ));
        CvRect equ_rect = { 0, 0, 0, 0 };
        int *p0 = 0, *p1 = 0, *p2 = 0, *p3 = 0;
        int *pq0 = 0, *pq1 = 0, *pq2 = 0, *pq3 = 0;
        int pass, stage_offset = 0;
        int stopHeight = cvRound((size.height - winSize.height - ystep) / ystep);

        cvSetImagesForHaarClassifierCascade( hid_cascade, sum, sqsum, tilted, factor );
        cvZero( temp );

        if( do_canny_pruning )
        {
            equ_rect.x = cvRound(winSize.width*0.3);
            equ_rect.y = cvRound(winSize.height*0.3);
            equ_rect.width = cvRound(winSize.width*0.7);
            equ_rect.height = cvRound(winSize.height*0.7);

            p0 = (int*)(sumcanny->imageData + equ_rect.y*sumcanny->widthStep) + equ_rect.x;
            p1 = (int*)(sumcanny->imageData + equ_rect.y*sumcanny->widthStep)
                        + equ_rect.x + equ_rect.width;
            p2 = (int*)(sumcanny->imageData + (equ_rect.y + equ_rect.height)*sumcanny->widthStep) + equ_rect.x;
            p3 = (int*)(sumcanny->imageData + (equ_rect.y + equ_rect.height)*sumcanny->widthStep)
                        + equ_rect.x + equ_rect.width;

            pq0 = (int*)(sum->imageData + equ_rect.y*sum->widthStep) + equ_rect.x;
            pq1 = (int*)(sum->imageData + equ_rect.y*sum->widthStep)
                        + equ_rect.x + equ_rect.width;
            pq2 = (int*)(sum->imageData + (equ_rect.y + equ_rect.height)*sum->widthStep) + equ_rect.x;
            pq3 = (int*)(sum->imageData + (equ_rect.y + equ_rect.height)*sum->widthStep)
                        + equ_rect.x + equ_rect.width;
        }

        hid_cascade->count = split_stage;

        for( pass = 0; pass < npass; pass++ )
        {
#ifdef _OPENMP
    #pragma omp parallel for shared(hid_cascade, stopHeight, seq, ystep, temp, size, winSize, pass, npass, sum, p0, p1, p2 ,p3, pq0, pq1, pq2, pq3, stage_offset) 
#endif // _OPENMP

            for( int yInt = 0; yInt < stopHeight; yInt++ )
            {
                int iy = cvRound(yInt*ystep);
                int xIntStep = 1;
                int xInt, stopWidth = cvRound((size.width - winSize.width - 2*ystep) / ystep);

                char* mask_row = temp->imageData + temp->widthStep * iy;

                for( xInt = 0; xInt < stopWidth; xInt += xIntStep )
                {
                    int ix = cvRound(xInt*ystep);
                    
                    if( pass == 0 )
                    {
                        int result;

                        xIntStep = 2;

                        if( do_canny_pruning )
                        {
                            int offset;
                            int s, sq;
                        
                            offset = iy*(sum->widthStep/sizeof(p0[0])) + ix;
                            s = p0[offset] - p1[offset] - p2[offset] + p3[offset];
                            sq = pq0[offset] - pq1[offset] - pq2[offset] + pq3[offset];
                            if( s < 100 || sq < 20 )
                                continue;
                        }

                        result = cvRunHaarClassifierCascade( hid_cascade, cvPoint(ix,iy),
                                                             0/*stage_offset*/ );
#ifdef _OPENMP
#pragma omp critical
#endif
                        if( result > 0 )
                        {
                            if( pass < npass - 1 )
                            {
                                mask_row[ix] = 1;
                            }
                            else
                            {
                                CvRect rect = cvRect(ix,iy,winSize.width,winSize.height);
                                cvSeqPush( seq, &rect );
                            }
                        }
                        if( result < 0 )
                            xIntStep = 1;
                    }
                    else if( mask_row[ix] )
                    {
                        int result = cvRunHaarClassifierCascade( hid_cascade, cvPoint(ix,iy),
                                                                 stage_offset );
#ifdef _OPENMP
#pragma omp critical
#endif
                        if( result > 0 )
                        {
                            if( pass == npass - 1 )
                            {
                                CvRect rect = cvRect(ix,iy,winSize.width,winSize.height);
                                cvSeqPush( seq, &rect );
                            }
                        }
                        else
                            mask_row[ix] = 0;
                    }
                }
            }
            stage_offset = hid_cascade->count;
            hid_cascade->count = hid_cascade_count0;
        }
    }

    if( min_neighbors != 0 )
    {
        // group retrieved rectangles in order to filter out noise 
        ncomp = cvPartitionSeq( seq, 0, &idx_seq, is_equal, 0, 0 );
        CV_CALL( comps = (CvAvgComp*)cvAlloc( (ncomp+1)*sizeof(comps[0])));
        memset( comps, 0, (ncomp+1)*sizeof(comps[0]));

        // count number of neighbors
        for( i = 0; i < seq->total; i++ )
        {
            CvRect r1 = *(CvRect*)cvGetSeqElem( seq, i );
            int idx = *(int*)cvGetSeqElem( idx_seq, i );
            assert( (unsigned)idx < (unsigned)ncomp );

            comps[idx].neighbors++;
             
            comps[idx].rect.x += r1.x;
            comps[idx].rect.y += r1.y;
            comps[idx].rect.width += r1.width;
            comps[idx].rect.height += r1.height;
        }

        // calculate average bounding box
        for( i = 0; i < ncomp; i++ )
        {
            int n = comps[i].neighbors;
            if( n >= min_neighbors )
            {
                CvAvgComp comp;
                comp.rect.x = (comps[i].rect.x*2 + n)/(2*n);
                comp.rect.y = (comps[i].rect.y*2 + n)/(2*n);
                comp.rect.width = (comps[i].rect.width*2 + n)/(2*n);
                comp.rect.height = (comps[i].rect.height*2 + n)/(2*n);
                comp.neighbors = comps[i].neighbors;

                cvSeqPush( seq2, &comp );
            }
        }

        // filter out small face rectangles inside large face rectangles
        for( i = 0; i < seq2->total; i++ )
        {
            CvAvgComp r1 = *(CvAvgComp*)cvGetSeqElem( seq2, i );
            int j, flag = 1;

            for( j = 0; j < seq2->total; j++ )
            {
                CvAvgComp r2 = *(CvAvgComp*)cvGetSeqElem( seq2, j );
                int distance = cvRound( r2.rect.width * 0.2 );
            
                if( i != j &&
                    r1.rect.x >= r2.rect.x - distance &&
                    r1.rect.y >= r2.rect.y - distance &&
                    r1.rect.x + r1.rect.width <= r2.rect.x + r2.rect.width + distance &&
                    r1.rect.y + r1.rect.height <= r2.rect.y + r2.rect.height + distance &&
                    (r2.neighbors > MAX( 3, r1.neighbors ) || r1.neighbors < 3) )
                {
                    flag = 0;
                    break;
                }
            }

            if( flag )
            {
                cvSeqPush( result_seq, &r1 );
                /* cvSeqPush( result_seq, &r1.rect ); */
            }
        }
    }

    __END__;

    cvReleaseMemStorage( &temp_storage );
    cvReleaseImage( &sum );
    cvReleaseImage( &sqsum );
    cvReleaseImage( &tilted );
    cvReleaseImage( &temp );
    cvReleaseImage( &sumcanny );

    cvFree( (void**)&comps );

    return result_seq;
}


CV_IMPL void
cvReleaseHidHaarClassifierCascade( CvHidHaarClassifierCascade** cascade )
{
    CV_FUNCNAME("cvGetHaarClassifierCascadeWindowSize");

    __BEGIN__;

    void* c;

    if( !cascade )
        CV_ERROR( CV_StsNullPtr, "" );

    if( *cascade && (*cascade)->headerSize != sizeof(**cascade))
        CV_ERROR( CV_StsNullPtr, "Invalid CvHidHaarClassifierCascade header" );

    c = *cascade;
    *cascade = 0;
    cvFree( &c );

    __END__;
}


static CvHaarClassifierCascade*
icvLoadCascadeCART( const char** input_cascade, int n, CvSize origWindowSize )
{
    int i;
    CvHaarClassifierCascade* cascade = 0;

    cascade = (CvHaarClassifierCascade*)cvAlloc( sizeof(*cascade) +
                                           n*sizeof(*cascade->stageClassifier));
    cascade->count = n;
    cascade->origWindowSize = origWindowSize;
    cascade->stageClassifier = (CvHaarStageClassifier*)(cascade + 1);

    for( i = 0; i < n; i++ )
    {
        int j, count, l;
        float threshold = 0;
        const char* stage = input_cascade[i];
        int dl = 0;

        sscanf( stage, "%d%n", &count, &dl );
        stage += dl;
        
        assert( count > 0 );
        cascade->stageClassifier[i].count = count;
        cascade->stageClassifier[i].classifier =
            (CvHaarClassifier*)cvAlloc( count*sizeof(cascade->stageClassifier[i].classifier[0]));

        for( j = 0; j < count; j++ )
        {
            CvHaarClassifier* classifier = cascade->stageClassifier[i].classifier + j;
            int k, rects = 0;
            char str[100];
            
            sscanf( stage, "%d%n", &classifier->count, &dl );
            stage += dl;

            classifier->haarFeature = (CvHaarFeature*) cvAlloc( 
                classifier->count * ( sizeof( *classifier->haarFeature ) +
                                      sizeof( *classifier->threshold ) +
                                      sizeof( *classifier->left ) +
                                      sizeof( *classifier->right ) ) +
                (classifier->count + 1) * sizeof( *classifier->alpha ) );
            classifier->threshold = (float*) (classifier->haarFeature+classifier->count);
            classifier->left = (int*) (classifier->threshold + classifier->count);
            classifier->right = (int*) (classifier->left + classifier->count);
            classifier->alpha = (float*) (classifier->right + classifier->count);
            
            for( l = 0; l < classifier->count; l++ )
            {
                sscanf( stage, "%d%n", &rects, &dl );
                stage += dl;

                assert( rects >= 2 && rects <= CV_HAAR_FEATURE_MAX );

                for( k = 0; k < rects; k++ )
                {
                    CvRect r;
                    int band = 0;
                    sscanf( stage, "%d%d%d%d%d%f%n",
                            &r.x, &r.y, &r.width, &r.height, &band,
                            &(classifier->haarFeature[l].rect[k].weight), &dl );
                    stage += dl;
                    classifier->haarFeature[l].rect[k].r = r;
                }
                sscanf( stage, "%s%n", str, &dl );
                stage += dl;
            
                classifier->haarFeature[l].tilted = strncmp( str, "tilted", 6 ) == 0;
            
                for( k = rects; k < CV_HAAR_FEATURE_MAX; k++ )
                {
                    memset( classifier->haarFeature[l].rect + k, 0,
                            sizeof(classifier->haarFeature[l].rect[k]) );
                }
                
                sscanf( stage, "%f%d%d%n", &(classifier->threshold[l]), 
                                       &(classifier->left[l]),
                                       &(classifier->right[l]), &dl );
                stage += dl;
            }
            for( l = 0; l <= classifier->count; l++ )
            {
                sscanf( stage, "%f%n", &(classifier->alpha[l]), &dl );
                stage += dl;
            }
        }
       
        sscanf( stage, "%f%n", &threshold, &dl );
        stage += dl;

        cascade->stageClassifier[i].threshold = threshold;
    }

    return cascade;
}

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

extern const char* icvDefaultFaceCascade[];

CV_IMPL CvHaarClassifierCascade*
cvLoadHaarClassifierCascade( const char* directory, CvSize origWindowSize )
{
    const char** input_cascade = 0; 
    CvHaarClassifierCascade *cascade = 0;

    CV_FUNCNAME( "cvLoadHaarClassifierCascade" );

    __BEGIN__;

    int n;
    char name[_MAX_PATH];

    if( strcmp( directory, "<default_face_cascade>" ) == 0 )
    {
        input_cascade = icvDefaultFaceCascade;
        for( n = 0; input_cascade[n] != 0; n++ )
            ;
    }
    else
    {
        int i, size = 0;
        char* ptr;
        
        for( n = 0; ; n++ )
        {
            sprintf( name, "%s/%d/AdaBoostCARTHaarClassifier.txt", directory, n );
            FILE* f = fopen( name, "rb" );
            if( !f )
                break;
            fseek( f, 0, SEEK_END );
            size += ftell( f ) + 1;
            fclose(f);
        }

        size += (n+1)*sizeof(char*);
        CV_CALL( input_cascade = (const char**)cvAlloc( size ));
        ptr = (char*)(input_cascade + n + 1);

        for( i = 0; i < n; i++ )
        {
            sprintf( name, "%s/%d/AdaBoostCARTHaarClassifier.txt", directory, i );
            FILE* f = fopen( name, "rb" );
            if( !f )
                CV_ERROR( CV_StsError, "" );
            fseek( f, 0, SEEK_END );
            size = ftell( f );
            fseek( f, 0, SEEK_SET );
            fread( ptr, 1, size, f );
            fclose(f);
            input_cascade[i] = ptr;
            ptr += size;
            *ptr++ = '\0';
        }
        input_cascade[n] = 0;
    }

    if( n == 0 )
        CV_ERROR( CV_StsBadArg, "Invalid path" );

    cascade = icvLoadCascadeCART( input_cascade, n, origWindowSize );

    __END__;

    if( input_cascade != icvDefaultFaceCascade )
    {
        cvFree( (void**)&input_cascade );
    }

    if( cvGetErrStatus() < 0 )
        cvReleaseHaarClassifierCascade( &cascade );

    return cascade;
}


CV_IMPL void
cvReleaseHaarClassifierCascade( CvHaarClassifierCascade** cascade )
{
    if( cascade && *cascade )
    {
        int i;
        int j;

        for( i = 0; i < cascade[0]->count; i++ )
        {
            for( j = 0; j < cascade[0]->stageClassifier[i].count; j++ )
            {
                cvFree( (void**)
                    &(cascade[0]->stageClassifier[i].classifier[j].haarFeature) );
            }
            cvFree( (void**) &(cascade[0]->stageClassifier[i].classifier) );
        }
        cvFree( (void**)cascade );
    }
}


/* End of file. */




