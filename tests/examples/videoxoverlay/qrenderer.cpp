#include "qrenderer.h"

QRenderer::QRenderer(QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    setAttribute(Qt::WA_NoSystemBackground);
}

QRenderer::~QRenderer()
{

}

void QRenderer::paintEvent(QPaintEvent* event)
{
    emit exposeRequested();
}
