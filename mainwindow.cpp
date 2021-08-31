#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), rsrcPath(":/images")
{
    textEdit = new TextEditor(this);
    highlighter = new Highlighter(textEdit->document());

    findEdit = new QLineEdit();
    findEdit->setPlaceholderText("Find");
    replaceEdit = new QLineEdit();
    replaceEdit->setPlaceholderText("Replace");
    isFirstChange = true;

    setCentralWidget(textEdit);

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupFormatActions();
    setupViewActions();
    setupStatusbar();
    setupInfoActions();

    connect(textEdit->document(), &QTextDocument::modificationChanged,
            actionSave, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);
    connect(textEdit->document(), &QTextDocument::undoAvailable,
            actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable,
            actionRedo, &QAction::setEnabled);
    connect(textEdit, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateStatistics);

#ifndef QT_NO_CLIPBOARD
    actionCut->setEnabled(false);
    connect(textEdit, &QPlainTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
    actionCopy->setEnabled(false);
    connect(textEdit, &QPlainTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::clipboardDataChanged);
#endif
}

void MainWindow::loadFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    setCurrentFileName(fileName);
    if(QFileInfo(file).lastModified().date() == QDate::currentDate()){
        changeDate->setText("Changed: " + QFileInfo(file).lastModified().time().toString());
    }
    else {
        changeDate->setText("Changed: " + QFileInfo(file).lastModified().toString());
    }
}

void MainWindow::closeEvent(QCloseEvent *e)  {
    if (maybeSave()) {
        saveSettings();
        e->accept();
    }
    else
        e->ignore();
}

void MainWindow::saveSettings() {
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.setValue("MAIN/WordWrap", actionWordWrap->isChecked());
    settings.setValue("MAIN/BackgroundColor", textEdit->getBackgroundColor());
    settings.setValue("MAIN/CurrentLineColor", textEdit->getCurrentLineColor());
    settings.setValue("DISPLAY/LineNumbering", actionLineNumbering->isChecked());
    settings.setValue("DISPLAY/Toolbar", actionToolbar->isChecked());
    settings.setValue("DISPLAY/Statusbar", actionStatusbar->isChecked());
    settings.setValue("DISPLAY/Highlighter", actionHighlighter->isChecked());
    if (c89->isChecked()) {
        settings.setValue("DISPLAY/LanguageVersion", "C89");
    }
    if (cpp98_03->isChecked()) {
        settings.setValue("DISPLAY/LanguageVersion", "C++98/03");
    }
    if (cpp11->isChecked()) {
        settings.setValue("DISPLAY/LanguageVersion", "C++11");
    }
    highlighter->setStyle(highlighter->getStyle(), "SettingStyle");
}

void MainWindow::readStyleFromFile() {
    QFileDialog fileDialog(this, tr("Open File..."));
    if (fileDialog.exec() != QDialog::Accepted)
        return;

    const QString f = fileDialog.selectedFiles().first();

    if (!QFile::exists(f))
        return;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);

    std::istringstream stream(codec->toUnicode(data).toStdString());
    QString shortName = QFileInfo(f).baseName();

    if (!styleVersions.contains(shortName)) {
        highlighter->setStyle(stream, shortName);

        QAction *a = editStyle->addAction(shortName);
        a->setCheckable(true);
        a->setChecked(true);

        styleVersions.insert(a->text(), a);
    } else {
        highlighter->setStyle(shortName);
    }
    styleVersions.value(currentStyle)->setChecked(false);
    styleVersions.value(shortName)->setChecked(true);
    currentStyle = shortName;

    highlighter->setDocument(textEdit->document());
}

QString MainWindow::styleSaveAs() {
    QFileDialog fileDialog(this, tr("Save as..."));

    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList mimeTypes("application/json");
    fileDialog.setMimeTypeFilters(mimeTypes);

    fileDialog.setDefaultSuffix("json");

    if (fileDialog.exec() != QDialog::Accepted)
        return "";

    const QString fileName = fileDialog.selectedFiles().first();

    if (fileName.isEmpty())
        styleSaveAs();
    if (fileName.startsWith(QStringLiteral(":/")))
        styleSaveAs();

    return fileName;
}

void MainWindow::fileNew() {
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFileName(QString());
    }
}

void MainWindow::fileOpen() {
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::fileSave() {
    if (fileName.isEmpty()) {
        return fileSaveAs();
    } else {
        return saveFile(fileName);
    }
}

bool MainWindow::fileSaveAs() {
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setDefaultSuffix("txt");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first());
}

void MainWindow::findText() {
    QPushButton *findButton = new QPushButton("Find");
    createFindDialog(findButton, false);

    highlighter->selectSearch(findEdit->text());
    highlighter->setDocument(textEdit->document());
}

void MainWindow::replaceText() {
    QPushButton *findButton = new QPushButton("Find and replace");
    createFindDialog(findButton, true);

    textEdit->replaceSearch(findEdit->text(), replaceEdit->text());
    highlighter->selectSearch("");
    highlighter->setDocument(textEdit->document());
}

void MainWindow::createFindDialog(QPushButton* findButton, bool needReplace) {
    QBoxLayout *boxLayout = new QBoxLayout(QBoxLayout::LeftToRight);

    boxLayout->addWidget(findEdit);
    if (needReplace) {
        boxLayout->addWidget(replaceEdit);
    }
    boxLayout->addWidget(findButton);

    QDialog *dialog = new QDialog(this);

    dialog->setLayout(boxLayout);
    connect(findButton, SIGNAL(clicked()), dialog, SLOT(accept()));

    dialog->exec();
}

void MainWindow::setWordWrap() {
    if (actionWordWrap->isChecked()) {
        textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    } else {
        textEdit->setWordWrapMode(QTextOption::NoWrap);
    }
}

void MainWindow::setNewFont() {
    bool pressOk;
    QFont newFont = QFontDialog::getFont(&pressOk, textEdit->font());
    if(pressOk) {
        textEdit->setFont(newFont);
    }
}

void MainWindow::editBackgroundStyle() {
    Style style;
    style.keywordFormat.setForeground(textEdit->getBackgroundColor());
    Window *window = new Window(style, WindowType::BackgroundStyle);

    if(window->getIsSaved()) {
        textEdit->setBackgroundColor(window->getNewStyle().keywordFormat.foreground().color());
    }
}

void MainWindow::editCurrentLineStyle() {
    Style style;
    style.keywordFormat.setForeground(textEdit->getCurrentLineColor());
    Window *window = new Window(style, WindowType::CurrentLineStyle);

    if(window->getIsSaved()) {
        textEdit->setCurrentLineColor(window->getNewStyle().keywordFormat.foreground().color());
    }
}

void MainWindow::editTextStyle() {
    Window *window = new Window(highlighter->getStyle(), WindowType::TextStyle);

    if(window->getIsSaved()) {
        QString fileName = styleSaveAs();
        if (fileName != "") {

            QString shortName = QFileInfo(fileName).baseName();
            QAction *a = editStyle->addAction(shortName);
            a->setCheckable(true);
            a->setChecked(true);

            styleVersions.value(currentStyle)->setChecked(false);
            styleVersions.insert(a->text(), a);
            currentStyle = shortName;

            highlighter->setStyle(window->getNewStyle(), fileName);
            highlighter->setDocument(textEdit->document());
        }
    }
}

void MainWindow::setNewStyle(QAction *action) {
    if (action->text().left(1) != "&") {
        styleVersions.value(currentStyle)->setChecked(false);
        currentStyle = action->text();
        highlighter->setStyle(currentStyle);
        highlighter->setDocument(textEdit->document());
    }
}

void MainWindow::setLineNumberingActive() {
    if (actionLineNumbering->isChecked()) {
        textEdit->setLineNumberingActive(true);
    } else {
        textEdit->setLineNumberingActive(false);
    }
}

void MainWindow::setToolbarActive() {
    if (actionToolbar->isChecked()) {
        tb1->setVisible(true);
        tb2->setVisible(true);
        tb3->setVisible(true);
    } else {
        tb1->setVisible(false);
        tb2->setVisible(false);
        tb3->setVisible(false);
    }
}

void MainWindow::setStatusbarActive() {
    if (actionStatusbar->isChecked()) {
        statusBar()->setVisible(true);
    } else {
        statusBar()->setVisible(false);
    }
}

void MainWindow::setHighlighterActive() {
    if (actionHighlighter->isChecked()) {
        highlighter->setActive(true);
    } else {
        highlighter->setActive(false);
    }
    highlighter->setDocument(textEdit->document());
}

void MainWindow::updateStatistics() {
    int words = textEdit->toPlainText().split(
            QRegExp("(\\s|\\n|\\r|\\t)+"), Qt::SkipEmptyParts
        ).count();
    int symbols = textEdit->toPlainText().length();
    statistics->setText("Rows: "      + QString::number(textEdit->blockCount()) +
                        ", words: "   + QString::number(words) +
                        ", symbols: " + QString::number(symbols) +
                        ", size: "    + QString::number((symbols*1000/1024)/1000.) + "KB");
    if (!isFirstChange) {
        changeDate->setText("Changed: " + QTime::currentTime().toString());
    } else {
        isFirstChange = false;
    }
}

void MainWindow::setC89() {
    cpp98_03->setChecked(false);
    cpp11->setChecked(false);
    if (c89->isChecked()) {
        highlighter->setLanguageVersion(LanguageVersion::C89);
        highlighter->setDocument(textEdit->document());
    } else {
        c89->setChecked(true);
    }
}
void MainWindow::setCPP98_03() {
    c89->setChecked(false);
    cpp11->setChecked(false);
    if (cpp98_03->isChecked()) {
        highlighter->setLanguageVersion(LanguageVersion::CPP98_03);
        highlighter->setDocument(textEdit->document());
    } else {
        cpp98_03->setChecked(true);
    }
}
void MainWindow::setCPP11() {
    c89->setChecked(false);
    cpp98_03->setChecked(false);
    if (cpp11->isChecked()) {
        highlighter->setLanguageVersion(LanguageVersion::CPP11);
        highlighter->setDocument(textEdit->document());
    } else {
        cpp11->setChecked(true);
    }
}

void MainWindow::createDialogAbout() {
    QBoxLayout *boxLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QPixmap photo(rsrcPath + "/CreatorPhoto.jpg");

    QLabel *label = new QLabel(this);
    label->setPixmap(photo);
    label->resize(225, 400);

    boxLayout->addWidget(label);
    boxLayout->addWidget(new QLabel("Created by Alexander Berezin."));
    boxLayout->addWidget(new QLabel(tr("The program was compiled on ") + __DATE__ + tr(" / ")  + __TIME__ + tr(" in QT ") + QT_VERSION_STR));
    boxLayout->addWidget(new QLabel(tr("and was started in ") + qVersion()));

    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);

    QPushButton *closeButton = new QPushButton("Great!");
    boxLayout->addWidget(closeButton);
    connect(closeButton, SIGNAL(clicked()), dialog, SLOT(accept()));

    dialog->setLayout(boxLayout);

    dialog->exec();
    delete dialog;
}

void MainWindow::setupFileActions()
{
    tb1 = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/filenew.png"));
    QAction *a = menu->addAction(newIcon,  tr("&New"), this, &MainWindow::fileNew);
    tb1->addAction(a);
    a->setShortcut(QKeySequence::New);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(rsrcPath + "/fileopen.png"));
    a = menu->addAction(openIcon, tr("&Open..."), this, &MainWindow::fileOpen);
    a->setShortcut(QKeySequence::Open);
    tb1->addAction(a);

    menu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(rsrcPath + "/filesave.png"));
    actionSave = menu->addAction(saveIcon, tr("&Save"), this, &MainWindow::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    tb1->addAction(actionSave);

    const QIcon saveAsIcon = QIcon::fromTheme("document-saveAs", QIcon(rsrcPath + "/filesaveas.png"));
    a = menu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::fileSaveAs);
    menu->addSeparator();

    a = menu->addAction(tr("&Quit"), this, &QWidget::close);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
}

void MainWindow::setupEditActions() {
    tb2 = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Edit"));

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png"));
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &QPlainTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb2->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/editredo.png"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &QPlainTextEdit::redo);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb2->addAction(actionRedo);
    menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/editcut.png"));
    actionCut = menu->addAction(cutIcon, tr("&Cut"), textEdit, &QPlainTextEdit::cut);
    actionCut->setShortcut(QKeySequence::Cut);
    tb2->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/editcopy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copy"), textEdit, &QPlainTextEdit::copy);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb2->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/editpaste.png"));
    actionPaste = menu->addAction(pasteIcon, tr("&Paste"), textEdit, &QPlainTextEdit::paste);
    actionPaste->setShortcut(QKeySequence::Paste);
    tb2->addAction(actionPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
    tb3 = addToolBar(tr("Find Actions"));

    QToolButton *findButtons = new QToolButton();
    findButtons->setPopupMode(QToolButton::MenuButtonPopup);

    const QIcon findIcon = QIcon::fromTheme("edit-find", QIcon(rsrcPath + "/editfind.png"));
    actionFind = new QAction(findIcon, tr("&Find"));
    actionFind->setShortcut(QKeySequence::Find);
    connect(actionFind, SIGNAL(triggered()), this, SLOT(findText()));

    const QIcon findAndReplaceIcon = QIcon::fromTheme("edit-findAndReplace", QIcon(rsrcPath + "/editfindandreplace.png"));
    actionFindAndReplace = new QAction(findAndReplaceIcon, tr("&Find and replace"));
    actionFindAndReplace->setShortcut(QKeySequence::Replace);
    connect(actionFindAndReplace, SIGNAL(triggered()), this, SLOT(replaceText()));

    const QIcon findMenuIcon = QIcon::fromTheme("edit-findMenu", QIcon(rsrcPath + "/editfindmenu.png"));
    QMenu *findMenu = new QMenu();

    findMenu->setIcon(findMenuIcon);
    findMenu->setTitle("Find / Find and replace");
    findMenu->addAction(actionFind);
    findMenu->addAction(actionFindAndReplace);

    findButtons->setMenu(findMenu);
    findButtons->setIcon(findMenuIcon);
    menu->addMenu(findMenu);
    tb3->addWidget(findButtons);

    const QIcon selectAllIcon = QIcon::fromTheme("edit-selectAll", QIcon(rsrcPath + "/editselectall.png"));
    actionSelectAll = menu->addAction(selectAllIcon, tr("&SelectAll"), textEdit, &QPlainTextEdit::selectAll);
    actionSelectAll->setShortcut(QKeySequence::SelectAll);
}

void MainWindow::setupFormatActions() {
    QMenu *menu = menuBar()->addMenu(tr("&Format"));

    actionWordWrap = menu->addAction(tr("&Word wrap"), this, &MainWindow::setWordWrap);
    actionWordWrap->setCheckable(true);

    menu->addAction(tr("&Font selection"), this, &MainWindow::setNewFont);
}

void MainWindow::setupViewActions() {
    QMenu *menu = menuBar()->addMenu(tr("&View"));

    menu->addAction(tr("&Background color selection"), this, &MainWindow::editBackgroundStyle);
    menu->addAction(tr("&Current line color selection"), this, &MainWindow::editCurrentLineStyle);

    actionLineNumbering = menu->addAction(tr("&Line numbering display"), this, &MainWindow::setLineNumberingActive);
    actionToolbar       = menu->addAction(tr("&Toolbar display"),        this, &MainWindow::setToolbarActive);
    actionStatusbar     = menu->addAction(tr("&Statusbar display"),      this, &MainWindow::setStatusbarActive);
    actionHighlighter   = menu->addAction(tr("&Highlighter display"),    this, &MainWindow::setHighlighterActive);

    actionLineNumbering->setCheckable(true);
    actionToolbar      ->setCheckable(true);
    actionStatusbar    ->setCheckable(true);
    actionHighlighter  ->setCheckable(true);

    actionLineNumbering->setChecked(true);
    actionToolbar      ->setChecked(true);
    actionStatusbar    ->setChecked(true);
    actionHighlighter  ->setChecked(true);

    languageVersions = new QMenu("Language versions");

    c89 = languageVersions->addAction(tr("&C89"), this, &MainWindow::setC89);
    cpp98_03 = languageVersions->addAction(tr("&C++98/03"), this, &MainWindow::setCPP98_03);
    cpp11 = languageVersions->addAction(tr("&C++11"), this, &MainWindow::setCPP11);

    c89->setCheckable(true);
    c89->setChecked(true);
    cpp98_03->setCheckable(true);
    cpp11->setCheckable(true);

    menu->addMenu(languageVersions);

    editStyle = new QMenu("Choise and edit style");
    connect(editStyle, SIGNAL(triggered(QAction*)), this, SLOT(setNewStyle(QAction*)));

    editStyle->addAction(tr("&Edit"), this, &MainWindow::editTextStyle);
    editStyle->addAction(tr("&Load from file"), this, &MainWindow::readStyleFromFile);

    QAction *a = editStyle->addAction("Default");
    a->setCheckable(true);
    a->setChecked(true);
    styleVersions.insert(a->text(), a);
    currentStyle = a->text();

    a = editStyle->addAction("ATB");
    a->setCheckable(true);
    styleVersions.insert(a->text(), a);

    menu->addMenu(editStyle);
}

void MainWindow::setupInfoActions() {
    QMenu *menu = menuBar()->addMenu(tr("&Info"));

    menu->addAction(tr("&About"), this, &MainWindow::createDialogAbout);
}

void MainWindow::setupStatusbar() {
    saveDate   = new QLabel(this);
    changeDate = new QLabel(this);
    statistics = new QLabel(this);

    saveDate->setText("Saved: None");
    changeDate->setText("Changed: None");

    statusBar()->addWidget(textEdit->getCursorPos(), 1);
    statusBar()->addWidget(saveDate, 2);
    statusBar()->addWidget(changeDate, 2);
    statusBar()->addWidget(statistics, 3);
}

bool MainWindow::maybeSave() {
    if (!textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, QCoreApplication::applicationName(),
                             tr("The document has been modified.\n"
                                "Do you want to save your changes?"),
                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void MainWindow::setCurrentFileName(const QString &newFileName) {
    fileName = newFileName;
    textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(fileName).fileName();

    if (shownName.size() > 32) {
        shownName = shownName.left(32) + "...";
    }
    setWindowTitle(tr("[*]%1 - %2").arg(shownName, QCoreApplication::applicationName()));
    setWindowModified(false);
}

bool MainWindow::saveFile(const QString &fileName)
{
    saveDate->setText("Saved: " + QTime::currentTime().toString());
    changeDate->setText("Changed: None");
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << textEdit->toPlainText();
    } else {
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                       .arg(QDir::toNativeSeparators(fileName), file.errorString());
    }
    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(this, tr("Application"), errorMessage);
        return false;
    }

    file.close();
    setCurrentFileName(fileName);
    return true;
}

void MainWindow::clipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}
