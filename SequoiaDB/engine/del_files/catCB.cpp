#include "catCB.hpp"
#include "ossIO.hpp"
#include "../util/fromjson.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "rtn.hpp"
namespace engine
{
   INT32 _SDB_CATCB::_parseNetwork ( BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      rc = _network.addupdate ( obj, FALSE ) ;
      if ( rc )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "Failed to parse network configuration: %s",
                 obj.toString().c_str() ) ;
         goto error ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
   INT32 _SDB_CATCB::_parseLine ( const CHAR *pLine,
                                  _SDB_CATCB_INIT_PHASE &phase )
   {
      INT32 rc = SDB_OK ;
      const CHAR *pChar = pLine ;
      while ( 0 != *pChar )
      {
         // skip all space and tab
         if ( '\t' == *pChar || ' ' == *pChar )
         {
            ++ pChar ;
            continue ;
         }
         // if we read comment as first character, let's skip the line
         if ( '#' == *pChar )
            goto done ;
         if ( ossStrcmp ( CAT_NETWORK_CATAGORY, pChar ) == 0 )
         {
            phase = _SDB_CAT_INIT_NETWORK ;
            goto done ;
         }
         switch ( phase )
         {
         case _SDB_CAT_INIT_NONE:
            break ;
         case _SDB_CAT_INIT_NETWORK:
         {
            BSONObj obj ;
            rc = fromjson ( pChar, obj ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to parse line into BSONObj: %s", pChar ) ;
               goto error ;
            }
            rc = _parseNetwork ( obj ) ;
            if ( rc )
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to parse network topology, rc = %d",
                       rc  ) ;
               goto error ;
            }
            break ;
         }
         default:
            rc = SDB_SYS ;
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Invalid phase" ) ;
            goto error ;
         }
         goto done ;
      }
   done :
      return rc ;
   error :
      goto done ;
   }
#define SDB_CATCB_READ_BUFSZ 4096
   INT32 _SDB_CATCB::init ()
   {
      INT32 rc                                    = SDB_OK ;
      pmdKRCB *krcb                               = pmdGetKRCB () ;
      SDB_ROLE dbrole                             = krcb->getDBRole () ;
      OSSFILE file ;
      BOOLEAN isOpened                            = FALSE ;
      SDB_ASSERT ( SDB_ROLE_STANDALONE != dbrole, "Standalone mode shouldn't \
call init" )
      //_catalog.init ( &_network, &_collection ) ;
      // for non-catalog mode, we should read in a file that specifying where
      // the catalog server is, and then connect to it
      if ( SDB_ROLE_CATALOG != dbrole )
      {
         CHAR catFileBuffer[OSS_MAX_PATHSIZE+1]   = {0};
         CHAR buffer [ SDB_CATCB_READ_BUFSZ + 1 ] = { 0 } ;
         SINT64 readSize                          = 0 ;
         SINT64 count                             = 0 ;
         _SDB_CATCB_INIT_PHASE phase              = _SDB_CAT_INIT_NONE ;
         //load catalog information
         krcb->getCatFile ( catFileBuffer, sizeof(catFileBuffer) ) ;
         // first let's open the file
         rc = ossOpen ( catFileBuffer, OSS_CREATE|OSS_SHAREREAD|OSS_READONLY,
                        OSS_RU|OSS_WU|OSS_RG|OSS_RO, file ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to open catalog file: %s, rc = %d",
                    catFileBuffer, rc ) ;
            goto error ;
         }
         // mark we have successfully opened the file so that we are going to
         // close it later
         isOpened = TRUE ;

         // then read the file
         while ( rc != SDB_EOF )
         {
            // use a pointer mark start of each line
            SINT64 startingLine = 0 ;
            SINT64 actReadSize  = 0 ;
            // read into buffer
            rc = ossRead ( &file, &buffer [ readSize ],
                           SDB_CATCB_READ_BUFSZ - readSize, &actReadSize ) ;
            if ( rc )
            {
               // dump error if we hit errors other than End Of File or
               // interrupt
               if ( rc != SDB_EOF && rc != SDB_INTERRUPT )
               {
                  pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                          "Failed to read catalog file: %s, rc = %d",
                          catFileBuffer, rc ) ;
                  goto error ;
               }
            }

            startingLine = 0 ;
            readSize += actReadSize ;
            count = 0 ;
            SDB_ASSERT ( readSize <= SDB_CATCB_READ_BUFSZ,
                         "readSize is out of bound" )
            while ( count < readSize )
            {
               // both 0x0D and 0x0A are considered as newline character
               if ( buffer[count] == 0x0D || buffer[count] == 0x0A )
               {
                  // set the character as end of string
                  buffer[count] = '\0' ;
                  // if there's no other characters in the line, let's continue
                  // to the next
                  if ( startingLine == count )
                  {
                     ++startingLine ;
                     ++count ;
                     continue ;
                  }

                  // otherwise that means we have something in the line, let's
                  // parse it
                  rc = _parseLine ( &buffer[startingLine], phase ) ;
                  if ( rc )
                  {
                     pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                             "Failed to parse line: %s, rc = %d",
                             &buffer[startingLine], rc ) ;
                     goto error ;
                  }
                  // set pointer to next, this should be safe because we don't
                  // actually read anything even if count is already at end of
                  // the string. In the next while loop we are going to jump out
                  // right away
                  startingLine = count + 1 ;
               }
               ++count ;
            }
            // if we have partially read string, let's keep it
            if ( startingLine < readSize )
            {
               if ( 0 == startingLine )
               {
                  // we have a very long line so that entire line is considered
                  // as a full string. In this case we can't continue because we
                  // won't read anything at all in next round
                  pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                          "Line is too long, max line size is %d",
                          SDB_CATCB_READ_BUFSZ ) ;
                  rc = SDB_INVALIDARG ;
                  goto error ;
               }
               ossMemcpy ( &buffer[0], &buffer[startingLine],
                           readSize - startingLine ) ;
               readSize -= startingLine ;
            }
            else
               readSize = 0 ;
         }
         // when we hit here, that means we have rc = SDB_EOF, we should set it
         // back to SDB_OK
         rc = SDB_OK ;

         _type = _SDB_CAT_TYPE_FILE ;
#if defined (_DEBUG)
         {
            BSONObj obj ;
            rc = krcb->getCATCB()->_network.dumpInfo ( obj ) ;
            if ( rc )
            {
                  pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to dump network topology, rc = %d", rc ) ;
               goto error ;
            }
            pdLog ( PDDEBUG, __FUNC__, __FILE__, __LINE__,
                    "Catalog Topology: %s", obj.toString().c_str() ) ;
         }
#endif
      }
      // for catalog mode, we should attempt to load collection space
      else
      {
         SDB_DMSCB *dmsCB = krcb->getDMSCB() ;
         CHAR dbPathBuffer [ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
         // load DMS storage unit from dbpath
         if ( !krcb->getDBPath ( dbPathBuffer, OSS_MAX_PATHSIZE ) )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to retreive dbpath" ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         rc = rtnLoadCollectionSpace ( CAT_COLLECTION_CATALOG_CSNAME,
                                       dbPathBuffer, dmsCB ) ;
         if ( rc )
         {
            // if we failed due to not able to find the collection space, that
            // means most likely we are accessing the catalog first time, so we
            // have to initialize a new collectionspace
            if ( SDB_DMS_CS_NOTEXIST == rc )
            {
               rc = rtnCreateCollectionSpaceCommand (
                     CAT_COLLECTION_CATALOG_CSNAME, NULL,
                     dmsCB, NULL, DMS_PAGE_SIZE_DFT, TRUE ) ;
               if ( rc )
               {
                  pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                          "Failed to create %s collection space, rc = %d",
                          CAT_COLLECTION_CATALOG_CSNAME, rc ) ;
                  goto error ;
               }
            }
            else
            {
               pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                       "Failed to load %s collection space, rc = %d",
                       CAT_COLLECTION_CATALOG_CSNAME, rc ) ;
               goto error ;
            }
         }
         // load network
         rc = _network.init () ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to initialize network catalog, rc = %d",
                    rc ) ;
            goto error ;
         }
         // load collection
         rc = _collection.init () ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to initialize collection catalog, rc = %d",
                    rc ) ;
            goto error ;
         }
      }
   done :
      if ( isOpened )
         ossClose ( file ) ;
      return rc ;
   error :
      goto done ;
   }

   // resync with catalog
   INT32 _SDB_CATCB::resync ( BOOLEAN isInitial )
   {
      INT32 rc = SDB_OK ;
      pmdKRCB *krcb = pmdGetKRCB () ;
      // if we call resync on non-catalog nodes or not on primary catalog
      if ( SDB_ROLE_CATALOG != krcb->getDBRole () )
      {
         rc = _network.resync ( isInitial ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to resync with catalog" ) ;
            goto error ;
         }
         rc = _collection.resync ( isInitial ) ;
         if ( rc )
         {
            pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                    "Failed to resync with catalog" ) ;
            goto error ;
         }
      }
      else
      {
         // if we are primary catalog, we should do something special then?
         // or we should never hit such codepath??
         SDB_ASSERT ( FALSE,
                    "what should we do when calling resync on primary catalog" )
      }
   done :
      return rc ;
   error :
      goto done ;
   }
}
