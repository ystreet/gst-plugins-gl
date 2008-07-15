#include "gstthread.h"

GstThread::GstThread(const WId winId, QObject *parent):
    QThread(parent),
    m_pipeline(winId)
{
}

GstThread::~GstThread()
{
}

void GstThread::expose()
{
    m_pipeline.expose();
}

void GstThread::onMouseMove()
{
    m_pipeline.rotate();
}

void GstThread::run()
{
    m_pipeline.start();
}
