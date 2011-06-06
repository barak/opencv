#ifdef _CH_
#define WIN32
#error "The file needs cvaux, which is not wrapped yet. Sorry"
#endif

#ifndef _EiC
#include "cv.h"
#include "cvaux.h"
#include "highgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#endif

#ifdef _EiC
#define WIN32
#endif

#define ORIG_WIN_SIZE  24
static CvMemStorage* storage = 0;
static CvHidHaarClassifierCascade* hid_cascade = 0;

void detect_and_draw( IplImage* image );

int main( int argc, char** argv )
{
    CvCapture* capture = 0;
    IplImage *frame, *frame_copy = 0;

    CvHaarClassifierCascade* cascade =
    cvLoadHaarClassifierCascade( "<default_face_cascade>",
                         cvSize( ORIG_WIN_SIZE, ORIG_WIN_SIZE ));
    hid_cascade = cvCreateHidHaarClassifierCascade( cascade, 0, 0, 0, 1 );
    cvReleaseHaarClassifierCascade( &cascade );

    cvNamedWindow( "result", 1 );
    storage = cvCreateMemStorage(0);
    
    if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
        capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
    else if( argc == 2 )
        capture = cvCaptureFromAVI( argv[1] ); 

    if( capture )
    {
        for(;;)
        {
            if( !cvGrabFrame( capture ))
                break;
            frame = cvRetrieveFrame( capture );
            if( !frame )
                break;
            if( !frame_copy )
                frame_copy = cvCreateImage( cvSize(frame->width,frame->height),
                                            IPL_DEPTH_8U, frame->nChannels );
            if( frame->origin == IPL_ORIGIN_TL )
                cvCopy( frame, frame_copy, 0 );
            else
                cvFlip( frame, frame_copy, 0 );
            
            detect_and_draw( frame_copy );

            if( cvWaitKey( 10 ) >= 0 )
                break;
        }

        cvReleaseImage( &frame_copy );
        cvReleaseCapture( &capture );
    }
    else
    {
        char* filename = argc == 2 ? argv[1] : (char*)"lena.jpg";
        IplImage* image = cvLoadImage( filename, 1 );

        if( image )
        {
            detect_and_draw( image );
            cvWaitKey(0);
            cvReleaseImage( &image );
        }
    }
    
    cvDestroyWindow("result");

    return 0;
}

void detect_and_draw( IplImage* img )
{
    int scale = 2;
    IplImage* temp = cvCreateImage( cvSize(img->width/2,img->height/2), 8, 3 );
    CvPoint pt1, pt2;
    int i;

    cvPyrDown( img, temp, CV_GAUSSIAN_5x5 );
    cvClearMemStorage( storage );

    if( hid_cascade )
    {
        CvSeq* faces = cvHaarDetectObjects( temp, hid_cascade, storage,
                                            1.2, 2, CV_HAAR_DO_CANNY_PRUNING );
        for( i = 0; i < (faces ? faces->total : 0); i++ )
        {
            CvRect* r = (CvRect*)cvGetSeqElem( faces, i, 0 );
            pt1.x = r->x*scale;
            pt2.x = (r->x+r->width)*scale;
            pt1.y = r->y*scale;
            pt2.y = (r->y+r->height)*scale;
            cvRectangle( img, pt1, pt2, CV_RGB(255,0,0), 3 );
        }
    }

    cvShowImage( "result", img );
    cvReleaseImage( &temp );
}

#ifdef _EiC
main(1,"facedetect.c");
#endif
