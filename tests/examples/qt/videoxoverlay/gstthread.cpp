#include "gstthread.h"

GstThread::GstThread (const WId winId, const QString videoLocation, QObject * parent):
QThread (parent),
m_winId (winId), m_videoLocation (videoLocation)
{
}

GstThread::~GstThread ()
{
}

void
GstThread::exposeRequested ()
{
  m_pipeline->exposeRequested ();
}

void
GstThread::resize (int width, int height)
{
  emit resizeRequested (width, height);
}

void
GstThread::stop ()
{
  m_pipeline->stop ();
}

void
GstThread::run ()
{
  m_pipeline = new Pipeline (m_winId, m_videoLocation);
  connect (m_pipeline, SIGNAL (resizeRequested (int, int)), this,
      SLOT (resize (int, int)));
  m_pipeline->start ();         //it runs the gmainloop on win32


#ifndef WIN32
  //works like the gmainloop on linux (GstEvent are handled)
  connect (m_pipeline, SIGNAL (stopRequested ()), this, SLOT (quit ()));
  exec ();
#endif


  m_pipeline->unconfigure ();
}
