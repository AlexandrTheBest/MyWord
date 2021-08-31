#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>
#include <QTextDocument>
#include <QTextCodec>
#include <QMessageBox>
#include <QString>
#include <QMap>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>

#include <sstream>
#include <string>
#include <stdexcept>

enum class LanguageVersion {
    C89,
    CPP98_03,
    CPP11
};

struct Style {
    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat includeFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat searchFormat;
};

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QTextDocument *parent = 0);

    void setLanguageVersion(LanguageVersion version);

    void setStyle(QString styleName);

    void setStyle(Style newStyle, QString styleName);

    void setStyle(std::istringstream& style, QString styleName);

    void selectSearch(QString newSearchString);

    void setActive(bool isActive);

    Style getStyle() const;

protected:
    void highlightBlock(const QString &text) override;

private:
    void updateLanguageVersion();

    void updateStyleFormats();

    void setFormats(QTextCharFormat& format, std::istringstream& style);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QMap<QString, Style> styles;

    QString searchString;

    LanguageVersion languageVersion;
    QString styleVersion;

    bool isActive;
};

#endif // HIGHLIGHTER_H
