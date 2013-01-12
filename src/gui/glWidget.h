#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "block.h"

#include <QTimer>
#include <QtOpenGL>
#include <QGLWidget>
#include <QGLShader>
#include <QGLShaderProgram>


class GLWidget : public QGLWidget
{
public:
    GLWidget(Wisp::ImageBlock* output, QWidget* pParent = NULL);
    virtual ~GLWidget();

    QSize sizeHint() const;
    void refresh();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    QSize m_size;
    QGLShaderProgram m_shader;

    float m_scale;
    GLuint m_texture;

    Wisp::ImageBlock* m_output;
};

#endif // GLWIDGET_H
