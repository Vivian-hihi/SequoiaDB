#include "task.hpp"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <stdio.h>

CTask::CTask ( int No )
:m_bIsExit ( false ) ,
 m_pThrd ( NULL ) ,
 m_thrNo( No ) 
{
}

CTask::~CTask ( )
{
   delete m_pThrd ;
}

void CTask::run ( ) 
{
   if ( !Init( ) )
   {
      printf( "thread init failed\n" ) ;
      return ;
   }

   while ( !m_bIsExit)
   {
      Do ( ) ;
   }
}

void CTask::start()
{
   boost::function0<void> f = boost::bind( &CTask::run, this ) ;
   m_pThrd = new boost::thread ( f ) ;
}

void CTask::wait ( )
{
   m_pThrd->join ( ) ;
}


