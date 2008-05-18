#ifndef  __FREEGLUT_STD_H__
#define  __FREEGLUT_STD_H__

/*
 * freeglut_std.h
 *
 * The GLUT-compatible part of the freeglut library include file
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

#ifdef __cplusplus
    extern "C" {
#endif

/*
 * Under windows, we have to differentiate between static and dynamic libraries
 */
#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__)

/* #pragma may not be supported by some compilers.
 * Discussion by FreeGLUT developers suggests that
 * Visual C++ specific code involving pragmas may
 * need to move to a separate header.  24th Dec 2003
 */ 

#   define WIN32_LEAN_AND_MEAN
#   define NO_MIN_MAX
#undef UNICODE
#    include <windows.h>
#define UNICODE
#   undef min
#   undef max

#endif


#    define FGAPI
#    define FGAPIENTRY


/*
 * The freeglut and GLUT API versions
 */
#define  FREEGLUT             1
#define  GLUT_API_VERSION     4
#define  FREEGLUT_VERSION_2_0 1
#define  GLUT_XLIB_IMPLEMENTATION 13

/*
 * Always include OpenGL and GLU headers
 */
#include <GL/gl.h>
#include <GL/glu.h>

/*
 * GLUT API macro definitions -- the display mode definitions
 */
#define  GLUT_RGB                           0x0000
#define  GLUT_RGBA                          0x0000
#define  GLUT_INDEX                         0x0001
#define  GLUT_SINGLE                        0x0000
#define  GLUT_DOUBLE                        0x0002
#define  GLUT_ACCUM                         0x0004
#define  GLUT_ALPHA                         0x0008
#define  GLUT_DEPTH                         0x0010
#define  GLUT_STENCIL                       0x0020
#define  GLUT_STEREO                        0x0100

/*
 * GLUT API macro definitions -- windows related definitions
 */
#define  GLUT_NOT_VISIBLE                   0x0000
#define  GLUT_VISIBLE                       0x0001
#define  GLUT_HIDDEN                        0x0000

/*
 * Initialization functions, see fglut_init.c
 */
FGAPI void    FGAPIENTRY glutInit( int* pargc, char** argv );
FGAPI void    FGAPIENTRY glutInitWindowPosition( int x, int y );
FGAPI void    FGAPIENTRY glutInitWindowSize( int width, int height );
FGAPI void    FGAPIENTRY glutInitDisplayMode( unsigned int displayMode );

/*
 * Process loop function, see freeglut_main.c
 */
FGAPI void    FGAPIENTRY glutMainLoop( void );

/*
 * Window management functions, see freeglut_window.c
 */
FGAPI int     FGAPIENTRY glutCreateWindow( const char* title, unsigned long winId );
FGAPI void    FGAPIENTRY glutDestroyWindow( int window );
FGAPI void    FGAPIENTRY glutSetWindow( int window );
FGAPI int     FGAPIENTRY glutGetWindow( void );
FGAPI void    FGAPIENTRY glutSetWindowTitle( const char* title );
FGAPI void    FGAPIENTRY glutReshapeWindow( int width, int height );
FGAPI void    FGAPIENTRY glutShowWindow( void );
FGAPI void    FGAPIENTRY glutHideWindow( void );

/*
 * Display-connected functions, see freeglut_display.c
 */
FGAPI void    FGAPIENTRY glutPostRedisplay( void );
FGAPI void    FGAPIENTRY glutSwapBuffers( void );

/*
 * Global callback functions, see freeglut_callbacks.c
 */
FGAPI void    FGAPIENTRY glutIdleFunc( void (* callback)( void ) );

/*
 * Window-specific callback functions, see freeglut_callbacks.c
 */
FGAPI void    FGAPIENTRY glutReshapeFunc( void (* callback)( int, int ) );
FGAPI void    FGAPIENTRY glutDisplayFunc( void (* callback)( void ) );

#ifdef __cplusplus
    }
#endif

/*** END OF FILE ***/

#endif /* __FREEGLUT_STD_H__ */

