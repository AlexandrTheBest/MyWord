#include "ColorListEditor.h"

ColorListEditor::ColorListEditor(QWidget *widget) : QComboBox(widget) {
    populateList();
}

QColor ColorListEditor::color() const {
    return qvariant_cast<QColor>(itemData(currentIndex(), Qt::DecorationRole));
}

void ColorListEditor::setColor(const QColor &color) {
    setCurrentIndex(findData(color, Qt::DecorationRole));
}

void ColorListEditor::populateList() {
    const QStringList colorNames = QColor::colorNames();

    for (int i = 0; i < colorNames.size(); ++i) {
        QColor color(colorNames[i]);

        insertItem(i, colorNames[i]);
        setItemData(i, color, Qt::DecorationRole);
    }
}


Window::Window(const Style style, WindowType type) : newStyle(style), isSaved(false) {
    this->setModal(true);

    QItemEditorFactory *factory = new QItemEditorFactory;

    QItemEditorCreatorBase *colorListCreator =
        new QStandardItemEditorCreator<ColorListEditor>();

    factory->registerEditor(QVariant::Color, colorListCreator);

    QItemEditorFactory::setDefaultFactory(factory);

    createGUI(style, type);
}

void Window::createGUI(const Style& style, WindowType type) {
    QVector<QPair<QString, QColor> > list;

    switch (type) {
    case WindowType::BackgroundStyle :
        list = {{ tr("Background"), style.keywordFormat.foreground().color() }};
        break;
    case WindowType::CurrentLineStyle :
        list = {{ tr("Сurrent line"), style.keywordFormat.foreground().color() }};
        break;
    case WindowType::TextStyle :
        list =
            {{ tr("Keywords"),              style.keywordFormat          .foreground().color() },
             { tr("QT classes"),            style.classFormat            .foreground().color() },
             { tr("Single Line Comments"),  style.singleLineCommentFormat.foreground().color() },
             { tr("Multi Line Comments"),   style.multiLineCommentFormat .foreground().color() },
             { tr("Quotations"),            style.quotationFormat        .foreground().color() },
             { tr("Includes"),              style.includeFormat          .foreground().color() },
             { tr("Functions"),             style.functionFormat         .foreground().color() },
             { tr("Searches"),              style.searchFormat           .foreground().color() }};
        break;
    }

    QTableWidget *table = new QTableWidget(list.size(), 2);
    table->setHorizontalHeaderLabels({
        tr("Format of"), tr("Color")
    });
    table->verticalHeader()->setVisible(false);
    table->resize(150, 50);

    for (int i = 0; i < list.size(); ++i) {
        const QPair<QString, QColor> &pair = list.at(i);

        QTableWidgetItem *formatItem = new QTableWidgetItem(pair.first);
        QTableWidgetItem *colorItem = new QTableWidgetItem;
        colorItem->setData(Qt::DisplayRole, pair.second);

        table->setItem(i, 0, formatItem);
        table->setItem(i, 1, colorItem);
    }
    table->resizeColumnToContents(0);
    table->horizontalHeader()->setStretchLastSection(true);

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(table, 0, 0);

    QPushButton *saveButton = new QPushButton("Save");
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));

    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(saveButton);

    setLayout(mainLayout);

    setWindowTitle(tr("Edit Style"));

    this->exec();

    newStyle.keywordFormat.setForeground(QBrush(QColor(table->item(0, 1)->text())));

    if (type == WindowType::TextStyle) {
        newStyle.classFormat            .setForeground(QBrush(QColor(table->item(1, 1)->text())));
        newStyle.singleLineCommentFormat.setForeground(QBrush(QColor(table->item(2, 1)->text())));
        newStyle.multiLineCommentFormat .setForeground(QBrush(QColor(table->item(3, 1)->text())));
        newStyle.quotationFormat        .setForeground(QBrush(QColor(table->item(4, 1)->text())));
        newStyle.includeFormat          .setForeground(QBrush(QColor(table->item(5, 1)->text())));
        newStyle.functionFormat         .setForeground(QBrush(QColor(table->item(6, 1)->text())));
        newStyle.searchFormat           .setForeground(QBrush(QColor(table->item(7, 1)->text())));
    }
}

void Window::save() {
    isSaved = true;
    this->accept();
}
