#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#endif // TEXTEDIT_H

#include "HighLighter.h"

#include <QPlainTextEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QPoint>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QSize>
#include <QWidget>
#include <QTextBlock>
#include <QLabel>

class LineNumberArea;

class TextEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    TextEditor(QWidget *parent = nullptr);

    // Вызывается от LineNumberArea всякий раз, когда тот получает событие paint.
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    // Вычисление ширины области нумерации.
    // Количество цифр в последней строке редактора умножается на максимальную ширину цифры.
    int lineNumberAreaWidth();

    void replaceSearch(QString oldString, QString newString);

    void setBackgroundColor(QColor newColor = QColor(Qt::white));

    void setCurrentLineColor(QColor newColor = QColor(Qt::lightGray).lighter(120));

    QColor getBackgroundColor();

    QColor getCurrentLineColor();

    void setLineNumberingActive(bool isActive);

    QLabel* getCursorPos();

protected:
    // Дополнение стандартного контекстного меню.
    void contextMenuEvent(QContextMenuEvent *event) override;

    // Когда размер редактора изменяется, нам также нужно изменить размер области номера строки.
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void maybeCopy(bool yes);

    // Слот для выделения слова.
    // Моделирует двойной клик по заданной координате, который в QPlainTextEdit выделяет слово.
    void selectWord();

    // Слот для выделения строки.
    // Моделирует тройной клик по заданной координате, который в QPlainTextEdit выделяет строку.
    void selectString();

    // Обновление ширины области нумерации.
    void updateLineNumberAreaWidth(int newBlockCount = 0);

    // При изменении положения курсора мы выделяем текущую строку, то есть строку, содержащую курсор.
    // QPlainTextEdit дает возможность иметь более одного выбора одновременно.
    // Мы можем установить формат символов (QTextCharFormat) из этих выборок.
    // Мы очищаем выбор курсоров перед установкой нового нового Qplaintextedit::ExtraSelection,
    // иначе несколько строк будут выделены, когда пользователь выбирает несколько строк с помощью мыши.
    // При использовании свойства FullWidthSelection будет выделен текущий текстовый блок курсора (строка).
    void highlightCurrentLine();

    // Этот слот вызывается при прокрутке видового экрана редакторов.
    // QRect в качестве аргумента задается та часть области редактирования, которая должна быть обновлена (перерисована).
    // dy содержит количество пикселей, прокручиваемых видом по вертикали.
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QLabel *cursorPos;

    QPoint mousePos;
    QWidget *lineNumberArea;

    QColor backgroundColor;
    QColor currentLineColor;

    bool isLineNumberingActive;
    bool isSelection;
};

// Закрашиваем номера строк на этом виджете и помещаем его поверх CodeEditor.
// Используем защищенную функцию в QPlainTextEdit во время покраски участка.
// Чтобы все было просто, раскрашиваем область в CodeEditor класс.
// Область также просит редактора вычислить подсказку о ее размере.
class LineNumberArea : public QWidget {
public:
    LineNumberArea(TextEditor *editor) : QWidget(editor), textEditor(editor)
    {}

    QSize sizeHint() const override {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    TextEditor *textEditor;
};
