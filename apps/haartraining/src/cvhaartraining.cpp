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

/*
 * cvhaartraining.cpp
 *
 * training of cascade of boosted classifiers based on haar features
 */

#include <cvhaartraining.h>
#include <_cvhaartraining.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <highgui.h>

#ifdef CV_VERBOSE
#include <time.h>

#ifdef _WIN32
/* use clock() function insted of time() */
#define TIME( arg ) (((double) clock()) / CLOCKS_PER_SEC)
#else
#define TIME( arg ) (time( arg ))
#endif /* _WIN32 */

#endif /* CV_VERBOSE */

/*
 * Background reader
 * Created in each thread
 */
CvBackgroundReader* cvbgreader = NULL;

CvBackgroundData* cvbgdata = NULL;

/*
 * get sum image offsets for <rect> corner points 
 * step - row step (measured in image pixels!) of sum image
 */
#define CV_SUM_OFFSETS( p0, p1, p2, p3, rect, step )                      \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x + w, y) */                                                      \
    (p1) = (rect).x + (rect).width + (step) * (rect).y;                   \
    /* (x + w, y) */                                                      \
    (p2) = (rect).x + (step) * ((rect).y + (rect).height);                \
    /* (x + w, y + h) */                                                  \
    (p3) = (rect).x + (rect).width + (step) * ((rect).y + (rect).height);

/*
 * get tilted image offsets for <rect> corner points 
 * step - row step (measured in image pixels!) of tilted image
 */
#define CV_TILTED_OFFSETS( p0, p1, p2, p3, rect, step )                   \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x - h, y + h) */                                                  \
    (p1) = (rect).x - (rect).height + (step) * ((rect).y + (rect).height);\
    /* (x + w, y + w) */                                                  \
    (p2) = (rect).x + (rect).width + (step) * ((rect).y + (rect).width);  \
    /* (x + w - h, y + w + h) */                                          \
    (p3) = (rect).x + (rect).width - (rect).height                        \
           + (step) * ((rect).y + (rect).width + (rect).height);


CV_IMPL
CvIntHaarFeatures* icvCreateIntHaarFeatures( CvSize winsize,
                                             int mode,
                                             int symmetric )
{
    CvIntHaarFeatures* features = NULL;
    CvHaarFeature haarFeature;
    
    CvMemStorage* storage = NULL;
    CvSeq* seq = NULL;
    CvSeqWriter writer;

    int s0 = 36; /* minimum total area size of basic haar feature     */
    int s1 = 12; /* minimum total area size of tilted haar features 2 */
    int s2 = 18; /* minimum total area size of tilted haar features 3 */
    int s3 = 24; /* minimum total area size of tilted haar features 4 */

    int x  = 0;
    int y  = 0;
    int dx = 0;
    int dy = 0;

    float factor = 1.0F;

    factor = ((float) winsize.width) * winsize.height / (24 * 24);
    s0 = (int) (s0 * factor);
    s1 = (int) (s1 * factor);
    s2 = (int) (s2 * factor);
    s3 = (int) (s3 * factor);

    /* CV_VECTOR_CREATE( vec, CvIntHaarFeature, size, maxsize ) */
    storage = cvCreateMemStorage();
    cvStartWriteSeq( 0, sizeof( CvSeq ), sizeof( haarFeature ), storage, &writer );

    for( x = 0; x < winsize.width; x++ )
    {
        for( y = 0; y < winsize.height; y++ )
        {
            for( dx = 1; dx <= winsize.width; dx++ )
            {
                for( dy = 1; dy <= winsize.height; dy++ )
                {
                    // haar_x2
                    if ( (x+dx*2 <= winsize.width) && (y+dy <= winsize.height) ) {
                        if (dx*2*dy < s0) continue;
                        if (!symmetric || (x+x+dx*2 <=winsize.width)) {
                            haarFeature = cvHaarFeature( "haar_x2",
                                x,    y, dx*2, dy, -1,
                                x+dx, y, dx  , dy, +2 );
                            /* CV_VECTOR_PUSH( vec, CvIntHaarFeature, haarFeature, size, maxsize, step ) */
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }

                        // haar_y2
                    if ( (x+dx*2 <= winsize.height) && (y+dy <= winsize.width) ) {
                        if (dx*2*dy < s0) continue;
                        if (!symmetric || (y+y+dy <= winsize.width)) {
                            haarFeature = cvHaarFeature( "haar_y2",
                                y, x,    dy, dx*2, -1,
                                y, x+dx, dy, dx,   +2 );
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }

                    // haar_x3
                    if ( (x+dx*3 <= winsize.width) && (y+dy <= winsize.height) ) {
                        if (dx*3*dy < s0) continue;
                        if (!symmetric || (x+x+dx*3 <=winsize.width)) {
                            haarFeature = cvHaarFeature( "haar_x3",
                                x,    y, dx*3, dy, -1,
                                x+dx, y, dx,   dy, +3 );
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }

                    // haar_y3
                    if ( (x+dx*3 <= winsize.height) && (y+dy <= winsize.width) ) {
                        if (dx*3*dy < s0) continue;
                        if (!symmetric || (y+y+dy <= winsize.width)) {
                            haarFeature = cvHaarFeature( "haar_y3",
                                y, x,    dy, dx*3, -1,
                                y, x+dx, dy, dx,   +3 );
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }

                    if( mode != 0 /*BASIC*/ ) {
                        // haar_x4
                        if ( (x+dx*4 <= winsize.width) && (y+dy <= winsize.height) ) {
                            if (dx*4*dy < s0) continue;
                            if (!symmetric || (x+x+dx*4 <=winsize.width)) {
                                haarFeature = cvHaarFeature( "haar_x4",
                                    x,    y, dx*4, dy, -1,
                                    x+dx, y, dx*2, dy, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                            
                        // haar_y4
                        if ( (x+dx*4 <= winsize.height) && (y+dy <= winsize.width ) ) {
                            if (dx*4*dy < s0) continue;
                            if (!symmetric || (y+y+dy   <=winsize.width)) {
                                haarFeature = cvHaarFeature( "haar_y4",
                                    y, x,    dy, dx*4, -1,
                                    y, x+dx, dy, dx*2, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                    }

                    // x2_y2
                    if ( (x+dx*2 <= winsize.width) && (y+dy*2 <= winsize.height) ) {
                        if (dx*4*dy < s0) continue;
                        if (!symmetric || (x+x+dx*2 <=winsize.width)) {
                            haarFeature = cvHaarFeature( "haar_x2_y2",
                                x   , y,    dx*2, dy*2, -1,
                                x   , y   , dx  , dy,   +2,
                                x+dx, y+dy, dx  , dy,   +2 );
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }

                    if (mode != 0 /*BASIC*/) {                
                        // point
                        if ( (x+dx*3 <= winsize.width) && (y+dy*3 <= winsize.height) ) {
                            if (dx*9*dy < s0) continue;
                            if (!symmetric || (x+x+dx*3 <=winsize.width))  {
                                haarFeature = cvHaarFeature( "haar_point",
                                    x   , y,    dx*3, dy*3, -1,
                                    x+dx, y+dy, dx  , dy  , +9);
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                    }
                    
                    if (mode == 2 /*ALL*/) {                
                        // tilted haar_x2                                      (x, y, w, h, b, weight)
                        if ( (x+2*dx <= winsize.width) && (y+2*dx+dy <= winsize.height) && (x-dy>= 0) ) {
                            if (dx*2*dy < s1) continue;
                            
                            if (!symmetric || (x <= (winsize.width / 2) )) {
                                haarFeature = cvHaarFeature( "tilted_haar_x2",
                                    x, y, dx*2, dy, -1,
                                    x, y, dx  , dy, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        
                        // tilted haar_y2                                      (x, y, w, h, b, weight)
                        if ( (x+dx <= winsize.width) && (y+dx+2*dy <= winsize.height) && (x-2*dy>= 0) ) {
                            if (dx*2*dy < s1) continue;
                            
                            if (!symmetric || (x <= (winsize.width / 2) )) {
                                haarFeature = cvHaarFeature( "tilted_haar_y2",
                                    x, y, dx, 2*dy, -1,
                                    x, y, dx,   dy, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        
                        // tilted haar_x3                                   (x, y, w, h, b, weight)
                        if ( (x+3*dx <= winsize.width) && (y+3*dx+dy <= winsize.height) && (x-dy>= 0) ) {
                            if (dx*3*dy < s2) continue;
                            
                            if (!symmetric || (x <= (winsize.width / 2) )) {
                                haarFeature = cvHaarFeature( "tilted_haar_x3",
                                    x,    y,    dx*3, dy, -1,
                                    x+dx, y+dx, dx  , dy, +3 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        
                        // tilted haar_y3                                      (x, y, w, h, b, weight)
                        if ( (x+dx <= winsize.width) && (y+dx+3*dy <= winsize.height) && (x-3*dy>= 0) ) {
                            if (dx*3*dy < s2) continue;
                            
                            if (!symmetric || (x <= (winsize.width / 2) )) {
                                haarFeature = cvHaarFeature( "tilted_haar_y3",
                                    x,    y,    dx, 3*dy, -1,
                                    x-dy, y+dy, dx,   dy, +3 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        
                        
                        // tilted haar_x4                                   (x, y, w, h, b, weight)
                        if ( (x+4*dx <= winsize.width) && (y+4*dx+dy <= winsize.height) && (x-dy>= 0) ) {
                            if (dx*4*dy < s3) continue;
                            
                            if (!symmetric || (x <= (winsize.width / 2) )) {
                                haarFeature = cvHaarFeature( "tilted_haar_x4",


                                    x,    y,    dx*4, dy, -1,
                                    x+dx, y+dx, dx*2, dy, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        
                        // tilted haar_y4                                      (x, y, w, h, b, weight)
                        if ( (x+dx <= winsize.width) && (y+dx+4*dy <= winsize.height) && (x-4*dy>= 0) ) {
                            if (dx*4*dy < s3) continue;
                            
                            if (!symmetric || (x <= (winsize.width / 2) )) {
                                haarFeature = cvHaarFeature( "tilted_haar_y4",
                                    x,    y,    dx, 4*dy, -1,
                                    x-dy, y+dy, dx, 2*dy, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        

                        /*
                        
                          // tilted point
                          if ( (x+dx*3 <= winsize.width - 1) && (y+dy*3 <= winsize.height - 1) && (x-3*dy>= 0)) {
                          if (dx*9*dy < 36) continue;
                          if (!symmetric || (x <= (winsize.width / 2) ))  {
                            haarFeature = cvHaarFeature( "tilted_haar_point",
                                x, y,    dx*3, dy*3, -1,
                                x, y+dy, dx  , dy,   +9 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                          }
                          }
                        */
                    }
                }
            }
        }
    }

    seq = cvEndWriteSeq( &writer );
    features = (CvIntHaarFeatures*) cvAlloc( sizeof( CvIntHaarFeatures ) +
        ( sizeof( CvHaarFeature ) + sizeof( CvFastHaarFeature ) ) * seq->total );
    features->feature = (CvHaarFeature*) (features + 1);
    features->fastfeature = (CvFastHaarFeature*) ( features->feature + seq->total );
    features->count = seq->total;
    features->winsize = winsize;
    cvCvtSeqToArray( seq, (CvArr*) features->feature );
    cvReleaseMemStorage( &storage );
    
    icvConvertToFastHaarFeature( features->feature, features->fastfeature,
                                 features->count, (winsize.width + 1) );
    
    return features;
}

CV_IMPL
void icvReleaseIntHaarFeatures( CvIntHaarFeatures** intHaarFeatures )
{
    if( intHaarFeatures != NULL && (*intHaarFeatures) != NULL )
    {
        cvFree( (void**) intHaarFeatures );
        (*intHaarFeatures) = NULL;
    }
}

CV_IMPL
void icvConvertToFastHaarFeature( CvHaarFeature* haarFeature,
                                  CvFastHaarFeature* fastHaarFeature,
                                  int size, int step )
{
    int i = 0;
    int j = 0;

    for( i = 0; i < size; i++ )
    {
        fastHaarFeature[i].tilted = haarFeature[i].tilted;
        if( !fastHaarFeature[i].tilted )
        {
            for( j = 0; j < CV_HAAR_FEATURE_MAX; j++ )
            {
                fastHaarFeature[i].rect[j].weight = haarFeature[i].rect[j].weight;
                if( fastHaarFeature[i].rect[j].weight == 0.0F )
                {
                    break;
                }
                CV_SUM_OFFSETS( fastHaarFeature[i].rect[j].p0,
                                fastHaarFeature[i].rect[j].p1,
                                fastHaarFeature[i].rect[j].p2,
                                fastHaarFeature[i].rect[j].p3,
                                haarFeature[i].rect[j].r, step )
            }
            
        }
        else
        {
            for( j = 0; j < CV_HAAR_FEATURE_MAX; j++ )
            {
                fastHaarFeature[i].rect[j].weight = haarFeature[i].rect[j].weight;
                if( fastHaarFeature[i].rect[j].weight == 0.0F )
                {
                    break;
                }
                CV_TILTED_OFFSETS( fastHaarFeature[i].rect[j].p0,
                                   fastHaarFeature[i].rect[j].p1,
                                   fastHaarFeature[i].rect[j].p2,
                                   fastHaarFeature[i].rect[j].p3,
                                   haarFeature[i].rect[j].r, step )
            }
        }
    }
}

CV_IMPL
CvHaarTrainigData* icvCreateHaarTrainingData( CvSize winsize, int maxnumsamples )
{
    CvHaarTrainigData* data;
    
    CV_FUNCNAME( "icvCreateHaarTrainingData" );
    
    __BEGIN__;

    data = NULL;
    uchar* ptr = NULL;
    size_t datasize = 0;
    
    datasize = sizeof( CvHaarTrainigData ) +
          /* sum and tilted */
        ( 2 * (winsize.width + 1) * (winsize.height + 1) * sizeof( sum_type ) +
          sizeof( float ) +      /* normfactor */
          sizeof( float ) +      /* cls */
          sizeof( float )        /* weight */
        ) * maxnumsamples;

    CV_CALL( data = (CvHaarTrainigData*) cvAlloc( datasize ) );
    memset( (void*)data, 0, datasize );
    data->maxnum = maxnumsamples;
    data->winsize = winsize;
    ptr = (uchar*)(data + 1);
    data->sum = cvMat( maxnumsamples, (winsize.width + 1) * (winsize.height + 1),
                       CV_SUM_MAT_TYPE, (void*) ptr );
    ptr += sizeof( sum_type ) * maxnumsamples * (winsize.width+1) * (winsize.height+1);
    data->tilted = cvMat( maxnumsamples, (winsize.width + 1) * (winsize.height + 1),
                       CV_SUM_MAT_TYPE, (void*) ptr );
    ptr += sizeof( sum_type ) * maxnumsamples * (winsize.width+1) * (winsize.height+1);
    data->normfactor = cvMat( 1, maxnumsamples, CV_32FC1, (void*) ptr );
    ptr += sizeof( float ) * maxnumsamples;
    data->cls = cvMat( 1, maxnumsamples, CV_32FC1, (void*) ptr );
    ptr += sizeof( float ) * maxnumsamples;
    data->weights = cvMat( 1, maxnumsamples, CV_32FC1, (void*) ptr );

    data->valcache = NULL;
    data->idxcache = NULL;

    __END__;

    return data;
}

CV_IMPL
void icvReleaseHaarTrainingDataCache( CvHaarTrainigData** haarTrainingData )
{
    if( haarTrainingData != NULL && (*haarTrainingData) != NULL )
    {
        if( (*haarTrainingData)->valcache != NULL )
        {
            cvReleaseMat( &(*haarTrainingData)->valcache );
            (*haarTrainingData)->valcache = NULL;
        }
        if( (*haarTrainingData)->idxcache != NULL )
        {
            cvReleaseMat( &(*haarTrainingData)->idxcache );
            (*haarTrainingData)->idxcache = NULL;
        }
    }
}

CV_IMPL
void icvReleaseHaarTrainingData( CvHaarTrainigData** haarTrainingData )
{
    if( haarTrainingData != NULL && (*haarTrainingData) != NULL )
    {
        icvReleaseHaarTrainingDataCache( haarTrainingData );

        cvFree( (void**) haarTrainingData );
    }
}

CV_IMPL
void icvGetTrainingDataCallback( CvMat* mat, CvMat* sampleIdx, CvMat*,
                                 int first, int num, void* userdata )
{
    int i = 0;
    int j = 0;
    float val = 0.0F;
    float normfactor = 0.0F;
    
    CvHaarTrainingData* training_data;
    CvIntHaarFeatures* haar_features;

#ifdef CV_COL_ARRANGEMENT
    assert( mat->rows >= num );
#else
    assert( mat->cols >= num );
#endif

    training_data = ((CvUserdata*) userdata)->trainingData;
    haar_features = ((CvUserdata*) userdata)->haarFeatures;
    if( sampleIdx == NULL )
    {
        int num_samples;

#ifdef CV_COL_ARRANGEMENT
        num_samples = mat->cols;
#else
        num_samples = mat->rows;
#endif
        for( i = 0; i < num_samples; i++ )
        {
            for( j = 0; j < num; j++ )
            {
                val = cvEvalFastHaarFeature(
                        ( haar_features->fastfeature
                            + first + j ),
                        (sum_type*) (training_data->sum.data.ptr
                            + i * training_data->sum.step),
                        (sum_type*) (training_data->tilted.data.ptr
                            + i * training_data->tilted.step) );
                normfactor = training_data->normfactor.data.fl[i];
                val = ( normfactor == 0.0F ) ? 0.0F : (val / normfactor);

#ifdef CV_COL_ARRANGEMENT
                CV_MAT_ELEM( *mat, float, j, i ) = val;
#else
                CV_MAT_ELEM( *mat, float, i, j ) = val;
#endif
            }
        }
    }
    else
    {
        uchar* idxdata = NULL;
        size_t step    = 0;
        int    numidx  = 0;
        int    idx     = 0;

        assert( CV_MAT_TYPE( sampleIdx->type ) == CV_32FC1 );

        idxdata = sampleIdx->data.ptr;
        if( sampleIdx->rows == 1 )
        {
            step = sizeof( float );
            numidx = sampleIdx->cols;
        }
        else
        {
            step = sampleIdx->step;
            numidx = sampleIdx->rows;
        }

        for( i = 0; i < numidx; i++ )
        {
            for( j = 0; j < num; j++ )
            {
                idx = (int)( *((float*) (idxdata + i * step)) );
                val = cvEvalFastHaarFeature(
                        ( haar_features->fastfeature
                            + first + j ),
                        (sum_type*) (training_data->sum.data.ptr
                            + idx * training_data->sum.step),
                        (sum_type*) (training_data->tilted.data.ptr
                            + idx * training_data->tilted.step) );
                normfactor = training_data->normfactor.data.fl[idx];
                val = ( normfactor == 0.0F ) ? 0.0F : (val / normfactor);

#ifdef CV_COL_ARRANGEMENT
                CV_MAT_ELEM( *mat, float, j, idx ) = val;
#else
                CV_MAT_ELEM( *mat, float, idx, j ) = val;
#endif

            }
        }
    }
#if 0 /*def CV_VERBOSE*/
    if( first % 5000 == 0 )
    {
        fprintf( stderr, "%3d%%\r", (int) (100.0 * first / 
            haar_features->count) );
        fflush( stderr );
    }
#endif /* CV_VERBOSE */
}

CV_IMPL
void icvPrecalculate( CvHaarTrainingData* data, CvIntHaarFeatures* haarFeatures,
                      int numprecalculated )
{
    CV_FUNCNAME( "icvPrecalculate" );

    __BEGIN__;

    icvReleaseHaarTrainingDataCache( &data );

    numprecalculated -= numprecalculated % CV_STUMP_TRAIN_PORTION;
    numprecalculated = MIN( numprecalculated, haarFeatures->count );

    if( numprecalculated > 0 )
    {
        int portion = CV_STUMP_TRAIN_PORTION;
        int idx = 0;
        //size_t datasize;
        int m;
        CvUserdata userdata;

        /* private variables */
        #ifdef _OPENMP
        CvMat t_data;
        CvMat t_idx;
        int first;
        int t_portion;
        #endif /* _OPENMP */

        m = data->sum.rows;

#ifdef CV_COL_ARRANGEMENT
        CV_CALL( data->valcache = cvCreateMat( numprecalculated, m, CV_32FC1 ) );
#else
        CV_CALL( data->valcache = cvCreateMat( m, numprecalculated, CV_32FC1 ) );
#endif
        CV_CALL( data->idxcache = cvCreateMat( numprecalculated, m, CV_IDX_MAT_TYPE ) );

        userdata = cvUserdata( data, haarFeatures );

        #ifdef _OPENMP
        #pragma omp parallel for private(t_data, t_idx, first, t_portion)
        for( first = 0; first < numprecalculated; first += portion )
        {
            t_data = *data->valcache;
            t_idx = *data->idxcache;
            t_portion = MIN( portion, (numprecalculated - first) );
            
            /* indices */
            t_idx.rows = t_portion;
            t_idx.data.ptr = data->idxcache->data.ptr + first * ((size_t)t_idx.step);

            /* feature values */
#ifdef CV_COL_ARRANGEMENT
            t_data.rows = t_portion;
            t_data.data.ptr = data->valcache->data.ptr +
                first * ((size_t) t_data.step );
#else
            t_data.cols = t_portion;
            t_data.data.ptr = data->valcache->data.ptr +
                first * ((size_t) CV_ELEM_SIZE( t_data.type ));
#endif
            icvGetTrainingDataCallback( &t_data, NULL, NULL, first, t_portion,
                                        &userdata );
#ifdef CV_COL_ARRANGEMENT
            cvGetSortedIndices( &t_data, &t_idx, 0 );
#else
            cvGetSortedIndices( &t_data, &t_idx, 1 );
#endif

#ifdef CV_VERBOSE
            putc( '.', stderr );
            fflush( stderr );
#endif /* CV_VERBOSE */

        }

#ifdef CV_VERBOSE
        fprintf( stderr, "\n" );
        fflush( stderr );
#endif /* CV_VERBOSE */

        #else
        icvGetTrainingDataCallback( data->valcache, NULL, NULL, 0, numprecalculated,
                                    &userdata );
#ifdef CV_COL_ARRANGEMENT
        cvGetSortedIndices( data->valcache, data->idxcache, 0 );
#else
        cvGetSortedIndices( data->valcache, data->idxcache, 1 );
#endif
        #endif /* _OPENMP */
    }

    __END__;
}

CV_IMPL
void icvSplitIndicesCallback( int compidx, float threshold,
                              CvMat* idx, CvMat** left, CvMat** right,
                              void* userdata )
{
    CvHaarTrainingData* data;
    CvIntHaarFeatures* haar_features;
    int i;
    int m;
    CvFastHaarFeature* fastfeature;

    data = ((CvUserdata*) userdata)->trainingData;
    haar_features = ((CvUserdata*) userdata)->haarFeatures;
    fastfeature = &haar_features->fastfeature[compidx];

    m = data->sum.rows;
    *left = cvCreateMat( 1, m, CV_32FC1 );
    *right = cvCreateMat( 1, m, CV_32FC1 );
    (*left)->cols = (*right)->cols = 0;
    if( idx == NULL )
    {
        for( i = 0; i < m; i++ )
        {
            if( cvEvalFastHaarFeature( fastfeature,
                    (sum_type*) (data->sum.data.ptr + i * data->sum.step),
                    (sum_type*) (data->tilted.data.ptr + i * data->tilted.step) ) 
                < threshold * data->normfactor.data.fl[i] )
            {
                (*left)->data.fl[(*left)->cols++] = (float) i;
            }
            else
            {
                (*right)->data.fl[(*right)->cols++] = (float) i;
            }
        }
    }
    else
    {
        uchar* idxdata;
        int    idxnum;
        size_t idxstep;
        int    index;

        idxdata = idx->data.ptr;
        idxnum = (idx->rows == 1) ? idx->cols : idx->rows;
        idxstep = (idx->rows == 1) ? CV_ELEM_SIZE( idx->type ) : idx->step;
        for( i = 0; i < idxnum; i++ )
        {
            index = (int) *((float*) (idxdata + i * idxstep));
            if( cvEvalFastHaarFeature( fastfeature,
                    (sum_type*) (data->sum.data.ptr + index * data->sum.step),
                    (sum_type*) (data->tilted.data.ptr + index * data->tilted.step) ) 
                < threshold * data->normfactor.data.fl[index] )
            {
                (*left)->data.fl[(*left)->cols++] = (float) index;
            }
            else
            {
                (*right)->data.fl[(*right)->cols++] = (float) index;
            }
        }
    }
}

CV_IMPL
CvIntHaarClassifier* icvCreateCARTStageClassifier( CvHaarTrainingData* data,
                                                   CvMat* sampleIdx,
                                                   CvIntHaarFeatures* haarFeatures,
                                                   float minhitrate,
                                                   float maxfalsealarm,
                                                   int   symmetric,
                                                   float weightfraction,
                                                   int numsplits,
                                                   CvBoostType boosttype,
                                                   CvStumpError stumperror,
                                                   int maxsplits )
{

#ifdef CV_COL_ARRANGEMENT
    int flags = CV_COL_SAMPLE;
#else
    int flags = CV_ROW_SAMPLE;
#endif

    CvStageHaarClassifier* stage = NULL;
    CvBoostTrainer* trainer;
    CvCARTClassifier* cart = NULL;
    CvCARTTrainParams trainParams;
    CvMTStumpTrainParams stumpTrainParams;
    //CvMat* trainData = NULL;
    //CvMat* sortedIdx = NULL;
    CvMat eval;
    int n = 0;
    int m = 0;
    size_t datasize = 0;
    int numpos = 0;
    int numneg = 0;
    int numfalse = 0;
    float sum_stage = 0.0F;
    float threshold = 0.0F;
    float falsealarm = 0.0F;
    
    //CvMat* sampleIdx = NULL;
    CvMat* trimmedIdx;
    //float* idxdata = NULL;
    //float* tempweights = NULL;
    //int    idxcount = 0;
    CvUserdata userdata;

    int i = 0;
    int j = 0;
    int idx;
    int numsamples;
    int numtrimmed;
    
    CvCARTHaarClassifier* classifier;
    CvSeq* seq = NULL;
    CvMemStorage* storage = NULL;
    CvMat* weakTrainVals;
    float alpha;
    float sumalpha;
    int num_splits; /* total number of splits in all weak classifiers */

#ifdef CV_VERBOSE
    printf( "+----+----+-+---------+---------+---------+---------+\n" );
    printf( "|  N |%%SMP|F|  ST.THR |    HR   |    FA   | EXP. ERR|\n" );
    printf( "+----+----+-+---------+---------+---------+---------+\n" );
#endif /* CV_VERBOSE */
    
    n = haarFeatures->count;
    m = data->sum.rows;
    numsamples = (sampleIdx) ? MAX( sampleIdx->rows, sampleIdx->cols ) : m;

    userdata = cvUserdata( data, haarFeatures );

    stumpTrainParams.type = ( boosttype == CV_DABCLASS )
        ? CV_CLASSIFICATION_CLASS : CV_REGRESSION;
    stumpTrainParams.error = ( boosttype == CV_LBCLASS || boosttype == CV_GABCLASS )
        ? CV_SQUARE : stumperror;
    stumpTrainParams.portion = CV_STUMP_TRAIN_PORTION;
    stumpTrainParams.getTrainData = icvGetTrainingDataCallback;
    stumpTrainParams.numcomp = n;
    stumpTrainParams.userdata = &userdata;
    stumpTrainParams.sortedIdx = data->idxcache;

    trainParams.count = numsplits;
    trainParams.stumpTrainParams = (CvClassifierTrainParams*) &stumpTrainParams;
    trainParams.stumpConstructor = cvCreateMTStumpClassifier;
    trainParams.splitIdx = icvSplitIndicesCallback;
    trainParams.userdata = &userdata;

    eval = cvMat( 1, m, CV_32FC1, cvAlloc( sizeof( float ) * m ) );
    
    storage = cvCreateMemStorage();
    seq = cvCreateSeq( 0, sizeof( *seq ), sizeof( classifier ), storage );

    weakTrainVals = cvCreateMat( 1, m, CV_32FC1 );
    trainer = cvBoostStartTraining( &data->cls, weakTrainVals, &data->weights,
                                    sampleIdx, boosttype );
    num_splits = 0;
    sumalpha = 0.0F;
    do
    {     

#ifdef CV_VERBOSE
        int v_wt = 0;
        int v_flipped = 0;
#endif /* CV_VERBOSE */

        trimmedIdx = cvTrimWeights( &data->weights, sampleIdx, weightfraction );
        numtrimmed = (trimmedIdx) ? MAX( trimmedIdx->rows, trimmedIdx->cols ) : m;

#ifdef CV_VERBOSE
        v_wt = 100 * numtrimmed / numsamples;
        v_flipped = 0;

#endif /* CV_VERBOSE */

        cart = (CvCARTClassifier*) cvCreateCARTClassifier( data->valcache,
                        flags,
                        weakTrainVals, 0, 0, 0, trimmedIdx,
                        &(data->weights),
                        (CvClassifierTrainParams*) &trainParams );

        classifier = (CvCARTHaarClassifier*) icvCreateCARTHaarClassifier( numsplits );
        icvInitCARTHaarClassifier( classifier, cart, haarFeatures );

        num_splits += classifier->count;

        cart->release( (CvClassifier**) &cart );
        
        if( symmetric && (seq->total % 2) )
        {
            float normfactor = 0.0F;
            CvStumpClassifier* stump;
            
            /* flip haar features */
            for( i = 0; i < classifier->count; i++ )
            {
                if( classifier->feature[i].desc[0] == 'h' )
                {
                    for( j = 0; j < CV_HAAR_FEATURE_MAX &&
                                    classifier->feature[i].rect[j].weight != 0.0F; j++ )
                    {
                        classifier->feature[i].rect[j].r.x = data->winsize.width - 
                            classifier->feature[i].rect[j].r.x -
                            classifier->feature[i].rect[j].r.width;                
                    }
                }
                else
                {
                    int tmp = 0;

                    /* (x,y) -> (24-x,y) */
                    /* w -> h; h -> w    */
                    for( j = 0; j < CV_HAAR_FEATURE_MAX &&
                                    classifier->feature[i].rect[j].weight != 0.0F; j++ )
                    {
                        classifier->feature[i].rect[j].r.x = data->winsize.width - 
                            classifier->feature[i].rect[j].r.x;
                        CV_SWAP( classifier->feature[i].rect[j].r.width,
                                 classifier->feature[i].rect[j].r.height, tmp );
                    }
                }
            }
            icvConvertToFastHaarFeature( classifier->feature,
                                         classifier->fastfeature,
                                         classifier->count, data->winsize.width + 1 );
            for( i = 0; i < classifier->count; i++ )
            {
                for( j = 0; j < numtrimmed; j++ )
                {
                    idx = icvGetIdxAt( trimmedIdx, j );

                    eval.data.fl[idx] = cvEvalFastHaarFeature( &classifier->fastfeature[i],
                        (sum_type*) (data->sum.data.ptr + idx * data->sum.step),
                        (sum_type*) (data->tilted.data.ptr + idx * data->tilted.step) );
                    normfactor = data->normfactor.data.fl[idx];
                    eval.data.fl[idx] = ( normfactor == 0.0F )
                        ? 0.0F : (eval.data.fl[idx] / normfactor);
                }

                stumpTrainParams.getTrainData = NULL;
                stumpTrainParams.numcomp = 1;
                stumpTrainParams.userdata = NULL;
                stumpTrainParams.sortedIdx = NULL;

                stump = (CvStumpClassifier*) trainParams.stumpConstructor( &eval,
                    CV_COL_SAMPLE,
                    weakTrainVals, 0, 0, 0, trimmedIdx,
                    &(data->weights),
                    trainParams.stumpTrainParams );
            
                classifier->threshold[i] = stump->threshold;
                if( classifier->left[i] <= 0 )
                {
                    classifier->val[-classifier->left[i]] = stump->left;
                }
                if( classifier->right[i] <= 0 )
                {
                    classifier->val[-classifier->right[i]] = stump->right;
                }

                stump->release( (CvClassifier**) &stump );        
                
                stumpTrainParams.getTrainData = icvGetTrainingDataCallback;
                stumpTrainParams.numcomp = n;
                stumpTrainParams.userdata = &userdata;
                stumpTrainParams.sortedIdx = data->idxcache;
            }

#ifdef CV_VERBOSE
            v_flipped = 1;
#endif /* CV_VERBOSE */

        } /* if symmetric */
        if( trimmedIdx != sampleIdx )
        {
            cvReleaseMat( &trimmedIdx );
            trimmedIdx = NULL;
        }
        
        for( i = 0; i < numsamples; i++ )
        {
            idx = icvGetIdxAt( sampleIdx, i );

            eval.data.fl[idx] = classifier->eval( (CvIntHaarClassifier*) classifier,
                (sum_type*) (data->sum.data.ptr + idx * data->sum.step),
                (sum_type*) (data->tilted.data.ptr + idx * data->tilted.step),
                data->normfactor.data.fl[idx] );
        }

        alpha = cvBoostNextWeakClassifier( &eval, &data->cls, weakTrainVals,
                                           &data->weights, trainer );
        sumalpha += alpha;
        
        for( i = 0; i <= classifier->count; i++ )
        {
            if( boosttype == CV_RABCLASS ) 
            {
                classifier->val[i] = cvLogRatio( classifier->val[i] );
            }
            classifier->val[i] *= alpha;
        }

        cvSeqPush( seq, (void*) &classifier );

        numpos = 0;
        for( i = 0; i < numsamples; i++ )
        {
            idx = icvGetIdxAt( sampleIdx, i );

            if( data->cls.data.fl[idx] == 1.0F )
            {
                eval.data.fl[numpos] = 0.0F;
                for( j = 0; j < seq->total; j++ )
                {
                    classifier = *((CvCARTHaarClassifier**) cvGetSeqElem( seq, j ));
                    eval.data.fl[numpos] += classifier->eval( 
                        (CvIntHaarClassifier*) classifier,
                        (sum_type*) (data->sum.data.ptr + idx * data->sum.step),
                        (sum_type*) (data->tilted.data.ptr + idx * data->tilted.step),
                        data->normfactor.data.fl[idx] );
                }
                /* eval.data.fl[numpos] = 2.0F * eval.data.fl[numpos] - seq->total; */
                numpos++;
            }
        }
        icvSort_32f( eval.data.fl, numpos, 0 );
        threshold = eval.data.fl[(int) ((1.0F - minhitrate) * numpos)];

        numneg = 0;
        numfalse = 0;
        for( i = 0; i < numsamples; i++ )
        {
            idx = icvGetIdxAt( sampleIdx, i );

            if( data->cls.data.fl[idx] == 0.0F )
            {
                numneg++;
                sum_stage = 0.0F;
                for( j = 0; j < seq->total; j++ )
                {
                   classifier = *((CvCARTHaarClassifier**) cvGetSeqElem( seq, j ));
                   sum_stage += classifier->eval( (CvIntHaarClassifier*) classifier,
                        (sum_type*) (data->sum.data.ptr + idx * data->sum.step),
                        (sum_type*) (data->tilted.data.ptr + idx * data->tilted.step),
                        data->normfactor.data.fl[idx] );
                }
                /* sum_stage = 2.0F * sum_stage - seq->total; */
                if( sum_stage >= (threshold - CV_THRESHOLD_EPS) )
                {
                    numfalse++;
                }
            }
        }
        falsealarm = ((float) numfalse) / ((float) numneg);

#ifdef CV_VERBOSE
        {
            float v_hitrate    = 0.0F;
            float v_falsealarm = 0.0F;
            /* expected error of stage classifier regardless threshold */
            float v_experr = 0.0F;

            for( i = 0; i < numsamples; i++ )
            {
                idx = icvGetIdxAt( sampleIdx, i );

                sum_stage = 0.0F;
                for( j = 0; j < seq->total; j++ )
                {
                    classifier = *((CvCARTHaarClassifier**) cvGetSeqElem( seq, j ));
                    sum_stage += classifier->eval( (CvIntHaarClassifier*) classifier,
                        (sum_type*) (data->sum.data.ptr + idx * data->sum.step),
                        (sum_type*) (data->tilted.data.ptr + idx * data->tilted.step),
                        data->normfactor.data.fl[idx] );
                }
                /* sum_stage = 2.0F * sum_stage - seq->total; */
                if( sum_stage >= (threshold - CV_THRESHOLD_EPS) )
                {
                    if( data->cls.data.fl[idx] == 1.0F )
                    {
                        v_hitrate += 1.0F;
                    }
                    else
                    {
                        v_falsealarm += 1.0F;
                    }
                }
                if( ( sum_stage >= 0.0F ) != (data->cls.data.fl[idx] == 1.0F) )
                {
                    v_experr += 1.0F;
                }
            }
            v_experr /= numsamples;
            printf( "|%4d|%3d%%|%c|%9f|%9f|%9f|%9f|\n",
                seq->total, v_wt, ( (v_flipped) ? '+' : '-' ),
                threshold, v_hitrate / numpos, v_falsealarm / numneg,
                v_experr );
            printf( "+----+----+-+---------+---------+---------+---------+\n" );
            fflush( stdout );
        }
#endif /* CV_VERBOSE */
        
    } while( falsealarm > maxfalsealarm && (!maxsplits || (num_splits < maxsplits) ) );
    cvBoostEndTraining( &trainer );

    if( falsealarm > maxfalsealarm )
    {
        stage = NULL;
    }
    else
    {
        stage = (CvStageHaarClassifier*) icvCreateStageHaarClassifier( seq->total,
                                                                       threshold );
        cvCvtSeqToArray( seq, (CvArr*) stage->classifier );
    }
    
    /* CLEANUP */
    cvReleaseMemStorage( &storage );
    cvReleaseMat( &weakTrainVals );
    cvFree( (void**) &(eval.data.ptr) );
    
    return (CvIntHaarClassifier*) stage;
}


CV_IMPL
CvBackgroundData* icvCreateBackgroundData( const char* filename, CvSize winsize )
{
    CvBackgroundData* data = NULL;
    
    const char* dir = NULL;    
    char full[PATH_MAX];
    char* imgfilename = NULL;
    size_t datasize = 0;
    int    count = 0;
    FILE*  input = NULL;
    char*  tmp   = NULL;
    int    len   = 0;

    assert( filename != NULL );
    
    dir = strrchr( filename, '\\' );
    if( dir == NULL )
    {
        dir = strrchr( filename, '/' );
    }
    if( dir == NULL )
    {
        imgfilename = &(full[0]);
    }
    else
    {
        strncpy( &(full[0]), filename, (dir - filename + 1) );
        imgfilename = &(full[(dir - filename + 1)]);
    }

    input = fopen( filename, "r" );
    if( input != NULL )
    {
        count = 0;
        datasize = 0;
        
        /* count */
        while( !feof( input ) )
        {
            *imgfilename = '\0';
            fscanf( input, "%s", imgfilename );
            len = strlen( imgfilename );
            if( len > 0 )
            {
                if( (*imgfilename) == '#' ) continue; /* comment */
                count++;
                datasize += sizeof( char ) * (strlen( &(full[0]) ) + 1);
            }
        }
        if( count > 0 )
        {
            rewind( input );
            datasize += sizeof( *data ) + sizeof( char* ) * count;
            data = (CvBackgroundData*) cvAlloc( datasize );
            memset( (void*) data, 0, datasize );
            data->count = count;
            data->filename = (char**) (data + 1);
            data->last = 0;
            data->round = 0;
            data->winsize = winsize;
            tmp = (char*) (data->filename + data->count);
            count = 0;
            while( !feof( input ) )
            {
                *imgfilename = '\0';
                fscanf( input, "%s", imgfilename );
                len = strlen( imgfilename );
                if( len > 0 )
                {
                    if( (*imgfilename) == '#' ) continue; /* comment */
                    data->filename[count++] = tmp;
                    strcpy( tmp, &(full[0]) );
                    tmp += strlen( &(full[0]) ) + 1;
                }
            }
        }
        fclose( input );
    }

    return data;
}

CV_IMPL
void icvReleaseBackgroundData( CvBackgroundData** data )
{
    assert( data != NULL && (*data) != NULL );

    cvFree( (void**) data );
}

CV_IMPL
CvBackgroundReader* icvCreateBackgroundReader()
{
    CvBackgroundReader* reader = NULL;

    reader = (CvBackgroundReader*) cvAlloc( sizeof( *reader ) );
    memset( (void*) reader, 0, sizeof( *reader ) );
    reader->src = cvMat( 0, 0, CV_8UC1, NULL );
    reader->img = cvMat( 0, 0, CV_8UC1, NULL );
    reader->offset = cvPoint( 0, 0 );
    reader->scale       = 1.0F;
    reader->scalefactor = 1.4142135623730950488016887242097F;
    reader->stepfactor  = 0.5F;
    reader->point = reader->offset;

    return reader;
}

CV_IMPL
void icvReleaseBackgroundReader( CvBackgroundReader** reader )
{
    assert( reader != NULL && (*reader) != NULL );

    if( (*reader)->src.data.ptr != NULL )
    {
        cvFree( (void**) &((*reader)->src.data.ptr) );
    }
    if( (*reader)->img.data.ptr != NULL )
    {
        cvFree( (void**) &((*reader)->img.data.ptr) );
    }

    cvFree( (void**) reader );
}

CV_IMPL
void icvGetNextFromBackgroundData( CvBackgroundData* data,
                                   CvBackgroundReader* reader )
{
    IplImage* img = NULL;
    char* filename = NULL;
    size_t datasize = 0;
    int round = 0;
    int i = 0;

    assert( data != NULL && reader != NULL );

    if( reader->src.data.ptr != NULL )
    {
        cvFree( (void**) &(reader->src.data.ptr) );
        reader->src.data.ptr = NULL;
    }
    if( reader->img.data.ptr != NULL )
    {
        cvFree( (void**) &(reader->img.data.ptr) );
        reader->img.data.ptr = NULL;
    }

    #ifdef _OPENMP
    #pragma omp critical(c_background_data)
    #endif /* _OPENMP */
    {
        for( i = 0; i < data->count; i++ )
        {
            round = data->round;

//#ifdef CV_VERBOSE 
//            printf( "Open background image: %s\n", data->filename[data->last] );
//#endif /* CV_VERBOSE */
          
            img = cvLoadImage( data->filename[data->last++], 0 );
            data->round += data->last / data->count;
            data->round = data->round % (data->winsize.width * data->winsize.height);
            data->last %= data->count;

            if( img != NULL && img->depth == IPL_DEPTH_8U && img->nChannels == 1 &&
                img->width >= data->winsize.width && img->height >= data->winsize.height )
            {
                break;
            }
            if( img != NULL )
                cvReleaseImage( &img );
            img = NULL;
        }
    }
    if( img == NULL )
    {
        /* no appropriate image */

#ifdef CV_VERBOSE
        printf( "Invalid background description file.\n" );
#endif /* CV_VERBOSE */

        assert( 0 );
        exit( 1 );
    }
    datasize = sizeof( uchar ) * img->width * img->height;
    reader->src = cvMat( img->height, img->width, CV_8UC1, (void*) cvAlloc( datasize ) );
    cvCopy( img, &reader->src, NULL );
    cvReleaseImage( &img );
    img = NULL;

    reader->offset.x = round % data->winsize.width;
    reader->offset.y = round / data->winsize.width;
    reader->point = reader->offset;
    reader->scale = MAX(
        ((float) data->winsize.width + reader->point.x) / ((float) reader->src.cols),
        ((float) data->winsize.height + reader->point.y) / ((float) reader->src.rows) );
    
    reader->img = cvMat( (int) (reader->scale * reader->src.rows + 0.5F),
                         (int) (reader->scale * reader->src.cols + 0.5F),
                          CV_8UC1, (void*) cvAlloc( datasize ) );
    cvResize( &(reader->src), &(reader->img) );
}

CV_IMPL
void icvGetBackgroundImage( CvBackgroundData* data,
                            CvBackgroundReader* reader,
                            CvMat* img )
{
    CvMat mat;

    assert( data != NULL && reader != NULL && img != NULL );
    assert( CV_MAT_TYPE( img->type ) == CV_8UC1 );
    assert( img->cols == data->winsize.width );
    assert( img->rows == data->winsize.height );

    if( reader->img.data.ptr == NULL )
    {
        icvGetNextFromBackgroundData( data, reader );
    }

    mat = cvMat( data->winsize.height, data->winsize.width, CV_8UC1 );
    cvSetData( &mat, (void*) (reader->img.data.ptr + reader->point.y * reader->img.step
                              + reader->point.x * sizeof( uchar )), reader->img.step );

    cvCopy( &mat, img, 0 );
    if( (int) ( reader->point.x + (1.0F + reader->stepfactor ) * data->winsize.width )
            < reader->img.cols )
    {
        reader->point.x += (int) (reader->stepfactor * data->winsize.width);
    }
    else
    {
        reader->point.x = reader->offset.x;
        if( (int) ( reader->point.y + (1.0F + reader->stepfactor ) * data->winsize.height )
                < reader->img.rows )
        {
            reader->point.y += (int) (reader->stepfactor * data->winsize.height);
        }
        else
        {
            reader->point.y = reader->offset.y;
            reader->scale *= reader->scalefactor;
            if( reader->scale <= 1.0F )
            {
                reader->img = cvMat( (int) (reader->scale * reader->src.rows),
                                     (int) (reader->scale * reader->src.cols),
                                      CV_8UC1, (void*) (reader->img.data.ptr) );
                cvResize( &(reader->src), &(reader->img) );
            }
            else
            {
                icvGetNextFromBackgroundData( data, reader );
            }
        }
    }
}


CV_IMPL
int icvInitBackgroundReaders( const char* filename, CvSize winsize )
{
    if( cvbgdata == NULL )
    {
        cvbgdata = icvCreateBackgroundData( filename, winsize );
    }

    #ifdef _OPENMP
    #pragma omp parallel
    #endif /* _OPENMP */
    {
        #ifdef _OPENMP
        #pragma omp critical(c_create_bg_data)
        #endif /* _OPENMP */
        {
            if( cvbgreader == NULL )
            {
                cvbgreader = icvCreateBackgroundReader();
            }
        }
    }

    return (cvbgdata != NULL);
}

CV_IMPL
void icvDestroyBackgroundReaders()
{
    /* release background reader in each thread */
    #ifdef _OPENMP
    #pragma omp parallel
    #endif /* _OPENMP */
    {
        #ifdef _OPENMP
        #pragma omp critical(c_release_bg_data)
        #endif /* _OPENMP */
        {
            if( cvbgreader != NULL )
            {
                icvReleaseBackgroundReader( &cvbgreader );
                cvbgreader = NULL;
            }
        }
    }

    if( cvbgdata != NULL )
    {
        icvReleaseBackgroundData( &cvbgdata );
        cvbgdata = NULL;
    }
}

CV_IMPL
void icvGetAuxImages( CvMat* img, CvMat* sum, CvMat* tilted,
                      CvMat* sqsum, float* normfactor )
{
    CvRect normrect;
    int p0, p1, p2, p3;
    sum_type   valsum   = 0;
    sqsum_type valsqsum = 0;
    double area = 0.0;
    
    cvIntegralImage( img, sum, sqsum, tilted );
    normrect = cvRect( 1, 1, img->cols - 2, img->rows - 2 );
    CV_SUM_OFFSETS( p0, p1, p2, p3, normrect, img->cols + 1 )
    
    area = normrect.width * normrect.height;
    valsum = ((sum_type*) (sum->data.ptr))[p0] - ((sum_type*) (sum->data.ptr))[p1]
           - ((sum_type*) (sum->data.ptr))[p2] + ((sum_type*) (sum->data.ptr))[p3];
    valsqsum = ((sqsum_type*) (sqsum->data.ptr))[p0]
             - ((sqsum_type*) (sqsum->data.ptr))[p1]
             - ((sqsum_type*) (sqsum->data.ptr))[p2]
             + ((sqsum_type*) (sqsum->data.ptr))[p3];

    /* sqrt( valsqsum / area - ( valsum / are )^2 ) * area */
    (*normfactor) = (float) sqrt( (double) (area * valsqsum - valsum * valsum) );
}

CV_IMPL
int icvGetHaarTrainingData( CvHaarTrainingData* data, int first, int count,
                            CvIntHaarClassifier* cascade,
                            CvGetHaarTrainingDataCallback callback, void* userdata,
                            int* consumed )
{
    int i = 0;
    int next = 1;
    int getcount = 0;
    int consumedcount = 0;

    CvMat img;
    CvMat sum;
    CvMat tilted;
    CvMat sqsum;
    sum_type* sumdata    = NULL;
    sum_type* tilteddata = NULL;
    float*    normfactor = NULL;

    assert( data != NULL );
    assert( first + count <= data->maxnum );
    assert( cascade != NULL );
    assert( callback != NULL );

    img = cvMat( data->winsize.height, data->winsize.width, CV_8UC1,
        cvAlloc( sizeof( uchar ) * data->winsize.height * data->winsize.width ) );
    sum = cvMat( data->winsize.height + 1, data->winsize.width + 1,
                 CV_SUM_MAT_TYPE, NULL );
    tilted = cvMat( data->winsize.height + 1, data->winsize.width + 1,
                    CV_SUM_MAT_TYPE, NULL );
    sqsum = cvMat( data->winsize.height + 1, data->winsize.width + 1, CV_SQSUM_MAT_TYPE,
                   cvAlloc( sizeof( sqsum_type ) * (data->winsize.height + 1)
                                                 * (data->winsize.width + 1) ) );
    next = 1;
    consumedcount = 0;
    getcount = 0;
    for( i = first; (i < first + count) && next; i++ )
    {
        for( ; ; )
        {
            next = callback( &img, userdata );
            
            if( !next ) break;

            consumedcount++;
            sumdata = (sum_type*) (data->sum.data.ptr + i * data->sum.step);
            tilteddata = (sum_type*) (data->tilted.data.ptr + i * data->tilted.step);
            normfactor = data->normfactor.data.fl + i;
            sum.data.ptr = (uchar*) sumdata;
            tilted.data.ptr = (uchar*) tilteddata;
            icvGetAuxImages( &img, &sum, &tilted, &sqsum, normfactor );            
            if( cascade->eval( cascade, sumdata, tilteddata, *normfactor ) != 0.0F )
            {
                getcount++;
                break;
            }
        }        
    }
    if( consumed != NULL ) (*consumed) = consumedcount;

    cvFree( (void**) &(img.data.ptr) );
    cvFree( (void**) &(sqsum.data.ptr) );

    return getcount;
}

CV_IMPL
int icvGetHaarTrainingDataFromBG( CvHaarTrainingData* data, int first, int count,
                                  CvIntHaarClassifier* cascade, int* consumed )
{
    int i = 0;
    int consumedcount = 0;

    /* private variables */
    CvMat img;
    CvMat sum;
    CvMat tilted;
    CvMat sqsum;

    sum_type* sumdata;
    sum_type* tilteddata;
    float*    normfactor;
    /* end private variables */

    assert( data != NULL );
    assert( first + count <= data->maxnum );
    assert( cascade != NULL );

    consumedcount = 0;
    
    #ifdef _OPENMP
    #pragma omp parallel reduction(+: consumedcount) private(img, sum, tilted, sqsum, \
                                                         sumdata, tilteddata, normfactor)
    #endif /* _OPENMP */
    {
        sumdata    = NULL;
        tilteddata = NULL;
        normfactor = NULL;

        img = cvMat( data->winsize.height, data->winsize.width, CV_8UC1,
            cvAlloc( sizeof( uchar ) * data->winsize.height * data->winsize.width ) );
        sum = cvMat( data->winsize.height + 1, data->winsize.width + 1,
                     CV_SUM_MAT_TYPE, NULL );
        tilted = cvMat( data->winsize.height + 1, data->winsize.width + 1,
                        CV_SUM_MAT_TYPE, NULL );
        sqsum = cvMat( data->winsize.height + 1, data->winsize.width + 1,
                       CV_SQSUM_MAT_TYPE,
                       cvAlloc( sizeof( sqsum_type ) * (data->winsize.height + 1)
                                                     * (data->winsize.width + 1) ) );
        
        #ifdef _OPENMP
        #pragma omp for schedule(static, 1)
        #endif /* _OPENMP */
        for( i = first; i < first + count; i++ )
        {
            for( ; ; )
            {
                icvGetBackgroundImage( cvbgdata, cvbgreader, &img );
                consumedcount++;
                sumdata = (sum_type*) (data->sum.data.ptr + i * data->sum.step);
                tilteddata = (sum_type*) (data->tilted.data.ptr + i * data->tilted.step);
                normfactor = data->normfactor.data.fl + i;
                sum.data.ptr = (uchar*) sumdata;
                tilted.data.ptr = (uchar*) tilteddata;
                icvGetAuxImages( &img, &sum, &tilted, &sqsum, normfactor );            
                if( cascade->eval( cascade, sumdata, tilteddata, *normfactor ) != 0.0F )
                {
                    break;
                }
            }

#ifdef CV_VERBOSE
            if( (i - first) % 500 == 0 )
            {
                fprintf( stderr, "%3d%%\r", (int) ( 100.0 * (i - first) / count ) );
                fflush( stderr );
            }
#endif /* CV_VERBOSE */
            
        }

        cvFree( (void**) &(img.data.ptr) );
        cvFree( (void**) &(sqsum.data.ptr) );
    } /* omp parallel */

    if( consumed != NULL ) (*consumed) = consumedcount;

    return count;
}

typedef struct CvVecFile
{
    FILE*  input;
    int    count;
    int    vecsize;
    int    last;
    short* vector;
} CvVecFile;

CV_IMPL
int icvGetHaarTraininDataFromVecCallback( CvMat* img, void* userdata )
{
    uchar tmp = 0;
    int r = 0;
    int c = 0;

    assert( img->rows * img->cols == ((CvVecFile*) userdata)->vecsize );
    
    fread( &tmp, sizeof( tmp ), 1, ((CvVecFile*) userdata)->input );
    fread( ((CvVecFile*) userdata)->vector, sizeof( short ),
           ((CvVecFile*) userdata)->vecsize, ((CvVecFile*) userdata)->input );
    
    if( feof( ((CvVecFile*) userdata)->input ) || 
        (((CvVecFile*) userdata)->last)++ >= ((CvVecFile*) userdata)->count )
    {
        return 0;
    }
    
    for( r = 0; r < img->rows; r++ )
    {
        for( c = 0; c < img->cols; c++ )
        {
            CV_MAT_ELEM( *img, uchar, r, c ) = 
                (uchar) ( ((CvVecFile*) userdata)->vector[r * img->cols + c] );
        }
    }

    return 1;
}

/*
 * icvGetHaarTrainingDataFromVec
 * Get training data from .vec file
 */
CV_IMPL
int icvGetHaarTrainingDataFromVec( CvHaarTrainingData* data, int first, int count,                                   
                                   CvIntHaarClassifier* cascade,
                                   const char* filename,
                                   int* consumed )
{
    int getcount = 0;

    CV_FUNCNAME( "icvGetHaarTrainingDataFromVec" );

    __BEGIN__;

    CvVecFile file;
    short tmp = 0;    
    
    file.input = fopen( filename, "rb" );

    if( file.input != NULL )
    {
        fread( &file.count, sizeof( file.count ), 1, file.input );
        fread( &file.vecsize, sizeof( file.vecsize ), 1, file.input );
        fread( &tmp, sizeof( tmp ), 1, file.input );
        fread( &tmp, sizeof( tmp ), 1, file.input );
        if( !feof( file.input ) )
        {
            if( file.vecsize != data->winsize.width * data->winsize.height )
            {
                fclose( file.input );
                CV_ERROR( CV_StsError, "Vec file sample size mismatch" );
            }

            file.last = 0;
            file.vector = (short*) cvAlloc( sizeof( *file.vector ) * file.vecsize );
            getcount = icvGetHaarTrainingData( data, first, count, cascade,
                icvGetHaarTraininDataFromVecCallback, &file, consumed );
            cvFree( (void**) &file.vector );
        }
        fclose( file.input );
    }

    __END__;

    return getcount;
}

CV_IMPL
void icvRandomQuad( int width, int height, double quad[4][2], 
                    double maxxangle,
                    double maxyangle,
                    double maxzangle )
{
    double distfactor = 3.0;
    double distfactor2 = 1.0;

    double halfw, halfh;
    int i;
    
    double rotVectData[3];
    double vectData[3];
    double rotMatData[9];

    CvMat rotVect;
    CvMat rotMat;
    CvMat vect;

    double d;

    rotVect = cvMat( 3, 1, CV_64FC1, &rotVectData[0] );
    rotMat = cvMat( 3, 3, CV_64FC1, &rotMatData[0] );
    vect = cvMat( 3, 1, CV_64FC1, &vectData[0] );

    rotVectData[0] = maxxangle * (2.0 * rand() / RAND_MAX - 1.0);
    rotVectData[1] = ( maxyangle - fabs( rotVectData[0] ) )
        * (2.0 * rand() / RAND_MAX - 1.0);
    rotVectData[2] = maxzangle * (2.0 * rand() / RAND_MAX - 1.0);
    d = (distfactor + distfactor2 * (2.0 * rand() / RAND_MAX - 1.0)) * width;

/*
    rotVectData[0] = maxxangle;
    rotVectData[1] = maxyangle;
    rotVectData[2] = maxzangle;

    d = distfactor * width;
*/

    cvRodrigues( &rotMat, &rotVect, NULL, CV_RODRIGUES_V2M );

    halfw = 0.5 * width;
    halfh = 0.5 * height;

    quad[0][0] = -halfw;
    quad[0][1] = -halfh;
    quad[1][0] =  halfw;
    quad[1][1] = -halfh;
    quad[2][0] =  halfw;
    quad[2][1] =  halfh;
    quad[3][0] = -halfw;
    quad[3][1] =  halfh;

    for( i = 0; i < 4; i++ )
    {
        rotVectData[0] = quad[i][0];
        rotVectData[1] = quad[i][1];
        rotVectData[2] = 0.0;
        cvMatMulAdd( &rotMat, &rotVect, 0, &vect );
        quad[i][0] = vectData[0] * d / (d + vectData[2]) + halfw;
        quad[i][1] = vectData[1] * d / (d + vectData[2]) + halfh;
        
        /*
        quad[i][0] += halfw;
        quad[i][1] += halfh;
        */
    }
}

typedef struct CvSampleDistortionData
{
    IplImage* src;
    IplImage* erode;
    IplImage* dilate;
    IplImage* mask;
    IplImage* img;
    IplImage* maskimg;
    int dx;
    int dy;
    int bgcolor;
} CvSampleDistortionData;

int icvStartSampleDistortion( const char* imgfilename, int bgcolor, int bgthreshold,
                              CvSampleDistortionData* data )
{
    memset( data, 0, sizeof( *data ) );
    data->src = cvLoadImage( imgfilename, 0 );
    if( data->src != NULL && data->src->nChannels == 1 
        && data->src->depth == IPL_DEPTH_8U )
    {
        int r, c;
        uchar* pmask;
        uchar* psrc;
        uchar* perode;
        uchar* pdilate;
        uchar dd, de;
        
        data->dx = data->src->width / 2;
        data->dy = data->src->height / 2;
        data->bgcolor = bgcolor;

        data->mask = cvCloneImage( data->src );
        data->erode = cvCloneImage( data->src );
        data->dilate = cvCloneImage( data->src );
            
        /* make mask image */
        for( r = 0; r < data->mask->height; r++ )
        {
            for( c = 0; c < data->mask->width; c++ )
            {
                pmask = ( (uchar*) (data->mask->imageData + r * data->mask->widthStep)
                        + c );
                if( bgcolor - bgthreshold <= (int) (*pmask) &&
                    (int) (*pmask) <= bgcolor + bgthreshold )
                {
                    *pmask = (uchar) 0;
                }
                else
                {
                    *pmask = (uchar) 255;
                }
            }
        }
            
        /* extend borders of source image */
        cvErode( data->src, data->erode, 0, 1 );
        cvDilate( data->src, data->dilate, 0, 1 );
        for( r = 0; r < data->mask->height; r++ )
        {
            for( c = 0; c < data->mask->width; c++ )
            {
                pmask = ( (uchar*) (data->mask->imageData + r * data->mask->widthStep)
                        + c );
                if( (*pmask) == 0 )
                {
                    psrc = ( (uchar*) (data->src->imageData + r * data->src->widthStep)
                           + c );
                    perode = 
                        ( (uchar*) (data->erode->imageData + r * data->erode->widthStep)
                                + c );
                    pdilate = 
                        ( (uchar*)(data->dilate->imageData + r * data->dilate->widthStep)
                                + c );
                    de = bgcolor - (*perode);
                    dd = (*pdilate) - bgcolor;
                    if( de >= dd && de > bgthreshold )
                    {
                        (*psrc) = (*perode);
                    }
                    if( dd > de && dd > bgthreshold )
                    {
                        (*psrc) = (*pdilate);
                    }
                }
            }
        }

        data->img = cvCreateImage( cvSize( data->src->width + 2 * data->dx,
                                           data->src->height + 2 * data->dy ),
                                   IPL_DEPTH_8U, 1 );
        data->maskimg = cvCloneImage( data->img );

        return 1;
    }

    return 0;
}

void icvPlaceDistortedSample( CvArr* background,
                              int inverse, int maxintensitydev,
                              double maxxangle, double maxyangle, double maxzangle,
                              int inscribe, double maxshiftf, double maxscalef,
                              CvSampleDistortionData* data )
{
    double quad[4][2];
    int r, c;
    uchar* pimg;
    uchar* pbg;
    uchar* palpha;
    uchar chartmp;
    int forecolordev;
    float scale;
    IplImage* img;
    IplImage* maskimg;
    CvMat  stub;
    CvMat* bgimg;

    CvRect cr;
    CvRect roi;

    double xshift, yshift, randscale;

    icvRandomQuad( data->src->width, data->src->height, quad,
                   maxxangle, maxyangle, maxzangle );
    quad[0][0] += (double) data->dx;
    quad[0][1] += (double) data->dy;
    quad[1][0] += (double) data->dx;
    quad[1][1] += (double) data->dy;
    quad[2][0] += (double) data->dx;
    quad[2][1] += (double) data->dy;
    quad[3][0] += (double) data->dx;
    quad[3][1] += (double) data->dy;
    
    cvSet( data->img, cvScalar( data->bgcolor ) );
    cvSet( data->maskimg, cvScalar( 0.0 ) );

#ifdef HAVE_IPL
    iplWarpPerspectiveQ( data->src, data->img, quad, IPL_WARP_R_TO_Q,
                         IPL_INTER_CUBIC | IPL_SMOOTH_EDGE );
    iplWarpPerspectiveQ( data->mask, data->maskimg, quad, IPL_WARP_R_TO_Q,
                         IPL_INTER_CUBIC | IPL_SMOOTH_EDGE );
#endif /* HAVE_IPL */
    cvSmooth( data->maskimg, data->maskimg, CV_GAUSSIAN, 3, 3 );

    bgimg = cvGetMat( background, &stub );

    cr.x = data->dx;
    cr.y = data->dy;
    cr.width = data->src->width;
    cr.height = data->src->height;

    if( inscribe )
    {
        /* quad's circumscribing rectangle */
        cr.x = (int) MIN( quad[0][0], quad[3][0] );
        cr.y = (int) MIN( quad[0][1], quad[1][1] );
        cr.width  = (int) (MAX( quad[1][0], quad[2][0] ) + 0.5F ) - cr.x;
        cr.height = (int) (MAX( quad[2][1], quad[3][1] ) + 0.5F ) - cr.y;
    }
    
    xshift = maxshiftf * rand() / RAND_MAX;
    yshift = maxshiftf * rand() / RAND_MAX;

    cr.x -= (int) ( xshift * cr.width  );
    cr.y -= (int) ( yshift * cr.height );
    cr.width  = (int) ((1.0 + maxshiftf) * cr.width );
    cr.height = (int) ((1.0 + maxshiftf) * cr.height);

    randscale = maxscalef * rand() / RAND_MAX;
    cr.x -= (int) ( 0.5 * randscale * cr.width  );
    cr.y -= (int) ( 0.5 * randscale * cr.height );
    cr.width  = (int) ((1.0 + randscale) * cr.width );
    cr.height = (int) ((1.0 + randscale) * cr.height);

    scale = MAX( ((float) cr.width) / bgimg->cols, ((float) cr.height) / bgimg->rows );

    roi.x = (int) (-0.5F * (scale * bgimg->cols - cr.width) + cr.x);
    roi.y = (int) (-0.5F * (scale * bgimg->rows - cr.height) + cr.y);
    roi.width  = (int) (scale * bgimg->cols);
    roi.height = (int) (scale * bgimg->rows);

    img = cvCreateImage( cvSize( bgimg->cols, bgimg->rows ), IPL_DEPTH_8U, 1 );
    maskimg = cvCreateImage( cvSize( bgimg->cols, bgimg->rows ), IPL_DEPTH_8U, 1 );
    
    cvSetImageROI( data->img, roi );
    cvResize( data->img, img );
    cvResetImageROI( data->img );
    cvSetImageROI( data->maskimg, roi );
    cvResize( data->maskimg, maskimg );
    cvResetImageROI( data->maskimg );
    
    forecolordev = (int) (maxintensitydev * (2.0 * rand() / RAND_MAX - 1.0));

    for( r = 0; r < img->height; r++ )
    {
        for( c = 0; c < img->width; c++ )
        {
            pimg = (uchar*) img->imageData + r * img->widthStep + c;
            pbg = (uchar*) bgimg->data.ptr + r * bgimg->step + c;
            palpha = (uchar*) maskimg->imageData + r * maskimg->widthStep + c;
            chartmp = (uchar) MAX( 0, MIN( 255, forecolordev + (*pimg) ) );
            if( inverse )
            {
                chartmp ^= 0xFF;
            }
            *pbg = (uchar) (( chartmp*(*palpha )+(255 - (*palpha) )*(*pbg) ) / 255);
        }
    }

    cvReleaseImage( &img );
    cvReleaseImage( &maskimg );
}

void icvEndSampleDistortion( CvSampleDistortionData* data )
{
    if( data->src )
    {
        cvReleaseImage( &data->src );
    }
    if( data->mask )
    {
        cvReleaseImage( &data->mask );
    }
    if( data->erode )
    {
        cvReleaseImage( &data->erode );
    }
    if( data->dilate )
    {
        cvReleaseImage( &data->dilate );
    }
    if( data->img )
    {
        cvReleaseImage( &data->img );
    }
    if( data->maskimg )
    {
        cvReleaseImage( &data->maskimg );
    }
}

void icvWriteVecHeader( FILE* file, int count, int width, int height )
{
    int vecsize;
    short tmp;

    /* number of samples */
    fwrite( &count, sizeof( count ), 1, file );    
    /* vector size */
    vecsize = width * height;
    fwrite( &vecsize, sizeof( vecsize ), 1, file );
    /* min/max values */
    tmp = 0;
    fwrite( &tmp, sizeof( tmp ), 1, file );
    fwrite( &tmp, sizeof( tmp ), 1, file );    
}

void icvWriteVecSample( FILE* file, CvArr* sample )
{
    CvMat* mat, stub;
    int r, c;
    short tmp;
    uchar chartmp;

    mat = cvGetMat( sample, &stub );
    chartmp = 0;
    fwrite( &chartmp, sizeof( chartmp ), 1, file );
    for( r = 0; r < mat->rows; r++ )
    {
        for( c = 0; c < mat->cols; c++ )
        {
            tmp = (short) (CV_MAT_ELEM( *mat, uchar, r, c ));
            fwrite( &tmp, sizeof( tmp ), 1, file );
        }
    }
}

CV_IMPL
void cvCreateTrainingSamples( const char* filename,
                              const char* imgfilename, int bgcolor, int bgthreshold,
                              const char* bgfilename, int count,
                              int invert, int maxintensitydev,
                              double maxxangle, double maxyangle, double maxzangle,
                              int showsamples,
                              int winwidth, int winheight )
{
    CvSampleDistortionData data;

    assert( filename != NULL );
    assert( imgfilename != NULL );

    if( !icvMkDir( filename ) )
    {
        fprintf( stderr, "Unable to create output file: %s\n", filename );
        return;
    }
    if( icvStartSampleDistortion( imgfilename, bgcolor, bgthreshold, &data ) )
    {
        FILE* output = NULL;

        output = fopen( filename, "wb" );
        if( output != NULL )
        {
            int hasbg;
            int i;
            CvMat sample;
            int inverse;

            hasbg = 0;
            hasbg = (bgfilename != NULL && icvInitBackgroundReaders( bgfilename,
                     cvSize( winwidth,winheight ) ) );

            sample = cvMat( winheight, winwidth, CV_8UC1, cvAlloc( sizeof( uchar ) *
                            winheight * winwidth ) );

            icvWriteVecHeader( output, count, sample.cols, sample.rows );

            if( showsamples )
            {
                cvNamedWindow( "Sample", CV_WINDOW_AUTOSIZE );
            }

            inverse = invert;
            for( i = 0; i < count; i++ )
            {
                if( hasbg )
                {
                    icvGetBackgroundImage( cvbgdata, cvbgreader, &sample );
                }
                else
                {
                    cvSet( &sample, cvScalar( bgcolor ) );
                }

                if( invert == CV_RANDOM_INVERT )
                {
                    inverse = (rand() > (RAND_MAX/2));
                }
                icvPlaceDistortedSample( &sample, inverse, maxintensitydev,
                    maxxangle, maxyangle, maxzangle, 
                    0   /* nonzero means placing image without cut offs */,
                    0.0 /* nozero adds random shifting                  */,
                    0.0 /* nozero adds random scaling                   */,
                    &data );

                if( showsamples )
                {
                    cvShowImage( "Sample", &sample );
                    if( cvWaitKey( 0 ) == 27 )
                    {
                        showsamples = 0;
                    }
                }

                icvWriteVecSample( output, &sample );

#ifdef CV_VERBOSE
                if( i % 500 == 0 )
                {
                    printf( "\r%3d%%", 100 * i / count );
                }
#endif /* CV_VERBOSE */
            }
            icvDestroyBackgroundReaders();
            cvFree( (void**) &(sample.data.ptr) );
            fclose( output );
        } /* if( output != NULL ) */
        
        icvEndSampleDistortion( &data );
    }
    
#ifdef CV_VERBOSE
    printf( "\r      \r" );
#endif /* CV_VERBOSE */ 

}

#define CV_INFO_FILENAME "info.dat"

CV_IMPL
void cvCreateTestSamples( const char* infoname,
                          const char* imgfilename, int bgcolor, int bgthreshold,
                          const char* bgfilename, int count,
                          int invert, int maxintensitydev,
                          double maxxangle, double maxyangle, double maxzangle,
                          int showsamples,
                          int winwidth, int winheight )
{
    CvSampleDistortionData data;

    assert( infoname != NULL );
    assert( imgfilename != NULL );
    assert( bgfilename != NULL );

    if( !icvMkDir( infoname ) )
    {

#if CV_VERBOSE
        fprintf( stderr, "Unable to create directory hierarchy: %s\n", infoname );
#endif /* CV_VERBOSE */

        return;
    }
    if( icvStartSampleDistortion( imgfilename, bgcolor, bgthreshold, &data ) )
    {
        char fullname[PATH_MAX];
        char* filename;
        CvMat win;
        FILE* info;

        if( icvInitBackgroundReaders( bgfilename, cvSize( 10, 10 ) ) )
        {
            int i;
            int x, y, width, height;
            float scale;
            float maxscale;
            int inverse;

            if( showsamples )
            {
                cvNamedWindow( "Image", CV_WINDOW_AUTOSIZE );
            }
            
            info = fopen( infoname, "w" );
            strcpy( fullname, infoname );
            filename = strrchr( fullname, '\\' );
            if( filename == NULL )
            {
                filename = strrchr( fullname, '/' );
            }
            if( filename == NULL )
            {
                filename = fullname;
            }
            else
            {
                filename++;
            }

            count = MIN( count, cvbgdata->count );
            inverse = invert;
            for( i = 0; i < count; i++ )
            {
                icvGetNextFromBackgroundData( cvbgdata, cvbgreader );
                
                maxscale = MIN( 0.7F * cvbgreader->src.cols / winwidth,
                                   0.7F * cvbgreader->src.rows / winheight );
                if( maxscale < 1.0F ) continue;

                scale = (maxscale - 1.0F) * rand() / RAND_MAX + 1.0F;
                width = (int) (scale * winwidth);
                height = (int) (scale * winheight);
                x = (int) ((0.1+0.8 * rand()/RAND_MAX) * (cvbgreader->src.cols - width));
                y = (int) ((0.1+0.8 * rand()/RAND_MAX) * (cvbgreader->src.rows - height));

                cvGetSubArr( &cvbgreader->src, &win, cvRect( x, y ,width, height ) );
                if( invert == CV_RANDOM_INVERT )
                {
                    inverse = (rand() > (RAND_MAX/2));
                }
                icvPlaceDistortedSample( &win, inverse, maxintensitydev,
                                         maxxangle, maxyangle, maxzangle, 
                                         1, 0.0, 0.0, &data );
                
                
                sprintf( filename, "%04d_%04d_%04d_%04d_%04d.jpg",
                         (i + 1), x, y, width, height );
                
                if( info ) 
                {
                    fprintf( info, "%s %d %d %d %d %d\n",
                        filename, 1, x, y, width, height );
                }

                cvSaveImage( fullname, &cvbgreader->src );
                if( showsamples )
                {
                    cvShowImage( "Image", &cvbgreader->src );
                    if( cvWaitKey( 0 ) == 27 )
                    {
                        showsamples = 0;
                    }
                }
            }
            if( info ) fclose( info );
            icvDestroyBackgroundReaders();
        }
        icvEndSampleDistortion( &data );
    }
}

CV_IMPL
int cvCreateTrainingSamplesFromInfo( const char* infoname, const char* vecfilename,
                                     int num,
                                     int showsamples,
                                     int winwidth, int winheight )
{
    char fullname[PATH_MAX];
    char* filename;

    FILE* info;
    FILE* vec;
    IplImage* src;
    IplImage* sample;
    int line;
    int error;
    int i;
    int x, y, width, height;
    int total;

    assert( infoname != NULL );
    assert( vecfilename != NULL );

    total = 0;
    if( !icvMkDir( vecfilename ) )
    {

#if CV_VERBOSE
        fprintf( stderr, "Unable to create directory hierarchy: %s\n", vecfilename );
#endif /* CV_VERBOSE */

        return total;
    }

    info = fopen( infoname, "r" );
    if( info == NULL )
    {

#if CV_VERBOSE
        fprintf( stderr, "Unable to open file: %s\n", infoname );
#endif /* CV_VERBOSE */

        return total;
    }

    vec = fopen( vecfilename, "wb" );
    if( vec == NULL )
    {

#if CV_VERBOSE
        fprintf( stderr, "Unable to open file: %s\n", vecfilename );
#endif /* CV_VERBOSE */

        fclose( info );

        return total;
    }

    sample = cvCreateImage( cvSize( winwidth, winheight ), IPL_DEPTH_8U, 1 );

    icvWriteVecHeader( vec, num, sample->width, sample->height );

    if( showsamples )
    {
        cvNamedWindow( "Sample", CV_WINDOW_AUTOSIZE );
    }
    
    strcpy( fullname, infoname );
    filename = strrchr( fullname, '\\' );
    if( filename == NULL )
    {
        filename = strrchr( fullname, '/' );
    }
    if( filename == NULL )
    {
        filename = fullname;
    }
    else
    {
        filename++;
    }

    for( line = 1, error = 0, total = 0; total < num ;line++ )
    {
        int count;

        error = ( fscanf( info, "%s %d", filename, &count ) != 2 );
        if( !error )
        {
            src = cvLoadImage( fullname, 0 );
            error = ( src == NULL );
            if( error )
            {

#if CV_VERBOSE
                fprintf( stderr, "Unable to open image: %s\n", fullname );
#endif /* CV_VERBOSE */

            }
        }
        for( i = 0; (i < count) && (total < num); i++, total++ )
        {
            error = ( fscanf( info, "%d %d %d %d", &x, &y, &width, &height ) != 4 );
            if( error ) break;
            cvSetImageROI( src, cvRect( x, y, width, height ) );
            cvResize( src, sample );
            
            if( showsamples )
            {
                cvShowImage( "Sample", sample );
                if( cvWaitKey( 0 ) == 27 )
                {
                    showsamples = 0;
                }
            }            
            icvWriteVecSample( vec, sample );
        }
        
        if( src )
        {
            cvReleaseImage( &src );
        }

        if( error )
        {

#if CV_VERBOSE
            fprintf( stderr, "%s(%d) : parse error", infoname, line );
#endif /* CV_VERBOSE */

            break;
        }
    }
    
    if( sample )
    {
        cvReleaseImage( &sample );
    }

    fclose( vec );
    fclose( info );

    return total;
}

CV_IMPL
void cvShowVecSamples( const char* filename, int winwidth, int winheight )
{
    CvVecFile file;
    short tmp; 
    int i;
    CvMat* sample;
    
    tmp = 0;
    file.input = fopen( filename, "rb" );

    if( file.input != NULL )
    {
        fread( &file.count, sizeof( file.count ), 1, file.input );
        fread( &file.vecsize, sizeof( file.vecsize ), 1, file.input );
        fread( &tmp, sizeof( tmp ), 1, file.input );
        fread( &tmp, sizeof( tmp ), 1, file.input );
        
        if( file.vecsize != winwidth * winheight )
        {

#if CV_VERBOSE
            fprintf( stderr, "Invalid vec file sample size\n" );
#endif /* CV_VERBOSE */
            fclose( file.input );

            return;
        }

        if( !feof( file.input ) )
        {
            file.last = 0;
            file.vector = (short*) cvAlloc( sizeof( *file.vector ) * file.vecsize );
            sample = cvCreateMat( winheight, winwidth, CV_8UC1 );
            cvNamedWindow( "Sample", CV_WINDOW_AUTOSIZE );
            for( i = 0; i < file.count; i++ )
            {
                icvGetHaarTraininDataFromVecCallback( sample, &file );
                cvShowImage( "Sample", sample );
                if( cvWaitKey( 0 ) == 27 ) break;
            }
            cvReleaseMat( &sample );
            cvFree( (void**) &file.vector );
        }
        fclose( file.input );
    }
}

CV_IMPL
void cvCreateCascadeClassifier( const char* dirname,
                                const char* vecfilename,
                                const char* bgfilename, 
                                int npos, int nneg, int nstages,
                                int numprecalculated,
                                int numsplits,
                                float minhitrate, float maxfalsealarm,
                                float weightfraction,
                                int mode, int symmetric,
                                int equalweights,
                                int winwidth, int winheight,
                                int boosttype, int stumperror )
{
    CvCascadeHaarClassifier* cascade = NULL;
    CvHaarTrainingData* data = NULL;
    CvIntHaarFeatures* haar_features;
    CvSize winsize;
    size_t datasize = 0;
    int i = 0;
    int j = 0;
    int poscount = 0;
    int negcount = 0;
    int consumed = 0;
    char stagename[PATH_MAX];
    float posweight = 1.0F;
    float negweight = 1.0F;
    FILE* file;

#ifdef CV_VERBOSE
    double proctime = 0.0F;
#endif /* CV_VERBOSE */

    assert( dirname != NULL );
    assert( bgfilename != NULL );
    assert( vecfilename != NULL );
    assert( nstages > 0 );

    winsize = cvSize( winwidth, winheight );

    cascade = (CvCascadeHaarClassifier*) icvCreateCascadeHaarClassifier( nstages );
    cascade->count = 0;
    
    if( icvInitBackgroundReaders( bgfilename, winsize ) )
    {
        data = icvCreateHaarTrainingData( winsize, npos + nneg );
        haar_features = icvCreateIntHaarFeatures( winsize, mode, symmetric );

#ifdef CV_VERBOSE
        printf("Number of features used : %d\n", haar_features->count);
#endif /* CV_VERBOSE */

        for( i = 0; i < nstages; i++, cascade->count++ )
        {
            sprintf( stagename, "%s%d/%s", dirname, i, CV_STAGE_CART_FILE_NAME );
            cascade->classifier[i] = 
                icvLoadCARTStageHaarClassifier( stagename, winsize.width + 1 );

            if( !icvMkDir( stagename ) )
            {

#ifdef CV_VERBOSE
                printf( "UNABLE TO CREATE DIRECTORY: %s\n", stagename );
#endif /* CV_VERBOSE */

                break;
            }
            if( cascade->classifier[i] != NULL )
            {

#ifdef CV_VERBOSE
                printf( "STAGE: %d LOADED.\n", i );
#endif /* CV_VERBOSE */

                continue;
            }

#ifdef CV_VERBOSE
            printf( "STAGE: %d\n", i );
#endif /* CV_VERBOSE */

            poscount = icvGetHaarTrainingDataFromVec( data, 0, npos,
                (CvIntHaarClassifier*) cascade, vecfilename, &consumed );
#ifdef CV_VERBOSE
            printf( "POS: %d %d %f\n", poscount, consumed,
                    ((float) poscount) / consumed );
#endif /* CV_VERBOSE */

            if( poscount <= 0 )
            {

#ifdef CV_VERBOSE
            printf( "UNABLE TO OBTAIN POS SAMPLES\n" );
#endif /* CV_VERBOSE */

                break;
            }

#ifdef CV_VERBOSE
            proctime = -TIME( 0 );
#endif /* CV_VERBOSE */

            negcount = icvGetHaarTrainingDataFromBG( data, poscount, nneg,
                (CvIntHaarClassifier*) cascade, &consumed );
#ifdef CV_VERBOSE
            printf( "NEG: %d %d %f\n", negcount, consumed,
                    ((float) negcount) / consumed );
            printf( "BACKGROUND PROCESSING TIME: %.2f\n",
                (proctime + TIME( 0 )) );
#endif /* CV_VERBOSE */

            if( negcount <= 0 )
            {

#ifdef CV_VERBOSE
            printf( "UNABLE TO OBTAIN NEG SAMPLES\n" );
#endif /* CV_VERBOSE */

                break;
            }

            data->sum.rows = data->tilted.rows = poscount + negcount;
            data->normfactor.cols = data->weights.cols = data->cls.cols =
                    poscount + negcount;
        
            posweight = (equalweights) ? 1.0F / (poscount + negcount) : (0.5F / poscount);
            negweight = (equalweights) ? 1.0F / (poscount + negcount) : (0.5F / negcount);
            for( j = 0; j < poscount; j++ )
            {
                data->weights.data.fl[j] = posweight;
                data->cls.data.fl[j] = 1.0F;

            }
            for( j = poscount; j < poscount + negcount; j++ )
            {
                data->weights.data.fl[j] = negweight;
                data->cls.data.fl[j] = 0.0F;
            }

#ifdef CV_VERBOSE
            proctime = -TIME( 0 );
#endif /* CV_VERBOSE */

            icvPrecalculate( data, haar_features, numprecalculated );

#ifdef CV_VERBOSE
            printf( "PRECALCULATION TIME: %.2f\n", (proctime + TIME( 0 )) );
#endif /* CV_VERBOSE */

#ifdef CV_VERBOSE
            proctime = -TIME( 0 );
#endif /* CV_VERBOSE */

            cascade->classifier[i] = icvCreateCARTStageClassifier(  data, NULL,
                haar_features, minhitrate, maxfalsealarm, symmetric, weightfraction,
                numsplits, (CvBoostType) boosttype, (CvStumpError) stumperror, 0 );

#ifdef CV_VERBOSE
            printf( "STAGE TRAINING TIME: %.2f\n", (proctime + TIME( 0 )) );
#endif /* CV_VERBOSE */

            file = fopen( stagename, "w" );
            if( file != NULL )
            {
                cascade->classifier[i]->save( 
                    (CvIntHaarClassifier*) cascade->classifier[i], file );
                fclose( file );
            }
            else
            {

#ifdef CV_VERBOSE
                printf( "FAILED TO SAVE STAGE CLASSIFIER IN FILE %s\n", stagename );
#endif /* CV_VERBOSE */

            }

        }
        icvReleaseIntHaarFeatures( &haar_features );
        icvReleaseHaarTrainingData( &data );
    }
    else
    {
#ifdef CV_VERBOSE
        printf( "FAILED TO INITIALIZE BACKGROUND READERS\n" );
#endif /* CV_VERBOSE */
    }
    
    /* CLEAN UP */
    icvDestroyBackgroundReaders();
    cascade->release( (CvIntHaarClassifier**) &cascade );
}

/* End of file. */
