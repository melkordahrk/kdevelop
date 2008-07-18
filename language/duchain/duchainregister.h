/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef TYPEREGISTER_H
#define TYPEREGISTER_H

#include "duchainbase.h"

namespace KDevelop {
class DUChainBase;
class DUChainBaseData;
/**
 * \short A factory class for type data.
 *
 * This class provides an interface for creating private data
 * structures for classes.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainBaseFactory {
  public:
  /**
   * Create a new duchain item for the given \a data.
   */
  virtual DUChainBase* create(DUChainBaseData* data) const = 0;

  /**
   * Release the \a data from an item which is being destroyed.
   */
  virtual void callDestructor(DUChainBaseData* data) const = 0;


  /**
   * Copy data \a from one type \a to another.
   *
   * \param from data to copy from
   * \param to data to copy to
   * \param constant set to true if \a to is to be a static data type, or false if \a to is to be a dynamic data type.
   *
   * \todo David, please check this
   */
//   virtual void copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const = 0;

  /**
   * Return the memory size of the given private \a data, including dynamic data.
   *
   * \param data data structure
   * \returns the size in memory of the data.
   */
  virtual uint dynamicSize(const DUChainBaseData& data) const = 0;

  /// Destructor.
  virtual ~DUChainBaseFactory() {
  }
};

/**
 * Template class to implement factories for each DUChainBase subclass you want
 * to instantiate.
 */
template<class T, class Data>
class KDEVPLATFORMLANGUAGE_EXPORT TypeFactory : public DUChainBaseFactory {
  public:
  DUChainBase* create(DUChainBaseData* data) const {
    return new T(*static_cast<Data*>(data));
  }
/*  void copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const {
    Q_ASSERT(from.classId == T::Identity);

    if((bool)from.m_dynamic == (bool)!constant) {
      //We have a problem, "from" and "to" should both be either dynamic or constant. We must copy once more.
      Data* temp = &DUChainBase::copyData<Data>(static_cast<const Data&>(from));

      new (&to) Data(*temp); //Call the copy constructor to initialize the target

      Q_ASSERT((bool)to.m_dynamic == (bool)!constant);
      delete temp;
    }else{
      new (&to) Data(static_cast<const Data&>(from)); //Call the copy constructor to initialize the target
    }
  }*/
  void callDestructor(DUChainBaseData* data) const {
    Q_ASSERT(data->classId == T::Identity);
    static_cast<Data*>(data)->~Data();
  }

  uint dynamicSize(const DUChainBaseData& data) const {
    Q_ASSERT(data.classId == T::Identity);
    return static_cast<const Data&>(data).dynamicSize();
  }
};

/**
 * \short A class which registers data types and creates factories for them.
 *
 * DUChainItemSystem is a global static class which allows you to register new
 * DUChainBase subclasses for creation.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainItemSystem {
  public:
    /**
     * Register a new DUChainBase subclass.
     */
    template<class T, class Data>
    void registerTypeClass() {
      if(m_factories.size() <= T::Identity) {
        m_factories.resize(T::Identity+1);
        m_dataClassSizes.resize(T::Identity+1);
      }

      Q_ASSERT(!m_factories[T::Identity]);
      m_factories[T::Identity] = new TypeFactory<T, Data>();
      m_dataClassSizes[T::Identity] = sizeof(Data);
    }

    /**
     * Unregister an DUChainBase subclass.
     */
    template<class T, class Data>
    void unregisterTypeClass() {
      Q_ASSERT(m_factories.size() > T::Identity);
      Q_ASSERT(m_factories[T::Identity]);
      delete m_factories[T::Identity];
      m_factories[T::Identity] = 0;
      m_dataClassSizes[T::Identity] = 0;
    }

    /**
     * Create an DUChainBase for the given data. The returned type must be put into a DUChainBase::Ptr immediately.
     * Can return null if no type-factory is available for the given data (for example when a language-part is not loaded)
     */
    DUChainBase* create(DUChainBaseData* data) const;

    /**
     * This just calls the correct constructor on the target. The target must be big enough to hold all the data.
     */
//     void copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const;

    ///Calls the dynamicSize(..) member on the given data, in the most special class. Since we cannot use virtual functions, this is the only way.
    uint dynamicSize(const DUChainBaseData& data) const;

    ///Returns the size of the derived class, not including dynamic data.
    ///Returns zero if the class is not known.
    size_t dataClassSize(const DUChainBaseData& data) const;

    ///Calls the destructor, but does not delete anything. This is needed because the data classes must not contain virtual members.
    void callDestructor(DUChainBaseData* data) const;

    /// Access the static DUChainItemSystem instance.
    static DUChainItemSystem& self();

  private:
    QVector<DUChainBaseFactory*> m_factories;
    QVector<size_t> m_dataClassSizes;
};

/// Helper class to register an DUChainBase subclass.
///
/// Just use the REGISTER_TYPE(YourTypeClass) macro in your code, and you're done.
template<class T, class Data>
struct KDEVPLATFORMLANGUAGE_EXPORT DUChainItemRegistrator {
  DUChainItemRegistrator() {
    DUChainItemSystem::self().registerTypeClass<T, Data>();
  }
  ~DUChainItemRegistrator() {
    DUChainItemSystem::self().unregisterTypeClass<T, Data>();
  }
};

///You must add this into your source-files for every DUChainBase based class
///For this to work, the class must have an "Identity" enumerator.
///It should be a unique value, but as small as possible, because a buffer at least as big as that number is created internally.
#define REGISTER_DUCHAIN_ITEM(Class) DUChainItemRegistrator<Class, Class ## Data> register ## Class
#define REGISTER_DUCHAIN_ITEM_WITH_DATA(Class, Data) DUChainItemRegistrator<Class, Data> register ## Class

}

#endif
