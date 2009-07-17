#include <Cocoa/Cocoa.h>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

/* ============================================================= */
/*                                                               */
/*                          MainWindow                           */
/*                                                               */
/* ============================================================= */

@interface MainWindow: NSWindow {
  GstElement *m_pipeline;
}
- (id) initWithContentRect: (NSRect) contentRect Pipeline: (GstElement*) pipeline;
@end

@implementation MainWindow

- (id) initWithContentRect: (NSRect) contentRect Pipeline: (GstElement*) pipeline
{
  m_pipeline = pipeline;
  
  self = [super initWithContentRect: contentRect
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask)
    backing: NSBackingStoreBuffered defer: NO screen: nil];
    
  [self setReleasedWhenClosed:NO];
  [NSApp setDelegate:self];

  [self setTitle:@"gst-plugins-gl implements xoverlay interface"];

  return self;
}

- (BOOL) windowShouldClose:(id)sender {
  NSLog(@"close");
  gst_element_send_event (m_pipeline, gst_event_new_eos ());
  [NSApp stop:self];
  NSLog(@"eos sent");
  return YES;
}

- (void) windowDidResize: (NSNotification *) not {
  NSLog(@"window did resize");
}

- (void) applicationDidFinishLaunching: (NSNotification *) not {
  NSLog(@"applicationDidFinishLaunching");
  [self makeMainWindow];
  [self center];
  [self orderFront:self];
}

- (void) applicationWillFinishLaunching: (NSNotification *) not {
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
  return NO;
}

- (void) applicationWillTerminate:(NSNotification *)aNotification {
}

@end


/* ============================================================= */
/*                                                               */
/*                   gstreamer callbacks                         */
/*                                                               */
/* ============================================================= */


static GstBusSyncReply create_window (GstBus* bus, GstMessage* message, MainWindow* window)
{
  // ignore anything but 'prepare-xwindow-id' element messages
  if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
    return GST_BUS_PASS;

  if (!gst_structure_has_name (message->structure, "prepare-xwindow-id"))
    return GST_BUS_PASS;

  g_print ("setting xwindow id %lud\n", (gulong) window);

  gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (GST_MESSAGE_SRC (message)), (gulong) window);

  gst_message_unref (message);

  return GST_BUS_DROP;
}


static void end_stream_cb(GstBus* bus, GstMessage* message, GMainLoop* loop)
{
  g_print("End of stream\n");
  g_main_loop_quit (loop);
}


/*static gboolean expose_cb(GtkWidget* widget, GdkEventExpose* event, GstElement* videosink)
{
    gst_x_overlay_expose (GST_X_OVERLAY (videosink));
    return FALSE;
}*/

GstElement* pipeline = NULL;


static gpointer thread_func (GMainLoop* loop)
{
  g_print ("loop started\n");
  g_main_loop_run (loop);
  g_print ("loop left\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_print ("state null\n");
  return NULL;
}


/* ============================================================= */
/*                                                               */
/*                         application                           */
/*                                                               */
/* ============================================================= */

int main(int argc, char **argv)
{
	int width = 640;
  int height = 480;
  
  GMainLoop* loop = NULL;
      
  GstElement* videosrc  = NULL;
  GstElement* videosink = NULL;
  GstCaps* caps=NULL;
  gboolean ok=FALSE;
  GstBus *bus=NULL;
  GThread *loop_thread=NULL;
  NSAutoreleasePool *pool=nil;
  NSRect rect;
  MainWindow* window=nil;

  g_print("app created\n");
  
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);
  pipeline = gst_pipeline_new ("pipeline");
    
  videosrc  = gst_element_factory_make ("videotestsrc", "videotestsrc");
  videosink = gst_element_factory_make ("glimagesink", "glimagesink");
  
  g_object_set(G_OBJECT(videosrc), "num-buffers", 500, NULL);

  gst_bin_add_many (GST_BIN (pipeline), videosrc, videosink, NULL);
  
  caps = gst_caps_new_simple("video/x-raw-yuv",
                                      "width", G_TYPE_INT, width,
                                      "height", G_TYPE_INT, height,
                                      "framerate", GST_TYPE_FRACTION, 25, 1,
                                      "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
                                      NULL);

  ok = gst_element_link_filtered(videosrc, videosink, caps);
  gst_caps_unref(caps);
  if (!ok)
    g_warning("could not link videosrc to videosink\n");

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_signal_watch (bus);
  g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), loop);
  g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), loop);
  g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), loop);
  gst_object_unref (bus);
  
  pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  
  rect.origin.x = 0; rect.origin.y = 0;
  rect.size.width = width; rect.size.height = height;
  
  window = [[MainWindow alloc] initWithContentRect:rect Pipeline:pipeline];
  
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window, window);
  gst_object_unref (bus);
  
  g_print ("sync handler set\n");
  
  loop_thread = g_thread_create (
      (GThreadFunc) thread_func, loop, TRUE, NULL);
  
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  
  [window orderFront:window];	
	
  [NSApp run];
  
  g_print ("NSApp loop left\n");
  
  g_thread_join (loop_thread);
  
  g_print ("glib loop thread joined\n");
  
  [window release]; 
  
  [pool release];
	
  g_object_unref (pipeline);	
  
  return 0;
}
