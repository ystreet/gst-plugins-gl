#include <gst/interfaces/xoverlay.h>
#include <GL/glew.h>
#include "pipeline.h"

Pipeline::Pipeline(const WId id):
    m_winId(id),
    m_loop(NULL),
    m_pipeline(NULL),
    m_glimagesink(NULL)
{
    create();
}

Pipeline::~Pipeline()
{
}

void Pipeline::start()
{
    g_main_loop_run (m_loop);

    gst_element_set_state (m_pipeline, GST_STATE_NULL);
    gst_object_unref (m_pipeline);
    m_pipeline = NULL;
}

void Pipeline::create()
{
    gst_init (NULL, NULL);

    m_loop = g_main_loop_new (NULL, FALSE);
    m_pipeline = gst_pipeline_new ("pipeline");

    GstBus* bus = gst_pipeline_get_bus (GST_PIPELINE (m_pipeline));
    gst_bus_add_watch (bus, (GstBusFunc) bus_call, this);
    gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window, this);
    gst_object_unref (bus);

    GstElement* videosrc = gst_element_factory_make ("filesrc", "filesrc0");
    GstElement* avidemux = gst_element_factory_make ("avidemux", "avidemux0");
    GstElement* ffdec_mpeg4 = gst_element_factory_make ("ffdec_mpeg4", "ffdec_mpeg40");
	GstElement* queue = gst_element_factory_make ("queue", "queue0");
    GstElement* glupload  = gst_element_factory_make ("glupload", "glupload0");
    GstElement* glfilterapp  = gst_element_factory_make ("glfilterapp", "glfilterapp0");
    m_glimagesink  = gst_element_factory_make ("glimagesink", "glimagesink0");


    if (!videosrc || !avidemux || !ffdec_mpeg4 || !queue ||
        !glupload || !glfilterapp || !m_glimagesink)
    {
        qDebug ("one element could not be found \n");
    }

    GstCaps *outcaps = gst_caps_new_simple("video/x-raw-gl",
                                           "width", G_TYPE_INT, 800,
                                           "height", G_TYPE_INT, 600,
                                           NULL) ;

    //configure elements
    g_object_set(G_OBJECT(videosrc), "location", "../doublecube/data/lost.avi", NULL);
    g_object_set(G_OBJECT(glfilterapp), "client-reshape-callback", reshapeCallback, NULL);
    g_object_set(G_OBJECT(glfilterapp), "client-draw-callback", drawCallback, NULL);
    
    //add elements
    gst_bin_add_many (GST_BIN (m_pipeline), videosrc, avidemux, ffdec_mpeg4, queue, 
                                          glupload, glfilterapp, m_glimagesink, NULL);

    //link elements
	gst_element_link_pads (videosrc, "src", avidemux, "sink");

    g_signal_connect (avidemux, "pad-added", G_CALLBACK (cb_new_pad), ffdec_mpeg4);

    if (!gst_element_link_many(ffdec_mpeg4, queue, glupload, glfilterapp, NULL)) 
    {
        qDebug ("Failed to link one or more elements!\n");
    }
    gboolean link_ok = gst_element_link_filtered(glfilterapp, m_glimagesink, outcaps) ;
    gst_caps_unref(outcaps) ;
    if(!link_ok)
    {
        qDebug("Failed to link glfilterapp to glimagesink!\n") ;
        return;
    }
    
    //run
    GstStateChangeReturn ret = gst_element_set_state (m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) 
    {
        g_print ("Failed to start up pipeline!");

        //check if there is an error message with details on the bus
        GstMessage* msg = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
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

//we don't want a thread safe stop in this example
void Pipeline::stop() const
{
    g_main_loop_quit (m_loop);
}

//redraw the current frame in the drawable
void Pipeline::expose()
{
    if (m_pipeline && m_glimagesink)
        gst_x_overlay_expose (GST_X_OVERLAY (m_glimagesink));
}

//rotate the cube
void Pipeline::rotate()
{
    m_xrot += 3.0f;
    m_yrot += 2.0f;
    m_zrot += 4.0f; 
}


//-----------------------------------------------------------------------
//----------------------------- static members --------------------------
//-----------------------------------------------------------------------

float Pipeline::m_xrot = 0;
float Pipeline::m_yrot = 0;				
float Pipeline::m_zrot = 0;

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

    glEnable (GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
    glTranslatef(0.0f,0.0f,-5.0f);

    glRotatef(m_xrot,1.0f,0.0f,0.0f);
    glRotatef(m_yrot,0.0f,1.0f,0.0f);
    glRotatef(m_zrot,0.0f,0.0f,1.0f);

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

    //return TRUE causes a postRedisplay
    return FALSE;
}

gboolean Pipeline::bus_call (GstBus *bus, GstMessage *msg, const Pipeline* p)
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

void Pipeline::cb_new_pad (GstElement* avidemux, GstPad* pad, GstElement* ffdec_mpeg4)
{
  gst_element_link_pads (avidemux, "video_00", ffdec_mpeg4, "sink"); 
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
