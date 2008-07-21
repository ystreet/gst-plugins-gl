#include "qrenderer.h"

QRenderer::QRenderer(const QString videoLocation, QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags),
      m_gt(winId(), videoLocation)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setVisible(false);
    move(20, 10);
    resize(640, 480);

    QObject::connect(&m_gt, SIGNAL(finished()), this, SLOT(close()));
    QObject::connect(this, SIGNAL(exposeRequested()), &m_gt, SLOT(exposeRequested()));
    QObject::connect(this, SIGNAL(closeRequested()), &m_gt, SLOT(stop()), Qt::DirectConnection);
    QObject::connect(&m_gt, SIGNAL(showRequested()), this, SLOT(show()));
    QObject::connect(this, SIGNAL(mouseMoved()), &m_gt, SLOT(onMouseMove()));
    m_gt.start();
}

QRenderer::~QRenderer()
{
}

void QRenderer::paintEvent(QPaintEvent* event)
{
    emit exposeRequested();
}

void QRenderer::mouseMoveEvent(QMouseEvent* event)
{
    emit mouseMoved();
}

void QRenderer::closeEvent(QCloseEvent* event)
{
    emit closeRequested();
    m_gt.wait();
}
