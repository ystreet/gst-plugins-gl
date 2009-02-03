#include "qrenderer.h"

QRenderer::QRenderer (const QString videoLocation, QWidget * parent,
    Qt::WFlags flags)
:  QWidget (parent, flags), m_gt (winId (), videoLocation)
{
  setAttribute (Qt::WA_NoSystemBackground);
  setVisible (false);
  move (20, 10);

  QObject::connect (&m_gt, SIGNAL (resizeRequested (int, int)), this,
      SLOT (resizeRequested (int, int)));
  QObject::connect (&m_gt, SIGNAL (finished ()), this, SLOT (close ()));
  QObject::connect (this, SIGNAL (exposeRequested ()), &m_gt,
      SLOT (exposeRequested ()));
  QObject::connect (this, SIGNAL (closeRequested ()), &m_gt, SLOT (stop ()),
      Qt::DirectConnection);
  m_gt.start ();
}

QRenderer::~QRenderer ()
{
}

void
QRenderer::paintEvent (QPaintEvent * event)
{
  emit exposeRequested ();
}

void
QRenderer::closeEvent (QCloseEvent * event)
{
  emit closeRequested ();
  m_gt.wait ();
}

void
QRenderer::resizeRequested (int width, int height)
{
  resize (width, height);
  setVisible (true);
}
