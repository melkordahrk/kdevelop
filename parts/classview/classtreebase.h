/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CLASSTREEBASE_H_
#define _CLASSTREEBASE_H_

#include <klistview.h>
#include "parseditem.h"
#include "parsedscopecontainer.h"
#include "parsedclass.h"
#include "parsedstruct.h"
#include "parsedmethod.h"
#include "parsedattribute.h"
#include "classviewpart.h"

class ClassTreeItem;
class KPopupMenu;


class ClassTreeBase : public KListView
{
    Q_OBJECT
    
public: 
    ClassTreeBase( ClassViewPart *part, QWidget *parent=0, const char *name=0 );
    ~ClassTreeBase();

protected:
    typedef QValueList<QStringList> TreeState;
    typedef QValueList<QStringList>::Iterator TreeStateIterator;
    TreeState treeState() const;
    void setTreeState(TreeState state);
    
    ClassTreeItem *contextItem;
    virtual KPopupMenu *createPopup() = 0;
    
private slots:
    void slotItemExecuted(QListViewItem*);
    void slotItemPressed(int button, QListViewItem *item);
    void slotContextMenuRequested(QListViewItem *item, const QPoint &p);
    void slotGotoDeclaration();
    void slotGotoImplementation();
    void slotAddMethod();
    void slotAddAttribute();
    void slotClassBaseClasses();
    void slotClassDerivedClasses();
    void slotClassTool();
    
protected:
    ClassViewPart *m_part;
    friend class ClassTreeItem;
    friend class ClassTreeScopeItem;
};


class ClassTreeItem : public QListViewItem
{
public:
    ClassTreeItem( ClassTreeBase *parent, ClassTreeItem *lastSibling, ParsedItem *parsedItem )
        : QListViewItem(parent, lastSibling), m_item(parsedItem)
    {
    }
    ClassTreeItem( ClassTreeItem *parent, ClassTreeItem *lastSibling, ParsedItem *parsedItem )
        : QListViewItem(parent, lastSibling), m_item(parsedItem)
    {
    }
    ~ClassTreeItem()
    {
    }

    KPopupMenu *createPopup();
    bool isOrganizer() { return !m_item; }
    
    void getDeclaration(QString *toFile, int *toLine);
    void getImplementation(QString *toFile, int *toLine);

    virtual QString scopedText() const;
    virtual QString text( int ) const;
    virtual QString tipText() const;
    
protected:
    ClassTreeBase *classTree()
        { return static_cast<ClassTreeBase*>(listView()); }
    ParsedItem *m_item;
};


class ClassTreeOrganizerItem : public ClassTreeItem
{
public:
    ClassTreeOrganizerItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                            const QString &text )
        : ClassTreeItem(parent, lastSibling, 0 )
        , m_text( text )
        { init(); }
    ClassTreeOrganizerItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                            const QString &text )
        : ClassTreeItem(parent, lastSibling, 0 )
        , m_text( text )
        { init(); }
    ~ClassTreeOrganizerItem()
        {}

    virtual QString text( int ) const { return m_text; }
    
private:
    QString m_text;
    
    void init();
};


class ClassTreeScopeItem : public ClassTreeItem
{
public:
    ClassTreeScopeItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                        ParsedScopeContainer *parsedScope )
        : ClassTreeItem(parent, lastSibling, 0)
    { 
      if ( parsedScope )
        m_item = new ParsedScopeContainer( *parsedScope );
      init();
    }
    ClassTreeScopeItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                        ParsedScopeContainer *parsedScope )
        : ClassTreeItem(parent, lastSibling, 0)
    { 
      if ( parsedScope )
        m_item = new ParsedScopeContainer( *parsedScope );
      init();
    }
    ~ClassTreeScopeItem()
    {
      delete (ParsedScopeContainer*)m_item;
    }

    virtual QString text( int ) const;
    virtual void setOpen(bool o);
    
private:
    void init();
};


class ClassTreeClassItem : public ClassTreeItem
{
public:
    ClassTreeClassItem( ClassTreeBase *parent, ClassTreeItem *lastSibling,
                        ParsedClass *parsedClass )
        : ClassTreeItem(parent, lastSibling, 0)
        {
          if ( parsedClass )
            m_item = new ParsedClass( *parsedClass );
          init();
        }
    ClassTreeClassItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                        ParsedClass *parsedClass )
        : ClassTreeItem(parent, lastSibling, 0)
        {
          if ( parsedClass )
            m_item = new ParsedClass( *parsedClass );
          init();
        }
    ~ClassTreeClassItem()
        {
          delete (ParsedClass*)m_item;
        }

    virtual void setOpen(bool o);

private:
    void init();
};


class ClassTreeStructItem : public ClassTreeItem
{
public:
    ClassTreeStructItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                         ParsedStruct *parsedStruct );
    ~ClassTreeStructItem()
        {
          delete (ParsedStruct*)m_item;
        }

    virtual void setOpen(bool o);
};


class ClassTreeMethodItem : public ClassTreeItem
{
public:
    ClassTreeMethodItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                         ParsedMethod *parsedMethod );
    ~ClassTreeMethodItem()
        {
          delete (ParsedMethod*)m_item;
        }

    virtual QString text( int ) const;
};


class ClassTreeAttrItem : public ClassTreeItem
{
public:
    ClassTreeAttrItem( ClassTreeItem *parent, ClassTreeItem *lastSibling,
                       ParsedAttribute *parsedAttr );
    ~ClassTreeAttrItem()
        {
          delete (ParsedAttribute*)m_item;
        }

    virtual QString text( int ) const;
};

#endif
