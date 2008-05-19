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
 * freeglut_init.c
 *
 * Various freeglut initialization functions.
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 2 1999
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

/* -- GLOBAL VARIABLES ----------------------------------------------------- */

/*
 * A structure pointed by g_pDisplay holds all information
 * regarding the display, screen, root window etc.
 */
SFG_Display fgDisplay;

/*
 * The settings for the current freeglut session
 */
SFG_State fgState = { { -1, -1, GL_FALSE },  /* Position */
                      { 300, 300, GL_TRUE }, /* Size */
                      GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH,  /* DisplayMode */
                      GL_FALSE,              /* Initialised */
                      GLUT_TRY_DIRECT_CONTEXT,  /* DirectContext */
                      GL_FALSE,              /* UseCurrentContext */
                      GL_FALSE,              /* GLDebugSwitch */
                      GL_FALSE,              /* XSyncSwitch */
                      0,                     /* SwapCount */
                      0,                     /* SwapTime */
#if TARGET_HOST_WIN32 || TARGET_HOST_WINCE
                      { 0, GL_FALSE },       /* Time */
#else
                      { { 0, 0 }, GL_FALSE },
#endif
                      { NULL, NULL },         /* Timers */
                      { NULL, NULL },         /* FreeTimers */
                      NULL,                   /* IdleCallback */
                      GLUT_ACTION_EXIT,       /* ActionOnWindowClose */
                      GLUT_EXEC_STATE_INIT,   /* ExecState */
                      NULL                    /* ProgramName */
};


/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */

/*
 * A call to this function should initialize all the display stuff...
 */
static void fghInitialize( const char* displayName )
{
#if TARGET_HOST_UNIX_X11
    fgDisplay.Display = XOpenDisplay( displayName );

    if( fgDisplay.Display == NULL )
        fgError( "failed to open display '%s'", XDisplayName( displayName ) );

    if( !glXQueryExtension( fgDisplay.Display, NULL, NULL ) )
        fgError( "OpenGL GLX extension not supported by display '%s'",
            XDisplayName( displayName ) );

    fgDisplay.Screen = DefaultScreen( fgDisplay.Display );
    fgDisplay.RootWindow = RootWindow(
        fgDisplay.Display,
        fgDisplay.Screen
    );

    fgDisplay.ScreenWidth  = DisplayWidth(
        fgDisplay.Display,
        fgDisplay.Screen
    );
    fgDisplay.ScreenHeight = DisplayHeight(
        fgDisplay.Display,
        fgDisplay.Screen
    );

    fgDisplay.ScreenWidthMM = DisplayWidthMM(
        fgDisplay.Display,
        fgDisplay.Screen
    );
    fgDisplay.ScreenHeightMM = DisplayHeightMM(
        fgDisplay.Display,
        fgDisplay.Screen
    );

    fgDisplay.Connection = ConnectionNumber( fgDisplay.Display );

    /* Create the window deletion atom */
    fgDisplay.DeleteWindow = XInternAtom(
        fgDisplay.Display,
        "WM_DELETE_WINDOW",
        FALSE
    );

#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE

    WNDCLASS wc;
    ATOM atom;

    /* What we need to do is to initialize the fgDisplay global structure here. */
    fgDisplay.Instance = GetModuleHandle( NULL );

    atom = GetClassInfo( fgDisplay.Instance, "FREEGLUT", &wc );

    if( atom == 0 )
    {
        ZeroMemory( &wc, sizeof(WNDCLASS) );

        /*
         * Each of the windows should have its own device context, and we
         * want redraw events during Vertical and Horizontal Resizes by
         * the user.
         *
         * XXX Old code had "| CS_DBCLCKS" commented out.  Plans for the
         * XXX future?  Dead-end idea?
         */
        wc.lpfnWndProc    = fgWindowProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = 0;
        wc.hInstance      = fgDisplay.Instance;
        wc.hIcon          = LoadIcon( fgDisplay.Instance, "GLUT_ICON" );

#if TARGET_HOST_WIN32
        wc.style          = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        if (!wc.hIcon)
          wc.hIcon        = LoadIcon( NULL, IDI_WINLOGO );
#else /* TARGET_HOST_WINCE */
        wc.style          = CS_HREDRAW | CS_VREDRAW;
#endif

        wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
        wc.hbrBackground  = NULL;
        wc.lpszMenuName   = NULL;
        wc.lpszClassName  = "FREEGLUT";

        /* Register the window class */
        atom = RegisterClass( &wc );
        FREEGLUT_INTERNAL_ERROR_EXIT ( atom, "Window Class Not Registered", "fghInitialize" );
    }

    /* The screen dimensions can be obtained via GetSystemMetrics() calls */
    fgDisplay.ScreenWidth  = GetSystemMetrics( SM_CXSCREEN );
    fgDisplay.ScreenHeight = GetSystemMetrics( SM_CYSCREEN );

    {
        HWND desktop = GetDesktopWindow( );
        HDC  context = GetDC( desktop );

        fgDisplay.ScreenWidthMM  = GetDeviceCaps( context, HORZSIZE );
        fgDisplay.ScreenHeightMM = GetDeviceCaps( context, VERTSIZE );

        ReleaseDC( desktop, context );
    }

    /* Set the timer granularity to 1 ms */
    timeBeginPeriod ( 1 );

#endif

    fgState.Initialised = GL_TRUE;
}

/*
 * Perform the freeglut deinitialization...
 */
void fgDeinitialize( void )
{
    SFG_Timer *timer;

    if( !fgState.Initialised )
    {
        fgWarning( "fgDeinitialize(): "
                   "no valid initialization has been performed" );
        return;
    }

    fgDestroyStructure( );

    while( ( timer = fgState.Timers.First) )
    {
        fgListRemove( &fgState.Timers, &timer->Node );
        free( timer );
    }

    while( ( timer = fgState.FreeTimers.First) )
    {
        fgListRemove( &fgState.FreeTimers, &timer->Node );
        free( timer );
    }

    fgState.Initialised = GL_FALSE;

    fgState.Position.X = -1;
    fgState.Position.Y = -1;
    fgState.Position.Use = GL_FALSE;

    fgState.Size.X = 300;
    fgState.Size.Y = 300;
    fgState.Size.Use = GL_TRUE;

    fgState.DisplayMode = GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH;

    fgState.DirectContext  = GLUT_TRY_DIRECT_CONTEXT;
    fgState.UseCurrentContext   = GL_FALSE;
    fgState.GLDebugSwitch       = GL_FALSE;
    fgState.XSyncSwitch         = GL_FALSE;
    fgState.ActionOnWindowClose = GLUT_ACTION_EXIT;
    fgState.ExecState           = GLUT_EXEC_STATE_INIT;

    fgState.Time.Set = GL_FALSE;

    fgListInit( &fgState.Timers );
    fgListInit( &fgState.FreeTimers );

    fgState.IdleCallback = NULL;

    fgState.SwapCount   = 0;
    fgState.SwapTime    = 0;

    if( fgState.ProgramName )
    {
        free( fgState.ProgramName );
        fgState.ProgramName = NULL;
    }

#if TARGET_HOST_UNIX_X11

    /*
     * Make sure all X-client data we have created will be destroyed on
     * display closing
     */
    XSetCloseDownMode( fgDisplay.Display, DestroyAll );

    /*
     * Close the display connection, destroying all windows we have
     * created so far
     */
    XCloseDisplay( fgDisplay.Display );

#elif TARGET_HOST_WIN32 || TARGET_HOST_WINCE

    /* Reset the timer granularity */
    timeEndPeriod ( 1 );

#endif

    fgState.Initialised = GL_FALSE;
}


/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * Perform initialization. This usually happens on the program startup
 * and restarting after glutMainLoop termination...
 */
void FGAPIENTRY glutInit( int* pargc, char** argv )
{
    char* displayName = NULL;
    int i, j, argc = *pargc;

    if( fgState.Initialised )
        fgError( "illegal glutInit() reinitialization attempt" );

    if (pargc && *pargc && argv && *argv && **argv)
    {
        fgState.ProgramName = strdup (*argv);

        if( !fgState.ProgramName )
            fgError ("Could not allocate space for the program's name.");
    }

    fgCreateStructure( );

    fgElapsedTime( );

#if !TARGET_HOST_WINCE

    for( i = 1; i < argc; i++ )
    {
        if( strcmp( argv[ i ], "-display" ) == 0 )
        {
            if( ++i >= argc )
                fgError( "-display parameter must be followed by display name" );

            displayName = argv[ i ];

            argv[ i - 1 ] = NULL;
            argv[ i     ] = NULL;
            ( *pargc ) -= 2;
        }
        else if( strcmp( argv[ i ], "-direct" ) == 0)
        {
            if( fgState.DirectContext == GLUT_FORCE_INDIRECT_CONTEXT )
                fgError( "parameters ambiguity, -direct and -indirect "
                    "cannot be both specified" );

            fgState.DirectContext = GLUT_FORCE_DIRECT_CONTEXT;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
        else if( strcmp( argv[ i ], "-indirect" ) == 0 )
        {
            if( fgState.DirectContext == GLUT_FORCE_DIRECT_CONTEXT )
                fgError( "parameters ambiguity, -direct and -indirect "
                    "cannot be both specified" );

            fgState.DirectContext = GLUT_FORCE_INDIRECT_CONTEXT;
            argv[ i ] = NULL;
            (*pargc)--;
        }
        else if( strcmp( argv[ i ], "-gldebug" ) == 0 )
        {
            fgState.GLDebugSwitch = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
        else if( strcmp( argv[ i ], "-sync" ) == 0 )
        {
            fgState.XSyncSwitch = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
    }

    /* Compact {argv}. */
    for( i = j = 1; i < *pargc; i++, j++ )
    {
        /* Guaranteed to end because there are "*pargc" arguments left */
        while ( argv[ j ] == NULL )
            j++;
        if ( i != j )
            argv[ i ] = argv[ j ];
    }

#endif /* TARGET_HOST_WINCE */

    /*
     * Have the display created now. If there wasn't a "-display"
     * in the program arguments, we will use the DISPLAY environment
     * variable for opening the X display (see code above):
     */
    fghInitialize( displayName );
}

/*
 * Sets the default initial window position for new windows
 */
void FGAPIENTRY glutInitWindowPosition( int x, int y )
{
    fgState.Position.X = x;
    fgState.Position.Y = y;

    if( ( x >= 0 ) && ( y >= 0 ) )
        fgState.Position.Use = GL_TRUE;
    else
        fgState.Position.Use = GL_FALSE;
}

/*
 * Sets the default initial window size for new windows
 */
void FGAPIENTRY glutInitWindowSize( int width, int height )
{
    fgState.Size.X = width;
    fgState.Size.Y = height;

    if( ( width > 0 ) && ( height > 0 ) )
        fgState.Size.Use = GL_TRUE;
    else
        fgState.Size.Use = GL_FALSE;
}

/*
 * Sets the default display mode for all new windows
 */
void FGAPIENTRY glutInitDisplayMode( unsigned int displayMode )
{
    /* We will make use of this value when creating a new OpenGL context... */
    fgState.DisplayMode = displayMode;
}

/*** END OF FILE ***/
