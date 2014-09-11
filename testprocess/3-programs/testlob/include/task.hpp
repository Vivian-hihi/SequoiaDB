#ifndef __TEST_TASK_HPP__
#define __TEST_TASK_HPP__

#include <boost/thread/thread.hpp>

class CTask
{
   public:
         CTask ( int No ) ;
         virtual ~CTask ( ) ;

         void run ( ) ;
         void start ( ) ;
         virtual void stop ( ) 
         { 
            m_bIsExit = true ; 
         }
         bool isStop ( ) 
         {
            return m_bIsExit ;
         }
         void wait ( ) ;
         int getNo ( )
         {
            return m_thrNo ;
         }
         
   private:
         virtual bool Init ( ) = 0 ;
         virtual void Do ( ) = 0 ;

   private:
         bool m_bIsExit ;
         boost::thread* m_pThrd ;
         int m_thrNo ;
         
} ;

#endif

