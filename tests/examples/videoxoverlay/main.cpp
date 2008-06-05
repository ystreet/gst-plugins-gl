#include <QtGui/QApplication>
#include "qrenderer.h"
#include "gstthread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    QRenderer w;
    w.setWindowTitle("glimagesink implements the gstxoverlay interface");
    w.move(20, 10);
    w.resize(640, 480);
    w.show();

    GstThread gt(w.winId());
    QObject::connect(&gt, SIGNAL(finished()), &w, SLOT(close()));
    QObject::connect(&w, SIGNAL(exposeRequested()), &gt, SLOT(exposeRequested()));

    QTimer::singleShot(1000, &gt, SLOT(start()));
    //or
    //gt.start(); //works fine on win32 but not on linux
    
    return a.exec();
}
