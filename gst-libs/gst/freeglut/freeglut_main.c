/*
 * freeglut_main.c
 *
 * The windows message processing methods.
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Fri Dec 3 1999
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "gstfreeglut.h"
#include "freeglut_internal.h"
#include <errno.h>
#include <stdarg.h>
#if TARGET_HOST_WIN32
#    define VFPRINTF(s,f,a) vfprintf((s),(f),(a))
#else
#    if HAVE_VPRINTF
#        define VFPRINTF(s,f,a) vfprintf((s),(f),(a))
#    elif HAVE_DOPRNT
#        define VFPRINTF(s,f,a) _doprnt((f),(a),(s))
#    else
#        define VFPRINTF(s,f,a)
#    endif
#endif

#if TARGET_HOST_WINCE

typedef struct GXDisplayProperties GXDisplayProperties;
typedef struct GXKeyList GXKeyList;
#include <gx.h>

typedef struct GXKeyList (*GXGETDEFAULTKEYS)(int);
typedef int (*GXOPENINPUT)();

GXGETDEFAULTKEYS GXGetDefaultKeys_ = NULL;
GXOPENINPUT GXOpenInput_ = NULL;

struct GXKeyList gxKeyList;

#endif

#if TARGET_HOST_UNIX_X11
#include <sys/time.h> 
#include <unistd.h>
#endif

/*
 * Try to get the maximum value allowed for ints, falling back to the minimum
 * guaranteed by ISO C99 if there is no suitable header.
 */
#if HAVE_LIMITS_H
#    include <limits.h>
#endif
#ifndef INT_MAX
#    define INT_MAX 32767
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif

/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */

/*
 * Handle a window configuration change. When no reshape
 * callback is hooked, the viewport size is updated to
 * match the new window size.
 */
static void fghReshapeWindow ( SFG_Window *window, int width, int height )
{
    //SFG_Window *current_window = fgStructure.CurrentWindow;

    freeglut_return_if_fail( window != NULL );


#if TARGET_HOST_UNIX_X11

    XResizeWindow( fgDisplay.Display, window->Window.Handle,
                   width, height );
    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32
    {
        RECT winRect;
        int x, y, w, h;

        /* "GetWindowRect" returns the pixel coordinates of the outside of the window */
        GetWindowRect( window->Window.Handle, &winRect );
        x = winRect.left;
        y = winRect.top;
        w = width;
        h = height;

        w += GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
        h += GetSystemMetrics( SM_CYSIZEFRAME ) * 2 +
             GetSystemMetrics( SM_CYCAPTION );

        SetWindowPos( window->Window.Handle,
                      HWND_TOP,
                      x, y, w, h,
                      SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING |
                      SWP_NOZORDER
        );
    }
#endif

    /*
     * XXX Should update {window->State.OldWidth, window->State.OldHeight}
     * XXX to keep in lockstep with UNIX_X11 code.
     */
    if( FETCH_WCB( *window, Reshape ) )
        INVOKE_WCB( *window, Reshape, ( width, height ) );
    else
    {
        fgSetWindow( window );
        glViewport( 0, 0, width, height );
    }

    /*
     * Force a window redraw.  In Windows at least this is only a partial
     * solution:  if the window is increasing in size in either dimension,
     * the already-drawn part does not get drawn again and things look funny.
     * But without this we get this bad behaviour whenever we resize the
     * window.
     */
    window->State.Redisplay = GL_TRUE;
}

/*
 * Calls a window's redraw method. This is used when
 * a redraw is forced by the incoming window messages.
 */
static void fghRedrawWindow ( SFG_Window *window )
{
    SFG_Window *current_window = fgStructure.CurrentWindow;

    freeglut_return_if_fail( window );
    freeglut_return_if_fail( FETCH_WCB ( *window, Display ) );

    window->State.Redisplay = GL_FALSE;

    freeglut_return_if_fail( window->State.Visible );

    fgSetWindow( window );

    if( window->State.NeedToResize )
    {
        fghReshapeWindow(
            window,
            window->State.Width,
            window->State.Height
        );

        window->State.NeedToResize = GL_FALSE;
    }

    INVOKE_WCB( *window, Display, ( ) );

    fgSetWindow( current_window );
}

/*
 * A static helper function to execute display callback for a window
 */
static void fghcbDisplayWindow( SFG_Window *window,
                                SFG_Enumerator *enumerator )
{
    if( window->State.Redisplay &&
        window->State.Visible )
    {
        window->State.Redisplay = GL_FALSE;

#if TARGET_HOST_UNIX_X11
        fghRedrawWindow ( window ) ;
#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE
        if (window->Window.isInternal)
            RedrawWindow(
                window->Window.Handle, NULL, NULL,
                RDW_NOERASE | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW
            );
        else
        {
            RECT destsurf_rect;
            POINT dest_surf_point;
            PAINTSTRUCT ps;

            dest_surf_point.x = 0;
            dest_surf_point.y = 0;
            ClientToScreen (window->Window.Handle, &dest_surf_point);
            GetClientRect (window->Window.Handle, &destsurf_rect);
            OffsetRect (&destsurf_rect, dest_surf_point.x, dest_surf_point.y);

            if (window->State.Width != (destsurf_rect.right - destsurf_rect.left) ||
                window->State.Height != (destsurf_rect.bottom - destsurf_rect.top))
            {
                window->State.Width = destsurf_rect.right - destsurf_rect.left;
                window->State.Height = destsurf_rect.bottom - destsurf_rect.top;
                window->State.NeedToResize = GL_FALSE;
                if( FETCH_WCB( *window, Reshape ) )
                    INVOKE_WCB( *window, Reshape, ( window->State.Width, window->State.Height ) );
                glViewport( 0, 0, window->State.Width, window->State.Height );
            }
         
            window->State.NeedToResize = GL_FALSE;
            window->State.Visible = GL_TRUE;
            BeginPaint( window->Window.Handle, &ps );
            fghRedrawWindow( window );
            EndPaint( window->Window.Handle, &ps );
        }
#endif
    }
}

/*
 * Make all windows perform a display call
 */
static void fghDisplayAll( void )
{
    SFG_Enumerator enumerator;

    enumerator.found = GL_FALSE;
    enumerator.data  =  NULL;

    fgEnumWindows( fghcbDisplayWindow, &enumerator );
}

/*
 * Check the global timers
 */
static void fghCheckTimers( void )
{
    long checkTime = fgElapsedTime( );

    while( fgState.Timers.First )
    {
        SFG_Timer *timer = fgState.Timers.First;

        if( timer->TriggerTime > checkTime )
            break;

        fgListRemove( &fgState.Timers, &timer->Node );
        fgListAppend( &fgState.FreeTimers, &timer->Node );

        timer->Callback( timer->ID );
    }
}

/*
 * Elapsed Time
 */
long fgElapsedTime( void )
{
    if ( fgState.Time.Set )
    {
#if TARGET_HOST_UNIX_X11
        struct timeval now;
        long elapsed;

        gettimeofday( &now, NULL );

        elapsed = (now.tv_usec - fgState.Time.Value.tv_usec) / 1000;
        elapsed += (now.tv_sec - fgState.Time.Value.tv_sec) * 1000;

        return elapsed;
#elif TARGET_HOST_WIN32
        return timeGetTime() - fgState.Time.Value;
#elif TARGET_HOST_WINCE
        return GetTickCount() - fgState.Time.Value;
#endif
    }
    else
    {
#if TARGET_HOST_UNIX_X11
        gettimeofday( &fgState.Time.Value, NULL );
#elif TARGET_HOST_WIN32
        fgState.Time.Value = timeGetTime ();
#elif TARGET_HOST_WINCE
        fgState.Time.Value = GetTickCount();
#endif
        fgState.Time.Set = GL_TRUE ;

        return 0 ;
    }
}

/*
 * Error Messages.
 */
void fgError( const char *fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );

    fprintf( stderr, "freeglut ");
    if( fgState.ProgramName )
        fprintf( stderr, "(%s): ", fgState.ProgramName );
    VFPRINTF( stderr, fmt, ap );
    fprintf( stderr, "\n" );

    va_end( ap );

    if ( fgState.Initialised )
        fgDeinitialize ();

    exit( 1 );
}

void fgWarning( const char *fmt, ... )
{
    va_list ap;

    va_start( ap, fmt );

    fprintf( stderr, "freeglut ");
    if( fgState.ProgramName )
        fprintf( stderr, "(%s): ", fgState.ProgramName );
    VFPRINTF( stderr, fmt, ap );
    fprintf( stderr, "\n" );

    va_end( ap );
}

static void fghHavePendingRedisplaysCallback( SFG_Window* w, SFG_Enumerator* e)
{
    if( w->State.Redisplay )
    {
        e->found = GL_TRUE;
        e->data = w;
    }
}
static int fghHavePendingRedisplays (void)
{
    SFG_Enumerator enumerator;

    enumerator.found = GL_FALSE;
    enumerator.data = NULL;
    fgEnumWindows( fghHavePendingRedisplaysCallback, &enumerator );
    return !!enumerator.data;
}
/*
 * Returns the number of GLUT ticks (milliseconds) till the next timer event.
 */
static long fghNextTimer( void )
{
    long ret = INT_MAX;
    SFG_Timer *timer = fgState.Timers.First;

    if( timer )
        ret = timer->TriggerTime - fgElapsedTime();
    if( ret < 0 )
        ret = 0;

    return ret;
}
/*
 * Does the magic required to relinquish the CPU until something interesting
 * happens.
 */
static void fghSleepForEvents( void )
{
    long msec;

    if( fgState.IdleCallback || fghHavePendingRedisplays( ) )
        return;

    msec = fghNextTimer( );

#if TARGET_HOST_UNIX_X11
    /*
     * Possibly due to aggressive use of XFlush() and friends,
     * it is possible to have our socket drained but still have
     * unprocessed events.  (Or, this may just be normal with
     * X, anyway?)  We do non-trivial processing of X events
     * after the event-reading loop, in any case, so we
     * need to allow that we may have an empty socket but non-
     * empty event queue.
     */
    if( ! XPending( fgDisplay.Display ) )
    {
        fd_set fdset;
        int err;
        int socket;
        struct timeval wait;

        socket = ConnectionNumber( fgDisplay.Display );
        FD_ZERO( &fdset );
        FD_SET( socket, &fdset );
        wait.tv_sec = msec / 1000;
        wait.tv_usec = (msec % 1000) * 1000;
        err = select( socket+1, &fdset, NULL, NULL, &wait );

        if( ( -1 == err ) && ( errno != EINTR ) )
            fgWarning ( "freeglut select() error: %d", errno );
    }
#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE
    MsgWaitForMultipleObjects( 0, NULL, FALSE, msec, QS_ALLEVENTS );
#endif
}


/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * Executes a single iteration in the freeglut processing loop.
 */
void FGAPIENTRY glutMainLoopEvent( void )
{
#if TARGET_HOST_UNIX_X11
    SFG_Window* window;
    XEvent event;

    /* This code was repeated constantly, so here it goes into a definition: */
#define GETWINDOW(a)                             \
    window = fgWindowByHandle( event.a.window ); \
    if( window == NULL )                         \
        break;

#define GETMOUSE(a)                              \
    window->State.MouseX = event.a.x;            \
    window->State.MouseY = event.a.y;

    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutMainLoopEvent" );

    while( XPending( fgDisplay.Display ) )
    {
        XNextEvent( fgDisplay.Display, &event );

        switch( event.type )
        {
        case ClientMessage:
            /* Destroy the window when the WM_DELETE_WINDOW message arrives */
            if( (Atom) event.xclient.data.l[ 0 ] == fgDisplay.DeleteWindow )
            {
                GETWINDOW( xclient );

                fgDestroyWindow ( window );

                if( fgState.ActionOnWindowClose == GLUT_ACTION_EXIT )
                {
                    fgDeinitialize( );
                    exit( 0 );
                }
                else if( fgState.ActionOnWindowClose == GLUT_ACTION_GLUTMAINLOOP_RETURNS )
                    fgState.ExecState = GLUT_EXEC_STATE_STOP;

                return;
            }
            break;

            /*
             * CreateNotify causes a configure-event so that sub-windows are
             * handled compatibly with GLUT.  Otherwise, your sub-windows
             * (in freeglut only) will not get an initial reshape event,
             * which can break things.
             *
             * GLUT presumably does this because it generally tries to treat
             * sub-windows the same as windows.
             *
             * XXX Technically, GETWINDOW( xconfigure ) and
             * XXX {event.xconfigure} may not be legit ways to get at
             * XXX data for CreateNotify events.  In practice, the data
             * XXX is in a union which is laid out much the same either
             * XXX way.  But if you want to split hairs, this isn't legit,
             * XXX and we should instead duplicate some code.
             */
        case CreateNotify:
        case ConfigureNotify:
            GETWINDOW( xconfigure );
            {
                int width = event.xconfigure.width;
                int height = event.xconfigure.height;

                if( ( width != window->State.OldWidth ) ||
                    ( height != window->State.OldHeight ) )
                {
                    //SFG_Window *current_window = fgStructure.CurrentWindow;

                    window->State.OldWidth = width;
                    window->State.OldHeight = height;
                    if( FETCH_WCB( *window, Reshape ) )
                        INVOKE_WCB( *window, Reshape, ( width, height ) );
                    else
                    {
                        fgSetWindow( window );
                        glViewport( 0, 0, width, height );
                    }
                    glutPostRedisplay( );
                }
            }
            break;

        case DestroyNotify:
            /*
             * This is sent to confirm the XDestroyWindow call.
             *
             * XXX WHY is this commented out?  Should we re-enable it?
             */
            /* fgAddToWindowDestroyList ( window ); */
            break;

        case Expose:
            /*
             * We are too dumb to process partial exposes...
             *
             * XXX Well, we could do it.  However, it seems to only
             * XXX be potentially useful for single-buffered (since
             * XXX double-buffered does not respect viewport when we
             * XXX do a buffer-swap).
             *
             */
            if( event.xexpose.count == 0 )
            {
                GETWINDOW( xexpose );
                window->State.Redisplay = GL_TRUE;
            }
            break;

        case MapNotify:
        case UnmapNotify:
            /*
             * If we never do anything with this, can we just not ask to
             * get these messages?
             */
            break;

        case MappingNotify:
            /*
             * Have the client's keyboard knowledge updated (xlib.ps,
             * page 206, says that's a good thing to do)
             */
            XRefreshKeyboardMapping( (XMappingEvent *) &event );
            break;

        case VisibilityNotify:
        {
            GETWINDOW( xvisibility );
            /*
             * XXX INVOKE_WCB() does this check for us.
             */
            //if( ! FETCH_WCB( *window, WindowStatus ) )
            //    break;
            fgSetWindow( window );

            /*
             * Sending this event, the X server can notify us that the window
             * has just acquired one of the three possible visibility states:
             * VisibilityUnobscured, VisibilityPartiallyObscured or
             * VisibilityFullyObscured
             */
            switch( event.xvisibility.state )
            {
            case VisibilityUnobscured:
                //INVOKE_WCB( *window, WindowStatus, ( GLUT_FULLY_RETAINED ) );
                window->State.Visible = GL_TRUE;
                break;

            case VisibilityPartiallyObscured:
                //INVOKE_WCB( *window, WindowStatus,
                //            ( GLUT_PARTIALLY_RETAINED ) );
                window->State.Visible = GL_TRUE;
                break;

            case VisibilityFullyObscured:
                //INVOKE_WCB( *window, WindowStatus, ( GLUT_FULLY_COVERED ) );
                window->State.Visible = GL_FALSE;
                break;

            default:
                fgWarning( "Unknown X visibility state: %d",
                           event.xvisibility.state );
                break;
            }
        }
        break;

        default:
            fgWarning ("Unknown X event type: %d", event.type);
            break;
        }
    }

#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE

    MSG stMsg;

    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutMainLoopEvent" );

    while( PeekMessage( &stMsg, NULL, 0, 0, PM_NOREMOVE ) )
    {

        if( GetMessage( &stMsg, NULL, 0, 0 ) == 0 )
        {
            if( fgState.ActionOnWindowClose == GLUT_ACTION_EXIT )
            {
                fgDeinitialize( );
                exit( 0 );
            }
            else if( fgState.ActionOnWindowClose == GLUT_ACTION_GLUTMAINLOOP_RETURNS )
                fgState.ExecState = GLUT_EXEC_STATE_STOP;

            return;
        }
            
        TranslateMessage( &stMsg );
        DispatchMessage( &stMsg );
    }
#endif

    if( fgState.Timers.First )
        fghCheckTimers( );
    fghDisplayAll( );

    fgCloseWindows( );
}

/*
 * Enters the freeglut processing loop.
 * Stays until the "ExecState" changes to "GLUT_EXEC_STATE_STOP".
 */
void FGAPIENTRY glutMainLoop( void )
{
    int action;

#if TARGET_HOST_WIN32 || TARGET_HOST_WINCE
    SFG_Window *window = (SFG_Window *)fgStructure.Windows.First ;
#endif

    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutMainLoop" );

#if TARGET_HOST_WIN32 || TARGET_HOST_WINCE
    /*
     * Processing before the main loop:  If there is a window which is open and
     * which has a visibility callback, call it.  I know this is an ugly hack,
     * but I'm not sure what else to do about it.  Ideally we should leave
     * something uninitialized in the create window code and initialize it in
     * the main loop, and have that initialization create a "WM_ACTIVATE"
     * message.  Then we would put the visibility callback code in the
     * "case WM_ACTIVATE" block below.         - John Fay -- 10/24/02
     */
    while( window )
    {
        if ( FETCH_WCB( *window, Visibility ) )
        {
            SFG_Window *current_window = fgStructure.CurrentWindow ;

            INVOKE_WCB( *window, Visibility, ( window->State.Visible ) );
            fgSetWindow( current_window );
        }

        window = (SFG_Window *)window->Node.Next ;
    }
#endif

    fgState.ExecState = GLUT_EXEC_STATE_RUNNING ;
    while( fgState.ExecState == GLUT_EXEC_STATE_RUNNING )
    {
        SFG_Window *window;
        
        glutMainLoopEvent( );

        window = ( SFG_Window * )fgStructure.Windows.First;

        if( ! window )
            fgState.ExecState = GLUT_EXEC_STATE_STOP;
        else
        {
            if( fgState.IdleCallback )
                fgState.IdleCallback( );
            fghSleepForEvents( );
        }
    }

    /*
     * When this loop terminates, destroy the display, state and structure
     * of a freeglut session, so that another glutInit() call can happen
     *
     * Save the "ActionOnWindowClose" because "fgDeinitialize" resets it.
     */
    action = fgState.ActionOnWindowClose;
    fgDeinitialize( );
    if( action == GLUT_ACTION_EXIT )
        exit( 0 );
}

/*
 * Leaves the freeglut processing loop.
 */
void FGAPIENTRY glutLeaveMainLoop( void )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutLeaveMainLoop" );
    fgState.ExecState = GLUT_EXEC_STATE_STOP ;
}


#if TARGET_HOST_WIN32 || TARGET_HOST_WINCE

/*
 * The window procedure for handling Win32 events
 */
LRESULT CALLBACK fgWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                               LPARAM lParam )
{
    SFG_Window* window;
    PAINTSTRUCT ps;

    FREEGLUT_INTERNAL_ERROR_EXIT_IF_NOT_INITIALISED ( "Event Handler" ) ;

    window = fgWindowByHandle( hWnd );

    if ( ( window == NULL ) && ( uMsg != WM_CREATE ) )
      return DefWindowProc( hWnd, uMsg, wParam, lParam );

    /* printf ( "Window %3d message <%04x> %12d %12d\n", window?window->ID:0,
             uMsg, wParam, lParam ); */
    switch( uMsg )
    {
    case WM_CREATE:
        /* The window structure is passed as the creation structure paramter... */
        window = (SFG_Window *) (((LPCREATESTRUCT) lParam)->lpCreateParams);
        FREEGLUT_INTERNAL_ERROR_EXIT ( ( window != NULL ), "Cannot create window",
                                       "fgWindowProc" );

        window->Window.Handle = hWnd;
        fgOnCreateWindow (window);
        break;

    case WM_SIZE:
        /*
         * If the window is visible, then it is the user manually resizing it.
         * If it is not, then it is the system sending us a dummy resize with
         * zero dimensions on a "glutIconifyWindow" call.
         */
        if( window->State.Visible )
        {
            window->State.NeedToResize = GL_TRUE;
#if TARGET_HOST_WINCE
            window->State.Width  = HIWORD(lParam);
            window->State.Height = LOWORD(lParam);
#else
            window->State.Width  = LOWORD(lParam);
            window->State.Height = HIWORD(lParam);
#endif /* TARGET_HOST_WINCE */
        }

        break;

    case WM_SHOWWINDOW:
        window->State.Visible = GL_TRUE;
        window->State.Redisplay = GL_TRUE;
        break;

    case WM_PAINT:
        /* Turn on the visibility in case it was turned off somehow */
        window->State.Visible = GL_TRUE;
        BeginPaint( hWnd, &ps );
        fghRedrawWindow( window );
        EndPaint( hWnd, &ps );
        break;

    case WM_CLOSE:
        fgDestroyWindow ( window );
        if ( fgState.ActionOnWindowClose != GLUT_ACTION_CONTINUE_EXECUTION )
            PostQuitMessage(0);
        break;

    case WM_DESTROY:
        /*
         * The window already got destroyed, so don't bother with it.
         */
        return 0;

    case WM_CAPTURECHANGED:
        /* User has finished resizing the window, force a redraw */
        INVOKE_WCB( *window, Display, ( ) );
        break;

    default:
        break;
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );;
}
#endif

/*** END OF FILE ***/
