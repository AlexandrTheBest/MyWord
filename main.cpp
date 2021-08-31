#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("TextEditor");
    QCoreApplication::applicationVersion();

    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open.");
    parser.process(a);

    MainWindow mw;

    const QRect availableGeometry = mw.screen()->availableGeometry();
    mw.resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
    mw.move((availableGeometry.width() - mw.width()) / 2,
            (availableGeometry.height() - mw.height()) / 2);

    mw.fileNew();

    mw.show();
    return a.exec();
}
