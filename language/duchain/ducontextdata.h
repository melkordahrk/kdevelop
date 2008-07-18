/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2006 Hamish Rodda <rodda@kde.org>                       *
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

#ifndef ducontext_p_H
#define ducontext_p_H

#include <QMutex>
#include <QMultiHash>
#include <QMap>

#include "duchainbase.h"
#include "ducontext.h"
#include "duchainpointer.h"
#include "simplecursor.h"
#include "declaration.h"
#include "use.h"
#include "../languageexport.h"

namespace KTextEditor {
  class SmartRange;
}


namespace KDevelop{
class DUContext;

///This class contains data that needs to be stored to disk
class KDEVPLATFORMLANGUAGE_EXPORT DUContextData : public DUChainBaseData
{
public:
  DUContextData();
  DUContext::ContextType m_contextType;
  QualifiedIdentifier m_scopeIdentifier;
  IndexedDeclaration m_owner;
  QVector<DUContext::Import> m_importedContexts;
  QVector<IndexedDUContext> m_childContexts;
  QVector<IndexedDUContext> m_importers;

  ///@warning: Whenever m_localDeclarations is read or written, DUContextDynamicData::m_localDeclarationsMutex must be locked.
  QVector<IndexedDeclaration> m_localDeclarations;
  /**
   * Vector of all uses in this context
   * Mutable for range synchronization
   * */
  mutable QVector<Use> m_uses;

  bool m_inSymbolTable : 1;
  bool m_anonymousInParent : 1; //Whether this context was added anonymously into the parent. This means that it cannot be found as child-context in the parent.
  bool m_propagateDeclarations : 1;
};

///This class contains data that is only runtime-dependant and does not need to be stored to disk
class DUContextDynamicData
{
public:
  DUContextDynamicData( DUContext* );
  DUContextPointer m_parentContext;

  TopDUContext* m_topContext;
  
  //Use DeclarationPointer instead of declaration, so we can locate management-problems
  typedef QMultiHash<Identifier, DeclarationPointer> DeclarationsHash;
  
  static QMutex m_localDeclarationsMutex;
  ///@warning: Whenever m_localDeclarations is read or written, m_localDeclarationsHash must be locked.
  DeclarationsHash m_localDeclarationsHash; //This hash can contain more declarations than m_localDeclarations, due to declarations propagated up from children.
  
  uint m_indexInTopContext; //Index of this DUContext in the top-context
  
  /**
   * If this document is loaded, this contains a smart-range for each use.
   * This may temporarily contain zero ranges.
   * */
  mutable QVector<KTextEditor::SmartRange*> m_rangesForUses;
  
  DUContext* m_context;

  mutable bool m_rangesChanged : 1;
   /**
   * Adds a child context.
   *
   * \note Be sure to have set the text location first, so that
   * the chain is sorted correctly.
   */
  void addChildContext(DUContext* context);
  
  /**Removes the context from childContexts
   * @return Whether a context was removed
   * */
  bool removeChildContext(DUContext* context);

  void addImportedChildContext( DUContext * context );
  void removeImportedChildContext( DUContext * context );

  void addDeclaration(Declaration* declaration);
  
  /**Removes the declaration from localDeclarations
   * @return Whether a declaration was removed
   * */
  bool removeDeclaration(Declaration* declaration);

  //Files the scope identifier into target
  void scopeIdentifier(bool includeClasses, QualifiedIdentifier& target) const;
  
  /**
   * This propagates the declaration into the parent search-hashes,
   * up to the first parent that has m_propagateDeclarations set to false.
   * 
   * Must be called with m_localDeclarationsMutex locked
  */
  void addDeclarationToHash(const Identifier& identifer, Declaration* declaration);
  ///Must be called with m_localDeclarationsMutex locked
  void removeDeclarationFromHash(const Identifier& identifer, Declaration* declaration);
  
  /**
   * Returns true if this context is imported by the given one, on any level.
   * */
  bool isThisImportedBy(const DUContext* context) const;
};

}


#endif

//kate: space-indent on; indent-width 2; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
