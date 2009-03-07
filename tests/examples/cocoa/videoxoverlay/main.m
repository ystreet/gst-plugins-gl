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
  return YES;
}

- (void) windowDidResize: (NSNotification *) not {
  NSLog(@"window did resize");
}

- (void) applicationDidFinishLaunching: (NSNotification *) not {
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

  g_print ("setting xwindow id\n");

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

static gpointer thread_func (GMainLoop* loop)
{
  g_print ("loop started\n");
  g_main_loop_run (loop);
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
  
  gst_init (&argc, &argv);

  GMainLoop* loop = g_main_loop_new (NULL, FALSE);
  GstElement* pipeline = gst_pipeline_new ("pipeline");
    
  GstElement* videosrc  = gst_element_factory_make ("videotestsrc", "videotestsrc");
  GstElement* videosink = gst_element_factory_make ("glimagesink", "glimagesink");
  
  g_object_set(G_OBJECT(videosrc), "num-buffers", 200, NULL);

  gst_bin_add_many (GST_BIN (pipeline), videosrc, videosink, NULL);
  
  GstCaps* caps = gst_caps_new_simple("video/x-raw-yuv",
                                      "width", G_TYPE_INT, width,
                                      "height", G_TYPE_INT, height,
                                      "framerate", GST_TYPE_FRACTION, 25, 1,
                                      "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
                                      NULL);

  gboolean ok = gst_element_link_filtered(videosrc, videosink, caps);
  gst_caps_unref(caps);
  if (!ok)
    g_warning("could not link videosrc to videosink\n");

  GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_signal_watch (bus);
  g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), loop);
  g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), loop);
  g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), loop);
  gst_object_unref (bus);
  
  GThread *loop_thread = g_thread_create (
      (GThreadFunc) thread_func, loop, TRUE, NULL);
      
  g_print ("glib loop thread created\n");
  
  gst_element_set_state (pipeline, GST_STATE_PAUSED);
  
  g_print("waitting for paused state\n");
  
  GstState state = GST_STATE_PAUSED;
  gst_element_get_state (pipeline, &state, &state, GST_CLOCK_TIME_NONE);
  
  g_print("pipeline paused\n");
  
#ifdef GNUSTEP
  GSRegisterCurrentThread();
#endif
  
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[NSApplication sharedApplication];
  
  g_print("app created\n");
  
  NSRect rect;
  rect.origin.x = 0; rect.origin.y = 0;
  rect.size.width = width; rect.size.height = height;
  
  MainWindow* window = [[MainWindow alloc] initWithContentRect:rect Pipeline:pipeline];
  
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window, window);
  gst_object_unref (bus);
  
  g_print ("sync handler set\n");
  
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  
	if (![NSApp isRunning])
    [NSApp run];
  
  g_thread_join (loop_thread);
  
  g_print ("glib loop thread joined\n");
  
  [window release]; 
  
	[pool release];
  
	return 0;
}
