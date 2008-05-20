#include "gstthread.h"

GstThread::GstThread(const WId winId, QObject *parent): 
    m_pipeline(winId),
    QThread(parent)
{
}

GstThread::~GstThread()
{
}

void GstThread::expose()
{
    m_pipeline.expose();
}

void GstThread::run()
{
    m_pipeline.start();
}