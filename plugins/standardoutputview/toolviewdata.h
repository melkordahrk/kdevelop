/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef TOOLVIEWDATA_H
#define TOOLVIEWDATA_H

#include <outputview/ioutputview.h>
#include <QObject>
#include <QMap>

namespace Sublime
{
class View;
}

class QItemDelegate;
class ToolViewData;
class QString;
class QAbstractItemModel;
class QListView;
class StandardOutputView;

class OutputData : public QObject
{
Q_OBJECT
public:
    OutputData( ToolViewData* tv );
    QAbstractItemDelegate* delegate;
    QAbstractItemModel* model;
    int id;
    ToolViewData* toolView;
    KDevelop::IOutputView::Behaviours behaviour;
    QString title;
    QListView* view;
    void setModel( QAbstractItemModel* model);
    void setDelegate( QAbstractItemDelegate* delegate);
signals:
    void modelChanged( int );
    void delegateChanged( int );
};

class ToolViewData : public QObject
{
Q_OBJECT
public:
    ToolViewData( QObject* parent );
    ~ToolViewData();
    int toolViewId;
    OutputData* addOutput( int id, const QString& title, KDevelop::IOutputView::Behaviours behave );
    QList<Sublime::View*> views;
    StandardOutputView* plugin;
    QMap<int, OutputData*> outputdata;
    KDevelop::IOutputView::ViewType type;
    QString title;
signals:
    void outputAdded( int );
};

#endif
