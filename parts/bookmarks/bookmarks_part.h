/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef __KDEVPART_BOOKMARKS_H__
#define __KDEVPART_BOOKMARKS_H__


#include <qguardedptr.h>
#include <qvaluelist.h>
#include <qdict.h>
#include <qpair.h>

#include <kparts/part.h>
#include <kurl.h>

#include <kdevplugin.h>

struct EditorData
{
	KURL url;
	QValueList< QPair<int,QString> > marks;
};

class BookmarksWidget;
class QDomElement;

class BookmarksPart : public KDevPlugin
{
	Q_OBJECT

public:

	BookmarksPart(QObject *parent, const char *name, const QStringList &);
	~BookmarksPart();

	// reimplemented from KDevPlugin
	void restorePartialProjectSession( const QDomElement * el );
	void savePartialProjectSession( QDomElement * el );

private slots:
	// connected to partcontroller
	void partAdded( KParts::Part * part );

	// connected to KTextEditor::MarkInterface
	void marksChanged();

	// connected to KParts::ReadOnlyPart
	void reload();

	// connected to BookmarksWidget
	void removeAllBookmarksForURL( const KURL & );
	void removeBookmarkForURL( const KURL &, int );

private:
	bool setBookmarksForURL( KParts::ReadOnlyPart * );
	bool clearBookmarksForURL( KParts::ReadOnlyPart * );
	void setBookmarksForAllURLs();

	EditorData * storeBookmarksForURL( KParts::ReadOnlyPart * );
	void storeBookmarksForAllURLs();

	void updateContextStringForURL( KParts::ReadOnlyPart * );
	void updateContextStringForURL( KURL const & url );
	void updateContextStringForAll();

	KParts::ReadOnlyPart * partForURL( KURL const & url );

	QGuardedPtr<BookmarksWidget> _widget;
	QDict<EditorData> _editorMap;
	uint _context;	// the number of lines of 'context' to use in bookmark tooltip
	bool _settingMarks;	//	are we currently in the process of setting bookmarks?
};


#endif
