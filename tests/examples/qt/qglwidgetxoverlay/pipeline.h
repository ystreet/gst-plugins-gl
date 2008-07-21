#ifndef PIPELINE_H
#define PIPELINE_H

#include <QtGui>
#include <gst/gst.h>
//#include <QtCore/private/qeventdispatcher_glib_p.h>

class Pipeline : public QObject
{
    Q_OBJECT

public:
    Pipeline(const WId windId, const QString videoLocation);
    ~Pipeline();
    void start();
    void exposeRequested();
    void stop();
    void unconfigure() const;
    void show();
    GstElement* getVideoSink() { return m_glupload; } ;

signals:
    void showRequested();
    void stopRequested();

private:
    const WId m_winId;
    const QString m_videoLocation;
    GMainLoop* m_loop;
    GstBus* m_bus;
    GstElement* m_pipeline;
    GstElement* m_glupload;
    GstElement* m_glimagesink;
    static float m_xrot;
    static float m_yrot;
    static float m_zrot;

    void create();
    WId winId() const { return m_winId; }
    void doExpose() const;

    static void reshapeCallback (uint width, uint height);
    static gboolean drawCallback (uint texture, uint width, uint height);
    static gboolean bus_call (GstBus *bus, GstMessage *msg, Pipeline* p);
    static void cb_new_pad (GstElement* decodebin, GstPad* pad, gboolean last, Pipeline* p);
    static gboolean cb_expose (gpointer data);
    static GstBusSyncReply create_window (GstBus* bus, GstMessage* message, const Pipeline* pipeline);
};

#endif
