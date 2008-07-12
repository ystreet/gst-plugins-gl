#include "qrenderer.h"

QRenderer::QRenderer(QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setVisible(false);
    move(20, 10);
}

QRenderer::~QRenderer()
{

}

void QRenderer::paintEvent(QPaintEvent* event)
{
    emit exposeRequested();
}

void QRenderer::resizeRequested(int width, int height)
{ 
    resize(width, height);
}
