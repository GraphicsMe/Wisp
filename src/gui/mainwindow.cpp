#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "glWidget.h"
#include "camera.h"
#include <Windows.h>

using namespace Wisp;

MainWindow::MainWindow(ImageBlock* output, Scene* scene, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_scene(scene)
{
    ui->setupUi(this);

    m_glView = new GLWidget(output, ui->centralWidget);
    this->setCentralWidget(m_glView);

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1500);

    connect(m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(stop()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh()
{
    if (GetAsyncKeyState('K'))
    {
        QPoint mousePos = m_glView->mapFromGlobal(QCursor::pos());
        std::cout << mousePos.x() << ", " << mousePos.y() << std::endl;
    }
    if (this->isActiveWindow())
        m_glView->refresh();
}

void MainWindow::stop()
{
    m_scene->stop();
    m_refreshTimer->stop();
    std::cout << "Rendering Stopped!" << std::endl;
}
