#ifndef PIPELINE_H
#define PIPELINE_H

#include <QtGui>
#include <gst/gst.h>

class Pipeline : public QObject
{
    Q_OBJECT

public:
    Pipeline(const WId windId = 0);
    ~Pipeline();
    void start();
    void expose();
    void rotate();

private:
    const WId m_winId; 
    GMainLoop* m_loop;
    GstElement* m_pipeline;
    GstElement* m_glimagesink;
    static float m_xrot;
    static float m_yrot;				
    static float m_zrot;

    void create();
    WId winId() const { return m_winId; }
    void stop() const;

    static void reshapeCallback (uint width, uint height);
    static gboolean drawCallback (uint texture, uint width, uint height);
    static gboolean bus_call (GstBus*, GstMessage*, const Pipeline*);
    static void cb_new_pad (GstElement* avidemux, GstPad* pad, GstElement* ffdec_mpeg4);
    static GstBusSyncReply create_window (GstBus*, GstMessage*, const Pipeline*);

};

#endif
