#include <GL/glew.h>
#include <gst/gst.h>

#include <iostream>
#include <sstream>
#include <string>

static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop*)data;

    switch (GST_MESSAGE_TYPE (msg)) 
    {
        case GST_MESSAGE_EOS:
              g_print ("End-of-stream\n");
              g_main_loop_quit (loop);
              break;
        case GST_MESSAGE_ERROR: 
          {
              gchar *debug = NULL;
              GError *err = NULL;

              gst_message_parse_error (msg, &err, &debug);

              g_print ("Error: %s\n", err->message);
              g_error_free (err);

              if (debug) 
              {
                  g_print ("Debug deails: %s\n", debug);
                  g_free (debug);
              }

              g_main_loop_quit (loop);
              break;
          } 
        default:
          break;
    }

    return TRUE;
}

GstElement *textoverlay ;

//display video framerate
static void identityCallback (GstElement *src, GstBuffer  *buffer, GstPad *pad, gpointer user_data)
{
  static GstClockTime last_timestamp = 0;
  static gint nbFrames = 0 ;

  //display estimated video FPS 
  nbFrames++ ;
  if (GST_BUFFER_TIMESTAMP(buffer) - last_timestamp >= 1000000000)
  {
    std::ostringstream oss ;
    oss << "video framerate = " << nbFrames ;
    std::string s(oss.str()) ;
    g_object_set(G_OBJECT(textoverlay), "text", s.c_str(), NULL);
    last_timestamp = GST_BUFFER_TIMESTAMP(buffer) ;
    nbFrames = 0 ; 
  }
}


//client reshape callback
void reshapeCallback (GLuint width, GLuint height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (gfloat)width/(gfloat)height, 0.1, 100);  
    glMatrixMode(GL_MODELVIEW);	
}


//client draw callback
gboolean drawCallback (GLuint texture, GLuint width, GLuint height)
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
        std::cout << "GRPHIC FPS = " << nbFrames << std::endl;
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

    xrot+=0.3f;
    yrot+=0.2f;
    zrot+=0.4f;

    //return TRUE causes a postRedisplay
    return FALSE;
}


static void
cb_new_pad (GstElement *avidemux, GstPad *pad, gpointer data)
{
  GstElement *ffdec_mpeg4 = (GstElement*)data;
  
  gst_element_link_pads (avidemux, "video_00", ffdec_mpeg4, "sink"); 
}


gint main (gint argc, gchar *argv[])
{
    GstStateChangeReturn ret;
    GstElement *pipeline, *videosrc, *avidemux, *ffdec_mpeg4, *identity, *colorspace, *tee;
    GstElement *queue0, *glgraphicmaker0, *glfilterapp0, *glimagesink0; 
    GstElement *queue1, *glimagesink1;

    GMainLoop *loop;
    GstBus *bus;

    /* initialization */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    /* create elements */
    pipeline = gst_pipeline_new ("pipeline");

    /* watch for messages on the pipeline's bus (note that this will only
     * work like this when a GLib main loop is running) */
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    /* create elements */
    videosrc = gst_element_factory_make ("filesrc", "filesrc0");
    avidemux = gst_element_factory_make ("avidemux", "avidemux0");
    ffdec_mpeg4 = gst_element_factory_make ("ffdec_mpeg4", "ffdec_mpeg40");
    identity  = gst_element_factory_make ("identity", "identity0");
    textoverlay = gst_element_factory_make ("textoverlay", "textoverlay0");
    colorspace = gst_element_factory_make ("ffmpegcolorspace", "ffmpegcolorspace0");
    tee = gst_element_factory_make ("tee", "tee0");

    queue0 = gst_element_factory_make ("queue", "queue0");
    glgraphicmaker0  = gst_element_factory_make ("glgraphicmaker", "glgraphicmaker0");
    glfilterapp0  = gst_element_factory_make ("glfilterapp", "glfilterapp0");
    glimagesink0  = gst_element_factory_make ("glimagesink", "glimagesink0");

    queue1 = gst_element_factory_make ("queue", "queue1");
    glimagesink1  = gst_element_factory_make ("glimagesink", "glimagesink1");


    if (!videosrc || !avidemux || !ffdec_mpeg4 || !identity || !textoverlay || !colorspace || !tee ||
        !queue0 || !glgraphicmaker0 || !glfilterapp0 || !glimagesink0 ||
        !queue1 || !glimagesink1) 
    {
        g_print ("one element could not be found \n");
        return -1;
    }

    GstCaps *outcaps = gst_caps_new_simple("video/x-raw-gl",
                                           "width", G_TYPE_INT, 300,
                                           "height", G_TYPE_INT, 200,
                                           NULL) ;

    /* configure elements */
    g_object_set(G_OBJECT(videosrc), "location", "data/lost.avi", NULL);
    g_signal_connect(identity, "handoff", G_CALLBACK(identityCallback), NULL) ;
    g_object_set(G_OBJECT(textoverlay), "font_desc", "Ahafoni CLM Bold 30", NULL);
    g_object_set(G_OBJECT(glfilterapp0), "client-reshape-callback", reshapeCallback, NULL);
    g_object_set(G_OBJECT(glfilterapp0), "client-draw-callback", drawCallback, NULL);


	GstCaps *caps = gst_caps_new_simple("video/x-raw-rgb", NULL) ;
    
    /* add elements */
    gst_bin_add_many (GST_BIN (pipeline), videosrc, avidemux, ffdec_mpeg4, identity, textoverlay, colorspace, tee, 
                                          queue0, glgraphicmaker0, glfilterapp0, glimagesink0, 
                                          queue1, glimagesink1, NULL);
    

    gst_element_link_pads (videosrc, "src", avidemux, "sink");

    g_signal_connect (avidemux, "pad-added", G_CALLBACK (cb_new_pad), ffdec_mpeg4);

    if (!gst_element_link_many(ffdec_mpeg4, identity, textoverlay, colorspace, NULL)) 
    {
        g_print ("Failed to link one or more elements!\n");
        return -1;
    }
	gboolean link_ok = gst_element_link_filtered(colorspace, tee, caps) ;
    gst_caps_unref(caps) ;
    if(!link_ok)
    {
        g_warning("Failed to link colorspace to tee!\n") ;
        return -1 ;
    }
	if (!gst_element_link_many(tee, queue0, glgraphicmaker0, glfilterapp0, NULL)) 
    {
        g_print ("Failed to link one or more elements!\n");
        return -1;
    }
    link_ok = gst_element_link_filtered(glfilterapp0, glimagesink0, outcaps) ;
    gst_caps_unref(outcaps) ;
    if(!link_ok)
    {
        g_warning("Failed to link glfilterapp to glimagesink!\n") ;
        return -1 ;
    }
    if (!gst_element_link_many(tee, queue1, glimagesink1, NULL)) 
    {
        g_print ("Failed to link one or more elements!\n");
        return -1;
    }
    
    /* run */
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) 
    {
        g_print ("Failed to start up pipeline!\n");

        /* check if there is an error message with details on the bus */
        GstMessage* msg = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
        if (msg) 
        {
          GError *err = NULL;

          gst_message_parse_error (msg, &err, NULL);
          g_print ("ERROR: %s\n", err->message);
          g_error_free (err);
          gst_message_unref (msg);
        }
        return -1;
    }

    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    return 0;

}
    
