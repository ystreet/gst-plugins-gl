#include <QtGui/QApplication>
#include "qrenderer.h"
#include "gstthread.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    QString videolocation = QFileDialog::getOpenFileName(0, "Select a video file", 
        ".", "Format (*.avi *.mkv *.ogg *.asf *.mov)");

    if (videolocation.isEmpty())
        return -1;

    QRenderer w(videolocation);
    w.setWindowTitle("glimagesink implements the gstxoverlay interface");

    return a.exec();
}
