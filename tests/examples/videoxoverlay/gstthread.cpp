#include "gstthread.h"

GstThread::GstThread(const WId winId, QObject *parent):
    QThread(parent),
    m_pipeline(winId)   
{
    connect(&m_pipeline, SIGNAL(resizeRequested(int, int)), this, SLOT(resize(int, int)));
}

GstThread::~GstThread()
{
}

void GstThread::exposeRequested()
{
    m_pipeline.exposeRequested();
}

void GstThread::resize(int width, int height)
{
    emit resizeRequested(width, height);
}

void GstThread::run()
{
    m_pipeline.start();
}
