#ifndef QGLRENDERER_H
#define QGLRENDERER_H

#include <QGLWidget>

class QGLRenderer : public QGLWidget
{
    Q_OBJECT

public:
    QGLRenderer(QWidget *parent = 0);
    ~QGLRenderer();
    void paintEvent(QPaintEvent* event);

signals:
    void exposeRequested();
};

#endif // QGLRENDERER_H
