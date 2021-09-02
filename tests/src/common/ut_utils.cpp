/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author:     zhangteng <zhangteng@uniontech.com>
* Maintainer: zhangteng <zhangteng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ut_utils.h"
#include "utils.h"
#include "datatypedef.h"
#include "vnoteitem.h"

#include <DStyle>
#include <DApplicationHelper>

ut_utils_test::ut_utils_test()
{
}

void ut_utils_test::SetUp()
{
    m_utils = new Utils;
}

void ut_utils_test::TearDown()
{
    delete m_utils;
}

TEST_F(ut_utils_test, convertDateTime)
{
    QDateTime currDateTime = QDateTime::currentDateTime();
    currDateTime = currDateTime.addSecs(-10);
    EXPECT_EQ(DApplication::translate("Utils", "1 min ago"), m_utils->convertDateTime(currDateTime));
    EXPECT_EQ(DApplication::translate("Utils", "1 min ago"), m_utils->convertDateTime(currDateTime.addSecs(-70)));
    currDateTime = currDateTime.addSecs(-3600);
    EXPECT_EQ(currDateTime.toString("hh:mm"), m_utils->convertDateTime(currDateTime));
    currDateTime = currDateTime.addDays(-1);
    for (int i = 1; i <= 777; i++) {
        QDateTime dateTime = currDateTime.addDays(-1 * i);
        if (QDate::currentDate().year() == dateTime.date().year()) {
            EXPECT_EQ(dateTime.toString("MM-dd"), m_utils->convertDateTime(dateTime)) << i << " days age";
        } else {
            EXPECT_EQ(dateTime.toString("yyyy-MM-dd"), m_utils->convertDateTime(dateTime)) << i << " days age";
        }
    }
}

TEST_F(ut_utils_test, loadSVG)
{
    QString fileName = "play.svg";
    EXPECT_TRUE(m_utils->loadSVG(fileName, false).isNull()) << "fCommon = false";
    EXPECT_FALSE(m_utils->loadSVG(fileName, true).isNull()) << "fCommon = true";
}

TEST_F(ut_utils_test, highTextEdit)
{
    QTextDocument document;
    document.setPlainText("testeee");
    QString searchKey = "test";
    DPalette pb;
    QColor highColor = pb.color(DPalette::Highlight);
    EXPECT_EQ(1, m_utils->highTextEdit(&document, searchKey, highColor)) << "searchKey is 'test'";
    EXPECT_EQ(2, m_utils->highTextEdit(&document, "t", highColor, true)) << "searchKey is 't'";
    EXPECT_EQ(4, m_utils->highTextEdit(&document, "e", highColor, true)) << "searchKey is 'e'";
}

TEST_F(ut_utils_test, setDefaultColor)
{
    QTextDocument document;
    DPalette pb;
    QColor highColor = pb.color(DPalette::Highlight);
    m_utils->setDefaultColor(&document, highColor);
}

TEST_F(ut_utils_test, blockToDocument)
{
    VNTextBlock block;
    QTextDocument document;
    m_utils->blockToDocument(&block, &document);
}

TEST_F(ut_utils_test, documentToBlock)
{
    VNTextBlock block;
    QTextDocument document;
    block.blockText = "abc";
    document.setPlainText("test");
    m_utils->documentToBlock(&block, &document);
    EXPECT_EQ(document.toPlainText(), block.blockText);
}

TEST_F(ut_utils_test, formatMillisecond)
{
    qint64 tmpvoicesize = 890;
    m_utils->formatMillisecond(tmpvoicesize, 4);
    EXPECT_EQ(QDateTime::fromTime_t(static_cast<uint>(4)).toUTC().toString("mm:ss"), m_utils->formatMillisecond(tmpvoicesize, 4));
    for (int i = 5301; i < 7300000; i *= 1.5) {
        if (i < 3600000) {
            EXPECT_EQ(QDateTime::fromTime_t(static_cast<uint>(i / 1000)).toUTC().toString("mm:ss"), m_utils->formatMillisecond(i, 4));
        } else {
            EXPECT_EQ("60:00", m_utils->formatMillisecond(i, 4));
        }
    }
    tmpvoicesize = 18901111;
    EXPECT_EQ("60:00", m_utils->formatMillisecond(tmpvoicesize));
}
