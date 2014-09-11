#ifndef __TASKMNG_HPP__
#define __TASKMNG_HPP__

#include "task.hpp"

#include <vector>
#include <string>

using namespace std ;

class Task ;
class IClient ;
class CTaskMng
{
   public:
         static CTaskMng& getInstance ( ) ;
         void setRunPara (int runMode ,
                                   int totalNum ,
                                   int thrNum ,
                                   int interval ,
                                   const char* connectStr ,
                                   const char* dbName ) ;
         bool Init ( ) ;
         void run ( ) ;
         void wait ( ) ;
         
   private:
         CTaskMng ( ) ;
         ~CTaskMng ( ) ;

         int GetFullPath ( char* pPath ) ;
         typedef IClient* (*CreateFun) ( const char* pSvrAddr ,
                                         const char* pDbName ,
                                         int startID ) ;
         IClient* GetClient ( int startID ) ;

   private:
         int m_runMode ;
         int m_totalNum ;
         int m_thrNum ;
         int m_interval ;
         const char* m_connectStr ;
         const char* m_dbName ;

         static void *m_handle ;
         static CreateFun m_createFun ;
         std::vector<CTask* > m_tasks ; 
} ;

#endif
