#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QTextBrowser>

/* I wanted to use QTextCursor, spent some time on it but did not manage to understand how it works...
 * So Qt example with QSintaxHighlighter helped me :) (did everything instead of me) */


class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Highlighter(QString word, QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QString word;

    QTextCharFormat keywordFormat;
};

#endif // HIGHLIGHTER_H
