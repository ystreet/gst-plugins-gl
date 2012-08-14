/*
 * GStreamer
 * Copyright (C) 2008-2009 Julien Isorce <julien.isorce@gmail.com>
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
 */

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "../gstgtk.h"


static GstBusSyncReply create_window (GstBus* bus, GstMessage* message, GtkWidget* widget)
{
    // ignore anything but 'prepare-window-handle' element messages
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;

    if (!gst_is_video_overlay_prepare_window_handle_message (message))
        return GST_BUS_PASS;

    g_print ("setting window handle\n");

    //at this point we are sure that gdk_window_ensure_native has been already
    //called (see area_realize_cb)

    //do not call gdk_window_ensure_native for the first time here because
    //we are in a different thread than the main thread
    //(and the main thread the onne)
    gst_video_overlay_set_gtk_window (GST_VIDEO_OVERLAY (GST_MESSAGE_SRC (message)), widget);

    gst_message_unref (message);

    return GST_BUS_DROP;
}


static void end_stream_cb(GstBus* bus, GstMessage* message, GstElement* pipeline)
{
    g_print("End of stream\n");

    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    gtk_main_quit();
}


static void area_realize_cb(GtkWidget* widget, GstElement* pipeline)
{
    g_print ("drawing area realized\n");

#if GTK_CHECK_VERSION(2,18,0)
    //Tries to ensure that there is a native window
    //Call it in the same thread as the one who created this widget
    if (!gdk_window_ensure_native (widget->window))
        g_error ("Failed to create native window!");
#endif

    //avoid flickering when resizing or obscuring the main window
    gdk_window_set_back_pixmap(widget->window, NULL, FALSE);
    gtk_widget_set_app_paintable(widget,TRUE);
    gtk_widget_set_double_buffered(widget, FALSE);

    //now we have a native window we can play the pipeline
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
}


static gboolean expose_cb(GtkWidget* widget, GdkEventExpose* event, GstElement* videosink)
{
    g_print ("expose_cb\n");
    //expose callback is garanted to be called after realized callback
    gst_video_overlay_expose (GST_VIDEO_OVERLAY (videosink));
    return FALSE;
}


static void destroy_cb(GtkWidget* widget, GdkEvent* event, GstElement* pipeline)
{
    g_print("Close\n");

    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    gtk_main_quit();
}


static void button_state_null_cb(GtkWidget* widget, GstElement* pipeline)
{
    gst_element_set_state (pipeline, GST_STATE_NULL);
    g_print ("GST_STATE_NULL\n");
}


static void button_state_ready_cb(GtkWidget* widget, GstElement* pipeline)
{
    gst_element_set_state (pipeline, GST_STATE_READY);
    g_print ("GST_STATE_READY\n");
}


static void button_state_paused_cb(GtkWidget* widget, GstElement* pipeline)
{
    gst_element_set_state (pipeline, GST_STATE_PAUSED);
    g_print ("GST_STATE_PAUSED\n");
}


static void button_state_playing_cb(GtkWidget* widget, GstElement* pipeline)
{
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_print ("GST_STATE_PLAYING\n");
}


static gchar* slider_fps_cb (GtkScale* scale, gdouble value, GstElement* pipeline)
{
    //change the video frame rate dynamically
    return g_strdup_printf ("video framerate: %0.*g", gtk_scale_get_digits (scale), value);
}



gint main (gint argc, gchar *argv[])
{
    gtk_init (&argc, &argv);
    gst_init (&argc, &argv);

    GstElement* pipeline = gst_pipeline_new ("pipeline");

    //window that contains an area where the video is drawn
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (window, 640, 480);
    gtk_window_move (GTK_WINDOW (window), 300, 10);
    gtk_window_set_title (GTK_WINDOW (window), "glimagesink implement the gstvideooverlay interface");
    GdkGeometry geometry;
    geometry.min_width = 1;
    geometry.min_height = 1;
    geometry.max_width = -1;
    geometry.max_height = -1;
    gtk_window_set_geometry_hints (GTK_WINDOW (window), window, &geometry, GDK_HINT_MIN_SIZE);

    //window to control the states
    GtkWidget* window_control = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    geometry.min_width = 1;
    geometry.min_height = 1;
    geometry.max_width = -1;
    geometry.max_height = -1;
    gtk_window_set_geometry_hints (GTK_WINDOW (window_control), window_control, &geometry, GDK_HINT_MIN_SIZE);
    gtk_window_set_resizable (GTK_WINDOW (window_control), FALSE);
    gtk_window_move (GTK_WINDOW (window_control), 10, 10);
    GtkWidget* table = gtk_table_new (2, 1, TRUE);
    gtk_container_add (GTK_CONTAINER (window_control), table);

    //control state null
    GtkWidget* button_state_null = gtk_button_new_with_label ("GST_STATE_NULL");
    g_signal_connect (G_OBJECT (button_state_null), "clicked",
        G_CALLBACK (button_state_null_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), button_state_null, 0, 1, 0, 1);
    gtk_widget_show (button_state_null);

    //control state ready
    GtkWidget* button_state_ready = gtk_button_new_with_label ("GST_STATE_READY");
    g_signal_connect (G_OBJECT (button_state_ready), "clicked",
        G_CALLBACK (button_state_ready_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), button_state_ready, 0, 1, 1, 2);
    gtk_widget_show (button_state_ready);

    //control state paused
    GtkWidget* button_state_paused = gtk_button_new_with_label ("GST_STATE_PAUSED");
    g_signal_connect (G_OBJECT (button_state_paused), "clicked",
        G_CALLBACK (button_state_paused_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), button_state_paused, 0, 1, 2, 3);
    gtk_widget_show (button_state_paused);

    //control state playing
    GtkWidget* button_state_playing = gtk_button_new_with_label ("GST_STATE_PLAYING");
    g_signal_connect (G_OBJECT (button_state_playing), "clicked",
        G_CALLBACK (button_state_playing_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), button_state_playing, 0, 1, 3, 4);
    gtk_widget_show (button_state_playing);

    //change framerate
    GtkWidget* slider_fps = gtk_vscale_new_with_range (1, 30, 2);
    g_signal_connect (G_OBJECT (slider_fps), "format-value",
        G_CALLBACK (slider_fps_cb), pipeline);
    gtk_table_attach_defaults (GTK_TABLE (table), slider_fps, 1, 2, 0, 4);
    gtk_widget_show (slider_fps);

    gtk_widget_show (table);
    gtk_widget_show (window_control);

    //configure the pipeline
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(destroy_cb), pipeline);

    GstElement* videosrc  = gst_element_factory_make ("videotestsrc", "videotestsrc");
    GstElement* videosink = gst_element_factory_make ("glimagesink", "glimagesink");
    GstElement* upload = gst_element_factory_make ("glupload", "glupload");

    gst_bin_add_many (GST_BIN (pipeline), videosrc, upload, videosink, NULL);

    gboolean link_ok = gst_element_link_many(videosrc, upload, videosink, NULL) ;
    if(!link_ok)
    {
        g_warning("Failed to link an element!\n") ;
        return -1;
    }

    //area where the video is drawn
    GtkWidget* area = gtk_drawing_area_new();
    gtk_container_add (GTK_CONTAINER (window), area);
    g_signal_connect (area, "realize",
        G_CALLBACK (area_realize_cb), pipeline);

    //set window id on this event
    GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window, area, NULL);
    gst_bus_add_signal_watch (bus);
    g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), pipeline);
    g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), pipeline);
    g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), pipeline);
    gst_object_unref (bus);

    //needed when being in GST_STATE_READY, GST_STATE_PAUSED
    //or resizing/obscuring the window
    g_signal_connect(area, "expose-event", G_CALLBACK(expose_cb), videosink);

    gtk_widget_show_all (window);

    gtk_main();

    return 0;
}

