/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "highlighter.h"

//! [0]
Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(QColor("#f92672"));//#66d9ef
    //keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bprint\\b" << "\\bimport\\b" << "\\bfrom\\b"
                    << "\\band\\b" << "\\bdel\\b" << "\\bwhile\\b"
                    << "\\bnot\\b" << "\\bas\\b" << "\\belif\\b"
                    << "\\bglobal\\b" << "\\bor\\b" << "\\bwith\\b"
                    << "\\bassert\\b" << "\\belse\\b" << "\\bif\\b"
                    << "\\bpass\\b" << "\\byield\\b" << "\\bbreak\\b"
                    << "\\bexcept\\b" << "\\bexec\\b" << "\\bin\\b"
                    << "\\braise\\b" << "\\bcontinue\\b" << "\\bfinally\\b"
                    << "\\bis\\b" << "\\breturn\\b" << "\\bfor\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
//! [1]
    keywordFormat.setForeground(QColor("#66d9ef"));
    QStringList specKeywordPatterns;
    specKeywordPatterns << "\\bdef\\b"<< "\\blambda\\b"<< "\\bclass\\b"<<"\\b__init__\\b"
                           <<"\\bsuper\\b";
    foreach (const QString &pattern, specKeywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    keywordFormat.setForeground(QColor("#d07f1f"));
    QStringList spec2KeywordPatterns;
    spec2KeywordPatterns << "\\bself\\b"<< "\\bTrue\\b"<< "\\bFalse\\b";
    foreach (const QString &pattern, spec2KeywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
//! [2]
    numbersFormat.setForeground(QColor("#ae81ff"));
    rule.pattern = QRegExp("\\b[0-9]+\\b");//rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = numbersFormat;
    highlightingRules.append(rule);
//! [2]
//! [5]
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(QColor("#a6e22e"));
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
//! [5]

//! [3]
    singleLineCommentFormat.setForeground(QColor("#75715e"));
    rule.pattern = QRegExp("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(QColor("#e6db74"));
//! [3]

//! [4]
    quotationFormat.setForeground(QColor("#e6db74"));
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    singleQuotationFormat.setForeground(QColor("#e6dd84"));
    rule.pattern = QRegExp("\'.*\'");
    rule.format = singleQuotationFormat;
    highlightingRules.append(rule);
//! [4]



//! [6]
    commentStartExpression = QRegExp("\'");
    commentEndExpression = QRegExp("\'");
}
//! [6]

//! [7]
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
//! [7] //! [8]
    setCurrentBlockState(0);
//! [8]
/*
//! [9]
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

//! [9] //! [10]
    while (startIndex >= 0) {
//! [10] //! [11]
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
    */
}
//! [11]
