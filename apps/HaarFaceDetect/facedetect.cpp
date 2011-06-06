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

#include "cvfltk.h"
#include "highgui.h"
#include "cv.h"
#include "cvaux.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

class VideoWindow;

Fl_Window *root_window=(Fl_Window *)0;
VideoWindow* video_window = 0;
CvCapture* capture = 0;
CvAVIWriter* writer = 0;
static int started = 0;
double timeout = 0.005;
IplImage* video_image = 0;
double old_pos = 0;
Fl_Value_Slider* video_pos = 0;
int guard = 0;
int is_avi = 0;
int is_recorded = 0;
double fps = -1;
double fps_alpha = 0.1;
char fps_buffer[100];
double cpu_freq = 0;
double prev_frame_stamp = 0;
int start_pos = 0;
int total_frames0 = 0;
int total_frames = 0;
int is_loopy = 0;

Fl_Button* stop_button = 0;
Fl_Button* play_button = 0;
Fl_Button* video_button = 0;
Fl_Button* cam_button = 0;
Fl_Button* record_button = 0;
Fl_Box* fps_box = 0;

#define ORIG_WIN_SIZE  24

#define MIN_NEIGHBORS 2

static CvMemStorage* storage = cvCreateMemStorage();
static CvHidHaarClassifierCascade* hid_cascade = 0;

int InitFaceDetect( const char* classifier_cascade_path )
{
    CvHaarClassifierCascade* cascade = cvLoadHaarClassifierCascade(
        classifier_cascade_path && strlen(classifier_cascade_path) > 0 ?
        classifier_cascade_path : "<default_face_cascade>",
        cvSize( ORIG_WIN_SIZE, ORIG_WIN_SIZE ));
    if( !cascade )
        return 0;
    hid_cascade = cvCreateHidHaarClassifierCascade( cascade );
    cvReleaseHaarClassifierCascade( &cascade );
    return 1;
}

void DetectAndDrawFaces( IplImage* img )
{
    if( hid_cascade && img )
    {
        int scale = 2;
        CvSize img_size = cvGetSize( img );
        IplImage* temp = cvCreateImage( cvSize(img_size.width/2,img_size.height/2), 8, 3 );
        int i;

        cvPyrDown( img, temp );
        cvClearMemStorage( storage );

        if( hid_cascade )
        {
            CvSeq* faces = cvHaarDetectObjects( temp, hid_cascade, storage, 1.2, 2, 1 );
            for( i = 0; i < (faces ? faces->total : 0); i++ )
            {
                CvRect r = *(CvRect*)cvGetSeqElem( faces, i );
                cvRectangle( img,
                             cvPoint(r.x*scale,/*img->height - */r.y*scale),
                             cvPoint((r.x+r.width)*scale,/*img->height - */(r.y+r.height)*scale),
                             CV_RGB(255,0,0), 3 );
            }
        }

        cvReleaseImage( &temp );
    }
}

class VideoWindow : public Fl_Box
{
public:
    VideoWindow( int x, int y, int w, int h, const char* t = 0 );
    ~VideoWindow();
    void draw();
};

VideoWindow::VideoWindow( int x, int y, int w, int h, const char* t ):
    Fl_Box( x, y, w, h, t )
{
}

VideoWindow::~VideoWindow()
{
}

void VideoWindow::draw()
{
}

static void end_capture()
{
    started = 0;
    cvReleaseCapture( &capture );
}

static void cb_Stop( Fl_Widget*, void* );
static void cb_StartStopRecord( Fl_Widget*, void* );
static void cb_Exit( Fl_Widget*, void* );

static double get_time_accurate(void)
{
    return (double)cvGetTickCount()*1e-3/(cpu_freq+1e-10);
}

static void get_next_frame(void*)
{
    static int repositioning = 0;
    IplImage* frame = 0;
    double new_pos = video_pos->value();
    
    if( (new_pos-old_pos >= 1e-10 || new_pos-old_pos <= -1e-10) && !repositioning)
    {
        video_window->redraw();
        cvSetCaptureProperty( capture, CV_CAP_PROP_POS_AVI_RATIO, new_pos );
        new_pos = cvGetCaptureProperty( capture, CV_CAP_PROP_POS_AVI_RATIO );
        printf("Repositioning\n");
        repositioning = 1;
    }
    else
    {
        new_pos = cvGetCaptureProperty( capture, CV_CAP_PROP_POS_AVI_RATIO );
        video_pos->value(new_pos);
        repositioning = 0;
    }
    old_pos = new_pos;
    frame = cvQueryFrame( capture );

    if( frame == 0 && is_avi )
    {
        cb_Stop(0,0);
        return;
    }

    if( video_window && frame )
    {
        if( video_window->w() < frame->width || video_window->h() < frame->height )
            root_window->size( (short)(frame->width + 40), (short)(frame->height + 150));

        CvRect rect = { video_window->x(), video_window->y(),
                        frame->width, frame->height };
        
        if( !video_image || video_image->width < rect.width ||
            video_image->height < rect.height )
        {
            cvReleaseImage( &video_image );
            video_image = cvCreateImage( cvSize( rect.width, rect.height ), 8, 3 );
        }

        cvSetImageROI( video_image, cvRect(0,0,rect.width, rect.height));
        if( frame->origin == 1 )
            cvFlip( frame, video_image, 0 );
        else
            cvCopy( frame, video_image, 0 );

        DetectAndDrawFaces( video_image );
        if( writer && is_recorded )
        {
            cvWriteToAVI( writer, video_image );
        }
        cvCvtColor( video_image, video_image, CV_RGB2BGR );

        uchar* data = 0;
        int step = 0;
        CvSize size;
        cvGetRawData( video_image, &data, &step, &size );

        video_window->redraw();
        fl_draw_image( (uchar*)data, video_window->x(), video_window->y(),
                       size.width, size.height, 3, step );
    }

    if( started )
    {
        double cur_frame_stamp = get_time_accurate();
        // update fps
        if( fps < 0 )
            fps = 1000/(cur_frame_stamp - prev_frame_stamp);
        else
            fps = (1-fps_alpha)*fps + fps_alpha*1000/(cur_frame_stamp - prev_frame_stamp);
        prev_frame_stamp = cur_frame_stamp;
        sprintf( fps_buffer, "FPS: %5.1f", fps );
        fps_box->label( fps_buffer );
        fps_box->redraw();
        if( total_frames > 0 )
        {
            if( --total_frames == 0 )
                if( !is_loopy )
                    cb_Exit(0,0);
                else
                {
                    total_frames = total_frames0;
                    cvSetCaptureProperty( capture, CV_CAP_PROP_POS_FRAMES, start_pos );
                }
        }
        Fl::add_timeout( timeout, get_next_frame, 0 );
    }
}

static void cb_PauseResume( Fl_Widget*, void* )
{
    video_button->deactivate();
    cam_button->deactivate();
    stop_button->activate();

    if( !started )
    {
        started = 1;
        play_button->label("@||");
        play_button->activate();
        play_button->redraw();
        stop_button->activate();
        stop_button->redraw();
        record_button->activate();

        video_window->redraw();
        prev_frame_stamp = get_time_accurate();
        Fl::add_timeout( timeout, get_next_frame, 0 );
    }
    else
    {
        started = 0;
        play_button->label("@>");
        play_button->redraw();
    }
}


static void cb_Stop( Fl_Widget*, void* )
{
    if( started )
        cb_PauseResume(0,0);
    if( is_recorded )
        cb_StartStopRecord(0,0);
    cvSetCaptureProperty( capture, CV_CAP_PROP_POS_AVI_RATIO, 0 );
    video_button->activate();
    cam_button->activate();
    stop_button->deactivate();
    record_button->deactivate();
}

static void cb_StartAVI( const char* newfile, int start_pos, int was_started )
{
    if( newfile != 0 )
    {
        end_capture();
        capture = cvCaptureFromAVI( newfile );
        if( capture )
        {
            was_started = 1;
            if( start_pos != 0 )
                cvSetCaptureProperty( capture, CV_CAP_PROP_POS_FRAMES, start_pos );
        }
        is_avi = 1;
    }

    if( was_started )
        cb_PauseResume(0,0);
}


static void cb_Open( Fl_Widget*, void* )
{
    char filename[1000];
    filename[0] = '\0';
    int was_started = 0;
    
    if( started )
    {
        was_started = 1;
        cb_PauseResume(0,0);
    }

    char *newfile = fl_file_chooser("Open File?", "*", filename);
    cb_StartAVI( newfile, 0, was_started );
}


static void cb_StartCam( Fl_Widget*, void* )
{
    if( started )
        cb_PauseResume(0,0);
    end_capture();
#ifdef WIN32    
    Sleep(10);
#endif    
    is_avi = 0;
    capture = cvCaptureFromCAM( -1 );
    cb_PauseResume(0,0);
}


static void cb_StartStopRecord( Fl_Widget*, void* )
{
    if( !is_recorded )
    {
        if( video_image )
        {
            writer = cvCreateAVIWriter( "c:\\test.avi", -1, 15, cvGetSize( video_image ));
            if( writer )
            {
                record_button->box( FL_DOWN_BOX );
                is_recorded = 1;
            }
        }
    }
    else
    {
        record_button->box( FL_UP_BOX );
        cvReleaseAVIWriter( &writer );
        is_recorded = 0;
    }
}

static void cb_AutoRun( void* _aviname )
{
    char* aviname = (char*)_aviname;
    char* ext_pos = strrchr( aviname, '.' );
    if( ext_pos )
    {
        char* colon_pos = strchr( ext_pos, ':' );
        if( colon_pos )
        {
            *colon_pos = '\0';
            start_pos = strtoul( colon_pos+1, &colon_pos, 10 );
            if( colon_pos && *colon_pos == ':' )
                total_frames0 = total_frames = strtoul( colon_pos+1, &colon_pos, 10 );
            if( colon_pos && *colon_pos == ':' )
                is_loopy = 1;
        }
    }

    cb_StartAVI( aviname, start_pos, 0 );
}


static void cb_Exit( Fl_Widget*, void* )
{
    cvReleaseCapture( &capture );
    cvReleaseAVIWriter( &writer );
    exit(0);
}

const int root_w = 400, root_h = 400;

int main( int argc, char **argv )
{
    const char* facebaseopt="--facebase=";
    char* classifierbase = 0;
    char* aviname = 0;
    int auto_run = 0;
    
    if( argc > 1 && argv[argc-1][0] != '-' )
    {
        aviname = argv[argc-1];
        auto_run = 1;
        argc--;
    }
    
    if( argc > 1 && strncmp(argv[argc-1],facebaseopt,strlen(facebaseopt))==0 )
    {
        classifierbase=argv[argc-1] + strlen(facebaseopt);
        argc--;
    }
    
    if( !InitFaceDetect(classifierbase))
    {
        fprintf( stderr, "Could not locate face classifier base at %s\n"
                         "Use --facebase=<classifier base path> option to specify the base\n",
                 classifierbase );
        return -1;
    }
	
    cpu_freq = cvGetTickFrequency(); 
    printf("Tick frequency (*10^-6): %g\n", cpu_freq );
 
    Fl_Window* w;
    {
        Fl_Window* o = root_window = new Fl_Window( root_w, root_h );
        w = o;
        {
            Fl_Tabs* o = new Fl_Tabs( 10, 10, root_w - 20, root_h - 100 );
            // camera tab
            {
                Fl_Group* o = new Fl_Group( 10, 30, root_w - 20, root_h - 110, "Face Detection" );
                {
                    VideoWindow* o = new VideoWindow( 15, 35, root_w - 30, root_h - 120 );
                    video_window = o;
                    o->box( FL_BORDER_BOX );
                    o->color(0);
                }
                o->end();
            }
            o->end();
            Fl_Group::current()->resizable(o);
        }
        {
            const int bwidth = 30, bheight = 30;
            play_button = new Fl_Button( 10, root_h - 35, bwidth, bheight, "@>" );
            play_button->callback((Fl_Callback*)cb_PauseResume);
            play_button->deactivate();
            stop_button = new Fl_Button( 10 + bwidth, root_h - 35, bwidth, bheight, "@square" );
            stop_button->callback((Fl_Callback*)cb_Stop);
            stop_button->deactivate();
            video_button = new Fl_Button( 10 + bwidth*2, root_h - 35, bwidth, bheight, "..." );
            video_button->callback((Fl_Callback*)cb_Open);
            cam_button = new Fl_Button( 10 + bwidth*3, root_h - 35, bwidth, bheight, "[o]" );
            cam_button->callback((Fl_Callback*)cb_StartCam);
            video_pos = new Fl_Value_Slider( 10 + bwidth*4 + 10, root_h - 35, 200, 20, "Position" );
            video_pos->type( FL_HOR_NICE_SLIDER );
            record_button = new Fl_Button( 10 + bwidth*4 + 230, root_h - 35, bwidth, bheight, "@circle" );
            record_button->labelcolor(FL_RED);
            record_button->callback((Fl_Callback*)cb_StartStopRecord );
            record_button->deactivate();
            fps_box = new Fl_Box( 10, root_h - 75, bwidth*4, bheight, "<No data>" );
            fps_box->box( FL_DOWN_BOX );
        }
        o->end();
    }
    Fl::visual(FL_RGB);
    w->show(argc, argv);
    if( auto_run )
        Fl::add_timeout( 0.1, cb_AutoRun, aviname );
    Fl::run();
    cb_Exit(0,0);
    return 0;
}

