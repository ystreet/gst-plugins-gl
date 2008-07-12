#ifndef PIPELINE_H
#define PIPELINE_H

#include <QtGui>
#include <gst/gst.h>
//#include <qeventdispatcher_glib_p.h>

class Pipeline : public QObject
{
    Q_OBJECT

public:
    Pipeline(const WId windId = 0);
    ~Pipeline();
    void start();
    void exposeRequested();
    void resize(int width, int height);

signals:
    void resizeRequested(int width, int height);

private:
    const WId m_winId; 
    GMainLoop* m_loop;
    GstBus* m_bus;
    GstElement* m_pipeline;
    GstElement* m_glimagesink;

    void create();
    WId winId() const { return m_winId; }
    void stop() const;
    void doExpose () const;

    static gboolean bus_call (GstBus *bus, GstMessage *msg, const Pipeline* p);
    static void cb_new_pad (GstElement* avidemux, GstPad* pad, GstElement* ffdec_mpeg4);
    static void cb_video_size (GstPad* pad, GParamSpec* pspec, Pipeline* p);
    static GstBusSyncReply create_window (GstBus* bus, GstMessage* message, const Pipeline* pipeline);

};

#endif
