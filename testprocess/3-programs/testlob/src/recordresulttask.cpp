#include "recordresulttask.hpp"
#include "common.h"
#include <unistd.h>

CRecordResultTask* CRecordResultTask::m_instance ;
using std::fstream ;
using std::endl ;

CRecordResultTask::CRecordResultTask ( int No )
:CTask ( No )
{
}

CRecordResultTask::~CRecordResultTask ( )
{
   if ( m_foutResult.is_open( ) )
   {
      m_foutResult.close ( ) ;
   }
}

CRecordResultTask* CRecordResultTask::getInstance ( )
{
   if ( NULL == m_instance )
   {
      m_instance = new CRecordResultTask ( 0 ) ;
   }
   return m_instance ;
}

void CRecordResultTask::addStatResult (int num, int diffsec, int diffUsec, int No)
{
   Stat stat ;
   stat.thrNo = No ;
   stat.totalnum = num ;
   stat.diffSec = diffsec ;
   stat.diffUSec = diffUsec ;

   {
      boost::lock_guard<boost::mutex> lock( m_statmutex ) ;
      m_stats.push_back ( stat ) ;
   }

   boost::unique_lock<boost::mutex> lock2 ( m_condmutex ) ;
   m_cond.notify_all ( ) ;
   return ;
}

void CRecordResultTask::stop ( ) 
{
   if ( m_stats.size( ) )
   {
      sleep ( 5 ) ;
   }
   CTask::stop ( ) ;
   boost::unique_lock<boost::mutex> lock2 ( m_condmutex ) ;
   m_cond.notify_all ( ) ;
}

bool CRecordResultTask::Init ( ) 
{
   m_foutResult.open ( "./result.log", std::fstream::in | std::fstream::out | std::fstream::trunc ) ;

   if ( !m_foutResult.is_open( ) ) 
   {
      std::cout << "open result.log failed!" << endl ;
      return false ;
   }

   return true ;

}

void CRecordResultTask::Do ( ) 
{
   do
   {
      boost::unique_lock<boost::mutex> lock ( m_condmutex ) ;
      while ( !isStop( ) && m_stats.size( ) == 0 )
      {
         m_cond.wait ( m_condmutex ) ;
      }

      std::vector<Stat> stats ;
      {
         boost::lock_guard<boost::mutex> lock( m_statmutex ) ;
         stats = m_stats ;
         m_stats.clear ( ) ;
      }

      for ( std::vector<Stat>::size_type i = 0; i < stats.size ( ) ; ++i )
      {
         m_foutResult << stats[i].thrNo << "," ;
         std::streamsize width = m_foutResult.width ( 9 ) ;
         m_foutResult.fill ( '0' ) ;
         m_foutResult << stats[i].totalnum << "," ;
         m_foutResult << stats[i].diffSec << "." ;
         m_foutResult.width( 6 ) ;
         m_foutResult << stats[i].diffUSec << endl ;
         m_foutResult.width ( width ) ;
      }

      if ( isStop ( ) )
      {
         break ;
      }
         
   }while ( true ) ;
}


