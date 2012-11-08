/*
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@gmail.com>
 * Copyright (C) 2009 Andrey Nechypurenko <andreynech@gmail.com>
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

#ifndef QGLRENDERER_H
#define QGLRENDERER_H

#include <QGLWidget>
#include "GstGLBufferDef.h"


class GstThread;

class QGLRenderer : public QGLWidget
{
    Q_OBJECT

public:
    QGLRenderer(const QString &videoLocation, QWidget *parent = 0);
    ~QGLRenderer();

    void closeEvent(QCloseEvent* event);

Q_SIGNALS:
    void closeRequested();

public Q_SLOTS:
    void newFrame();

protected:
    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();

private:
    QString videoLoc;
    GstThread *gst_thread;
    bool closing;
    GstGLBuffer *frame;
};

#endif // QGLRENDERER_H
