/*
 * GStreamer
 * Copyright (C) 2008 Julien Isorce <julien.isorce@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * freeglut_window.c
 *
 * Window management methods.
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

#if TARGET_HOST_WINCE
#include <aygshell.h>
#pragma comment( lib, "Aygshell.lib" )

static wchar_t* fghWstrFromStr(const char* str)
{
    int i,len=strlen(str);
    wchar_t* wstr = (wchar_t*)malloc(2*len+2);
    for(i=0; i<len; i++)
        wstr[i] = str[i];
    wstr[len] = 0;
    return wstr;
}

#endif /* TARGET_HOST_WINCE */


/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */

/*
 * Chooses a visual basing on the current display mode settings
 */
#if TARGET_HOST_UNIX_X11

XVisualInfo* fgChooseVisual( void )
{
#define BUFFER_SIZES 6
    int bufferSize[BUFFER_SIZES] = { 16, 12, 8, 4, 2, 1 };
    GLboolean wantIndexedMode = GL_FALSE;
    int attributes[ 32 ];
    int where = 0;

    /* First we have to process the display mode settings... */
/*
 * XXX Why is there a semi-colon in this #define?  The code
 * XXX that uses the macro seems to always add more semicolons...
 */
#define ATTRIB(a) attributes[where++]=a;
#define ATTRIB_VAL(a,v) {ATTRIB(a); ATTRIB(v);}

    if( fgState.DisplayMode & GLUT_INDEX )
    {
        ATTRIB_VAL( GLX_BUFFER_SIZE, 8 );
        wantIndexedMode = GL_TRUE;
    }
    else
    {
        ATTRIB( GLX_RGBA );
        ATTRIB_VAL( GLX_RED_SIZE,   1 );
        ATTRIB_VAL( GLX_GREEN_SIZE, 1 );
        ATTRIB_VAL( GLX_BLUE_SIZE,  1 );
        if( fgState.DisplayMode & GLUT_ALPHA )
            ATTRIB_VAL( GLX_ALPHA_SIZE, 1 );
    }

    if( fgState.DisplayMode & GLUT_DOUBLE )
        ATTRIB( GLX_DOUBLEBUFFER );

    if( fgState.DisplayMode & GLUT_STEREO )
        ATTRIB( GLX_STEREO );

    if( fgState.DisplayMode & GLUT_DEPTH )
        ATTRIB_VAL( GLX_DEPTH_SIZE, 1 );

    if( fgState.DisplayMode & GLUT_STENCIL )
        ATTRIB_VAL( GLX_STENCIL_SIZE, 1 );

    if( fgState.DisplayMode & GLUT_ACCUM )
    {
        ATTRIB_VAL( GLX_ACCUM_RED_SIZE,   1 );
        ATTRIB_VAL( GLX_ACCUM_GREEN_SIZE, 1 );
        ATTRIB_VAL( GLX_ACCUM_BLUE_SIZE,  1 );
        if( fgState.DisplayMode & GLUT_ALPHA )
            ATTRIB_VAL( GLX_ACCUM_ALPHA_SIZE, 1 );
    }

    if( fgState.DisplayMode & GLUT_AUX1 )
        ATTRIB_VAL( GLX_AUX_BUFFERS, 1 );
    if( fgState.DisplayMode & GLUT_AUX2 )
        ATTRIB_VAL( GLX_AUX_BUFFERS, 2 );
    if( fgState.DisplayMode & GLUT_AUX3 )
        ATTRIB_VAL( GLX_AUX_BUFFERS, 3 );
    if( fgState.DisplayMode & GLUT_AUX4 )
        ATTRIB_VAL( GLX_AUX_BUFFERS, 4 );


    /* Push a null at the end of the list */
    ATTRIB( None );

    if( ! wantIndexedMode )
        return glXChooseVisual( fgDisplay.Display, fgDisplay.Screen,
                                attributes );
    else
    {
        XVisualInfo* visualInfo;
        int i;

        /*
         * In indexed mode, we need to check how many bits of depth can we
         * achieve.  We do this by trying each possibility from the list
         * given in the {bufferSize} array.  If we match, we return to caller.
         */
        for( i=0; i<BUFFER_SIZES; i++ )
        {
            attributes[ 1 ] = bufferSize[ i ];
            visualInfo = glXChooseVisual( fgDisplay.Display, fgDisplay.Screen,
                                          attributes );
            if( visualInfo != NULL )
                return visualInfo;
        }
        return NULL;
    }
}
#endif

/*
 * Setup the pixel format for a Win32 window
 */
#if TARGET_HOST_WIN32
GLboolean fgSetupPixelFormat( SFG_Window* window, GLboolean checkOnly,
                              unsigned char layer_type )
{
#if TARGET_HOST_WINCE
    return GL_TRUE;
#else
    PIXELFORMATDESCRIPTOR* ppfd, pfd;
    int flags, pixelformat;

    freeglut_return_val_if_fail( window != NULL, 0 );
    flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    if( fgState.DisplayMode & GLUT_DOUBLE )
        flags |= PFD_DOUBLEBUFFER;

    /* Specify which pixel format do we opt for... */
    pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion        = 1;
    pfd.dwFlags         = flags;
    pfd.iPixelType      = PFD_TYPE_RGBA;
    pfd.cColorBits      = 24;
    pfd.cRedBits        = 0;
    pfd.cRedShift       = 0;
    pfd.cGreenBits      = 0;
    pfd.cGreenShift     = 0;
    pfd.cBlueBits       = 0;
    pfd.cBlueShift      = 0;
    pfd.cAlphaBits      = 0;
    pfd.cAlphaShift     = 0;
    pfd.cAccumBits      = 0;
    pfd.cAccumRedBits   = 0;
    pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits  = 0;
    pfd.cAccumAlphaBits = 0;
#if 0
    pfd.cDepthBits      = 32;
    pfd.cStencilBits    = 0;
#else
    pfd.cDepthBits      = 24;
    pfd.cStencilBits    = 8;
#endif
    if( fgState.DisplayMode & GLUT_AUX4 )
        pfd.cAuxBuffers = 4;
    else if( fgState.DisplayMode & GLUT_AUX3 )
        pfd.cAuxBuffers = 3;
    else if( fgState.DisplayMode & GLUT_AUX2 )
        pfd.cAuxBuffers = 2;
    else if( fgState.DisplayMode & GLUT_AUX1 )
        pfd.cAuxBuffers = 1;
    else
        pfd.cAuxBuffers = 0;

    pfd.iLayerType      = layer_type;
    pfd.bReserved       = 0;
    pfd.dwLayerMask     = 0;
    pfd.dwVisibleMask   = 0;
    pfd.dwDamageMask    = 0;

    pfd.cColorBits = (BYTE) GetDeviceCaps( window->Window.Device, BITSPIXEL );
    ppfd = &pfd;

    pixelformat = ChoosePixelFormat( window->Window.Device, ppfd );
    if( pixelformat == 0 )
        return GL_FALSE;

    if( checkOnly )
        return GL_TRUE;
    return SetPixelFormat( window->Window.Device, pixelformat, ppfd );
#endif /* TARGET_HOST_WINCE */
}
#endif

/*
 * Sets the OpenGL context and the fgStructure "Current Window" pointer to
 * the window structure passed in.
 */
void fgSetWindow ( SFG_Window *window )
{
#if TARGET_HOST_UNIX_X11
    if ( window )
        glXMakeCurrent(
            fgDisplay.Display,
            window->Window.Handle,
            window->Window.Context
        );
#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE
    if( fgStructure.CurrentWindow )
        ReleaseDC( fgStructure.CurrentWindow->Window.Handle,
                   fgStructure.CurrentWindow->Window.Device );

    if ( window )
    {
        window->Window.Device = GetDC( window->Window.Handle );
        wglMakeCurrent(
            window->Window.Device,
            window->Window.Context
        );
    }
#endif
    fgStructure.CurrentWindow = window;
}


/*
 * Opens a window. Requires a SFG_Window object created and attached
 * to the freeglut structure. OpenGL context is created here.
 */
void fgOpenWindow( SFG_Window* window, const char* title,
                   int x, int y, int w, int h)
{
#if TARGET_HOST_UNIX_X11
    XSetWindowAttributes winAttr;
    XTextProperty textProperty;
    XSizeHints sizeHints;
    XWMHints wmHints;
    unsigned long mask;

    window->Window.VisualInfo = fgChooseVisual( );


    if( ! window->Window.VisualInfo )
    {
        /*
         * The "fgChooseVisual" returned a null meaning that the visual
         * context is not available.
         * Try a couple of variations to see if they will work.
         */
        if( !( fgState.DisplayMode & GLUT_DOUBLE ) )
        {
            fgState.DisplayMode |= GLUT_DOUBLE ;
            window->Window.VisualInfo = fgChooseVisual( );
            fgState.DisplayMode &= ~GLUT_DOUBLE;
        }

        /*
         * GLUT also checks for multi-sampling, but I don't see that
         * anywhere else in FREEGLUT so I won't bother with it for the moment.
         */
    }

    FREEGLUT_INTERNAL_ERROR_EXIT( window->Window.VisualInfo != NULL,
                                  "Visual with necessary capabilities not found", "fgOpenWindow" );

    /*
     * XXX HINT: the masks should be updated when adding/removing callbacks.
     * XXX       This might speed up message processing. Is that true?
     * XXX
     * XXX A: Not appreciably, but it WILL make it easier to debug.
     * XXX    Try tracing old GLUT and try tracing freeglut.  Old GLUT
     * XXX    turns off events that it doesn't need and is a whole lot
     * XXX    more pleasant to trace.  (Think mouse-motion!  Tons of
     * XXX    ``bonus'' GUI events stream in.)
     */
    winAttr.event_mask        =
        StructureNotifyMask | SubstructureNotifyMask | ExposureMask |
        ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
        VisibilityChangeMask | EnterWindowMask | LeaveWindowMask |
        PointerMotionMask | ButtonMotionMask;
    winAttr.background_pixmap = None;
    winAttr.background_pixel  = 0;
    winAttr.border_pixel      = 0;

    winAttr.colormap = XCreateColormap(
        fgDisplay.Display, fgDisplay.RootWindow,
        window->Window.VisualInfo->visual, AllocNone
    );

    mask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;


    if (!window->Window.Handle)
    {
        window->Window.isInternal = TRUE;
        window->Window.Handle = XCreateWindow(
            fgDisplay.Display,
            fgDisplay.RootWindow,
            x, y, w, h, 0,
            window->Window.VisualInfo->depth, InputOutput,
            window->Window.VisualInfo->visual, mask,
            &winAttr
        );

    }
    else
    {
        window->Window.isInternal = FALSE;
        window->State.OldWidth = w;
        window->State.OldHeight = h;
    }

    /*
     * The GLX context creation, possibly trying the direct context rendering
     *  or else use the current context if the user has so specified
     */
    if( fgState.UseCurrentContext )
    {
        window->Window.Context = glXGetCurrentContext( );

        if( ! window->Window.Context )
            window->Window.Context = glXCreateContext(
                fgDisplay.Display, window->Window.VisualInfo,
                NULL, ( fgState.DirectContext != GLUT_FORCE_INDIRECT_CONTEXT )
            );
    }
    else
        window->Window.Context = glXCreateContext(
            fgDisplay.Display, window->Window.VisualInfo,
            NULL, ( fgState.DirectContext != GLUT_FORCE_INDIRECT_CONTEXT )
        );

#if !defined( __FreeBSD__ ) && !defined( __NetBSD__ )
    if(  !glXIsDirect( fgDisplay.Display, window->Window.Context ) )
    {
      if( fgState.DirectContext == GLUT_FORCE_DIRECT_CONTEXT )
        fgError( "Unable to force direct context rendering for window '%s'",
                 title );
      else if( fgState.DirectContext == GLUT_TRY_DIRECT_CONTEXT )
        fgWarning( "Unable to create direct context rendering for window '%s'\nThis may hurt performance.",
                 title );
    }
#endif

    /*
     * XXX Assume the new window is visible by default
     * XXX Is this a  safe assumption?
     */
    window->State.Visible = GL_TRUE;

    sizeHints.flags = 0;
    if ( fgState.Position.Use )
        sizeHints.flags |= USPosition;
    if ( fgState.Size.Use )
        sizeHints.flags |= USSize;

    /*
     * Fill in the size hints values now (the x, y, width and height
     * settings are obsolete, are there any more WMs that support them?)
     * Unless the X servers actually stop supporting these, we should
     * continue to fill them in.  It is *not* our place to tell the user
     * that they should replace a window manager that they like, and which
     * works, just because *we* think that it's not "modern" enough.
     */
    sizeHints.x      = x;
    sizeHints.y      = y;
    sizeHints.width  = w;
    sizeHints.height = h;

    wmHints.flags = StateHint;
    wmHints.initial_state = NormalState;
    /* Prepare the window and iconified window names... */
    XStringListToTextProperty( (char **) &title, 1, &textProperty );

    XSetWMProperties(
        fgDisplay.Display,
        window->Window.Handle,
        &textProperty,
        &textProperty,
        0,
        0,
        &sizeHints,
        &wmHints,
        NULL
    );
    XFree( textProperty.value );

    XSetWMProtocols( fgDisplay.Display, window->Window.Handle,
                     &fgDisplay.DeleteWindow, 1 );

    glXMakeCurrent(
        fgDisplay.Display,
        window->Window.Handle,
        window->Window.Context
    );

    XMapWindow( fgDisplay.Display, window->Window.Handle );

#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE

    WNDCLASS wc;
    DWORD flags;
    DWORD exFlags = 0;
    ATOM atom;

    /* Grab the window class we have registered on glutInit(): */
    atom = GetClassInfo( fgDisplay.Instance, "FREEGLUT", &wc );
    FREEGLUT_INTERNAL_ERROR_EXIT ( atom, "Window Class Info Not Found",
                                   "fgOpenWindow" );

#if !TARGET_HOST_WINCE
    /*
     * Update the window dimensions, taking account of window
     * decorations.  "freeglut" is to create the window with the
     * outside of its border at (x,y) and with dimensions (w,h).
     */
    w += (GetSystemMetrics( SM_CXSIZEFRAME ) )*2;
    h += (GetSystemMetrics( SM_CYSIZEFRAME ) )*2 +
        GetSystemMetrics( SM_CYCAPTION );

#endif /* TARGET_HOST_WINCE */

    if( ! fgState.Position.Use )
    {
        x = CW_USEDEFAULT;
        y = CW_USEDEFAULT;
    }
    if( ! fgState.Size.Use )
    {
        w = CW_USEDEFAULT;
        h = CW_USEDEFAULT;
    }

    flags = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;

#if !TARGET_HOST_WINCE
    flags |= WS_OVERLAPPEDWINDOW;
#endif

#if TARGET_HOST_WINCE
    {
        wchar_t* wstr = fghWstrFromStr(title);

        window->Window.Handle = CreateWindow(
            _T("FREEGLUT"),
            wstr,
            WS_VISIBLE | WS_POPUP,
            0,0, 240,320,
            NULL,
            NULL,
            fgDisplay.Instance,
            (LPVOID) window
        );

        free(wstr);

        SHFullScreen(window->Window.Handle, SHFS_HIDESTARTICON);
        SHFullScreen(window->Window.Handle, SHFS_HIDESIPBUTTON);
        SHFullScreen(window->Window.Handle, SHFS_HIDETASKBAR);
        MoveWindow(window->Window.Handle, 0, 0, 240, 320, TRUE);
        ShowWindow(window->Window.Handle, SW_SHOW);
        UpdateWindow(window->Window.Handle);
    }
#else
    if (!window->Window.Handle)
    {

        window->Window.isInternal = TRUE;
        window->Window.Handle = CreateWindowEx(
            exFlags,
            "FREEGLUT",
            title,
            flags,
            x, y, w, h,
            (HWND) NULL,
            (HMENU) NULL,
            fgDisplay.Instance,
            (LPVOID) window
        );
    }
    else
    {
        window->Window.isInternal = FALSE;
        fgOnCreateWindow (window);
    }
#endif /* TARGET_HOST_WINCE */

    if( !( window->Window.Handle ) )
        fgError( "Failed to create a window (%s)!", title );

    ShowWindow( window->Window.Handle, SW_SHOW );

    UpdateWindow( window->Window.Handle );
    ShowCursor( TRUE );  /* XXX Old comments say "hide cursor"! */

#endif

    fgSetWindow( window );

    window->Window.DoubleBuffered =
        ( fgState.DisplayMode & GLUT_DOUBLE ) ? 1 : 0;

    if ( ! window->Window.DoubleBuffered )
    {
        glDrawBuffer ( GL_FRONT );
        glReadBuffer ( GL_FRONT );
    }
}

void fgOnCreateWindow (SFG_Window *window)
{

#if TARGET_HOST_WIN32 || TARGET_HOST_WINCE
    window->Window.Device = GetDC( window->Window.Handle );

#if !TARGET_HOST_WINCE
    fgSetupPixelFormat( window, GL_FALSE, PFD_MAIN_PLANE );
#endif

    if( ! fgState.UseCurrentContext )
        window->Window.Context =
            wglCreateContext( window->Window.Device );
    else
    {
        window->Window.Context = wglGetCurrentContext( );
        if( ! window->Window.Context )
            window->Window.Context =
                wglCreateContext( window->Window.Device );
    }

    window->State.NeedToResize = GL_TRUE;
    window->State.Width  = fgState.Size.X;
    window->State.Height = fgState.Size.Y;

    ReleaseDC( window->Window.Handle, window->Window.Device );

#if TARGET_HOST_WINCE
    /* Take over button handling */
    {
        HINSTANCE dxDllLib=LoadLibrary(_T("gx.dll"));
        if (dxDllLib)
        {
            GXGetDefaultKeys_=(GXGETDEFAULTKEYS)GetProcAddress(dxDllLib, _T("?GXGetDefaultKeys@@YA?AUGXKeyList@@H@Z"));
            GXOpenInput_=(GXOPENINPUT)GetProcAddress(dxDllLib, _T("?GXOpenInput@@YAHXZ"));
        }

        if(GXOpenInput_)
            (*GXOpenInput_)();
        if(GXGetDefaultKeys_)
            gxKeyList = (*GXGetDefaultKeys_)(GX_LANDSCAPEKEYS);
    }

#endif /* TARGET_HOST_WINCE */
#endif
}

/*
 * Closes a window, destroying the frame and OpenGL context
 */
void fgCloseWindow( SFG_Window* window )
{
#if TARGET_HOST_UNIX_X11

    glXDestroyContext( fgDisplay.Display, window->Window.Context );
    if(window->Window.isInternal)
    {
        XDestroyWindow( fgDisplay.Display, window->Window.Handle );
        XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */
    }

#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE

    /* Make sure we don't close a window with current context active */
    if( fgStructure.CurrentWindow == window )
        wglMakeCurrent( NULL, NULL );

    wglDeleteContext( window->Window.Context );

    DestroyWindow( window->Window.Handle );
#endif
}


/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

#if TARGET_HOST_UNIX_X11
typedef unsigned long long UINT64;
#endif

/*
 * Creates a new top-level freeglut window
 */
int FGAPIENTRY glutCreateWindow( const char* title, unsigned long winId )
{
    /* XXX GLUT does not exit; it simply calls "glutInit" quietly if the
     * XXX application has not already done so.  The "freeglut" community
     * XXX decided not to go this route (freeglut-developer e-mail from
     * XXX Steve Baker, 12/16/04, 4:22 PM CST, "Re: [Freeglut-developer]
     * XXX Desired 'freeglut' behaviour when there is no current window"
     */
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutCreateWindow" );

    return fgCreateWindow( title, fgState.Position.X, fgState.Position.Y,
                           fgState.Size.X, fgState.Size.Y,
                           (SFG_WindowHandleType)(UINT64)winId)->ID;
}


/*
 * Destroys a window and all of its subwindows
 */
void FGAPIENTRY glutDestroyWindow( int windowID )
{
    SFG_Window* window;
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutDestroyWindow" );
    window = fgWindowByID( windowID );
    freeglut_return_if_fail( window != NULL );
    {
        fgExecutionState ExecState = fgState.ExecState;
        fgAddToWindowDestroyList( window );
        fgState.ExecState = ExecState;
    }
}

/*
 * This function selects the current window
 */
void FGAPIENTRY glutSetWindow( int ID )
{
    SFG_Window* window = NULL;

    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutSetWindow" );
    if( fgStructure.CurrentWindow != NULL )
        if( fgStructure.CurrentWindow->ID == ID )
            return;

    window = fgWindowByID( ID );
    if( window == NULL )
    {
        fgWarning( "glutSetWindow(): window ID %d not found!", ID );
        return;
    }

    fgSetWindow( window );
}

/*
 * This function returns the ID number of the current window, 0 if none exists
 */
int FGAPIENTRY glutGetWindow( void )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutGetWindow" );
    if( fgStructure.CurrentWindow == NULL )
        return 0;
    return fgStructure.CurrentWindow->ID;
}

/*
 * This function makes the current window visible
 */
void FGAPIENTRY glutShowWindow( void )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutShowWindow" );
    FREEGLUT_EXIT_IF_NO_WINDOW ( "glutShowWindow" );

#if TARGET_HOST_UNIX_X11

    XMapWindow( fgDisplay.Display, fgStructure.CurrentWindow->Window.Handle );
    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE

    ShowWindow( fgStructure.CurrentWindow->Window.Handle, SW_SHOW );

#endif

    fgStructure.CurrentWindow->State.Redisplay = GL_TRUE;
    fgStructure.CurrentWindow->State.Visible = GL_TRUE;
}

/*
 * This function hides the current window
 */
void FGAPIENTRY glutHideWindow( void )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutHideWindow" );
    FREEGLUT_EXIT_IF_NO_WINDOW ( "glutHideWindow" );

#if TARGET_HOST_UNIX_X11

    XWithdrawWindow( fgDisplay.Display,
                     fgStructure.CurrentWindow->Window.Handle,
                     fgDisplay.Screen );

    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE

    ShowWindow( fgStructure.CurrentWindow->Window.Handle, SW_HIDE );

#endif

    fgStructure.CurrentWindow->State.Redisplay = GL_FALSE;
}

/*
 * Set the current window's title
 */
void FGAPIENTRY glutSetWindowTitle( const char* title )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutSetWindowTitle" );
    FREEGLUT_EXIT_IF_NO_WINDOW ( "glutSetWindowTitle" );

#if TARGET_HOST_UNIX_X11

    XTextProperty text;

    text.value = (unsigned char *) title;
    text.encoding = XA_STRING;
    text.format = 8;
    text.nitems = strlen( title );

    XSetWMName(
        fgDisplay.Display,
        fgStructure.CurrentWindow->Window.Handle,
        &text
    );

    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32

    SetWindowText( fgStructure.CurrentWindow->Window.Handle, title );

#elif TARGET_HOST_WINCE
    {
        wchar_t* wstr = fghWstrFromStr(title);

        SetWindowText( fgStructure.CurrentWindow->Window.Handle, wstr );

        free(wstr);
    }
#endif
}

/*
 * Change the current window's size
 */
void FGAPIENTRY glutReshapeWindow( int width, int height )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutReshapeWindow" );
    FREEGLUT_EXIT_IF_NO_WINDOW ( "glutReshapeWindow" );

    fgStructure.CurrentWindow->State.NeedToResize = GL_TRUE;
    fgStructure.CurrentWindow->State.Width  = width ;
    fgStructure.CurrentWindow->State.Height = height;
}

/*** END OF FILE ***/
