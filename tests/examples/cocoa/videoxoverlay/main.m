#include <Cocoa/Cocoa.h>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

/* ============================================================= */
/*                                                               */
/*                          MainWindow                           */
/*                                                               */
/* ============================================================= */

@interface MainWindow: NSWindow {
  GMainLoop *m_loop;
  GstElement *m_pipeline;
  gboolean m_isClosed;
}
- (id) initWithContentRect:(NSRect) contentRect Loop:(GMainLoop*)loop Pipeline:(GstElement*)pipeline;
- (GMainLoop*) loop;
- (GstElement*) pipeline;
- (gboolean) isClosed;
@end

@implementation MainWindow

- (id) initWithContentRect:(NSRect)contentRect Loop:(GMainLoop*)loop Pipeline:(GstElement*)pipeline
{
  m_loop = loop;
  m_pipeline = pipeline;
  m_isClosed = FALSE;

  self = [super initWithContentRect: contentRect
		styleMask: (NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask)
    backing: NSBackingStoreBuffered defer: NO screen: nil];

  [self setReleasedWhenClosed:NO];
  [NSApp setDelegate:self];

  [self setTitle:@"gst-plugins-gl implements xoverlay interface"];

  return self;
}

- (GMainLoop*) loop {
  return m_loop;
}

- (GstElement*) pipeline {
  return m_pipeline;
}

- (gboolean) isClosed {
  return m_isClosed;
}

- (void) customClose {
  m_isClosed = TRUE;
}

- (BOOL) windowShouldClose:(id)sender {
  gst_element_send_event (m_pipeline, gst_event_new_eos ());
  return YES;
}

- (void) applicationDidFinishLaunching: (NSNotification *) not {
  [self makeMainWindow];
  [self center];
  [self orderFront:self];
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app {
  return NO;
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

  gst_x_overlay_set_window_handle (GST_X_OVERLAY (GST_MESSAGE_SRC (message)), (gulong) window);

  gst_message_unref (message);

  return GST_BUS_DROP;
}


static void end_stream_cb(GstBus* bus, GstMessage* message, MainWindow* window)
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  gst_element_set_state ([window pipeline], GST_STATE_NULL);
  g_object_unref ([window pipeline]);
  g_main_loop_quit ([window loop]);

  [window performSelectorOnMainThread:@selector(customClose) withObject:nil waitUntilDone:YES];

  [pool release];
}

static gpointer thread_func (MainWindow* window)
{
#ifdef GNUSTEP
  GSRegisterCurrentThread();
#endif

  g_main_loop_run ([window loop]);

#ifdef GNUSTEP
  GSUnregisterCurrentThread();
#endif
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

  GMainLoop *loop = NULL;
  GstElement *pipeline = NULL;

  GstElement *videosrc  = NULL;
  GstElement *videosink = NULL;
  GstCaps *caps=NULL;
  gboolean ok=FALSE;
  GstBus *bus=NULL;
  GThread *loop_thread=NULL;
  NSAutoreleasePool *pool=nil;
  NSRect rect;
  MainWindow *window=nil;

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

#ifdef GNUSTEP
  gst_element_set_state (pipeline, GST_STATE_PAUSED);
  GstState state = GST_STATE_PAUSED;
  gst_element_get_state (pipeline, &state, &state, GST_CLOCK_TIME_NONE);
  g_print("pipeline paused\n");
  GSRegisterCurrentThread();
#endif

  pool = [[NSAutoreleasePool alloc] init];
#ifndef GNUSTEP
  [NSApplication sharedApplication];
#endif

  rect.origin.x = 0; rect.origin.y = 0;
  rect.size.width = width; rect.size.height = height;

  window = [[MainWindow alloc] initWithContentRect:rect Loop:loop Pipeline:pipeline];

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_signal_watch (bus);
  g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), window);
  g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), window);
  g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), window);
  gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window, window);
  gst_object_unref (bus);

  loop_thread = g_thread_create (
      (GThreadFunc) thread_func, window, TRUE, NULL);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  [window orderFront:window];

#ifndef GNUSTEP
  while (![window isClosed]) {
    NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask
      untilDate:[NSDate dateWithTimeIntervalSinceNow:1]
      inMode:NSDefaultRunLoopMode dequeue:YES];
    if (event)
      [NSApp sendEvent:event];
  }
#endif

  g_thread_join (loop_thread);

  [window release];

  [pool release];

  gst_element_set_state (pipeline, GST_STATE_NULL);

#ifdef GNUSTEP
  GSUnregisterCurrentThread();
#endif

  return 0;
}
