/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_ARRAYTYPE_H
#define KDEVPLATFORM_ARRAYTYPE_H

#include "abstracttype.h"

namespace KDevelop {
class ArrayTypeData;

class KDEVPLATFORMLANGUAGE_EXPORT ArrayType
    : public AbstractType
{
public:
    typedef TypePtr<ArrayType> Ptr;

    /// Default constructor
    ArrayType();
    /// Copy constructor. \param rhs type to copy
    ArrayType(const ArrayType& rhs);
    /// Constructor using raw data. \param data internal data.
    explicit ArrayType(ArrayTypeData& data);
    /// Destructor
    ~ArrayType() override;

    AbstractType* clone() const override;

    bool equals(const AbstractType* rhs) const override;

    /**
     * Retrieve the dimension of this array type. Multiple-dimensioned
     * arrays will have another array type as their elementType().
     *
     * \returns the dimension of the array, or zero if the array is dimensionless (eg. int[])
     */
    int dimension () const;

    /**
     * Set this array type's dimension.
     * If @p dimension is zero, the array is considered dimensionless (eg. int[]).
     *
     * \param dimension new dimension, set to zero for a dimensionless type (eg. int[])
     */
    void setDimension(int dimension);

    /**
     * Retrieve the element type of the array, e.g. "int" for int[3].
     *
     * \returns the element type.
     */
    AbstractType::Ptr elementType () const;

    /**
     * Set the element type of the array, e.g. "int" for int[3].
     */
    void setElementType(const AbstractType::Ptr& type);

    QString toString() const override;

    uint hash() const override;

    WhichType whichType() const override;

    void exchangeTypes(TypeExchanger* exchanger) override;

    enum {
        Identity = 7
    };

    typedef ArrayTypeData Data;

protected:
    void accept0 (TypeVisitor* v) const override;

    TYPE_DECLARE_DATA(ArrayType)
};

template <>
inline ArrayType* fastCast<ArrayType*>(AbstractType* from)
{
    if (!from || from->whichType() != AbstractType::TypeArray)
        return nullptr;
    else
        return static_cast<ArrayType*>(from);
}
}

#endif // KDEVPLATFORM_TYPESYSTEM_H
