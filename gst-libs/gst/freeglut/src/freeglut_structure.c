/*
 * freeglut_structure.c
 *
 * Windows need tree structure
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Sat Dec 18 1999
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

/* -- GLOBAL EXPORTS ------------------------------------------------------- */

/*
 * The SFG_Structure container holds information about windows
 * created between glutInit() and glutMainLoop() return.
 */

SFG_Structure fgStructure = { { NULL, NULL },  /* The list of windows       */
                              { NULL, NULL },  /* Windows to Destroy list   */
                              NULL,            /* The current window        */
                              0 };             /* The current new window ID */


/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */

static void fghClearCallBacks( SFG_Window *window )
{
    if( window )
    {
        int i;
        for( i = 0; i < TOTAL_CALLBACKS; ++i )
            window->CallBacks[ i ] = NULL;
    }
}

/*
 * This private function creates, opens and adds to the hierarchy
 * a freeglut window complete with OpenGL context and stuff...
 */
SFG_Window* fgCreateWindow( const char* title,
                            int x, int y, int w, int h,
                            SFG_WindowHandleType winId)
{
    /* Have the window object created */
    SFG_Window *window = (SFG_Window *)calloc( sizeof(SFG_Window), 1 );

    fghClearCallBacks( window );

    /* Initialize the object properties */
    window->ID = ++fgStructure.WindowID;
    window->State.OldHeight = window->State.OldWidth = -1;
    window->Window.Handle = winId;

    fgListAppend( &fgStructure.Windows, &window->Node );

    /*
     * Open the window now. The fgOpenWindow() function is system
     * dependant, and resides in freeglut_window.c. Uses fgState.
     */
    fgOpenWindow( window, title, x, y, w, h);

    return window;
}


/*
 * Function to add a window to the linked list of windows to destroy.
 * Subwindows are automatically added because they hang from the window
 * structure.
 */
void fgAddToWindowDestroyList( SFG_Window* window )
{
    SFG_WindowList *new_list_entry =
        ( SFG_WindowList* )malloc( sizeof(SFG_WindowList ) );
    new_list_entry->window = window;
    fgListAppend( &fgStructure.WindowsToDestroy, &new_list_entry->node );

    /* Check if the window is the current one... */
    if( fgStructure.CurrentWindow == window )
        fgStructure.CurrentWindow = NULL;

    /*
     * Clear all window callbacks except Destroy, which will
     * be invoked later.  Right now, we are potentially carrying
     * out a freeglut operation at the behest of a client callback,
     * so we are reluctant to re-enter the client with the Destroy
     * callback, right now.  The others are all wiped out, however,
     * to ensure that they are no longer called after this point.
     */
    {
        FGCBDestroy destroy = (FGCBDestroy)FETCH_WCB( *window, Destroy );
        fghClearCallBacks( window );
        SET_WCB( *window, Destroy, destroy );
    }
}

/*
 * Function to close down all the windows in the "WindowsToDestroy" list
 */
void fgCloseWindows( )
{
    while( fgStructure.WindowsToDestroy.First )
    {
        SFG_WindowList *window_ptr = fgStructure.WindowsToDestroy.First;
        fgDestroyWindow( window_ptr->window );
        fgListRemove( &fgStructure.WindowsToDestroy, &window_ptr->node );
        free( window_ptr );
    }
}

/*
 * This function destroys a window and all of its subwindows. Actually,
 * another function, defined in freeglut_window.c is called, but this is
 * a whole different story...
 */
void fgDestroyWindow( SFG_Window* window )
{
    FREEGLUT_INTERNAL_ERROR_EXIT ( window, "Window destroy function called with null window",
                                   "fgDestroyWindow" );

    //SFG_Window *activeWindow = fgStructure.CurrentWindow;
    INVOKE_WCB( *window, Destroy, ( ) );
    //fgSetWindow( activeWindow );
    
    fgListRemove( &fgStructure.Windows, &window->Node );

    fghClearCallBacks( window );
    fgCloseWindow( window );
    free( window );
    if( fgStructure.CurrentWindow == window )
        fgStructure.CurrentWindow = NULL;
}

/*
 * This function should be called on glutInit(). It will prepare the internal
 * structure of freeglut to be used in the application. The structure will be
 * destroyed using fgDestroyStructure() on glutMainLoop() return. In that
 * case further use of freeglut should be preceeded with a glutInit() call.
 */
void fgCreateStructure( void )
{
    /*
     * We will be needing a list containing windows
     * Also, no current window is set, as none has been created yet.
     */

    fgListInit(&fgStructure.Windows);
    fgListInit(&fgStructure.WindowsToDestroy);

    fgStructure.CurrentWindow = NULL;
    fgStructure.WindowID = 0;
}

/*
 * This function is automatically called on glutMainLoop() return.
 * It should deallocate and destroy all remnants of previous
 * glutInit()-enforced structure initialization...
 */
void fgDestroyStructure( void )
{
    /* Clean up the WindowsToDestroy list. */
    fgCloseWindows( );

    while( fgStructure.Windows.First )
        fgDestroyWindow( ( SFG_Window * )fgStructure.Windows.First );
}

/*
 * Helper function to enumerate through all registered top-level windows
 */
void fgEnumWindows( FGCBenumerator enumCallback, SFG_Enumerator* enumerator )
{
    SFG_Window *window;

    FREEGLUT_INTERNAL_ERROR_EXIT ( enumCallback && enumerator,
                                   "Enumerator or callback missing from window enumerator call",
                                   "fgEnumWindows" );

    /* Check every of the top-level windows */
    for( window = ( SFG_Window * )fgStructure.Windows.First;
         window;
         window = ( SFG_Window * )window->Node.Next )
    {
        enumCallback( window, enumerator );
        if( enumerator->found )
            return;
    }
}

/*
 * A static helper function to look for a window given its handle
 */
static void fghcbWindowByHandle( SFG_Window *window,
                                 SFG_Enumerator *enumerator )
{
    if ( enumerator->found )
        return;

    /* Check the window's handle. Hope this works. Looks ugly. That's for sure. */
    if( window->Window.Handle == (SFG_WindowHandleType) (enumerator->data) )
    {
        enumerator->found = GL_TRUE;
        enumerator->data = window;

        return;
    }
}

/*
 * fgWindowByHandle returns a (SFG_Window *) value pointing to the
 * first window in the queue matching the specified window handle.
 * The function is defined in freeglut_structure.c file.
 */
SFG_Window* fgWindowByHandle ( SFG_WindowHandleType hWindow )
{
    SFG_Enumerator enumerator;

    /* This is easy and makes use of the windows enumeration defined above */
    enumerator.found = GL_FALSE;
    enumerator.data = (void *)hWindow;
    fgEnumWindows( fghcbWindowByHandle, &enumerator );

    if( enumerator.found )
        return( SFG_Window *) enumerator.data;
    return NULL;
}

/*
 * A static helper function to look for a window given its ID
 */
static void fghcbWindowByID( SFG_Window *window, SFG_Enumerator *enumerator )
{
    /* Make sure we do not overwrite our precious results... */
    if( enumerator->found )
        return;

    /* Check the window's handle. Hope this works. Looks ugly. That's for sure. */
    if( window->ID == *( int *)(enumerator->data) )
    {
        enumerator->found = GL_TRUE;
        enumerator->data = window;

        return;
    }
}

/*
 * This function is similiar to the previous one, except it is
 * looking for a specified (sub)window identifier. The function
 * is defined in freeglut_structure.c file.
 */
SFG_Window* fgWindowByID( int windowID )
{
    SFG_Enumerator enumerator;

    /* Uses a method very similiar for fgWindowByHandle... */
    enumerator.found = GL_FALSE;
    enumerator.data = ( void * )&windowID;
    fgEnumWindows( fghcbWindowByID, &enumerator );
    if( enumerator.found )
        return ( SFG_Window * )enumerator.data;
    return NULL;
}

/*
 * List functions...
 */
void fgListInit(SFG_List *list)
{
    list->First = NULL;
    list->Last = NULL;
}

void fgListAppend(SFG_List *list, SFG_Node *node)
{
    if ( list->Last )
    {
        SFG_Node *ln = (SFG_Node *) list->Last;
        ln->Next = node;
        node->Prev = ln;
    }
    else
    {
        node->Prev = NULL;
        list->First = node;
    }

    node->Next = NULL;
    list->Last = node;
}

void fgListRemove(SFG_List *list, SFG_Node *node)
{
    if( node->Next )
        ( ( SFG_Node * )node->Next )->Prev = node->Prev;
    if( node->Prev )
        ( ( SFG_Node * )node->Prev )->Next = node->Next;
    if( ( ( SFG_Node * )list->First ) == node )
        list->First = node->Next;
    if( ( ( SFG_Node * )list->Last ) == node )
        list->Last = node->Prev;
}

int fgListLength(SFG_List *list)
{
    SFG_Node *node;
    int length = 0;

    for( node =( SFG_Node * )list->First;
         node;
         node = ( SFG_Node * )node->Next )
        ++length;

    return length;
}


void fgListInsert(SFG_List *list, SFG_Node *next, SFG_Node *node)
{
    SFG_Node *prev;

    if( (node->Next = next) )
    {
        prev = next->Prev;
        next->Prev = node;
    }
    else
    {
        prev = list->Last;
        list->Last = node;
    }

    if( (node->Prev = prev) )
        prev->Next = node;
    else
        list->First = node;
}

/*** END OF FILE ***/
