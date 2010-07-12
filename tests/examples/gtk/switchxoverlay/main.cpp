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
    // ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;

    if (!gst_structure_has_name (message->structure, "prepare-xwindow-id"))
        return GST_BUS_PASS;

    g_print ("setting xwindow id\n");

    gst_x_overlay_set_gtk_window (GST_X_OVERLAY (GST_MESSAGE_SRC (message)), widget);

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


static gboolean expose_cb(GtkWidget* widget, GdkEventExpose* event, GstElement* videosink)
{
    gst_x_overlay_expose (GST_X_OVERLAY (videosink));
    return FALSE;
}


static void area_realize_cb(GtkWidget* widget, GstElement* pipeline)
{
#if GTK_CHECK_VERSION(2,18,0)
    if (!gdk_window_ensure_native (widget->window))
        g_error ("Failed to create native window!");
#endif

    //avoid flickering when resizing or obscuring the main window
    gdk_window_set_back_pixmap(widget->window, NULL, FALSE);
    gtk_widget_set_app_paintable(widget,TRUE);

    gst_element_set_state (pipeline, GST_STATE_PLAYING);
}


static gboolean on_click_drawing_area(GtkWidget* widget, GdkEventButton* event, GstElement* videosink)
{
    g_print ("switch the drawing area\n");
    gst_x_overlay_set_gtk_window (GST_X_OVERLAY (videosink), widget);
    gst_x_overlay_expose (GST_X_OVERLAY (videosink));
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


gint main (gint argc, gchar *argv[])
{
    gtk_init (&argc, &argv);
    gst_init (&argc, &argv);

    GstElement* pipeline = gst_pipeline_new ("pipeline");

    //window that contains several ares where the video is drawn
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (window, 640, 240);
    gtk_window_move (GTK_WINDOW (window), 300, 10);
    gtk_window_set_title (GTK_WINDOW (window), "click on left, right or outside the main window to switch the drawing area");
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
    GtkWidget* table = gtk_table_new (4, 1, TRUE);
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

    gtk_widget_show (table);
    gtk_widget_show (window_control);

    //configure the pipeline
    g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(destroy_cb), pipeline);

    GstElement* videosrc  = gst_element_factory_make ("videotestsrc", "videotestsrc");
    GstElement* videosink = gst_element_factory_make ("glimagesink", "glimagesink");

    GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv",
                                        "width", G_TYPE_INT, 320,
                                        "height", G_TYPE_INT, 240,
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

    //areas where the video is drawn
    GtkWidget* table_areas = gtk_table_new (1, 2, TRUE);
    gtk_container_add (GTK_CONTAINER (window), table_areas);
    GtkWidget* area_top_left = gtk_drawing_area_new();
    gtk_widget_set_events(area_top_left, GDK_BUTTON_PRESS_MASK);
    gtk_table_attach_defaults (GTK_TABLE (table_areas), area_top_left, 0, 1, 0, 1);
    GtkWidget* area_top_right = gtk_drawing_area_new();
    gtk_widget_set_events(area_top_right, GDK_BUTTON_PRESS_MASK);
    gtk_table_attach_defaults (GTK_TABLE (table_areas), area_top_right, 1, 2, 0, 1);

    //set window id on this event
    GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window, area_top_right);
    gst_bus_add_signal_watch (bus);
    g_signal_connect(bus, "message::error", G_CALLBACK(end_stream_cb), pipeline);
    g_signal_connect(bus, "message::warning", G_CALLBACK(end_stream_cb), pipeline);
    g_signal_connect(bus, "message::eos", G_CALLBACK(end_stream_cb), pipeline);
    gst_object_unref (bus);

    //needed when being in GST_STATE_READY, GST_STATE_PAUSED
    //or resizing/obscuring the window
    g_signal_connect(area_top_left, "expose-event", G_CALLBACK(expose_cb), videosink);
    g_signal_connect(area_top_right, "expose-event", G_CALLBACK(expose_cb), videosink);

    //switch the drawing area
    g_signal_connect(area_top_left, "button-press-event", G_CALLBACK(on_click_drawing_area), videosink);
    g_signal_connect(area_top_right, "button-press-event", G_CALLBACK(on_click_drawing_area), videosink);

    g_signal_connect (area_top_left, "realize", G_CALLBACK (area_realize_cb), pipeline);
    g_signal_connect (area_top_right, "realize", G_CALLBACK (area_realize_cb), pipeline);

    gtk_widget_show_all (window);

    gtk_main();

    return 0;
}

