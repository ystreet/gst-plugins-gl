#include <gst/interfaces/xoverlay.h>
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
    m_glimagesink  = gst_element_factory_make ("glimagesink", "sink0");
    if (!videosrc || !avidemux || !ffdec_mpeg4 || !m_glimagesink ) 
    {
        qDebug ("one element could not be found");
        return;
    }

    g_object_set(G_OBJECT(videosrc), "location", "../doublecube/data/lost.avi", NULL);

    gst_bin_add_many (GST_BIN (m_pipeline), videosrc, avidemux, ffdec_mpeg4, m_glimagesink, NULL);
    if (!gst_element_link(ffdec_mpeg4, m_glimagesink)) 
    {
        qDebug ("Failed to link ffdec_mpeg4 to glimagesink!");
        return;
    }

    gst_element_link_pads (videosrc, "src", avidemux, "sink");

    g_signal_connect (avidemux, "pad-added", G_CALLBACK (cb_new_pad), ffdec_mpeg4);

    GstStateChangeReturn ret = gst_element_set_state (m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) 
    {
        qDebug ("Failed to start up pipeline!");

        /* check if there is an error message with details on the bus */
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


//-----------------------------------------------------------------------
//----------------------------- static members --------------------------
//-----------------------------------------------------------------------


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
