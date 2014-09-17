#ifndef __SEQUOIALOB_HPP__
#define __SEQUOIALOB_HPP__

#include "iclient.h"
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>

using namespace std ;

class CSdbLob:public IClient
{
   public:
         CSdbLob ( const std::string& strSrvAddr,
                               const std::string& strDbName, 
                               int startID ) ;
         ~CSdbLob ( ) ;

         virtual int Connect ( ) ;
         virtual int Putfile ( int id ) ;
         virtual int Getfile ( int id ) ;
         virtual int Mix ( int id ) ;
         virtual void Release ( ) ;

   private:
         string ChooseFile ( int id ) ;
         char* getCharPtr ( string &str ) ;
         int init ( ) ;
         
   private:
         std::string m_strSrvAddr ;
         std::string m_dbName ;
         std::string m_fullClName ;
         int m_startID ;


   private:
         const static UINT64 lobSize = 1024*1024*16 ;
      
         char* pSrvAddr ;
         char* pCSName ;
         char* pCLName ;
         int rc ;

         sdbConnectionHandle m_conn ;
         sdbCSHandle m_cs ;
         sdbCollectionHandle m_cl ;
         sdbLobHandle m_lob ;
         char* readBuf ;
         char* lobWriteBuf ;
         char* lobReadBuf ;

         vector<string> fileslist ;
         string lobdir ;
         
  
} ;

#endif

