#ifndef COLORLISTEDITOR_H
#define COLORLISTEDITOR_H

#include "HighLighter.h"

#include <QComboBox>
#include <QBoxLayout>
#include <QColor>
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QTableWidget>

#include <QItemEditorFactory>
#include <QHeaderView>

enum class WindowType {
    BackgroundStyle,
    CurrentLineStyle,
    TextStyle
};

class ColorListEditor : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor USER true)

public:
    ColorListEditor(QWidget *widget = nullptr);

public:
    QColor color() const;

    void setColor(const QColor &color);

private:
    void populateList();
};


class Window : public QDialog
{
    Q_OBJECT

public:
    Window(const Style style, WindowType type);

private:
    void createGUI(const Style& style, WindowType type);

private slots:
    void save();

public:
    Style getNewStyle() { return newStyle; }

    bool getIsSaved() { return isSaved; }

private:
    Style newStyle;
    bool isSaved;
};


#endif // COLORLISTEDITOR_H
