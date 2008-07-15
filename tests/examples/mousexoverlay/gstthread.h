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
    void expose();
    void onMouseMove();
    
protected:
    void run();

private:
    Pipeline m_pipeline;

};

#endif
