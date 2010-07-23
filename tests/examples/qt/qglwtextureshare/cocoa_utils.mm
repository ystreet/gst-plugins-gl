#import <Cocoa/Cocoa.h>
void *qt_current_nsopengl_context()
{
    return [NSOpenGLContext currentContext];
}
