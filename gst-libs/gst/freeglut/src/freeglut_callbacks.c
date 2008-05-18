/*
 * freeglut_callbacks.c
 *
 * The callbacks setting methods.
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

#include <GL/freeglut.h>
#include "freeglut_internal.h"

/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * All of the callbacks setting methods can be generalized to this:
 */
#define SET_CALLBACK(a)                                         \
do                                                              \
{                                                               \
    if( fgStructure.CurrentWindow == NULL )                     \
        return;                                                 \
    SET_WCB( ( *( fgStructure.CurrentWindow ) ), a, callback ); \
} while( 0 )

/*
 * Sets the Display callback for the current window
 */
void FGAPIENTRY glutDisplayFunc( void (* callback)( void ) )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutDisplayFunc" );
    if( !callback )
        fgError( "Fatal error in program.  NULL display callback not "
                 "permitted in GLUT 3.0+ or freeglut 2.0.1+" );
    SET_CALLBACK( Display );
}

/*
 * Sets the Reshape callback for the current window
 */
void FGAPIENTRY glutReshapeFunc( void (* callback)( int, int ) )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutReshapeFunc" );
    SET_CALLBACK( Reshape );
}

/*
 * Sets the global idle callback
 */
void FGAPIENTRY glutIdleFunc( void (* callback)( void ) )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutIdleFunc" );
    fgState.IdleCallback = callback;
}

/*
 * Window destruction callbacks
 */
void FGAPIENTRY glutCloseFunc( void (* callback)( void ) )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutCloseFunc" );
    SET_CALLBACK( Destroy );
}

/*** END OF FILE ***/
