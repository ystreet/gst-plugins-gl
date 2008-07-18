#ifndef QGLRENDERER_H
#define QGLRENDERER_H

#include <QGLWidget>
#include "gstthread.h"

class QGLRenderer : public QGLWidget
{
    Q_OBJECT

public:
    QGLRenderer(const QString videoLocation, QWidget *parent = 0);
    ~QGLRenderer();
    void paintEvent(QPaintEvent* event);
    void closeEvent (QCloseEvent* event);

signals:
    void exposeRequested();
    void closeRequested();

private:
    GstThread m_gt;
};

#endif // QGLRENDERER_H
