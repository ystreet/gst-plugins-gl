#ifndef PIPELINE_H
#define PIPELINE_H

#include <QtGui>
#include <gst/gst.h>
//#include <QtCore/private/qeventdispatcher_glib_p.h>

class Pipeline : public QObject
{
    Q_OBJECT

public:
    Pipeline(const WId windId = 0);
    ~Pipeline();
    void start();
    void exposeRequested();
    void resize(int width, int height);
    void stop();
    void unconfigure() const;

signals:
    void resizeRequested(int width, int height);
    void stopRequested();

private:
    const WId m_winId;
    GMainLoop* m_loop;
    GstBus* m_bus;
    GstElement* m_pipeline;
    GstElement* m_glimagesink;

    void create();
    WId winId() const { return m_winId; }
    void doExpose () const;

    static gboolean bus_call (GstBus *bus, GstMessage *msg, Pipeline* p);
    static void cb_new_pad (GstElement* avidemux, GstPad* pad, GstElement* ffdec_mpeg4);
    static void cb_video_size (GstPad* pad, GParamSpec* pspec, Pipeline* p);
    static gboolean cb_expose (gpointer data);
    static GstBusSyncReply create_window (GstBus* bus, GstMessage* message, const Pipeline* pipeline);
};

#endif
