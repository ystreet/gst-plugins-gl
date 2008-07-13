#include "gstthread.h"

GstThread::GstThread(const WId winId, QObject *parent):
    QThread(parent),
    m_winId(winId)
{
}

GstThread::~GstThread()
{
}

void GstThread::exposeRequested()
{
    m_pipeline->exposeRequested();
}

void GstThread::resize(int width, int height)
{
    emit resizeRequested(width, height);
}

void GstThread::stop()
{
    m_pipeline->stop();
}

void GstThread::run()
{
    m_pipeline = new Pipeline(m_winId);
    connect(m_pipeline, SIGNAL(resizeRequested(int, int)), this, SLOT(resize(int, int)));
    m_pipeline->start();

    //works like the gmainloop on linux (GstEvent are handled)
    //exec();

    m_pipeline->unconfigure();
}
