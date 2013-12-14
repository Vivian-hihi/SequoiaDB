#ifndef CATCB_HPP__
#define CATCB_HPP__

#include "core.hpp"
#include "cat.hpp"
#include "catNet.hpp"
#include "catCollection.hpp"
#include "catCatalog.hpp"
#include "../bson/bson.h"
namespace engine
{
#define CAT_NETWORK_CATAGORY  "[NETWORK]"
   enum _SDB_CATCB_INIT_PHASE
   {
      _SDB_CAT_INIT_NONE    = 0,
      _SDB_CAT_INIT_NETWORK
   } ;
   enum _SDB_CATCB_TYPE
   {
      _SDB_CAT_TYPE_NONE = 0,
      _SDB_CAT_TYPE_FILE,
      _SDB_CAT_TYPE_NET
   } ;
   // catalog control block, include information about
   // network topology
   // collection / collectionspace def
   // user info
   struct _SDB_CATCB
   {
   public :
      catNetwork _network ;
      catCollection _collection ;
      // this _catalog is only useful in CATALOG mode
      catCatalog _catalog ;
      // initialize using local catalog file
      INT32 init () ;
      INT32 resync ( BOOLEAN isInitial = FALSE ) ;
      _SDB_CATCB ()
      {
         _type = _SDB_CAT_TYPE_NONE ;
      }
   private :
      _SDB_CATCB_TYPE _type ;
      INT32 _parseLine ( const CHAR *pLine, _SDB_CATCB_INIT_PHASE &phase ) ;
      INT32 _parseNetwork ( BSONObj &obj ) ;
   } ;
   typedef struct _SDB_CATCB SDB_CATCB ;
}

#endif
