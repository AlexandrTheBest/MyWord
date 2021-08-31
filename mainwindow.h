#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TextEdit.h"
#include "HighLighter.h"
#include "ColorListEditor.h"

#include <QClipboard>
#include <QApplication>
#include <QTime>
#include <QMenuBar>
#include <QFontDialog>
#include <QScreen>
#include <QStatusBar>
#include <QMimeData>

#include <QAction>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QResizeEvent>
#include <QFile>
#include <QDialog>
#include <QFileDialog>
#include <QBoxLayout>
#include <QByteArray>
#include <QMouseEvent>
#include <QTextCodec>
#include <QStringList>
#include <QCloseEvent>
#include <QMenu>
#include <QToolBar>
#include <QIcon>
#include <QLineEdit>
#include <QToolButton>

#include <QSettings>
#include <sstream>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void loadFile(const QString &fileName);


protected:    
    void closeEvent(QCloseEvent *e) override;

private:
    void saveSettings();

    void readStyleFromFile();

    QString styleSaveAs();

public slots:
    void fileNew();

private slots:
    void fileOpen();

    bool fileSave();

    bool fileSaveAs();

    void findText();

    void replaceText();

    void createFindDialog(QPushButton* findButton, bool needReplace);

    void setWordWrap();

    void setNewFont();

    void editBackgroundStyle();

    void editCurrentLineStyle();

    void editTextStyle();

    void setNewStyle(QAction *action);

    void setLineNumberingActive();

    void setToolbarActive();

    void setStatusbarActive();

    void setHighlighterActive();

    void updateStatistics();

    void setC89();
    void setCPP98_03();
    void setCPP11();

    void createDialogAbout();

private:
    void setupFileActions();

    void setupEditActions();

    void setupFormatActions();

    void setupViewActions();

    void setupInfoActions();

    void setupStatusbar();

    bool maybeSave();

    void setCurrentFileName(const QString &newFileName);

    bool saveFile(const QString &fileName);

    void clipboardDataChanged();

private:
    QAction *actionSave;
    QAction *actionUndo;
    QAction *actionRedo;
#ifndef QT_NO_CLIPBOARD
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
#endif
    QAction *actionFind;
    QAction *actionFindAndReplace;
    QAction *actionSelectAll;
    QAction *actionWordWrap;
    QAction *actionLineNumbering;
    QAction *actionToolbar;
    QAction *actionStatusbar;
    QAction *actionHighlighter;

    QMenu *languageVersions;
    QAction *c89;
    QAction *cpp98_03;
    QAction *cpp11;

    QMenu *editStyle;

    QLabel *saveDate;
    QLabel *changeDate;
    QLabel *statistics;
    bool isFirstChange;

    QLineEdit *findEdit;
    QLineEdit *replaceEdit;

    QToolBar *tb1;
    QToolBar *tb2;
    QToolBar *tb3;
    QMap<QString, QAction*> styleVersions;
    QString currentStyle;
    QString fileName;

    TextEditor *textEdit;
    Highlighter *highlighter;
    const QString rsrcPath;
};
#endif // MAINWINDOW_H
