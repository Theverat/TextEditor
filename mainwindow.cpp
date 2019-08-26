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
    
    ui->editor->installEventFilter(this);
    
    // Debug
    loadFile("/home/simon/Projekte/BlendLuxCore/handlers/draw_imageeditor.py");
    ui->search->setText("context");
    search();
}

MainWindow::~MainWindow() {
    delete ui;
}


bool MainWindow::eventFilter(QObject* target, QEvent* event) {
    if(target == ui->editor) {
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Return) {
                // We are at the end of the old line
                QTextCursor cursor = ui->editor->textCursor();
                const QString text = ui->editor->toPlainText();
                // Go back to the start of the line
                int pos = cursor.position();
                while (pos > 0) {
                    --pos;
                    if (text[pos] == '\n') {
                        ++pos;
                        break;
                    }
                }
                
                // Find the first word in the old line
                QString firstWord;
                int i = pos;
                while (i < text.size()) {
                    const QChar c = text[i];
                    if (c == ' ' || c == '(') {
                        if (firstWord.size() != 0) {
                            // End of first word
                            break;
                        }
                    } else {
                        firstWord.append(c);
                    }
                    ++i;
                }
                
                // Insert correct amount of spaces in the new line
                QString spaces("\n");
                while (pos < text.size() && text[pos] == ' ') {
                    spaces.append(' ');
                    ++pos;
                }
                // Increase indentation level after certain keywords
                QStringList keywords;
                keywords << "if" << "while" << "for" << "def" << "class";
                if (keywords.contains(firstWord)) {
                    spaces.append("    ");
                }
                cursor.insertText(spaces);
                return true;
            } else if (keyEvent->key() == Qt::Key_Tab) {
                QTextCursor cursor = ui->editor->textCursor();
                cursor.insertText("    ");
                return true;
            }
            // TODO backspace: delete four spaces if there are enough
        }
    }
    return QWidget::eventFilter(target, event);
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
//    qDebug() << "keypress";
    
    const bool ctrlPressed = event->modifiers() & Qt::ControlModifier;
    
    if (ctrlPressed && event->key() == Qt::Key_F) {
        qDebug() << "ctrl F";
        ui->search->show();
        ui->search->setFocus();
        createHighlights();
    } else if (ctrlPressed && event->key() == Qt::Key_R) {
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
    bool foundMatch = false;
    for (int match : matches) {
        if (match > cursor.position()) {
            cursor.setPosition(match);
            cursor.setPosition(match + searchTerm.size(), QTextCursor::KeepAnchor);
            foundMatch = true;
            break;
        }
    }
    if (!foundMatch && matches.size() > 0) {
        // Wrap around
        cursor.setPosition(matches[0]);
        cursor.setPosition(matches[0] + searchTerm.size(), QTextCursor::KeepAnchor);
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
