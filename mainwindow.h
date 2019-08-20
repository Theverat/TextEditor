#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool eventFilter(QObject* target, QEvent* event);
    
protected:
    virtual void keyPressEvent(QKeyEvent *event);
    
private:
    Ui::MainWindow *ui;
    
    void loadFile(const QString &path);
    std::vector<int> findMatches(const QString &searchTerm);
    void createHighlights();
    void clearHighlights();
    
private slots:
    void search();
    void refreshSearch();
    void replaceOne();
};

#endif // MAINWINDOW_H
