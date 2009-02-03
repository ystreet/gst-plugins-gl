#include "qglrenderer.h"

QGLRenderer::QGLRenderer (const QString videoLocation, QWidget * parent)
    :
QGLWidget (parent),
m_gt (winId (), videoLocation)
{
  setAttribute (Qt::WA_NoSystemBackground);
  setVisible (false);
  move (20, 10);
  resize (640, 480);

  QObject::connect (&m_gt, SIGNAL (finished ()), this, SLOT (close ()));
  QObject::connect (this, SIGNAL (exposeRequested ()), &m_gt,
      SLOT (exposeRequested ()));
  QObject::connect (this, SIGNAL (closeRequested ()), &m_gt, SLOT (stop ()),
      Qt::DirectConnection);
  QObject::connect (&m_gt, SIGNAL (showRequested ()), this, SLOT (show ()));
  m_gt.start ();
}

QGLRenderer::~QGLRenderer ()
{
}

void
QGLRenderer::paintEvent (QPaintEvent * event)
{
  emit exposeRequested ();
}

void
QGLRenderer::closeEvent (QCloseEvent * event)
{
  emit closeRequested ();
  m_gt.wait ();
}
