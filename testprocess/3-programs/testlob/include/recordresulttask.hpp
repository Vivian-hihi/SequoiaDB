#ifndef __RECORDRESULTTASK_HPP__
#define __RECORDRESULTTASK_HPP__

#include "dataopertask.hpp"
#include "task.hpp"
#include <vector>
#include <fstream>
#include <boost/thread/thread.hpp>

class CRecordResultTask:public CTask
{
   private:
         CRecordResultTask ( int No ) ;
         ~CRecordResultTask ( ) ;
         
   public:
         static CRecordResultTask* getInstance ( ) ;
         void addStatResult ( int num, int diffsec, int diffUsec, int No ) ;
         void runMode ( int runMode )
         {
            m_runMode = runMode ;
         }
         virtual void stop ( ) ;

   private:
         virtual bool Init ( ) ;
         virtual void Do ( ) ;

         typedef struct struStat
         {
            int thrNo ;
            long totalnum ;
            int diffSec ;
            int diffUSec ;
          } Stat ;

   private:
         int m_runMode ;
         boost::mutex m_statmutex ;
         std::vector<Stat> m_stats ;

         boost::mutex m_condmutex ;
         boost::condition_variable_any m_cond ;
         std::fstream m_foutResult ;

         static CRecordResultTask* m_instance ;
} ;


#endif
