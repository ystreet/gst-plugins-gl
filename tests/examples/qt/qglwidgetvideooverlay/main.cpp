/*
 * GStreamer
 * Copyright (C) 2008-2009 Julien Isorce <julien.isorce@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QtGui/QApplication>
#include "qglrenderer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    QString videolcoation = QFileDialog::getOpenFileName(0, "Select a video file", 
        ".", "Format (*.avi *.mkv *.ogg *.asf *.mov)");

    if (videolcoation.isEmpty())
        return -1;

    QGLRenderer w(videolcoation);
    w.setWindowTitle("glimagesink implements the gstvideooverlay interface");

    return a.exec();
}
