/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptReturnVal.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_RETURNVAL_HPP_
#define SPT_RETURNVAL_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "../bson/bson.hpp"

namespace engine
{
   class _sptProperty : public SDBObject
   {
   public:
      _sptProperty() ;
      virtual ~_sptProperty() ;

      _sptProperty( const _sptProperty & ) ;
      _sptProperty &operator=(const _sptProperty &) ;

   public:
      INT32 assign( const CHAR *name,
                    bson::BSONType type,
                    const void *value ) ;

      void assignObject( const CHAR *name,
                         void *value ) ;

      void assignVoid() ;

      const std::string &getName()const
      {
         return _name ;
      }

      INT32 getINT32() const ;
      FLOAT64 getFLOAT64() const ;
      BOOLEAN getBool() const ;
      const CHAR * getString() const ;
      bson::BSONType getType() const
      {
         return _type ;
      }
      void *getObj() const
      {
         return (void *)_value ;
      }

   protected:
      std::string _name ;
      UINT64 _value ;
      bson::BSONType _type ;
   } ;
   typedef class _sptProperty sptProperty ;

   typedef std::vector<sptProperty> SPT_PROPERTIES ;

   class _sptReturnVal : public SDBObject
   {
   public:
      _sptReturnVal()
      : _classDef(NULL)
      {}

      virtual ~_sptReturnVal()
      {
         _classDef = NULL ;
      }

      INT32 setNativeVal( const CHAR *name,
                          bson::BSONType type,
                          const void *value ) ;

      void setObjectVal( const CHAR *name,
                         void *value,
                         const void *classDef ) ;

      const sptProperty &getVal() const
      {
         return _property ;
      }

      const void *getClassDef()const
      {
         return _classDef ;
      }

      void addReturnValProperty( const sptProperty &property )
      {
         _properties.push_back( property ) ;
      }
    
      const SPT_PROPERTIES &getValProperties()const
      {
         return _properties ;
      }

   private:
      /// property name in parent.
      /// if this field is assigned,
      /// return value will be set into parent object as a property.
      sptProperty _property ;
      const void *_classDef ;

      /// properties of return val.
      SPT_PROPERTIES _properties ;
   } ;

   typedef class _sptReturnVal sptReturnVal ;
}

#endif

