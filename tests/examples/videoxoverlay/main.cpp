#include <QtGui/QApplication>
#include "qrenderer.h"
#include "gstthread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    QRenderer w;
    w.setWindowTitle("glimagesink implements the gstxoverlay interface");

    return a.exec();
}
