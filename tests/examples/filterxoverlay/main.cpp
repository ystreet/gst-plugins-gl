#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#ifdef WIN32
#include <gdk/gdkwin32.h>
#else
#include <gdk/gdkx.h>
#endif
#include <gst/interfaces/xoverlay.h>

static GstBusSyncReply create_window (GstBus* bus, GstMessage* message, GtkWidget* widget)
{
    // ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;

    if (!gst_structure_has_name (message->structure, "prepare-xwindow-id"))
        return GST_BUS_PASS;

    g_print ("setting xwindow id\n");

#ifdef WIN32
    gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (GST_MESSAGE_SRC (message)),
        reinterpret_cast<gulong>GDK_WINDOW_HWND(widget->window));
#else
    gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (GST_MESSAGE_SRC (message)),
        GDK_WINDOW_XWINDOW(widget->window));
#endif

    gst_message_unref (message);

    return GST_BUS_DROP;
}

static gboolean expose_cb(GtkWidget* widget, GdkEventExpose* event, GstElement* videosink)
{
     gst_x_overlay_expose (GST_X_OVERLAY (videosink));

     return FALSE;
}

static void destroy_cb(GtkWidget* widget, GdkEvent* event, GstElement* pipeline)
{
     gtk_main_quit();
}

gint main (gint argc, gchar *argv[])
{
    gtk_init (&argc, &argv);
    gst_init (&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (window, 640, 480);
    gtk_window_set_title (GTK_WINDOW (window), "glimagesink implement the gstxoverlay interface");

    GstElement* pipeline = gst_pipeline_new ("pipeline");

    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(destroy_cb), pipeline);

    GstElement* videosrc  = gst_element_factory_make ("videotestsrc", "videotestsrc");
    GstElement* videosink = gst_element_factory_make ("glimagesink", "glimagesink");

    GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv",
                                        "width", G_TYPE_INT, 640,
                                        "height", G_TYPE_INT, 480,
                                        "framerate", GST_TYPE_FRACTION, 25, 1,
                                        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('A', 'Y', 'U', 'V'),
                                        NULL) ;

    gst_bin_add_many (GST_BIN (pipeline), videosrc, videosink, NULL);

    gboolean link_ok = gst_element_link_filtered(videosrc, videosink, caps) ;
    gst_caps_unref(caps) ;
    if(!link_ok)
    {
        g_warning("Failed to link videosrc to videosink!\n") ;
        return -1;
    }

    GtkWidget* area = gtk_drawing_area_new();
    gtk_container_add (GTK_CONTAINER (window), area);

    GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window, area);
    gst_object_unref (bus);

    g_signal_connect(area, "expose-event", G_CALLBACK(expose_cb), videosink);

    GstStateChangeReturn ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_print ("Failed to start up pipeline!\n");
        return -1;
    }

    //From GTK+ doc: "The application is then entirely responsible for drawing the widget background"
    //It seems to be not working, the background is still drawn when resizing/obscured the window ...
    gtk_widget_set_app_paintable (window, TRUE);

    //From GDK+ doc: "May also be used to set a background of "None" on window,
    //by setting a background pixmap of NULL
    //It seems to be not working, the background is still drawn when resizing/obscured the window ...
    GdkScreen* screen = gtk_widget_get_screen (window);
    gdk_window_set_back_pixmap (gdk_screen_get_root_window (screen), NULL, TRUE);

    gtk_widget_show_all (window);

    gtk_main();

    gst_object_unref(pipeline);

    return 0;
}

