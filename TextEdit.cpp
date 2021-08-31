#include "TextEdit.h"

TextEditor::TextEditor(QWidget *parent) : QPlainTextEdit(parent) {
    this->setWordWrapMode(QTextOption::NoWrap);

    cursorPos = new QLabel(this->parentWidget());
    isLineNumberingActive = true;
    isSelection = false;

    this->setBackgroundVisible(true);
    setCurrentLineColor();
    setBackgroundColor();

    // Табуляция в 4 символа
    this->setTabStopDistance(fontMetrics().averageCharWidth() * 4);

    // Создание новой области нумерации
    lineNumberArea = new LineNumberArea(this);

    // Привязка сигналов к слотам
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLineNumberArea);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);
    connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(maybeCopy(bool)));

    // Рассчет ширины области нумерации и подсветка 1-й строки
    updateLineNumberAreaWidth();
    highlightCurrentLine();
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    if (isLineNumberingActive) {
        // Задаем фон области нумерации
        QPainter painter(lineNumberArea);
        painter.fillRect(event->rect(), Qt::lightGray);

        // Теперь мы пройдемся по всем видимым линиям и нарисуем номера линий в дополнительной области для каждой линии.
        // При редактировании обычного текста один номер прикреплен к одному QTextBlock.
        // Если перенос строк включен, один номер может охватывать несколько строк в видовом окне редактирования текста.
        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        // Получаем верхнюю и нижнюю координаты y первого текстового блока и корректируем эти
        // значения на высоту текущего текстового блока в каждой итерации цикла.
        while (block.isValid() && top <= event->rect().bottom()) {

            // Проверяем, виден ли блок, а также проверяем, находится ли он в области просмотра -
            // блок может быть, например, скрыт окном, расположенным над текстовым редактором.
            if (block.isVisible() && bottom >= event->rect().top()) {
                QString number = QString::number(blockNumber + 1);
                painter.setPen(Qt::black);
                painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                                 Qt::AlignRight, number);
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++blockNumber;
        }
    }
}

int TextEditor::lineNumberAreaWidth() {
    int digits = 1;

    // Максимум из 1 и количества строк
    int max = qMax(1, blockCount());

    // Находим количество цифр (символов)
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // Умножаем размер символа на количество символов
    return (2 + fontMetrics().averageCharWidth()) * digits;
}

void TextEditor::replaceSearch(QString oldString, QString newString) {
    int count = this->toPlainText().count(oldString);
    QString string = this->toPlainText().replace(oldString, newString);
    this->selectAll();
    this->insertPlainText(string);
    QMessageBox::warning(this, "Find and replace", QString::number(count) + " words were replaced");
}

void TextEditor::setBackgroundColor(QColor newColor) {
    backgroundColor = newColor;

    QPalette palette = this->palette();
    palette.setColor(QPalette::Active, QPalette::Base, newColor);
    palette.setColor(QPalette::Inactive, QPalette::Base, newColor);
    palette.setColor(QPalette::Active, QPalette::Background, newColor);
    palette.setColor(QPalette::Inactive, QPalette::Background, newColor);

    this->setPalette(palette);
}

void TextEditor::setCurrentLineColor(QColor newColor) {
    currentLineColor = newColor;
    highlightCurrentLine();
}

QColor TextEditor::getBackgroundColor() {
    return backgroundColor;
}

QColor TextEditor::getCurrentLineColor() {
    return currentLineColor;
}

void TextEditor::setLineNumberingActive(bool isActive) {
    isLineNumberingActive = isActive;
    updateLineNumberAreaWidth();
}

QLabel* TextEditor::getCursorPos() {
    return cursorPos;
};

// Дополнение стандартного контекстного меню.
void TextEditor::contextMenuEvent(QContextMenuEvent *event) {

    // Создание стандартного контекстного меню.
    QMenu *menu = createStandardContextMenu();

    mousePos = event->pos();

    // Создание и добавление действий "Выделить строку" и "Выделить слово".
    QAction *selectString = new QAction();
    selectString->setText("Select String");
    connect(selectString, SIGNAL(triggered()), this, SLOT(selectString()));

    QAction *selectWord = new QAction();
    selectWord->setText("Select Word");
    connect(selectWord, SIGNAL(triggered()), this, SLOT(selectWord()));

    // Если текста нет (и выделять нечего) или есть выделение, кнопки деактивировать.
    if (this->document()->isEmpty() || isSelection) {
        selectString->setDisabled(true);
        selectWord->setDisabled(true);
    }

    // Добавление действий в меню.
    menu->addAction(selectString);
    menu->addAction(selectWord);

    menu->exec(event->globalPos());
    delete menu;
}

// Когда размер редактора изменяется, нам также нужно изменить размер области номера строки.
void TextEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditor::maybeCopy(bool yes) {
    isSelection = yes;
}

// Слот для выделения слова.
// Моделирует двойной клик по заданной координате, который в QPlainTextEdit выделяет слово.
void TextEditor::selectWord() {
    QMouseEvent *e = new QMouseEvent(QEvent::MouseButtonDblClick, mousePos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    this->mouseDoubleClickEvent(e);
}

// Слот для выделения строки.
// Моделирует тройной клик по заданной координате, который в QPlainTextEdit выделяет строку.
void TextEditor::selectString() {
    QMouseEvent *e1 = new QMouseEvent(QEvent::MouseButtonDblClick, mousePos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent *e2 = new QMouseEvent(QEvent::MouseButtonPress, mousePos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    this->mouseDoubleClickEvent(e1);
    this->mousePressEvent(e2);
}

// Обновление ширины области нумерации.
void TextEditor::updateLineNumberAreaWidth(int newBlockCount) {
    Q_UNUSED(newBlockCount)
    if (isLineNumberingActive) {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    } else {
        setViewportMargins(0, 0, 0, 0);
    }
}

// При изменении положения курсора мы выделяем текущую строку, то есть строку, содержащую курсор.
// QPlainTextEdit дает возможность иметь более одного выбора одновременно.
// Мы можем установить формат символов (QTextCharFormat) из этих выборок.
// Мы очищаем выбор курсоров перед установкой нового нового Qplaintextedit::ExtraSelection,
// иначе несколько строк будут выделены, когда пользователь выбирает несколько строк с помощью мыши.
// При использовании свойства FullWidthSelection будет выделен текущий текстовый блок курсора (строка).
void TextEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(currentLineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);

    cursorPos->setText(
                "(" + QString::number(textCursor().blockNumber()+1) + ":"
                    + QString::number(textCursor().columnNumber()+1)+ ")"
        );
}

// Этот слот вызывается при прокрутке видового экрана редакторов.
// QRect в качестве аргумента задается та часть области редактирования, которая должна быть обновлена (перерисована).
// dy содержит количество пикселей, прокручиваемых видом по вертикали.
void TextEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}
