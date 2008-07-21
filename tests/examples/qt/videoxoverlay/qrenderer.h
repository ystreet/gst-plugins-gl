#ifndef QRENDERER_H
#define QRENDERER_H

#include <QtGui/QWidget>
#include "gstthread.h"

class QRenderer : public QWidget
{
    Q_OBJECT

public:
    QRenderer(const QString videoLocation, QWidget *parent = 0, Qt::WFlags flags = 0);
    ~QRenderer();
    void paintEvent(QPaintEvent* event);
    void closeEvent (QCloseEvent* event);

public slots:
    void resizeRequested(int width, int height);

signals:
    void exposeRequested();
    void closeRequested();

private:
    GstThread m_gt;
};

#endif // QRENDERER_H
