/*
 * GStreamer
 * Copyright (C) 2009 Julien Isorce <julien.isorce@gmail.com>
 * Copyright (C) 2009 Andrey Nechypurenko <andreynech@gmail.com>
 * Copyright (C) 2010 Nuno Santos <nunosantos@imaginando.net>
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <QtGui>
#include "gstthread.h"
#include "qglrenderer.h"
#include "pipeline.h"

#if defined(Q_WS_MAC)
extern void *qt_current_nsopengl_context();
#endif

QGLRenderer::QGLRenderer(const QString &videoLocation,
                         QWidget *parent)
  : QGLWidget(parent),
    videoLoc(videoLocation),
    gst_thread(NULL),
    closing(false),
    frame(NULL)
{
    move(20, 10);
    resize(640, 480);
}

QGLRenderer::~QGLRenderer()
{
}

void
QGLRenderer::initializeGL()
{
    GLContextID ctx;

#if defined(Q_WS_WIN)
    ctx.contextId = wglGetCurrentContext();
    ctx.dc = wglGetCurrentDC();
#elif defined (Q_WS_MAC)
    ctx.contextId = (NSOpenGLContext*) qt_current_nsopengl_context();
#elif defined(Q_WS_X11)
    ctx.contextId = glXGetCurrentContext();
    const char *display_name = getenv("DISPLAY");
    if(display_name == NULL)
    {
        // actually we should look for --display command line parameter here
        display_name = ":0.0";
    }
    ctx.display = XOpenDisplay(display_name);
    ctx.wnd = this->winId();
#endif

    // We need to unset Qt context before initializing gst-gl plugin.
    // Otherwise the attempt to share gst-gl context with Qt will fail.
    this->doneCurrent();
    this->gst_thread = 
      new GstThread(ctx, this->videoLoc, SLOT(newFrame()), this);
    this->makeCurrent();

    QObject::connect(this->gst_thread, SIGNAL(finished()),
                   this, SLOT(close()));
    QObject::connect(this, SIGNAL(closeRequested()),
                   this->gst_thread, SLOT(stop()), Qt::QueuedConnection);

    qglClearColor(QApplication::palette().color(QPalette::Active,
                                                QPalette::Window));
    //glShadeModel(GL_FLAT);
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_RECTANGLE_ARB); // Enable Texture Mapping

    this->gst_thread->start();
}

void
QGLRenderer::resizeGL(int width, int height)
{
  // Reset The Current Viewport And Perspective Transformation
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
}

void
QGLRenderer::newFrame()
{
    Pipeline *pipeline = this->gst_thread->getPipeline();
    if(!pipeline)
      return;

    /* frame is initialized as null */
    if (this->frame)
        pipeline->queue_output_buf.put(this->frame);

    this->frame = pipeline->queue_input_buf.get();

    /* direct call to paintGL (no queued) */
    this->updateGL();
}

void
QGLRenderer::paintGL()
{
    static GLfloat	xrot = 0;
    static GLfloat	yrot = 0;
    static GLfloat	zrot = 0;

    if (this->frame)
    {

        GLfloat width = this->frame->width;
        GLfloat height = this->frame->height;

        glEnable(GL_DEPTH_TEST);

        glEnable(GL_TEXTURE_RECTANGLE_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, this->frame->texture);
        if(glGetError () != GL_NO_ERROR)
        {
          qDebug ("failed to bind texture that comes from gst-gl");
          emit closeRequested();
          return;
        }

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S,
                      GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T,
                      GL_CLAMP_TO_EDGE);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glTranslatef(0.0f,0.0f,-5.0f);

        glRotatef(xrot,1.0f,0.0f,0.0f);
        glRotatef(yrot,0.0f,1.0f,0.0f);
        glRotatef(zrot,0.0f,0.0f,1.0f);

        glBegin(GL_QUADS);
            // Front Face
            glTexCoord2f(width, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(0.0f, height); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(width, height); glVertex3f(-1.0f,  1.0f,  1.0f);
            // Back Face
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, height); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(width, height); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(width, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            // Top Face
            glTexCoord2f(width, height); glVertex3f(-1.0f,  1.0f, -1.0f);
            glTexCoord2f(width, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(0.0f, height); glVertex3f( 1.0f,  1.0f, -1.0f);
            // Bottom Face
            glTexCoord2f(width, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, height); glVertex3f( 1.0f, -1.0f,  1.0f);
            glTexCoord2f(width,height); glVertex3f(-1.0f, -1.0f,  1.0f);
            // Right face
            glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, height); glVertex3f( 1.0f,  1.0f, -1.0f);
            glTexCoord2f(width, height); glVertex3f( 1.0f,  1.0f,  1.0f);
            glTexCoord2f(width, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
            // Left Face
            glTexCoord2f(width, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
            glTexCoord2f(0.0f, height); glVertex3f(-1.0f,  1.0f,  1.0f);
            glTexCoord2f(width, height); glVertex3f(-1.0f,  1.0f, -1.0f);
        glEnd();

        xrot+=0.3f;
        yrot+=0.2f;
        zrot+=0.4f;

        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    }
}

void
QGLRenderer::closeEvent(QCloseEvent* event)
{
    if(this->closing == false)
    {
        this->closing = true;
        emit closeRequested();
        event->ignore();
    }
}
