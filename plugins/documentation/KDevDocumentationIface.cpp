/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@mksat.net                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *                                                                         *
 ***************************************************************************/
#include "KDevDocumentationIface.h"

#include "documentation_part.h"

KDevDocumentationIface::KDevDocumentationIface(DocumentationPart *part)
    :QObject(part), DCOPObject("KDevDocumentation"), m_part(part)
{
}

KDevDocumentationIface::~KDevDocumentationIface()
{
}

void KDevDocumentationIface::lookupInIndex(QString term)
{
    m_part->lookInDocumentationIndex(term);
}

void KDevDocumentationIface::findInFinder(QString term)
{
    m_part->findInDocumentation(term);
}

void KDevDocumentationIface::searchInDocumentation(QString term)
{
    m_part->searchInDocumentation(term);
}

void KDevDocumentationIface::lookupInIndex()
{
    m_part->lookInDocumentationIndex();
}

void KDevDocumentationIface::searchInDocumentation()
{
    m_part->searchInDocumentation();
}

void KDevDocumentationIface::manPage(QString term)
{
    m_part->manPage(term);
}

void KDevDocumentationIface::infoPage(QString term)
{
    m_part->infoPage(term);
}

void KDevDocumentationIface::manPage()
{
    m_part->manPage();
}

void KDevDocumentationIface::infoPage()
{
    m_part->infoPage();
}

void KDevDocumentationIface::findInFinder( )
{
    m_part->findInDocumentation();
}

#include "KDevDocumentationIface.moc"
