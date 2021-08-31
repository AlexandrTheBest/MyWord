#include "HighLighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{   
    isActive = true;
    languageVersion = LanguageVersion::C89;

    // Добавление стиля ATB.
    if (QFile::exists(":Styles/ATB.json")) {
        QFile file(":Styles/ATB.json");
        if (file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            QTextCodec *codec = Qt::codecForHtml(data);

            std::istringstream stream(codec->toUnicode(data).toStdString());
            setStyle(stream, "ATB");
        }
    }

    // Добавление стиля по умолчанию.
    if (QFile::exists(":Styles/Default.json")) {
        QFile file(":Styles/Default.json");
        if (file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            QTextCodec *codec = Qt::codecForHtml(data);

            std::istringstream stream(codec->toUnicode(data).toStdString());
            setStyle(stream, "Default");
        }
    }
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
            QRegExp expression(rule.pattern);
            int index = expression.indexIn(text);
            while (index >= 0) {
                int length = expression.matchedLength();
                setFormat(index, length, rule.format);
                index = expression.indexIn(text, index + length);
            }
        }
        setCurrentBlockState(0);

        int startIndex = 0;
        if (previousBlockState() != 1)
            startIndex = commentStartExpression.indexIn(text);

        while (startIndex >= 0) {
            int endIndex = commentEndExpression.indexIn(text, startIndex);
            int commentLength;
            if (endIndex == -1) {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex
                                + commentEndExpression.matchedLength();
            }
            setFormat(startIndex, commentLength, styles.value(styleVersion).multiLineCommentFormat);
            startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
        }
}

void Highlighter::setLanguageVersion(LanguageVersion version) {
    languageVersion = version;
    updateStyleFormats();
}

void Highlighter::setStyle(QString styleName) {
    styleVersion = styleName;
    updateStyleFormats();
}

void Highlighter::setStyle(Style newStyle, QString styleName) {
    QString errorMessage;

    QFile file(styleName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << "{\n"
            << "  \"classFormat\": {\n"
            << "    \"FontItalic\": " <<    newStyle.classFormat.fontItalic() << ",\n"
            << "    \"FontUnderline\": " << newStyle.classFormat.fontUnderline() << ",\n"
            << "    \"FontWeight\": " <<    newStyle.classFormat.fontWeight() << ",\n"
            << "    \"Foreground\": \"" <<  newStyle.classFormat.foreground().color().name() << "\"\n"
            << "  },\n"
            << "  \"functionFormat\": {\n"
            << "    \"FontItalic\": " <<    newStyle.functionFormat.fontItalic() << ",\n"
            << "    \"FontUnderline\": " << newStyle.functionFormat.fontUnderline() << ",\n"
            << "    \"FontWeight\": " <<    newStyle.functionFormat.fontWeight() << ",\n"
            << "    \"Foreground\": \"" <<  newStyle.functionFormat.foreground().color().name() << "\"\n"
            << "  },\n"
            << "  \"includeFormat\": {\n"
            << "    \"FontItalic\": " <<    newStyle.includeFormat.fontItalic() << ",\n"
            << "    \"FontUnderline\": " << newStyle.includeFormat.fontUnderline() << ",\n"
            << "    \"FontWeight\": " <<    newStyle.includeFormat.fontWeight() << ",\n"
            << "    \"Foreground\": \"" <<  newStyle.includeFormat.foreground().color().name() << "\"\n"
            << "  },\n"
            << "  \"keywordFormat\": {\n"
            << "    \"FontItalic\": " <<    newStyle.keywordFormat.fontItalic() << ",\n"
            << "    \"FontUnderline\": " << newStyle.keywordFormat.fontUnderline() << ",\n"
            << "    \"FontWeight\": " <<    newStyle.keywordFormat.fontWeight() << ",\n"
            << "    \"Foreground\": \"" <<  newStyle.keywordFormat.foreground().color().name() << "\"\n"
            << "  },\n"
            << "  \"multiLineCommentFormat\": {\n"
            << "    \"FontItalic\": " <<    newStyle.multiLineCommentFormat.fontItalic() << ",\n"
            << "    \"FontUnderline\": " << newStyle.multiLineCommentFormat.fontUnderline() << ",\n"
            << "    \"FontWeight\": " <<    newStyle.multiLineCommentFormat.fontWeight() << ",\n"
            << "    \"Foreground\": \"" <<  newStyle.multiLineCommentFormat.foreground().color().name() << "\"\n"
            << "  },\n"
            << "  \"quotationFormat\": {\n"
            << "    \"FontItalic\": " <<    newStyle.quotationFormat.fontItalic() << ",\n"
            << "    \"FontUnderline\": " << newStyle.quotationFormat.fontUnderline() << ",\n"
            << "    \"FontWeight\": " <<    newStyle.quotationFormat.fontWeight() << ",\n"
            << "    \"Foreground\": \"" <<  newStyle.quotationFormat.foreground().color().name() << "\"\n"
            << "  },\n"
            << "  \"searchFormat\": {\n"
            << "    \"FontItalic\": " <<    newStyle.searchFormat.fontItalic() << ",\n"
            << "    \"FontUnderline\": " << newStyle.searchFormat.fontUnderline() << ",\n"
            << "    \"FontWeight\": " <<    newStyle.searchFormat.fontWeight() << ",\n"
            << "    \"Foreground\": \"" <<  newStyle.searchFormat.foreground().color().name() << "\"\n"
            << "  },\n"
            << "  \"singleLineCommentFormat\": {\n"
            << "    \"FontItalic\": " <<    newStyle.singleLineCommentFormat.fontItalic() << ",\n"
            << "    \"FontUnderline\": " << newStyle.singleLineCommentFormat.fontUnderline() << ",\n"
            << "    \"FontWeight\": " <<    newStyle.singleLineCommentFormat.fontWeight() << ",\n"
            << "    \"Foreground\": \"" <<  newStyle.singleLineCommentFormat.foreground().color().name() << "\"\n"
            << "  }\n"
            << "}";
        file.close();
    }

    styleName = QFileInfo(styleName).baseName();
    styles.insert(styleName, newStyle);
    styleVersion = styleName;
    updateStyleFormats();
}

void Highlighter::setStyle(std::istringstream& style, QString styleName) {

    QTextCharFormat newKeywordFormat;
    QTextCharFormat newClassFormat;
    QTextCharFormat newQuotationFormat;
    QTextCharFormat newFunctionFormat;
    QTextCharFormat newIncludeFormat;
    QTextCharFormat newSingleLineCommentFormat;
    QTextCharFormat newMultiLineCommentFormat;
    QTextCharFormat newSearchFormat;

    try {
        std::string str;
        style >> str;
        if (str != "{")
            throw std::invalid_argument("Данный формат стиля не поддерживается системой.");

        size_t styleNumber = 8;
        while (styleNumber > 0) {
            style >> str;
            if (str == "\"keywordFormat\":")
                setFormats(newKeywordFormat, style);
            else {
                if (str == "\"classFormat\":")
                    setFormats(newClassFormat, style);
                else {
                    if (str == "\"singleLineCommentFormat\":")
                        setFormats(newSingleLineCommentFormat, style);
                    else {
                        if (str == "\"multiLineCommentFormat\":")
                            setFormats(newMultiLineCommentFormat, style);
                        else {
                            if (str == "\"quotationFormat\":")
                                setFormats(newQuotationFormat, style);
                            else {
                                if (str == "\"functionFormat\":")
                                    setFormats(newFunctionFormat, style);
                                else {
                                    if (str == "\"includeFormat\":")
                                        setFormats(newIncludeFormat, style);
                                    else {
                                        if (str == "\"searchFormat\":")
                                            setFormats(newSearchFormat, style);
                                        else
                                            throw std::invalid_argument("Данный формат стиля не поддерживается системой.");
                                    }
                                }
                            }
                        }
                    }
                }
            }

            styleNumber--;
        }

        styles.insert(
            styleName,
            {
                newKeywordFormat,
                newClassFormat,
                newQuotationFormat,
                newFunctionFormat,
                newIncludeFormat,
                newSingleLineCommentFormat,
                newMultiLineCommentFormat,
                newSearchFormat
            }
        );
        styleVersion = styleName;
        updateStyleFormats();

    } catch (std::invalid_argument &exc) {
        QMessageBox *messageBox = new QMessageBox();
        messageBox->setText(exc.what());
        messageBox->show();
    }
}

void Highlighter::selectSearch(QString newSearchString) {
    searchString = newSearchString;
    updateStyleFormats();
}

void Highlighter::setActive(bool isActive) {
    this->isActive = isActive;
    updateStyleFormats();
}

Style Highlighter::getStyle() const {
    return styles.value(styleVersion);
}

void Highlighter::updateLanguageVersion() {
    if (isActive) {
        HighlightingRule rule;
        QVector<QString> keywordPatterns;
        if (languageVersion == LanguageVersion::C89) {
            keywordPatterns = {
                // C89.
                "\\bauto\\b", "\\bbreak\\b", "\\bcase\\b",
                "\\bchar\\b", "\\bconst\\b", "\\bcontinue\\b",
                "\\bdefault\\b", "\\bdo\\b", "\\bdouble\\b",
                "\\belse\\b", "\\benum\\b", "\\bextern\\b",
                "\\bfloat\\b", "\\bfor\\b", "\\bgoto\\b",
                "\\bif\\b", "\\bint\\b", "\\blong\\b",
                "\\bregister\\b", "\\breturn\\b", "\\bshort\\b",
                "\\bsigned\\b", "\\bsigeof\\b", "\\bstatic\\b",
                "\\bstruct\\b", "\\bswitch\\b", "\\btypedef\\b",
                "\\bunion\\b", "\\bunsigned\\b", "\\bvoid\\b",
                "\\bvolatile\\b", "\\bwhile\\b"
            };
        }

        if (languageVersion == LanguageVersion::CPP98_03) {
            keywordPatterns = {
                // C89.
                "\\bauto\\b", "\\bbreak\\b", "\\bcase\\b",
                "\\bchar\\b", "\\bconst\\b", "\\bcontinue\\b",
                "\\bdefault\\b", "\\bdo\\b", "\\bdouble\\b",
                "\\belse\\b", "\\benum\\b", "\\bextern\\b",
                "\\bfloat\\b", "\\bfor\\b", "\\bgoto\\b",
                "\\bif\\b", "\\bint\\b", "\\blong\\b",
                "\\bregister\\b", "\\breturn\\b", "\\bshort\\b",
                "\\bsigned\\b", "\\bsigeof\\b", "\\bstatic\\b",
                "\\bstruct\\b", "\\bswitch\\b", "\\btypedef\\b",
                "\\bunion\\b", "\\bunsigned\\b", "\\bvoid\\b",
                "\\bvolatile\\b", "\\bwhile\\b",

                // C++98/03 (new).
                "\\basm\\b", "\\bbool\\b", "\\bcatch\\b",
                "\\bclass\\b", "\\bconst_cast\\b", "\\bdelete\\b",
                "\\bdynamic_cast\\b", "\\bexplicit\\b", "\\bexport\\b",
                "\\bfalse\\b", "\\bfriend\\b", "\\binline\\b",
                "\\bmutable\\b", "\\bnamespace\\b", "\\bnew\\b",
                "\\boperator\\b", "\\bprivate\\b", "\\bprotected\\b",
                "\\bpublic\\b", "\\breinterpret_cast\\b", "\\bstatic_cast\\b",
                "\\btemplate\\b", "\\bthis\\b", "\\bthrow\\b",
                "\\btrue\\b", "\\btry\\b", "\\btypeid\\b",
                "\\btypename\\b", "\\busing\\b", "\\bvirtual\\b",
                "\\bwchar_t\\b"
            };
        }

        if (languageVersion == LanguageVersion::CPP11) {
            keywordPatterns = {
                // C89.
                "\\bauto\\b", "\\bbreak\\b", "\\bcase\\b",
                "\\bchar\\b", "\\bconst\\b", "\\bcontinue\\b",
                "\\bdefault\\b", "\\bdo\\b", "\\bdouble\\b",
                "\\belse\\b", "\\benum\\b", "\\bextern\\b",
                "\\bfloat\\b", "\\bfor\\b", "\\bgoto\\b",
                "\\bif\\b", "\\bint\\b", "\\blong\\b",
                "\\bregister\\b", "\\breturn\\b", "\\bshort\\b",
                "\\bsigned\\b", "\\bsigeof\\b", "\\bstatic\\b",
                "\\bstruct\\b", "\\bswitch\\b", "\\btypedef\\b",
                "\\bunion\\b", "\\bunsigned\\b", "\\bvoid\\b",
                "\\bvolatile\\b", "\\bwhile\\b",

                // C++98/03 (new).
                "\\basm\\b", "\\bbool\\b", "\\bcatch\\b",
                "\\bclass\\b", "\\bconst_cast\\b", "\\bdelete\\b",
                "\\bdynamic_cast\\b", "\\bexplicit\\b", "\\bexport\\b",
                "\\bfalse\\b", "\\bfriend\\b", "\\binline\\b",
                "\\bmutable\\b", "\\bnamespace\\b", "\\bnew\\b",
                "\\boperator\\b", "\\bprivate\\b", "\\bprotected\\b",
                "\\bpublic\\b", "\\breinterpret_cast\\b", "\\bstatic_cast\\b",
                "\\btemplate\\b", "\\bthis\\b", "\\bthrow\\b",
                "\\btrue\\b", "\\btry\\b", "\\btypeid\\b",
                "\\btypename\\b", "\\busing\\b", "\\bvirtual\\b",
                "\\bwchar_t\\b"

                // C++11 (new).
                "\\balignas\\b", "\\balignof\\b", "\\bchar16_t\\b",
                "\\bchar32_t\\b", "\\bconstexpr\\b", "\\bdecltype\\b",
                "\\bnoexcept\\b", "\\bnullptr\\b", "\\bstatic_assert\\b",
                "\\bthread_local\\b"
            };
        }

        for (const QString &pattern : keywordPatterns) {
            rule.pattern = QRegExp(pattern);
            rule.format = styles.value(styleVersion).keywordFormat;
            highlightingRules.append(rule);
        }
    }
}

void Highlighter::updateStyleFormats() {
    highlightingRules.clear();
    if (isActive) {
        HighlightingRule rule;

        Style style = styles.value(styleVersion);

        updateLanguageVersion();

        rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
        rule.format = style.classFormat;
        highlightingRules.append(rule);

        rule.pattern = QRegExp("\".*\"");
        rule.format = style.quotationFormat;
        highlightingRules.append(rule);

        rule.pattern = QRegExp("#include <.*>");
        rule.format = style.includeFormat;
        highlightingRules.append(rule);

        rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
        rule.format = style.functionFormat;
        highlightingRules.append(rule);

        rule.pattern = QRegExp("//[^\n]*");
        rule.format = style.singleLineCommentFormat;
        highlightingRules.append(rule);

        commentStartExpression = QRegExp("/\\*");
        commentEndExpression = QRegExp("\\*/");

        if (!searchString.isEmpty()) {
            style.searchFormat.setBackground(QColor(Qt::red).lighter(160));
            rule.pattern = QRegExp(searchString, Qt::CaseSensitive, QRegExp::FixedString);
            rule.format = style.searchFormat;
            highlightingRules.append(rule);
        }
    }
}

void Highlighter::setFormats(QTextCharFormat& format, std::istringstream& style) {
    std::string str;

    style >> str;
    if (str != "{")
        throw std::invalid_argument("Данный формат стиля не поддерживается системой.");

    size_t styleNumber = 4;
    while (styleNumber > 0) {
        if(!(style >> str)) {
            break;
        }
        if (str == "\"Foreground\":") {
            style >> str;
            std::string foreground = str.substr(1,7);
            format.setForeground(QBrush(QColor(QString(foreground.c_str()))));
        } else {
            if (str == "\"FontWeight\":") {
                int fontWeight;
                if (!(style >> fontWeight))
                    throw std::invalid_argument("Данный формат стиля не поддерживается системой.");
                style >> str;
                if (str != ",")
                    throw std::invalid_argument("Данный формат стиля не поддерживается системой.");
                format.setFontWeight(fontWeight);
            } else {
                if (str == "\"FontItalic\":") {
                    style >> str;
                    if (str == "true," || str == "true" || str == "1," || str == "1") {
                        format.setFontItalic(true);
                    } else {
                        if (str == "false," || str == "false" || str == "0," || str == "0") {
                            format.setFontItalic(false);
                        } else {
                            throw std::invalid_argument("Данный формат стиля не поддерживается системой.");
                        }
                    }
                } else {
                    if (str == "\"FontUnderline\":") {
                        style >> str;
                        if (str == "true," || str == "true" || str == "1," || str == "1") {
                            format.setFontUnderline(true);
                        } else {
                            if (str == "false," || str == "false" || str == "0," || str == "0") {
                                format.setFontUnderline(false);
                            } else {
                                throw std::invalid_argument("Данный формат стиля не поддерживается системой.");
                            }
                        }
                    } else {
                        throw std::invalid_argument("Данный формат стиля не поддерживается системой.");
                    }
                }
            }
        }
        styleNumber--;
    }
    style >> str;
}
