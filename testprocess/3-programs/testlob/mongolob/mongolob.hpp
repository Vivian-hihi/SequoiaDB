#ifndef __MONGLOB_HPP__
#define __MONGLOB_HPP__

#include "iclient.h"
#include "mongo/client/dbclient.h"

#include <stdlib.h>
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <string.h>

using namespace mongo ;
using namespace std ;

class CMongoLob:public IClient
{
   public:
         CMongoLob ( const std::string& strSrvAddr ,
                             const std::string& strDbName ,
                             int startID ) ;
         ~CMongoLob ( ) ;

         virtual int Connect ( ) ;
         virtual int Putfile ( int id ) ;
         virtual int Getfile ( int id ) ;
         virtual int Mix ( int id ) ;
         virtual void Release ( ) ;

   private:
         string ChooseFile ( int id ) ;
   
   private:
         std::string m_strSrvAddr ;
         std::string m_dbName ;
         int m_startID ;

         mongo::DBClientConnection m_conn ;
         mongo::GridFS* gridfs ;

         vector<string> fileslist ;
         string sourceDir ;
         string outDir ;
         
} ;

#endif

