#include "gstthread.h"

GstThread::GstThread(const WId winId, QObject *parent):
    QThread(parent),
    m_pipeline(winId)
    
{
}

GstThread::~GstThread()
{
}

void GstThread::exposeRequested()
{
    m_pipeline.exposeRequested();
}

void GstThread::run()
{
    m_pipeline.start();
}
