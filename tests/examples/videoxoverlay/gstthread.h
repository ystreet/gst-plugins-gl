#ifndef GSTTHREAD_H
#define GSTTHREAD_H

#include <QtGui>
#include <QtCore/QThread>

#include "pipeline.h"

class GstThread : public QThread
{
    Q_OBJECT

public:
    GstThread(const WId winId = 0, QObject *parent = 0);
    ~GstThread();

public slots:
    void exposeRequested();
    void resize(int width, int height);
    void stop();

signals:
    void resizeRequested(int width, int height);
    
protected:
    void run();

private:
    const WId m_winId;
    Pipeline* m_pipeline;

};

#endif
