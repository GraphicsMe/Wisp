#include "glWidget.h"

#include <assert.h>

#if !defined(GL_RGBA32F_ARB)
    #define GL_RGBA32F_ARB 0x8814
#endif

GLWidget::GLWidget(Wisp::ImageBlock* output, QWidget* pParent)
    : QGLWidget(pParent)
    , m_scale(1.0f)
    , m_output(output)
{
    assert (output != NULL);
    Wisp::Vector2i size = m_output->getSize();
    m_size.setWidth(size.x);
    m_size.setHeight(size.y);

    this->setFixedSize(m_size);
}

GLWidget::~GLWidget()
{
	glDeleteTextures(1, &m_texture);
}

QSize GLWidget::sizeHint() const
{
    return m_size;
}

void GLWidget::initializeGL()
{
    glClearColor(0.0,0.0,0.0,0.0);
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, m_size.width(), m_size.height(),
                 0, GL_RGBA, GL_FLOAT, 0);

    this->refresh();

    m_shader.addShaderFromSourceCode(QGLShader::Vertex,
            "void main() {\n"
            "	gl_Position = ftransform();\n"
            "   gl_TexCoord[0]  = gl_MultiTexCoord0;\n"
            "}\n");

    m_shader.addShaderFromSourceCode(QGLShader::Fragment,
            "//#version 120\n"
            "uniform sampler2D source;\n"
            "uniform float scale;\n"
            "\n"
            "float toSRGB(float value) {\n"
            "	if (value < 0.0031308)\n"
            "		return 12.92 * value;\n"
            "	return 1.055 * pow(value, 0.41666) - 0.055;\n"
            "}\n"
            "\n"
            "void main() {\n"
            "	vec4 color = texture2D(source, gl_TexCoord[0].xy);\n"
            "	color *= scale / color.w;\n"
            "	gl_FragColor = vec4(toSRGB(color.r), toSRGB(color.g), toSRGB(color.b), 1);\n"
            "}\n");

    m_shader.link();
}

void GLWidget::resizeGL(int w, int h)
{
    if (h == 0)
        h = 1;
    m_size=QSize(w, h);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 1, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void GLWidget::paintGL()
{
    assert(m_shader.isLinked());
    glClear(GL_COLOR_BUFFER_BIT);
    
	m_shader.bind();
    glBindTexture(GL_TEXTURE_2D, m_texture);
    m_shader.setUniformValue("scale", m_scale);
    m_shader.setUniformValue("source", 0);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();
    m_shader.release();
}

void GLWidget::refresh()
{
    m_output->lock();
    const Wisp::Vector2i& size = m_output->getSize();
	const Wisp::Vector2i& totalSize = m_output->getTotalSize();
	const Wisp::Vector2i& borderSize = m_output->getBorderSize();
    assert (size.x == m_size.width() && size.y == m_size.height());
	
    glPixelStorei(GL_UNPACK_ROW_LENGTH, totalSize.x);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, size.x, size.y,
                 0, GL_RGBA, GL_FLOAT, m_output->getData() + (borderSize.y*totalSize.x + borderSize.x));
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    m_output->unlock();
    if (m_shader.isLinked())
        this->updateGL();
}
