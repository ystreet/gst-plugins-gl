#include <QtGui/QApplication>
#include "qglrenderer.h"
#include "gstthread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    QGLRenderer w;
    w.setWindowTitle("glimagesink implements the gstxoverlay interface");
    w.move(20, 10);
    w.resize(640, 480);
    w.show();

    GstThread gt(w.winId());
    QObject::connect(&gt, SIGNAL(finished()), &w, SLOT(close()));
    QObject::connect(&w, SIGNAL(exposeRequested()), &gt, SLOT(expose()));
    gt.start();

    return a.exec();
}
