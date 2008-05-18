/*
 * freeglut_state.c
 *
 * Freeglut state query methods.
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 16 1999
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

/* -- LOCAL DEFINITIONS ---------------------------------------------------- */

/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */

#if TARGET_HOST_UNIX_X11
/*
 * Queries the GL context about some attributes
 */
static int fghGetConfig( int attribute )
{
  int returnValue = 0;

  if( fgStructure.CurrentWindow )
      glXGetConfig( fgDisplay.Display, fgStructure.CurrentWindow->Window.VisualInfo,
                    attribute, &returnValue );

  return returnValue;
}
#endif

/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * General settings assignment method
 */
void FGAPIENTRY glutSetOption( GLenum eWhat, int value )
{
    FREEGLUT_EXIT_IF_NOT_INITIALISED ( "glutSetOption" );

    /*
     * XXX In chronological code add order.  (WHY in that order?)
     */
    switch( eWhat )
    {
    case GLUT_ACTION_ON_WINDOW_CLOSE:
        fgState.ActionOnWindowClose = value;
        break;

    case GLUT_RENDERING_CONTEXT:
        fgState.UseCurrentContext =
            ( value == GLUT_USE_CURRENT_CONTEXT ) ? GL_TRUE : GL_FALSE;
        break;

    case GLUT_DIRECT_RENDERING:
        fgState.DirectContext = value;
        break;

    default:
        fgWarning( "glutSetOption(): missing enum handle %d", eWhat );
        break;
    }
}

/*** END OF FILE ***/
