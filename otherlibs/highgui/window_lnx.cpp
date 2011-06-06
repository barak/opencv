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

#include "_highgui.h"

#ifndef WIN32

#include <Xm/Xm.h>
#include <Xm/PanedW.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <X11/keysym.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct CvWindow;

typedef struct CvTrackbar
{
    int signature;
    Widget widget;
    char* name;
    CvTrackbar* next;
    CvWindow* parent;
    int* data;
    int pos;
    int maxval;
    void (*notify)(int);
}
CvTrackbar;


typedef struct CvWindow
{
    int signature;
    Widget area;
    Widget frame;
    Widget paned;
    Widget trackbars;
    char* name;
    CvWindow* prev;
    CvWindow* next;
    
    XShmSegmentInfo* xshmseg;

    GC gc;
    IplImage* image;
    XImage* dst_image;
    int converted;
    int last_key;
    int flags;

    CvMouseCallback on_mouse;

    struct
    {
        int pos;
        int rows;
        CvTrackbar* first;
    }
    toolbar;
}
CvWindow;


//#ifndef NDEBUG
#define Assert(exp)                                             \
if( !(exp) )                                                    \
{                                                               \
    printf("Assertion: %s  %s: %d\n", #exp, __FILE__, __LINE__);\
    assert(exp);                                                \
}

static void icvPutImage( CvWindow* window );
static void icvCloseWindow( Widget w, XEvent* event, String* params, Cardinal* num_params );
static void icvDrawingAreaCallback( Widget widget, XtPointer client_data, XtPointer call_data);
static void icvTrackbarJumpProc( Widget w, XtPointer client_data, XtPointer call_data );

static Widget       topLevel;
static XtAppContext appContext;
static Atom         wm_delete_window;
static XtActionsRec actions[] = { { "quit", icvCloseWindow  }, { 0, 0 } };
static XtTranslations translations;

static int             last_key;
static XtIntervalId    timer = 0;
Widget cvvTopLevelWidget = 0;

static CvWindow* hg_windows = 0;


HIGHGUI_IMPL int cvInitSystem( int argc, char** argv )
{
    static int wasInitialized = 0;    

    // check initialization status
    if( !wasInitialized )
    {
        hg_windows = 0;

        topLevel = XtAppInitialize( &appContext, "HighGUI",
                                    (XrmOptionDescList) NULL, 0, &argc, argv,
                                    (String *) NULL, (ArgList) NULL, 0);
        XtAppAddActions(appContext, actions, XtNumber(actions));
        wm_delete_window = XInternAtom(XtDisplay(topLevel),"WM_DELETE_WINDOW",False);
    
        translations = XtParseTranslationTable("<Message>WM_PROTOCOLS:quit()");
        wasInitialized = 1;
    }

    return HG_OK;
}

static CvWindow* icvFindWindowByName( const char* name )
{
    CvWindow* window = hg_windows;

    for( ; window != 0 && strcmp( name, window->name) != 0; window = window->next )
        ;

    return window;
}

static CvWindow* icvWindowByWidget( Widget widget )
{
    CvWindow* window = hg_windows;

    while( window != 0 && window->area != widget &&
           window->frame != widget && window->paned != widget )
        window = window->next;

    return window;
}


HIGHGUI_IMPL int cvNamedWindow( const char* name, int flags )
{
    int result = 0;
    CV_FUNCNAME( "cvNamedWindow" );

    __BEGIN__;

    CvWindow* window;
    int len, n;
    Arg args[16];

    cvInitSystem(0,0);

    if( !name )
        CV_ERROR( CV_StsNullPtr, "NULL name string" );

    // Check the name in the storage
    if( icvFindWindowByName( name ) != 0 )
    {
        result = 1;
        EXIT;
    }

    len = strlen(name);
    CV_CALL( window = (CvWindow*)cvAlloc(sizeof(CvWindow) + len + 1));
    memset( window, 0, sizeof(*window));

    window->signature = HG_WINDOW_SIGNATURE;

    n = 0;
    XtSetArg( args[n], XmNwidth, 100 ); n++;
    XtSetArg( args[n], XmNheight, 100 ); n++;
    
    window->frame = XtCreatePopupShell( name,
                        topLevelShellWidgetClass, topLevel,
                        args, n );
    n = 0;
    XtSetArg( args[n], XmNorientation, XmVERTICAL ); n++;
    window->paned = XmCreatePanedWindow( window->frame, "pane", args, n );
    n = 0;
    XtSetArg( args[n], XmNorientation, XmVERTICAL ); n++;
    XtSetArg( args[n], XmNpacking, XmPACK_COLUMN ); n++;
    window->trackbars = XmCreateRowColumn( window->paned, "trackbars", args, n );
    XtManageChild( window->trackbars );

    n = 0;
    window->area = XmCreateDrawingArea( window->paned, "area", args, n);
    XtAddCallback( window->area, XmNinputCallback, icvDrawingAreaCallback, window );
    XtAddCallback( window->area, XmNexposeCallback, icvDrawingAreaCallback, window );
    
    XtManageChild( window->area );
    XtManageChild( window->paned );
                                
    window->name = (char*)(window + 1);
    memcpy( window->name, name, len + 1 );
    window->flags = flags;
    window->signature = HG_WINDOW_SIGNATURE;
    window->image = 0;
    window->gc = 0;
    window->last_key = 0;
    
    window->on_mouse = 0;

    memset( &window->toolbar, 0, sizeof(window->toolbar));

    window->next = hg_windows;
    window->prev = 0;
    if( hg_windows )
        hg_windows->prev = window;
    hg_windows = window;
    
    window->xshmseg = (XShmSegmentInfo*)cvAlloc(sizeof(XShmSegmentInfo));
    memset( window->xshmseg, 0, sizeof(*window->xshmseg) );
    
    XtPopup(window->frame, XtGrabNone);
    /*XMapRaised( XtDisplay(window->frame), window->frame );*/

    XtOverrideTranslations( window->frame, XtParseTranslationTable("<Message>WM_PROTOCOLS:quit()") );

    result = 1;
    __END__;

    return result;
}


static void icvDeleteWindow( CvWindow* window )
{
    CvTrackbar* trackbar;
    
    if( window->prev )
        window->prev->next = window->next;
    else
        hg_windows = window->next;

    if( window->next )
        window->next->prev = window->prev;

    window->prev = window->next = 0;

    cvReleaseImage( &window->image );
    
    if( window->dst_image )
    {
        /*XImage* image = window->dst_image;
        window->dst_image = 0;
        cvFree( (void**)&image->data );
        XDestroyImage( image );*/

        XShmDetach( XtDisplay(window->area), window->xshmseg );
        XDestroyImage( window->dst_image );
        shmdt( window->xshmseg->shmaddr );
        shmctl( window->xshmseg->shmid, IPC_RMID, NULL );        
        window->dst_image = 0;
    }

    if( window->gc )
        ;

    XtDestroyWidget( window->frame );
    
    for( trackbar = window->toolbar.first; trackbar != 0; )
    {
        CvTrackbar* next = trackbar->next;
        cvFree( (void**)&trackbar );
        trackbar = next;
    }
    

    cvFree( (void**)&window->xshmseg ); 
    cvFree( (void**)&window );
}


HIGHGUI_IMPL void cvDestroyWindow( const char* name )
{
    CV_FUNCNAME( "cvDestroyWindow" );
    
    __BEGIN__;

    CvWindow* window;

    if(!name)
        CV_ERROR( CV_StsNullPtr, "NULL name string" );

    window = icvFindWindowByName( name );
    if( !window )
        EXIT;

    icvDeleteWindow( window );

    __END__;
}


HIGHGUI_IMPL void
cvDestroyAllWindows( void )
{
    while( hg_windows )
    {
        CvWindow* window = hg_windows;
        icvDeleteWindow( window );
    }
}


HIGHGUI_IMPL void
cvShowImage( const char* name, const CvArr* arr )
{
    CV_FUNCNAME( "cvShowImage" );

    __BEGIN__;
    
    CvWindow* window;
    int origin = 0;
    CvMat stub, dst, *image;

    if( !name )
        CV_ERROR( CV_StsNullPtr, "NULL name" );

    window = icvFindWindowByName(name);
    if( !window || !arr )
        EXIT; // keep silence here.

    if( CV_IS_IMAGE_HDR( arr ))
        origin = ((IplImage*)arr)->origin;
    
    CV_CALL( image = cvGetMat( arr, &stub ));

    if( !window->image )
        cvResizeWindow( name, image->width, image->height );

    if( window->image &&
        (window->image->width  != image->width  ||
         window->image->height != image->height) )
    {
        cvReleaseImage( &window->image );
    }

    if( !window->image )
    {
		Display* display = XtDisplay(window->area);
        int   depth = DefaultDepth(display, 0);
        int   channels = depth < 8 ? 1 : depth <= 16 ? 2 : 4;
        window->image = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, channels );
                
        if( window->flags )
            XtVaSetValues( window->area,
                           XtNwidth, image->width,
                           XtNheight, image->height, 0 );
    }

    cvConvertImage( image, window->image, origin != 0 );
    icvPutImage( window );

    __END__;
}


HIGHGUI_IMPL void cvResizeWindow(const char* name, int width, int height )
{
    CV_FUNCNAME( "cvResizeWindow" );

    __BEGIN__;
    
    CvWindow* window;
    CvTrackbar* trackbar;

    if( !name )
        CV_ERROR( CV_StsNullPtr, "NULL name" );

    window = icvFindWindowByName(name);
    if(!window)
        EXIT;

    for( trackbar = window->toolbar.first; trackbar != 0; trackbar = trackbar->next )
        height += 10;
    if( hg_windows->flags )
        XtVaSetValues( window->area, 
                       XtNminWidth, width, XtNmaxWidth, width,
                       XtNminHeight, height, XtNmaxHeight, height, 0 );
    XtVaSetValues( window->frame, XtNwidth, width, XtNheight, height, 0 );

    __END__;
}


HIGHGUI_IMPL void cvMoveWindow( const char* name, int x, int y )
{
    CV_FUNCNAME( "cvMoveWindow" );

    __BEGIN__;

    CvWindow* window;

    if( !name )
        CV_ERROR( CV_StsNullPtr, "NULL name" );

    window = icvFindWindowByName(name);
    if(!window)
        EXIT;
    
    XtVaSetValues( window->frame, XmNx, x, XmNy, y, 0 );

    __END__;
}


static CvTrackbar*
icvFindTrackbarByName( const CvWindow* window, const char* name )
{
    CvTrackbar* trackbar = window->toolbar.first;

    for( ; trackbar != 0 && strcmp( trackbar->name, name ) != 0; trackbar = trackbar->next )
        ;

    return trackbar;
}


HIGHGUI_IMPL int
cvCreateTrackbar( const char* trackbar_name, const char* window_name,
                  int* val, int count, CvTrackbarCallback on_notify )
{
    int result = 0;

    CV_FUNCNAME( "cvCreateTrackbar" );

    __BEGIN__;
    
    /*char slider_name[32];*/
    CvWindow* window = 0;
    CvTrackbar* trackbar = 0;

    if( !window_name || !trackbar_name )
        CV_ERROR( CV_StsNullPtr, "NULL window or trackbar name" );

    if( count <= 0 )
        CV_ERROR( CV_StsOutOfRange, "Bad trackbar maximal value" );

    window = icvFindWindowByName(window_name);
    if( !window )
        EXIT;

    trackbar = icvFindTrackbarByName(window,trackbar_name);
    if( !trackbar )
    {
        int height = 0, theight = 50;
        int len = strlen(trackbar_name);
        Arg args[16];
        int n = 0;
        XmString xmname = XmStringCreateLocalized( (String)trackbar_name );
        CvTrackbar* t;
        int tcount = 0;
        
        trackbar = (CvTrackbar*)cvAlloc(sizeof(CvTrackbar) + len + 1);
        memset( trackbar, 0, sizeof(*trackbar));
        trackbar->signature = HG_TRACKBAR_SIGNATURE;
        trackbar->name = (char*)(trackbar+1);
        memcpy( trackbar->name, trackbar_name, len + 1 );
        trackbar->parent = window;
        trackbar->next = window->toolbar.first;
        window->toolbar.first = trackbar;
        XtSetArg( args[n], XmNorientation, XmHORIZONTAL ); n++;
        XtSetArg(args[n], XmNtitleString, xmname ); n++;
        XtSetArg(args[n], XmNshowValue, True); n++;
        
        trackbar->widget = XmCreateScale( window->trackbars, (char*)trackbar_name, args, n );
        XmStringFree( xmname );
        
        for( t = window->toolbar.first; t != 0; t = t->next )
            tcount++;
            
        
        XtAddCallback( trackbar->widget, XmNdragCallback,
                       icvTrackbarJumpProc, (XtPointer)trackbar );
        XtAddCallback( trackbar->widget, XmNvalueChangedCallback,
                       icvTrackbarJumpProc, (XtPointer)trackbar );
                       
        XtVaSetValues( window->trackbars, XmNpaneMinimum,
                       theight*tcount, 0 );
        XtVaGetValues( window->frame, XmNheight, &height, 0 );
        XtVaSetValues( window->frame, XmNheight, height + theight, 0 );
        XtManageChild( trackbar->widget );
    }
    
    XtVaSetValues( trackbar->widget, XmNmaximum, count, 0 );

    trackbar->data = val;
    trackbar->pos = 0;
        
    if( val )
    {
        int value = *val;
        if( value < 0 )
            value = 0;
        if( value > count )
            value = count;
        XtVaSetValues( trackbar->widget, XmNvalue, value, 0 );
        trackbar->pos = value;
    }
        
    trackbar->maxval = count;
    trackbar->notify = on_notify;

    result = 1;

    __END__;

    return result;
}


HIGHGUI_IMPL void
cvSetMouseCallback( const char* window_name, CvMouseCallback on_mouse )
{
    CV_FUNCNAME( "cvSetMouseCallback" );

    __BEGIN__;
    
    CvWindow* window = 0;

    if( !window_name )
        CV_ERROR( CV_StsNullPtr, "NULL window name" );

    window = icvFindWindowByName(window_name);
    if( !window )
        EXIT;

    window->on_mouse = on_mouse;

    __END__;
}


HIGHGUI_IMPL int cvGetTrackbarPos( const char* trackbar_name, const char* window_name )
{
    int pos = -1;
    
    CV_FUNCNAME( "cvGetTrackbarPos" );

    __BEGIN__;

    CvWindow* window;
    CvTrackbar* trackbar = 0;

    if( trackbar_name == 0 || window_name == 0 )
        CV_ERROR( CV_StsNullPtr, "NULL trackbar or window name" );

    window = icvFindWindowByName( window_name );
    if( window )
        trackbar = icvFindTrackbarByName( window, trackbar_name );

    if( trackbar )
        pos = trackbar->pos;

    __END__;

    return pos;
}


HIGHGUI_IMPL void cvSetTrackbarPos( const char* trackbar_name, const char* window_name, int pos )
{
    CV_FUNCNAME( "cvSetTrackbarPos" );

    __BEGIN__;

    CvWindow* window;
    CvTrackbar* trackbar = 0;

    if( trackbar_name == 0 || window_name == 0 )
        CV_ERROR( CV_StsNullPtr, "NULL trackbar or window name" );

    window = icvFindWindowByName( window_name );
    if( window )
        trackbar = icvFindTrackbarByName( window, trackbar_name );

    if( trackbar )
    {
        if( pos < 0 )
            pos = 0;

        if( pos > trackbar->maxval )
            pos = trackbar->maxval;
    }
    XtVaSetValues( trackbar->widget, XmNvalue, pos, 0 );

    __END__;
}


HIGHGUI_IMPL void* cvGetWindowHandle( const char* window_name )
{
    void* widget = 0;
    
    CV_FUNCNAME( "cvGetWindowHandle" );

    __BEGIN__;

    CvWindow* window;

    if( window_name == 0 )
        CV_ERROR( CV_StsNullPtr, "NULL window name" );

    window = icvFindWindowByName( window_name );
    if( window )
        widget = (void*)window->area;

    __END__;

    return widget;
}
    

HIGHGUI_IMPL const char* cvGetWindowName( void* window_handle )
{
    const char* window_name = "";
    
    CV_FUNCNAME( "cvGetWindowName" );

    __BEGIN__;

    CvWindow* window;

    if( window_handle == 0 )
        CV_ERROR( CV_StsNullPtr, "NULL window" );

    window = icvWindowByWidget( (Widget)window_handle );
    if( window )
        window_name = window->name;

    __END__;

    return window_name;
}


/* draw image to frame */
static void icvPutImage( CvWindow* window )
{
    int width = 10, height = 10;
    Assert( window != 0 );

    if( window->image == 0 )
        return;

    XtVaGetValues( window->area, XtNwidth, &width, XtNheight, &height, 0 );
   
    if( width <= 0 || height <= 0 )
        return;

    if( window->flags )
    {
        width = MIN(width, window->image->width);
        height = MIN(height, window->image->height);
    }

    if( window->dst_image &&
        (window->dst_image->width < width  ||
        window->dst_image->height < height) )
    {
        /*cvFree( (void**)&window->dst_image->data );*/
        XShmDetach( XtDisplay(window->area), window->xshmseg );
        XDestroyImage( window->dst_image );
        shmdt( window->xshmseg->shmaddr );
        shmctl( window->xshmseg->shmid, IPC_RMID, NULL );
        window->dst_image = 0;
    }

    if( !window->dst_image )
    {
        int new_width = (width + 7) & -8;
        int new_height = (height + 7) & -8;

        /*int step = (new_width * (window->image->depth / 8) + 3) & ~3;
        char* data = (char*)cvAlloc( step * new_height );
        window->dst_image = XCreateImage( XtDisplay(window->area),
                                          CopyFromParent,
                                          window->image->depth,
                                          ZPixmap, 0, data, new_width, new_height,
                                          8, step );
        window->dst_image->bits_per_pixel = window->dst_image->depth;*/

        Display* display = XtDisplay(window->frame);
        Visual* visual = DefaultVisualOfScreen(DefaultScreenOfDisplay(display));
        int depth = DefaultDepth(display, 0);
        window->dst_image = XShmCreateImage( display, visual, depth, ZPixmap,
                                             NULL, window->xshmseg,
                                             new_width, new_height );
        window->xshmseg->shmid = shmget(IPC_PRIVATE,
                                        window->dst_image->height*
                                        window->dst_image->bytes_per_line,
                                        IPC_CREAT|0777);
        assert( window->xshmseg != 0 && window->xshmseg->shmid != -1);
        window->xshmseg->shmaddr = window->dst_image->data =
                (char*)shmat(window->xshmseg->shmid, 0, 0) ;
        window->xshmseg->readOnly = False;
        XShmAttach( display, window->xshmseg );
    }
    
    {
        CvMat src;
        CvMat dst;
        int dst_type = ((window->dst_image->bits_per_pixel/8) - 1)*8 + CV_8U;

        OPENCV_CALL( cvInitMatHeader( &dst, height, width, dst_type,
                                      window->dst_image->data,
                                      window->dst_image->bytes_per_line ));

        if( window->flags )
        {
            OPENCV_CALL( cvInitMatHeader( &src, height, width,
                                          dst_type, window->image->imageData,
                                          window->image->widthStep ));
            OPENCV_CALL( cvCopy( &src, &dst, 0 ));
        }
        else
        {
            OPENCV_CALL( cvInitMatHeader( &src, window->image->height, window->image->width,
                                          dst_type, window->image->imageData,
                                          window->image->widthStep ));
            OPENCV_CALL( cvResize( &src, &dst, CV_INTER_NN ));
        }
    }

    {
    Display* display = XtDisplay(window->area);
    GC gc = XCreateGC( display, XtWindow(window->area), 0, 0 );
    XShmPutImage( display, XtWindow(window->area), gc,
                  window->dst_image, 0, 0, 0, 0,
                  width, height, False );
    /*XPutImage( display, XtWindow(window->area), gc,
               window->dst_image, 0, 0, 0, 0,
               width, height );*/
    XFlush( display );
    XFreeGC( display, gc );
    }
}                                                   
                                                    

static void icvDrawingAreaCallback( Widget widget, XtPointer client_data, XtPointer call_data)
{
    XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct *) call_data;
    XEvent *event = cbs->event;
    CvWindow* window = (CvWindow*)client_data;
    
    if (cbs->reason == XmCR_INPUT)
    {
        if( (event->xany.type == ButtonPress || event->xany.type == ButtonRelease) &&
            window->on_mouse && window->image &&
            widget == window->area )
        {
            int event_type = (event->xany.type == ButtonPress ?
                            CV_EVENT_LBUTTONDOWN : CV_EVENT_LBUTTONUP) +
                            (event->xbutton.button == Button1 ? 0 :
                             event->xbutton.button == Button2 ? 1 : 2);
            int flags = (event->xbutton.state & Button1Mask ? CV_EVENT_FLAG_LBUTTON : 0) |
                        (event->xbutton.state & Button2Mask ? CV_EVENT_FLAG_RBUTTON : 0) |
                        (event->xbutton.state & Button3Mask ? CV_EVENT_FLAG_MBUTTON : 0) |
                        (event->xbutton.state & ShiftMask ? CV_EVENT_FLAG_SHIFTKEY : 0) |
                        (event->xbutton.state & ControlMask ? CV_EVENT_FLAG_CTRLKEY : 0) |
                        (event->xbutton.state & (Mod1Mask|Mod2Mask) ? CV_EVENT_FLAG_ALTKEY : 0);
            int x = event->xbutton.x;
            int y = event->xbutton.y;
            int width = 0, height = 0;
            
            XtVaGetValues( window->area, XmNwidth, &width, XmNheight, &height, 0 );
            
            x = cvRound(((double)x)*window->image->width/MAX(width,1));
            y = cvRound(((double)y)*window->image->height/MAX(height,1));
            
            if( x >= window->image->width )
                x = window->image->width - 1;
            
            if( y >= window->image->height )
                y = window->image->height - 1;
            
            if( window->on_mouse )
                window->on_mouse( event_type, x, y, flags );
        }
        if( event->xany.type == KeyPress )
        {
            KeySym key;
            XLookupString( &event->xkey, 0, 0, &key, 0 );
            if( !IsModifierKey(key) )
            {
                last_key = key;
                if( timer )
                {
                    XtRemoveTimeOut( timer );
                    timer = 0;
                }
            }
        }
    }
    
    if( (cbs->reason == XmCR_EXPOSE || cbs->reason == XmCR_RESIZE) && window->image )
    {
        icvPutImage( window );
    }
    
    XmProcessTraversal( window->area, XmTRAVERSE_CURRENT );
}



static void icvCloseWindow( Widget w, XEvent* event, String* params, Cardinal* num_params )
{
    CvWindow* window = icvWindowByWidget( w );
    if( window )
        icvDeleteWindow( window );
}


static void icvAlarm( XtPointer, XtIntervalId* )
{
    timer = 0;
}


static void icvTrackbarJumpProc( Widget w, XtPointer client_data, XtPointer call_data )
{
    XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *)call_data;
    
    Assert( client_data != 0 );
    Assert( call_data != 0 );

    CvTrackbar* trackbar = (CvTrackbar*)client_data;
    int value = cbs->value;
    
    if( trackbar->data )
        *trackbar->data = value;

    if( trackbar->notify )
        trackbar->notify( value );
    
    XmProcessTraversal( trackbar->parent->area, XmTRAVERSE_CURRENT );
}


int cvWaitKey( int delay )
{
    int exit_flag = 0;
    
    last_key = -1;
    if( delay > 0 )
        timer = XtAppAddTimeOut( appContext, delay, icvAlarm, &timer );
    
    do
    {
        XtAppProcessEvent( appContext, XtIMAll );
    }
    while( last_key < 0 && (delay <= 0 || timer != 0)
           && (exit_flag = XtAppGetExitFlag( appContext )) == 0
           && hg_windows != 0 );
    
    if( timer )
        XtRemoveTimeOut( timer );
    
    return last_key;
}

#endif /* WIN32 */
