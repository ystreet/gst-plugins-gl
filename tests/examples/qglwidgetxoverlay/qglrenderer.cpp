#include "qglrenderer.h"

QGLRenderer::QGLRenderer(QWidget *parent)
    : QGLWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
}

QGLRenderer::~QGLRenderer()
{
}

void QGLRenderer::paintEvent(QPaintEvent* event)
{
    emit exposeRequested();
}

