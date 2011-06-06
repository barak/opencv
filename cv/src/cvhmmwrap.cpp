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

CV_IMPL CvEHMM*
cvCreate2DHMM( int *state_number, int *num_mix, int obs_size )
{
    CvEHMM* hmm = 0;

    CV_FUNCNAME( "cvCreate2DHMM" );

    __BEGIN__;

    IPPI_CALL( icvCreate2DHMM( &hmm, state_number, num_mix, obs_size ));

    __END__;

    return hmm;
}

CV_IMPL void
cvRelease2DHMM( CvEHMM ** hmm )
{
    CV_FUNCNAME( "cvRelease2DHMM" );

    __BEGIN__;

    IPPI_CALL( icvRelease2DHMM( hmm ));
    __END__;
}

CV_IMPL CvImgObsInfo*
cvCreateObsInfo( CvSize num_obs, int obs_size )
{
    CvImgObsInfo *obs_info = 0;
    
    CV_FUNCNAME( "cvCreateObsInfo" );

    __BEGIN__;

    IPPI_CALL( icvCreateObsInfo( &obs_info, num_obs, obs_size ));

    __END__;

    return obs_info;
}

CV_IMPL void
cvReleaseObsInfo( CvImgObsInfo ** obs_info )
{
    CV_FUNCNAME( "cvReleaseObsInfo" );

    __BEGIN__;

    IPPI_CALL( icvReleaseObsInfo( obs_info ));

    __END__;
}


CV_IMPL void
cvUniformImgSegm( CvImgObsInfo * obs_info, CvEHMM * hmm )
{
    CV_FUNCNAME( "cvUniformImgSegm" );

    __BEGIN__;

    IPPI_CALL( icvUniformImgSegm( obs_info, hmm ));
    __CLEANUP__;
    __END__;
}

CV_IMPL void
cvInitMixSegm( CvImgObsInfo ** obs_info_array, int num_img, CvEHMM * hmm )
{
    CV_FUNCNAME( "cvInitMixSegm" );

    __BEGIN__;

    IPPI_CALL( icvInitMixSegm( obs_info_array, num_img, hmm ));

    __END__;
}

CV_IMPL void
cvEstimateHMMStateParams( CvImgObsInfo ** obs_info_array, int num_img, CvEHMM * hmm )
{
    CV_FUNCNAME( "cvEstimateHMMStateParams" );

    __BEGIN__;

    IPPI_CALL( icvEstimateHMMStateParams( obs_info_array, num_img, hmm ));

    __END__;
}

CV_IMPL void
cvEstimateTransProb( CvImgObsInfo ** obs_info_array, int num_img, CvEHMM * hmm )
{
    CV_FUNCNAME( "cvEstimateTransProb" );

    __BEGIN__;

    IPPI_CALL( icvEstimateTransProb( obs_info_array, num_img, hmm ));

    __END__;

}

CV_IMPL void
cvEstimateObsProb( CvImgObsInfo * obs_info, CvEHMM * hmm )
{
    CV_FUNCNAME( "cvEstimateObsProb" );

    __BEGIN__;

    IPPI_CALL( icvEstimateObsProb( obs_info, hmm ));

    __END__;
}

CV_IMPL float
cvEViterbi( CvImgObsInfo * obs_info, CvEHMM * hmm )
{
    float result = FLT_MAX;

    CV_FUNCNAME( "cvEViterbi" );

    __BEGIN__;

    if( (obs_info == NULL) || (hmm == NULL) )
        CV_ERROR( CV_BadDataPtr, "Null pointer." );

    result = icvEViterbi( obs_info, hmm );
    
    __END__;
    
    return result;
}

CV_IMPL void
cvMixSegmL2( CvImgObsInfo ** obs_info_array, int num_img, CvEHMM * hmm )
{
    CV_FUNCNAME( "cvMixSegmL2" );

    __BEGIN__;

    IPPI_CALL( icvMixSegmL2( obs_info_array, num_img, hmm ));

    __END__;
}


CV_IMPL void
cvKMeans2( const CvArr* samples_arr, int cluster_count,
           CvArr* cluster_idx_arr, CvTermCriteria termcrit )
{
    CvMat* centers = 0;
    CvMat* old_centers = 0;
    CvMat* counters = 0;
    
    CV_FUNCNAME( "cvKMeans2" );

    __BEGIN__;

    CvMat samples_stub, *samples = (CvMat*)samples_arr;
    CvMat cluster_idx_stub, *ids = (CvMat*)cluster_idx_arr;
    CvMat* temp = 0;
    CvRandState rng;
    int samples_count, dims;
    int ids_delta, iter;
    double max_dist;

    if( !CV_IS_MAT( samples ))
        CV_CALL( samples = cvGetMat( samples, &samples_stub ));
    
    if( !CV_IS_MAT( ids ))
        CV_CALL( ids = cvGetMat( ids, &cluster_idx_stub ));

    if( cluster_count < 1 )
        CV_ERROR( CV_StsOutOfRange, "Number of clusters should be positive" );

    if( CV_MAT_DEPTH(samples->type) != CV_32F || CV_MAT_TYPE(ids->type) != CV_32SC1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "samples should be floating-point matrix, cluster_idx - integer vector" );

    if( ids->rows != 1 && ids->cols != 1 || ids->rows + ids->cols - 1 != samples->rows )
        CV_ERROR( CV_StsUnmatchedSizes,
        "cluster_idx should be 1D vector of the same number of elements as samples' number of rows" ); 

    switch( termcrit.type )
    {
    case CV_TERMCRIT_EPS:
        if( termcrit.epsilon < 0 )
            termcrit.epsilon = 0;
        termcrit.maxIter = 100;
        break;
    case CV_TERMCRIT_ITER:
        if( termcrit.maxIter < 1 )
            termcrit.maxIter = 1;
        termcrit.epsilon = 1e-6;
        break;
    case CV_TERMCRIT_EPS|CV_TERMCRIT_ITER:
        if( termcrit.epsilon < 0 )
            termcrit.epsilon = 0;
        if( termcrit.maxIter < 1 )
            termcrit.maxIter = 1;
        break;
    default:
        CV_ERROR( CV_StsBadArg, "Invalid termination criteria" );
    }

    termcrit.epsilon *= termcrit.epsilon;

    samples_count = samples->rows;
    dims = samples->cols*CV_MAT_CN(samples->type);
    ids_delta = ids->step ? ids->step/(int)sizeof(int) : 1;

    CV_CALL( centers = cvCreateMat( cluster_count, dims, CV_64FC1 ));
    CV_CALL( old_centers = cvCreateMat( cluster_count, dims, CV_64FC1 ));
    CV_CALL( counters = cvCreateMat( cluster_count, 1, CV_32SC1 ));

    cvRandInit( &rng, 0, cluster_count, -1, CV_RAND_UNI );
    cvRand( &rng, ids );
    
    max_dist = termcrit.epsilon*2;

    for( iter = 0; iter < termcrit.maxIter; iter++ )
    {
        int i, j, k;
        cvZero( centers );
        cvZero( counters );

        for( i = 0; i < samples_count; i++ )
        {
            float* s = (float*)(samples->data.ptr + i*samples->step);
            int k = ids->data.i[i*ids_delta];
            double* c = (double*)(centers->data.ptr + k*centers->step);
            for( j = 0; j < dims; j++ )
                c[j] += s[j];
            counters->data.i[k]++;
        }

        if( iter > 0 )
            max_dist = 0;

        for( k = 0; k < cluster_count; k++ )
        {
            double* c = (double*)(centers->data.ptr + k*centers->step);
            if( counters->data.i[k] != 0 )
            {
                double scale = 1./counters->data.i[k];
                for( j = 0; j < dims; j++ )
                    c[j] *= scale;
            }
            else
            {
                int i = cvRandNext( &rng ) % samples_count;
                float* s = (float*)(samples->data.ptr + i*samples->step);
                for( j = 0; j < dims; j++ )
                    c[j] = s[j];
            }
            
            if( iter > 0 )
            {
                double dist = 0;
                double* c_o = (double*)(old_centers->data.ptr + k*old_centers->step);
                for( j = 0; j < dims; j++ )
                {
                    double t = c[j] - c_o[j];
                    dist += t*t;
                }
                if( max_dist < dist )
                    max_dist = dist;
            }
        }

        if( max_dist < termcrit.epsilon )
            break;

        for( i = 0; i < samples_count; i++ )
        {
            float* s = (float*)(samples->data.ptr + i*samples->step);
            int k_best = 0;
            double min_dist = DBL_MAX;

            for( k = 0; k < cluster_count; k++ )
            {
                double* c = (double*)(centers->data.ptr + k*centers->step);
                double dist = 0;
                
                for( j = 0; j < dims; j++ )
                {
                    double t = c[j] - s[j];
                    dist += t*t;
                }
                
                if( min_dist > dist )
                {
                    min_dist = dist;
                    k_best = k;
                }
            }

            ids->data.i[i*ids_delta] = k_best;
        }

        CV_SWAP( centers, old_centers, temp );
    }

    __END__;

    cvReleaseMat( &centers );
    cvReleaseMat( &old_centers );
    cvReleaseMat( &counters );
}

/* End of file. */
