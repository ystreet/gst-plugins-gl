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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <gst/video/videooverlay.h>
#include <GL/gl.h>
#include "pipeline.h"

Pipeline::Pipeline(const WId id, const QString videoLocation):
    m_winId(id),
    m_videoLocation(videoLocation),
    m_loop(NULL),
    m_bus(NULL),
    m_pipeline(NULL),
    m_glupload(NULL),
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
    m_glupload  = gst_element_factory_make ("glupload", "glupload0");
    m_glimagesink  = gst_element_factory_make ("glimagesink", "sink0");
    
    if (!videosrc || !decodebin || !m_glupload || !m_glimagesink )
    {
        qDebug ("one element could not be found");
        return;
    }

    GstCaps *outcaps = gst_caps_new_simple("video/x-raw",
                                           "width", G_TYPE_INT, 800,
                                           "height", G_TYPE_INT, 600,
                                           NULL) ;

    g_object_set(G_OBJECT(videosrc), "num-buffers", 800, NULL);
    g_object_set(G_OBJECT(videosrc), "location", m_videoLocation.toAscii().data(), NULL);
    g_object_set(G_OBJECT(m_glimagesink), "client-reshape-callback", reshapeCallback, NULL);
    g_object_set(G_OBJECT(m_glimagesink), "client-draw-callback", drawCallback, NULL);

    gst_bin_add_many (GST_BIN (m_pipeline), videosrc, decodebin, m_glupload, m_glimagesink, NULL);

    gboolean link_ok = gst_element_link_filtered(m_glupload, m_glimagesink, outcaps) ;
    gst_caps_unref(outcaps) ;
    if(!link_ok)
    {
        qDebug("Failed to link glupload to glimagesink!\n") ;
        return;
    }

    gst_element_link_pads (videosrc, "src", decodebin, "sink");

    g_signal_connect (decodebin, "new-decoded-pad", G_CALLBACK (cb_new_pad), this);
}

void Pipeline::start()
{
    GstStateChangeReturn ret = gst_element_set_state (m_pipeline, GST_STATE_PLAYING);
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

void Pipeline::show()
{
    emit showRequested();
}

//redraw the current frame in the drawable
void Pipeline::doExpose() const
{
    if (m_pipeline && m_glimagesink)
        gst_video_overlay_expose (GST_VIDEO_OVERLAY (m_glimagesink));
}

//post message to g_main_loop in order to call expose
//in the gt thread
void Pipeline::exposeRequested()
{
    g_idle_add(cb_expose, this);
}


//-----------------------------------------------------------------------
//----------------------------- static members --------------------------
//-----------------------------------------------------------------------

//client reshape callback
void Pipeline::reshapeCallback (uint width, uint height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (gfloat)width/(gfloat)height, 0.1, 100);  
    glMatrixMode(GL_MODELVIEW);	
}

//client draw callback
gboolean Pipeline::drawCallback (uint texture, uint width, uint height)
{
    static GLfloat	xrot = 0;
    static GLfloat	yrot = 0;				
    static GLfloat	zrot = 0;
    static GTimeVal current_time;
    static glong last_sec = current_time.tv_sec;
    static gint nbFrames = 0;  

    g_get_current_time (&current_time);
    nbFrames++ ;
    
    if ((current_time.tv_sec - last_sec) >= 1)
    {
        qDebug ("GRPHIC FPS = %d", nbFrames);
        nbFrames = 0;
        last_sec = current_time.tv_sec;
    }
  
    glEnable(GL_DEPTH_TEST);

    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
    glTranslatef(0.0f,0.0f,-5.0f);

    glRotatef(xrot,1.0f,0.0f,0.0f);
    glRotatef(yrot,0.0f,1.0f,0.0f);
    glRotatef(zrot,0.0f,0.0f,1.0f);

    glBegin(GL_QUADS);
	      // Front Face
	      glTexCoord2f((gfloat)width, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
	      glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
	      glTexCoord2f(0.0f, (gfloat)height); glVertex3f( 1.0f,  1.0f,  1.0f);
	      glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f(-1.0f,  1.0f,  1.0f);
	      // Back Face
	      glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	      glTexCoord2f(0.0f, (gfloat)height); glVertex3f(-1.0f,  1.0f, -1.0f);
	      glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f( 1.0f,  1.0f, -1.0f);
	      glTexCoord2f((gfloat)width, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	      // Top Face
	      glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f(-1.0f,  1.0f, -1.0f);
	      glTexCoord2f((gfloat)width, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
	      glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
	      glTexCoord2f(0.0f, (gfloat)height); glVertex3f( 1.0f,  1.0f, -1.0f);
	      // Bottom Face
	      glTexCoord2f((gfloat)width, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	      glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	      glTexCoord2f(0.0f, (gfloat)height); glVertex3f( 1.0f, -1.0f,  1.0f);
	      glTexCoord2f((gfloat)width,(gfloat)height); glVertex3f(-1.0f, -1.0f,  1.0f);
	      // Right face
	      glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
	      glTexCoord2f(0.0f, (gfloat)height); glVertex3f( 1.0f,  1.0f, -1.0f);
	      glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f( 1.0f,  1.0f,  1.0f);
	      glTexCoord2f((gfloat)width, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
	      // Left Face
	      glTexCoord2f((gfloat)width, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	      glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
	      glTexCoord2f(0.0f, (gfloat)height); glVertex3f(-1.0f,  1.0f,  1.0f);
	      glTexCoord2f((gfloat)width, (gfloat)height); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd(); 

	xrot+=0.03f;
    yrot+=0.02f;
    zrot+=0.04f;

    //return TRUE causes a postRedisplay
    return TRUE;
}

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
        default:
            break;
    }

    return TRUE;
}

void Pipeline::cb_new_pad (GstElement* decodebin, GstPad* pad, gboolean last, Pipeline* p)
{
    GstElement* glupload = p->getVideoSink();
    GstPad* glpad = gst_element_get_pad (glupload, "sink");
    
    //only link once 
    if (GST_PAD_IS_LINKED (glpad)) 
    {
        gst_object_unref (glpad);
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

    p->show();
}

gboolean Pipeline::cb_expose (gpointer data)
{
    ((Pipeline*)data)->doExpose();
    return FALSE;
}

GstBusSyncReply Pipeline::create_window (GstBus* bus, GstMessage* message, const Pipeline* p)
{
    // ignore anything but 'prepare-window-handle' element messages
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;

    if (!gst_is_video_overlay_prepare_window_handle_message (message))
        return GST_BUS_PASS;

    qDebug ("setting window handle");

    //Passing 0 as the window_handle will tell the overlay to stop using that window and create an internal one.
    //In the directdrawsink's gst_video_overlay_set_window_handle implementation, window_handle (parameter 2) is casted to HWND before it used.
    gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (GST_MESSAGE_SRC (message)), (guintptr)p->winId());

    gst_message_unref (message);

    return GST_BUS_DROP;
}
