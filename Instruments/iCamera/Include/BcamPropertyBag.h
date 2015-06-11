//-----------------------------------------------------------------------------
//  (c) 2002 by Basler Vision Technologies
//  Section:  Vision Components
//  Project:  BCAM
//  $Header: /home/moos/REPOSITORY/MOOS/iCamera/Include/Attic/BcamPropertyBag.h,v 1.1.2.1 2004/07/27 19:56:00 moosauthor Exp $
//-----------------------------------------------------------------------------
/**
  \file     BcamPropertyBag.h
 *
  \brief   Interface for the property bag classes
 *
 * Property bags are used save and restore a Bcam device's configuration 
 */
//-----------------------------------------------------------------------------
#pragma once

#include "BcamException.h"

#pragma warning (disable: 4996) // 4996: ... was declared deprecated
#if defined(USE_MFC)
#include <afxwin.h>
#else
#include <windows.h>
#endif


namespace Bcam
{


class CPropertyBagPtr;  // smart pointer encapsulating a property bag

//------------------------------------------------------------------------------
// class CPropertyBag
// Author: A.Happe
// Date: 04.09.2002
//------------------------------------------------------------------------------
/**
 * \brief   Abstract base class for property bag 
 *
 * A property bag is used to store key - value pairs.
 * Concrete implementations might use .ini files, .xml files or the registry 
 * to realize property bags.
 * 
 *
 */
//------------------------------------------------------------------------------

class CPropertyBag
{
public:
  /// Ask the property bag to create a new one. File based implementations should
  /// create a property bag which is bound to the same file
  virtual CPropertyBagPtr CreateBag(const CString&  Name) = 0;
  /// Ask the property bag for an existing property bag.
  virtual CPropertyBagPtr GetBag(const CString&  Name) = 0;

  // reading and writing of key value pairs

  /// Write a bool to the property bag
  virtual void WriteBool(const CString& name, const bool value);

  /// Read a bool from the property bag
  virtual bool ReadBool(const CString& name);

  /// Write a long to the property bag
  virtual void WriteLong(const CString& name, const long value);

  /// Read a long to the property bag
  virtual long ReadLong(const CString& name);

  /// Write a float to the property bag
  virtual void WriteFloat(const CString& name, const float value);

  /// Read a float from the property bag
  virtual float ReadFloat(const CString& name);

  /// Write a string to the property bag
  virtual void WriteString(const CString& name, const CString value);

  /// Read a string from the property bag
  virtual CString ReadString(const CString& name);


  /// Increments the reference count
  ULONG AddRef()
  {
    return InterlockedIncrement( (PLONG)&m_RefCount );
  }
  /// Decrements the reference count
  ULONG Release()
  { 
    const ULONG r = InterlockedDecrement( (PLONG)&m_RefCount );
    if (r == 0)
      delete this;
    return r;
  }

  virtual ~CPropertyBag() {};

protected:
  // Hide the constructor. Concrete property bag implementations should provide
  // factory methods like Create() or Open().
  CPropertyBag()  { m_RefCount = 0; }

  // overwrite these methods to implement the read and write mechanism
  /// Write a property. The function must return 0 if the operation succeeds.
  virtual DWORD WriteProperty(const CString& name, const CString& value) = 0;
  /// Read a property. The function must return 0 if the operation succeeds.
  virtual DWORD ReadProperty(const CString& name, CString& value) = 0;

private:
  /// Number of references
  ULONG m_RefCount;
};  // CPropertyBag


//------------------------------------------------------------------------------
// class CPropertyBagPtr
// Author: A.Happe
// Date: 04.09.2002
//------------------------------------------------------------------------------
/**
 * \brief   Smart pointer for property bags
 *
 */
//------------------------------------------------------------------------------

class CPropertyBagPtr
{
public:
  /// Default constructor
  CPropertyBagPtr()
    : m_p ( NULL )
  {
  }

  /// Destructor
  ~CPropertyBagPtr()
  {
    InternalRelease();
  }

  /// Copy constructor
  CPropertyBagPtr( const CPropertyBagPtr& ptr )
    : m_p ( NULL )
  {
    Attach( ptr.m_p , true );
  }

  /// Assignment operator - smart pointer
  CPropertyBagPtr& operator=( const CPropertyBagPtr& ptr )
  {
    if (ptr.m_p  != m_p )
    {
      InternalRelease();
      Attach( ptr.m_p , true );
    }
    return *this;
  }

  /// Wrap up a node
  CPropertyBagPtr( CPropertyBag * const pNode )
    : m_p ( NULL )
  {
    Attach( pNode, true );
  }

  /// Release encapsulated pointer
  CPropertyBagPtr& operator=(int /*null*/)
  {
    InternalRelease();
    return *this;
  }

  /// Attach a raw pointer
  void Attach( CPropertyBag* p, bool fAddRef=false )
  {
    if (m_p )
      m_p ->Release();
    m_p  = p;
    if (p && fAddRef)
    {
      p->AddRef();
    }
  }

  /// Detach the raw pointer
  CPropertyBag *Detach()
  {
    CPropertyBag *p = m_p ;
    m_p  = NULL;
    return p;
  }

  /// Address of encapsulated pointer
  CPropertyBag** operator &()
  {
    return &m_p ;
  }

  /// Return encapsulated pointer
  CPropertyBag* operator->() const
  {
    return m_p ;
  }
  /// 1 if a pointer is encapsulated pointer, 0 otherwise
  operator int()
  {
    return m_p  != NULL;
  }
  /// Return encapsulated pointer
  operator CPropertyBag*() const
  {
    return m_p ;
  }

protected:
  /// Release encapsulated pointer and reset internal pointer 
  void InternalRelease() 
  {
    if (m_p ) {
      m_p ->Release();
      m_p  = NULL;
    }
  }
private:
  /// Pointer to the managed property bag
  CPropertyBag *m_p ;
}; // CPropertyBagPtr


//------------------------------------------------------------------------------
// class CIniFilePropertyBag
// Author: 
// Date: 04.09.2002
//------------------------------------------------------------------------------
/**
 * \brief   Implemantation of a property bag based on .ini files
 *
 * Each property bag is bound to a specific section of an .ini file
 * 
 */
//------------------------------------------------------------------------------

class CIniFilePropertyBag  : public CPropertyBag
{
public:

  /// Creates a new .ini file. A property bag bound to the specified section is returned
  static CPropertyBagPtr Create(const CString& fileName, const CString& bagName);
  /// Returns a property bag which is bound to the section bagName in an existing .ini file
  static CPropertyBagPtr Open(const CString& fileName, const CString& bagName);

  /// Create a new section and returns a bag which is bound to this section
  CPropertyBagPtr CreateBag(const CString&  Name);

  /// Returns a bag which is bound to the given section
  CPropertyBagPtr GetBag(const CString&  Name);


protected:
    /// Constructor
    CIniFilePropertyBag (const CString& fileName, const CString& sectionName, const bool create);
    
    /// Writes an entry name=value to the property bag
    virtual DWORD WriteProperty(const CString& name, const CString& value);

    /// Reads the value of an name=value entry of the property bag
    virtual DWORD ReadProperty(const CString& name, CString& value);

private:
  CString m_strFileName;
  CString m_strSectionName;
}; // CIniFilePropertyBag 

struct RegKeyImpl;
//------------------------------------------------------------------------------
// class CRegistryPropertyBag
// Author: 
// Date: 06.09.2002
//------------------------------------------------------------------------------
/**
 * \brief   Implementation of a property bag which uses the registry
 *
 * Each property bag is associated with a registry key.
 * 
 */
//------------------------------------------------------------------------------
class CRegistryPropertyBag : public CPropertyBag  
{
public:
  /// open an existing bag
  static CPropertyBagPtr Open(const CString& bagName, HKEY hRoot = HKEY_CURRENT_USER);
  /// create a new bag
  static CPropertyBagPtr Create(const CString& bagName, HKEY hRoot = HKEY_CURRENT_USER);

  /// Ask the property bag to create new bag  
  CPropertyBagPtr CreateBag(const CString&  Name);
  /// Get an existing property bag
  CPropertyBagPtr GetBag(const CString&  Name);


protected:
  /// constructor
  CRegistryPropertyBag::CRegistryPropertyBag(HKEY parent, const CString& bagName, bool create);
  
  /// Writes an entry name=value to the property bag
  virtual DWORD WriteProperty(const CString& name, const CString& value);

  /// Reads the value of an name=value entry of the property bag
  virtual DWORD ReadProperty(const CString& name, CString& value);

  /// The registry key assoiciated with the bag
  RegKeyImpl *m_pKeyImpl;
}; // CRegistryPropertyBag


} // namespace Bcam