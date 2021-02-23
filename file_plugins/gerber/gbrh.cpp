// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* *
* Author : Damir Bakiev *
* Version : na *
* Date : 14 January 2021 *
* Website : na *
* Copyright : Damir Bakiev 2016-2021 *
* *
* License: *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt *
* *
*******************************************************************************/
#include "gbrh.h"
#include "mvector.h"
#include <ctre.hpp>

namespace Gerber {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    auto data = reinterpret_cast<const char16_t*>(text.data());

    static QTextCharFormat myClassFormat;
    //myClassFormat.setFontWeight(QFont::Bold);

    static constexpr auto pattern2 = ctll::fixed_string("^%.+\\*%$");
    for (auto m : ctre::range<pattern2>(text)) {
        myClassFormat.setForeground(QColor(0xFF, 0xFF, 0x00));
        setFormat(std::distance(data, m.data()), static_cast<int>(m.size()), myClassFormat);
        return;
    }

    static const std::map<QChar, QColor> color {
        { 'D', QColor(0x00, 0xFF, 0xFF) },
        { 'G', QColor(0x7F, 0x7F, 0x7F) },
        { 'I', QColor(0x00, 0x00, 0xFF) },
        { 'J', QColor(0xFF, 0x00, 0xFF) },
        { 'M', QColor(0x00, 0xFF, 0xFF) },
        { 'X', QColor(0xFF, 0x00, 0x00) },
        { 'Y', QColor(0x00, 0xFF, 0x00) },
    };
    using namespace std::string_view_literals;
    static constexpr auto pattern = ctll::fixed_string("[DGIJMXY][\\+\\-]?\\d+\\.?\\d*");
    for (auto m : ctre::range<pattern>(text)) {
        myClassFormat.setForeground(color.at(*m.data()));
        setFormat(std::distance(data, m.data()), static_cast<int>(m.size()), myClassFormat);
    }
}
}
