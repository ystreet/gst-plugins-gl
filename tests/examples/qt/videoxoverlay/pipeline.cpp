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

#include <gst/interfaces/xoverlay.h>
#include "pipeline.h"

Pipeline::Pipeline(const WId id, const QString videoLocation):
    m_winId(id),
    m_videoLocation(videoLocation),
    m_loop(NULL),
    m_bus(NULL),
    m_pipeline(NULL),
    m_glimagesink(NULL)
{
    create();
}

Pipeline::~Pipeline()
{
}

void Pipeline::create()
{
    qDebug("Loading video: %s", m_videoLocation.toAscii().data());

    gst_init (NULL, NULL);

#ifdef WIN32
    m_loop = g_main_loop_new (NULL, FALSE);
#endif
    m_pipeline = gst_pipeline_new ("pipeline");

    m_bus = gst_pipeline_get_bus (GST_PIPELINE (m_pipeline));
    gst_bus_add_watch (m_bus, (GstBusFunc) bus_call, this);
    gst_bus_set_sync_handler (m_bus, (GstBusSyncHandler) create_window, this);
    gst_object_unref (m_bus);

    GstElement* videosrc = gst_element_factory_make ("filesrc", "filesrc0");
    GstElement* decodebin = gst_element_factory_make ("decodebin", "decodebin0");
    m_glimagesink  = gst_element_factory_make ("glimagesink", "sink0");
    
    if (!videosrc || !decodebin || !m_glimagesink )
    {
        qDebug ("one element could not be found");
        return;
    }

    g_object_set(G_OBJECT(videosrc), "location", m_videoLocation.toAscii().data(), NULL);

    gst_bin_add_many (GST_BIN (m_pipeline), videosrc, decodebin, m_glimagesink, NULL);

    gst_element_link_pads (videosrc, "src", decodebin, "sink");

    g_signal_connect (decodebin, "new-decoded-pad", G_CALLBACK (cb_new_pad), m_glimagesink);

    GstPad* pad = gst_element_get_static_pad (m_glimagesink, "sink");
    g_signal_connect(pad, "notify::caps", G_CALLBACK(cb_video_size), this);
    gst_object_unref (pad);
}

void Pipeline::seek()
{
    if (gst_element_seek(
           m_pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
           GST_SEEK_TYPE_SET, 2 * GST_SECOND,
           GST_SEEK_TYPE_SET, 20 * GST_SECOND)
       )
        qDebug ("Success to seek");
    else
        qDebug ("Failed to seek");
}

void Pipeline::setState(GstState state)
{
    GstStateChangeReturn ret = gst_element_set_state (m_pipeline, state);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        qDebug ("Failed to start up pipeline!");

        /* check if there is an error message with details on the bus */
        GstMessage* msg = gst_bus_poll (m_bus, GST_MESSAGE_ERROR, 0);
        if (msg)
        {
            GError *err = NULL;
            gst_message_parse_error (msg, &err, NULL);
            qDebug ("ERROR: %s", err->message);
            g_error_free (err);
            gst_message_unref (msg);
        }
        return;
    }
}

void Pipeline::start()
{
    setState(GST_STATE_PLAYING);

#ifdef WIN32
    g_main_loop_run(m_loop);    
#endif
}

//we don't want a thread safe stop in this example
void Pipeline::stop()
{
#ifdef WIN32
    g_main_loop_quit(m_loop);
#else
    emit stopRequested();
#endif
}


void Pipeline::unconfigure() const
{
    gst_element_set_state (m_pipeline, GST_STATE_NULL);
    gst_object_unref (m_pipeline);
}

//redraw the current frame in the drawable
void Pipeline::doExpose() const
{
    if (m_pipeline && m_glimagesink)
        gst_x_overlay_expose (GST_X_OVERLAY (m_glimagesink));
}

//post message to g_main_loop in order to call expose
//in the gt thread
void Pipeline::exposeRequested()
{
    g_idle_add(cb_expose, this);
}

//post message to the Qt main loop in order to resize the renderer
void Pipeline::resize(int width, int height)
{
    emit resizeRequested(width, height);
}


//-----------------------------------------------------------------------
//----------------------------- static members --------------------------
//-----------------------------------------------------------------------


gboolean Pipeline::bus_call (GstBus *bus, GstMessage *msg, Pipeline* p)
{
  
    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_EOS:
            qDebug ("End-of-stream");
            p->stop();
            break;
        case GST_MESSAGE_ERROR:
        {
            gchar *debug = NULL;
            GError *err = NULL;
            gst_message_parse_error (msg, &err, &debug);
            qDebug ("Error: %s", err->message);
            g_error_free (err);
            if (debug)
            {
                qDebug ("Debug deails: %s", debug);
                g_free (debug);
            }
            p->stop();
            break;
        }
        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState oldState = GST_STATE_NULL; 
            GstState currentState = GST_STATE_NULL;
            GstState pendingState = GST_STATE_NULL; 
            gst_message_parse_state_changed (msg, &oldState, &currentState, &pendingState);
            if (oldState == GST_STATE_READY && currentState == GST_STATE_PAUSED && pendingState == GST_STATE_PLAYING)
                p->seek();
            break;
        }
        default:
            break;
    }

    return TRUE;
}

void Pipeline::cb_new_pad (GstElement* decodebin, GstPad* pad, gboolean last, GstElement* glimagesink)
{
    GstPad* glpad = gst_element_get_pad (glimagesink, "sink");
    
    //only link once 
    if (GST_PAD_IS_LINKED (glpad)) 
    {
        g_object_unref (glpad);
        return;
    }

    GstCaps* caps = gst_pad_get_caps (pad);
    GstStructure* str = gst_caps_get_structure (caps, 0);
    if (!g_strrstr (gst_structure_get_name (str), "video")) 
    {
        gst_caps_unref (caps);
        gst_object_unref (glpad);
        return;
    }
    gst_caps_unref (caps);

    GstPadLinkReturn ret = gst_pad_link (pad, glpad);
    if (ret != GST_PAD_LINK_OK) 
        g_warning ("Failed to link with decodebin!\n");
}

void Pipeline::cb_video_size (GstPad* pad, GParamSpec* pspec, Pipeline* p)
{
    GstCaps* caps = gst_pad_get_negotiated_caps(pad);
    if (caps)
    {
        qDebug ("negotiated caps : %s", gst_caps_to_string(caps)) ;
        const GstStructure* str = gst_caps_get_structure (caps, 0);
        gint width = 0;
        gint height = 0;
        if (gst_structure_get_int (str, "width", &width)  &&
            gst_structure_get_int (str, "height", &height) )
        p->resize(width, height);
        gst_caps_unref(caps) ;
    }
}

gboolean Pipeline::cb_expose (gpointer data)
{
    ((Pipeline*)data)->doExpose();
    return FALSE;
}


GstBusSyncReply Pipeline::create_window (GstBus* bus, GstMessage* message, const Pipeline* p)
{
    // ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;

    if (!gst_structure_has_name (message->structure, "prepare-xwindow-id"))
        return GST_BUS_PASS;

    qDebug ("setting xwindow id");

    //Passing 0 as the xwindow_id will tell the overlay to stop using that window and create an internal one.
    //In the directdrawsink's gst_x_overlay_set_xwindow_id implementation, xwindow_id (parameter 2) is casted to HWND before it used.
    gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (GST_MESSAGE_SRC (message)), (ulong)p->winId());

    gst_message_unref (message);

    return GST_BUS_DROP;
}
