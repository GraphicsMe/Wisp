#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "glWidget.h"

MainWindow::MainWindow(Wisp::ImageBlock* output, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_glView = new GLWidget(output, ui->centralWidget);
    this->setCentralWidget(m_glView);

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1000);

    connect(m_refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refresh()
{
    m_glView->refresh();
}
