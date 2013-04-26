#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "block.h"
#include "scene.h"

#include <QTimer>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class GLWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(Wisp::ImageBlock* output, Wisp::Scene* scene, QWidget *parent = 0);
    ~MainWindow();
    
    inline void startRefresh() { m_refreshTimer->start(); }
    inline void stopRefresh() { m_refreshTimer->stop(); }

private slots:
    void refresh();
    void stop();

private:
    Ui::MainWindow *ui;
    GLWidget* m_glView;
    QTimer* m_refreshTimer;
    Wisp::Scene* m_scene;
};

#endif // MAINWINDOW_H
