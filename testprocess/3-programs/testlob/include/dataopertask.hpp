#ifndef __TEST_DATAOPERTASK_HPP__
#define __TEST_DATAOPERTASK_HPP__

#include "task.hpp"

class IClient ;
class CDataOperTask:public CTask
{
   public:
         CDataOperTask ( int needFinishNum ,
                                   int runMode ,
                                   int interval ,
                                   int No ) ;
         ~CDataOperTask ( ) ;

         void setClient ( IClient* pClient )
         {
             m_pClient = pClient ;
         }

   private:
         bool Init ( ) ;
         void Do ( ) ;
         void AddUp ( int nCnt ) ;

   private:
         int m_needFinishNum ;
         int m_runMode ;
         int m_interval ;
         IClient* m_pClient ;
         timeval m_tb, m_te ;

         static int m_init ;
         
} ;

#endif
