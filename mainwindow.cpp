#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <vector>

const QColor SEARCH_HIGHLIGHT(150, 50, 0);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    connect(ui->search, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->search, SIGNAL(textChanged(QString)), this, SLOT(refreshSearch()));
    
    connect(ui->editor, SIGNAL(textChanged()), this, SLOT(refreshSearch()));
    
    QFile file(":/stylesheet.qss");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(file.readAll());
        file.close();
    }
    
    // Debug
    loadFile("/home/simon/Projekte/BlendLuxCore/handlers/draw_imageeditor.py");
    ui->search->setText("context");
    search();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
//    qDebug() << "keypress";
    
    if (true) { // TODO check for control modifier (for f and r)
        if (event->key() == Qt::Key_F) {
            qDebug() << "ctrl F";
            ui->search->show();
            ui->search->setFocus();
            createHighlights();
        } else if (event->key() == Qt::Key_R) {
            qDebug() << "ctrl R";
            if (ui->search->isVisible())
                ui->replace->setFocus();
            else
                ui->search->setFocus();
            ui->search->show();
            ui->replace->show();
            createHighlights();
        } else if (event->key() == Qt::Key_Escape) {
            qDebug() << "escape";
            ui->search->hide();
            ui->replace->hide();
            clearHighlights();
        }
    }
}

void MainWindow::loadFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open file";
        return;
    }
    ui->editor->setPlainText(f.readAll());
    f.close();
    
    QTextCursor cursor = ui->editor->cursorForPosition(QPoint(0, 0));
    ui->editor->setTextCursor(cursor);
}

std::vector<int> MainWindow::findMatches(const QString &searchTerm) {
    std::vector<int> matches;
    if (searchTerm.size() == 0) {
        return matches;
    }
    
    const QString text = ui->editor->toPlainText();
    int fromIndex = 0;
    int matchIndex;
    while ((matchIndex = text.indexOf(searchTerm, fromIndex, Qt::CaseInsensitive)) != -1) {
        matches.push_back(matchIndex);
        fromIndex = matchIndex + searchTerm.size();
    }
    return matches;
}

void MainWindow::createHighlights() {
    const QString searchTerm = ui->search->text();
    std::vector<int> matches = findMatches(searchTerm);
    
    if (matches.size() == 0) {
        clearHighlights();
        return;
    }
    
    QList<QTextEdit::ExtraSelection> selections;
    for(int i : matches) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = ui->editor->cursorForPosition(QPoint(0, 0));
        sel.cursor.setPosition(i);
        sel.cursor.setPosition(i + searchTerm.size(), QTextCursor::KeepAnchor);
        sel.format.setBackground(QBrush(SEARCH_HIGHLIGHT));
        selections.push_back(sel);
    }
    ui->editor->setExtraSelections(selections);
}

void MainWindow::clearHighlights() {
    ui->editor->setExtraSelections(QList<QTextEdit::ExtraSelection>());
}

// User has pressed return in search line
void MainWindow::search() {
    refreshSearch();
    
    const QString searchTerm = ui->search->text();
    std::vector<int> matches = findMatches(searchTerm);
    
    QTextCursor cursor = ui->editor->textCursor();
    for (int match : matches) {
        if (match > cursor.position()) {
            cursor.setPosition(match);
            cursor.setPosition(match + searchTerm.size(), QTextCursor::KeepAnchor);
            break;
        }
    }
    ui->editor->setTextCursor(cursor);
}

void MainWindow::refreshSearch() {
    if (ui->search->isHidden())
        return;
    createHighlights();
}

void MainWindow::replaceOne() {
    
}
