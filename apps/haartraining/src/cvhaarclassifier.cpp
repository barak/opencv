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
 * cvhaarclassifier.cpp
 *
 * haar classifiers (stump, CART, stage, cascade)
 */

#include <_cvhaartraining.h>

CV_IMPL
CvIntHaarClassifier* icvCreateCARTHaarClassifier( int count )
{
    CvCARTHaarClassifier* cart;
    size_t datasize;

    datasize = sizeof( *cart ) +
        ( sizeof( int ) +
          sizeof( CvHaarFeature ) + sizeof( CvFastHaarFeature ) + 
          sizeof( float ) + sizeof( int ) + sizeof( int ) ) * count +
        sizeof( float ) * (count + 1);

    cart = (CvCARTHaarClassifier*) cvAlloc( datasize );
    memset( cart, 0, datasize );

    cart->feature = (CvHaarFeature*) (cart + 1);
    cart->fastfeature = (CvFastHaarFeature*) (cart->feature + count);
    cart->threshold = (float*) (cart->fastfeature + count);
    cart->left = (int*) (cart->threshold + count);
    cart->right = (int*) (cart->left + count);
    cart->val = (float*) (cart->right + count);
    cart->compidx = (int*) (cart->val + count + 1 );
    cart->count = count;
    cart->eval = icvEvalCARTHaarClassifier;
    cart->save = icvSaveCARTHaarClassifier;
    cart->release = icvReleaseHaarClassifier;

    return (CvIntHaarClassifier*) cart;
}

CV_IMPL
void icvReleaseHaarClassifier( CvIntHaarClassifier** classifier )
{
    cvFree( (void**) classifier );
    *classifier = NULL;
}

CV_IMPL
void icvInitCARTHaarClassifier( CvCARTHaarClassifier* carthaar, CvCARTClassifier* cart,
                                CvIntHaarFeatures* intHaarFeatures )
{
    int i;

    for( i = 0; i < cart->count; i++ )
    {
        carthaar->feature[i] = intHaarFeatures->feature[cart->compidx[i]];
        carthaar->fastfeature[i] = intHaarFeatures->fastfeature[cart->compidx[i]];
        carthaar->threshold[i] = cart->threshold[i];
        carthaar->left[i] = cart->left[i];
        carthaar->right[i] = cart->right[i];
        carthaar->val[i] = cart->val[i];
        carthaar->compidx[i] = cart->compidx[i];
    }
    carthaar->count = cart->count;
    carthaar->val[cart->count] = cart->val[cart->count];
}

CV_IMPL
float icvEvalCARTHaarClassifier( CvIntHaarClassifier* classifier,
                                 sum_type* sum, sum_type* tilted, float normfactor )
{
    int idx = 0;

    do
    {
        if( cvEvalFastHaarFeature( 
                ((CvCARTHaarClassifier*) classifier)->fastfeature + idx, sum, tilted )
              < (((CvCARTHaarClassifier*) classifier)->threshold[idx] * normfactor) )
        {
            idx = ((CvCARTHaarClassifier*) classifier)->left[idx];
        }
        else
        {
            idx = ((CvCARTHaarClassifier*) classifier)->right[idx];
        }
    } while( idx > 0 );
    
    return ((CvCARTHaarClassifier*) classifier)->val[-idx];
}

CV_IMPL
CvIntHaarClassifier* icvCreateStageHaarClassifier( int count, float threshold )
{
    CvStageHaarClassifier* stage;
    size_t datasize;

    datasize = sizeof( *stage ) + sizeof( CvIntHaarClassifier* ) * count;
    stage = (CvStageHaarClassifier*) cvAlloc( datasize );
    memset( stage, 0, datasize );

    stage->count = count;
    stage->threshold = threshold;
    stage->classifier = (CvIntHaarClassifier**) (stage + 1);

    stage->eval = icvEvalStageHaarClassifier;
    stage->save = icvSaveStageHaarClassifier;
    stage->release = icvReleaseStageHaarClassifier;

    return (CvIntHaarClassifier*) stage;
}

CV_IMPL
void icvReleaseStageHaarClassifier( CvIntHaarClassifier** classifier )
{
    int i;

    for( i = 0; i < ((CvStageHaarClassifier*) *classifier)->count; i++ )
    {
        if( ((CvStageHaarClassifier*) *classifier)->classifier[i] != NULL )
        {
            ((CvStageHaarClassifier*) *classifier)->classifier[i]->release(
                &(((CvStageHaarClassifier*) *classifier)->classifier[i]) );
        }
    }

    cvFree( (void**) classifier );
    *classifier = NULL;
}

CV_IMPL
float icvEvalStageHaarClassifier( CvIntHaarClassifier* classifier,
                                  sum_type* sum, sum_type* tilted, float normfactor )
{
    int i;
    float stage_sum;

    stage_sum = 0.0F;
    for( i = 0; i < ((CvStageHaarClassifier*) classifier)->count; i++ )
    {
        stage_sum += 
            ((CvStageHaarClassifier*) classifier)->classifier[i]->eval(
                ((CvStageHaarClassifier*) classifier)->classifier[i],
                sum, tilted, normfactor );
    }

    return stage_sum;
}

CV_IMPL
CvIntHaarClassifier* icvCreateCascadeHaarClassifier( int count )
{
    CvCascadeHaarClassifier* ptr;
    size_t datasize;

    datasize = sizeof( *ptr ) + sizeof( CvIntHaarClassifier* ) * count;
    ptr = (CvCascadeHaarClassifier*) cvAlloc( datasize );
    memset( ptr, 0, datasize );

    ptr->count = count;
    ptr->classifier = (CvIntHaarClassifier**) (ptr + 1);

    ptr->eval = icvEvalCascadeHaarClassifier;
    ptr->save = NULL;
    ptr->release = icvReleaseCascadeHaarClassifier;

    return (CvIntHaarClassifier*) ptr;
}

CV_IMPL
void icvReleaseCascadeHaarClassifier( CvIntHaarClassifier** classifier )
{
    int i;

    for( i = 0; i < ((CvCascadeHaarClassifier*) *classifier)->count; i++ )
    {
        if( ((CvCascadeHaarClassifier*) *classifier)->classifier[i] != NULL )
        {
            ((CvCascadeHaarClassifier*) *classifier)->classifier[i]->release(
                &(((CvCascadeHaarClassifier*) *classifier)->classifier[i]) );
        }
    }

    cvFree( (void**) classifier );
    *classifier = NULL;
}

CV_IMPL
float icvEvalCascadeHaarClassifier( CvIntHaarClassifier* classifier,
                                    sum_type* sum, sum_type* tilted, float normfactor )
{
    int i;

    for( i = 0; i < ((CvCascadeHaarClassifier*) classifier)->count; i++ )
    {
        if( ((CvCascadeHaarClassifier*) classifier)->classifier[i]->eval(
                    ((CvCascadeHaarClassifier*) classifier)->classifier[i],
                    sum, tilted, normfactor )
            < ( ((CvStageHaarClassifier*) 
                    ((CvCascadeHaarClassifier*) classifier)->classifier[i])->threshold 
                            - CV_THRESHOLD_EPS) )
        {
            return 0.0;
        }
    }

    return 1.0;
}

CV_IMPL
void icvSaveHaarFeature( CvHaarFeature* feature, FILE* file )
{
    fprintf( file, "%d\n", ( ( feature->rect[2].weight == 0.0F ) ? 2 : 3) );
    fprintf( file, "%d %d %d %d %d %d\n",
        feature->rect[0].r.x,
        feature->rect[0].r.y,
        feature->rect[0].r.width,
        feature->rect[0].r.height,
        0,
        (int) (feature->rect[0].weight) );
    fprintf( file, "%d %d %d %d %d %d\n",
        feature->rect[1].r.x,
        feature->rect[1].r.y,
        feature->rect[1].r.width,
        feature->rect[1].r.height,
        0,
        (int) (feature->rect[1].weight) );
    if( feature->rect[2].weight != 0.0F )
    {
        fprintf( file, "%d %d %d %d %d %d\n",
            feature->rect[2].r.x,
            feature->rect[2].r.y,
            feature->rect[2].r.width,
            feature->rect[2].r.height,
            0,
            (int) (feature->rect[2].weight) );
    }
    fprintf( file, "%s\n", &(feature->desc[0]) );
}

CV_IMPL
void icvLoadHaarFeature( CvHaarFeature* feature, FILE* file )
{
    int nrect;
    int j;
    int tmp;
    int weight;
        
    nrect = 0;
    fscanf( file, "%d", &nrect );
    
    assert( nrect <= CV_HAAR_FEATURE_MAX );

    for( j = 0; j < nrect; j++ )
    {
        fscanf( file, "%d %d %d %d %d %d",
            &(feature->rect[j].r.x),
            &(feature->rect[j].r.y),
            &(feature->rect[j].r.width),
            &(feature->rect[j].r.height),
            &tmp, &weight );
        feature->rect[j].weight = (float) weight;
    }
    for( j = nrect; j < CV_HAAR_FEATURE_MAX; j++ )
    {
        feature->rect[j].r.x = 0;
        feature->rect[j].r.y = 0;
        feature->rect[j].r.width = 0;
        feature->rect[j].r.height = 0;                    
        feature->rect[j].weight = 0.0f;
    }
    fscanf( file, "%s", &(feature->desc[0]) );
    feature->tilted = ( feature->desc[0] == 't' );
}

CV_IMPL
void icvSaveCARTHaarClassifier( CvIntHaarClassifier* classifier, FILE* file )
{
    int i;
    int count;

    count = ((CvCARTHaarClassifier*) classifier)->count;
    fprintf( file, "%d\n", count );
    for( i = 0; i < count; i++ )
    {
        icvSaveHaarFeature( &(((CvCARTHaarClassifier*) classifier)->feature[i]), file );
        fprintf( file, "%e %d %d\n",
            ((CvCARTHaarClassifier*) classifier)->threshold[i],
            ((CvCARTHaarClassifier*) classifier)->left[i],
            ((CvCARTHaarClassifier*) classifier)->right[i] );        
    }
    for( i = 0; i <= count; i++ )
    {
        fprintf( file, "%e ", ((CvCARTHaarClassifier*) classifier)->val[i] );
    }
    fprintf( file, "\n" );
}    

CV_IMPL
CvIntHaarClassifier* icvLoadCARTHaarClassifier( FILE* file, int step )
{
    CvCARTHaarClassifier* ptr;
    int i;
    int count;

    ptr = NULL;
    fscanf( file, "%d", &count );
    if( count > 0 )
    {
        ptr = (CvCARTHaarClassifier*) icvCreateCARTHaarClassifier( count );
        for( i = 0; i < count; i++ )
        {
            icvLoadHaarFeature( &(ptr->feature[i]), file );
            fscanf( file, "%f %d %d", &(ptr->threshold[i]), &(ptr->left[i]),
                                      &(ptr->right[i]) );
        }
        for( i = 0; i <= count; i++ )
        {
            fscanf( file, "%f", &(ptr->val[i]) );
        }
        icvConvertToFastHaarFeature( ptr->feature, ptr->fastfeature, ptr->count, step );
    }
    
    return (CvIntHaarClassifier*) ptr;
}    

CV_IMPL
void icvSaveStageHaarClassifier( CvIntHaarClassifier* classifier, FILE* file )
{
    int count;
    int i;
    float threshold;

    count = ((CvStageHaarClassifier*) classifier)->count;
    fprintf( file, "%d\n", count );
    for( i = 0; i < count; i++ )
    {
        ((CvStageHaarClassifier*) classifier)->classifier[i]->save(
            ((CvStageHaarClassifier*) classifier)->classifier[i], file );
    }

    threshold = ((CvStageHaarClassifier*) classifier)->threshold;
    
    /* to be compatible with the previous implementation */    
    /* threshold = 2.0F * ((CvStageHaarClassifier*) classifier)->threshold - count; */

    fprintf( file, "%e\n", threshold );
}    


CV_IMPL
CvIntHaarClassifier* icvLoadCARTStageHaarClassifierF( FILE* file, int step )
{
    CvStageHaarClassifier* ptr = NULL;

    CV_FUNCNAME( "icvLoadCARTStageHaarClassifierF" );
    
    __BEGIN__;

    if( file != NULL )
    {
        int count;
        int i;
        float threshold;

        count = 0;
        fscanf( file, "%d", &count );
        if( count > 0 )
        {
            ptr = (CvStageHaarClassifier*) icvCreateStageHaarClassifier( count, 0.0F );
            for( i = 0; i < count; i++ )
            {
                ptr->classifier[i] = icvLoadCARTHaarClassifier( file, step );
            }

            fscanf( file, "%f", &threshold );
            
            ptr->threshold = threshold;
            /* to be compatible with the previous implementation */
            /* ptr->threshold = 0.5F * (threshold + count); */
        }
        if( feof( file ) )
        {
            ptr->release( (CvIntHaarClassifier**) &ptr );
            ptr = NULL;
        }
    }

    __END__;

    return (CvIntHaarClassifier*) ptr;
}

CV_IMPL
CvIntHaarClassifier* icvLoadCARTStageHaarClassifier( const char* filename, int step )
{
    CvIntHaarClassifier* ptr = NULL;

    CV_FUNCNAME( "icvLoadCARTStageHaarClassifier" );

    __BEGIN__;

    FILE* file;

    file = fopen( filename, "r" );
    if( file )
    {
        CV_CALL( ptr = icvLoadCARTStageHaarClassifierF( file, step ) );
        fclose( file );
    }

    __END__;

    return ptr;
}

/* End of file. */
