#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "block.h"

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
    explicit MainWindow(Wisp::ImageBlock* output, QWidget *parent = 0);
    ~MainWindow();
    
    inline void startRefresh() { m_refreshTimer->start(); }
    inline void stopRefresh() { m_refreshTimer->stop(); }

private slots:
    void refresh();

private:
    Ui::MainWindow *ui;
    GLWidget* m_glView;
    QTimer* m_refreshTimer;
};

#endif // MAINWINDOW_H
