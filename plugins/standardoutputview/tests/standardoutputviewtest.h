/*
    Copyright (C) 2011  Silvère Lestang <silvere.lestang@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef STANDARDOUTPUTVIEWTEST_H
#define STANDARDOUTPUTVIEWTEST_H

#include <QtCore/QObject>

#include "../standardoutputview.h"
#include <shell/uicontroller.h>

namespace KDevelop
{
    class TestCore;
}

namespace Sublime
{
    class View;
    class Controller;
    class ToolDocument;
    class Area;
}

class StandardOutputViewTest: public QObject
{
    Q_OBJECT

public:   

private:
    bool toolviewExist(QString toolviewTitle);
    KDevelop::TestCore* m_testCore;
    StandardOutputView* m_stdOutputView;
    //Sublime::Controller* m_controller;
    KDevelop::UiController* m_controller;
    Sublime::Area* m_area;
    Sublime::ToolDocument* m_tool;
    int toolviewId;
    int ouputId;
    static const QString toolviewTitle;
    
private slots:
    void init();
    void cleanupTestCase();
    void testRegisterAndRemoveToolView();
};

#endif // STANDARDOUTPUTVIEWTEST_H
