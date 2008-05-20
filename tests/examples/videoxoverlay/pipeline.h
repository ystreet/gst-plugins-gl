#ifndef PIPELINE_H
#define PIPELINE_H

#include <QtGui>
#include <gst/gst.h>

class Pipeline : QObject
{
    Q_OBJECT

public:
    Pipeline(const WId windId = 0);
    ~Pipeline();
    void start();
    void expose();

private:
    const WId m_winId; 
    GMainLoop* m_loop;
    GstElement* m_pipeline;
    GstElement* m_glimagesink;

    void create();
    WId winId() const { return m_winId; }
    void stop() const;

    static gboolean bus_call (GstBus *bus, GstMessage *msg, const Pipeline* p);
    static void cb_new_pad (GstElement* avidemux, GstPad* pad, GstElement* ffdec_mpeg4);
    static GstBusSyncReply create_window (GstBus* bus, GstMessage* message, const Pipeline* pipeline);

};

#endif
