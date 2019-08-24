/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = ossMem.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/1/2014  ly  Initial Draft

   Last Changed =

*******************************************************************************/
#include "ossMem.hpp"
#include "ossMem.c"

// in C++, we keep track of memory allocation
#if defined (__cplusplus) && defined (SDB_ENGINE)

#include "ossLatch.hpp"
#include "ossPrimitiveFileOp.hpp"
#include "filenames.hpp"
#include <set>
#include <map>

/*
   _ossMemInfoAssit define
*/
struct _ossMemInfoAssit
{
   UINT32   _file ;
   UINT32   _line ;
   UINT64   _size ;
   UINT32   _debugSize ;

   _ossMemInfoAssit( const void* p )
   {
      const CHAR *pAddr = ( const CHAR * )p ;

      _file = *(UINT32*)( pAddr + OSS_MEM_HEAD_FILEOFFSET ) ;
      _line = *(UINT32*)( pAddr + OSS_MEM_HEAD_LINEOFFSET ) ;
      _size = *(UINT64*)( pAddr + OSS_MEM_HEAD_SIZEOFFSET ) ;
      _debugSize = *(UINT32*)( pAddr + OSS_MEM_HEAD_DEBUGOFFSET ) ;
   }
} ;
typedef _ossMemInfoAssit ossMemInfoAssit ;

/*
   _ossMemTrackItem define
*/
struct _ossMemTrackItem
{
   const CHAR* _p ;
   UINT64      _fileline ;
   UINT64      _size ;
   UINT64      _time ;
   UINT32      _tid ;

   _ossMemTrackItem( void *p = NULL )
   {
      _p = ( const CHAR* )p ;
      _fileline = 0 ;
      _size = 0 ;
      _time = 0 ;
      _tid = 0 ;
   }
   _ossMemTrackItem( void *p, UINT32 file, UINT32 line, UINT64 size )
   {
      _p = ( const CHAR* )p ;
      _fileline = ((UINT64)file << 32) | (UINT64)line ;
      _size = size ;
      _time = 0 ;
      _tid = 0 ;
   }

   UINT32 getFile() const
   {
      return (UINT32)( _fileline >> 32 ) ;
   }

   UINT32 getLine() const
   {
      return (UINT32)_fileline ;
   }

   bool operator< ( const _ossMemTrackItem &rhs ) const
   {
      return _p < rhs._p ? true : false ;
   }
   bool operator== ( const _ossMemTrackItem &rhs ) const
   {
      return _p == rhs._p ? true : false ;
   }
} ;
typedef _ossMemTrackItem ossMemTrackItem ;

/*
   _ossMemStatItem define
*/
struct _ossMemStatItem
{
   UINT32      _times ;
   UINT64      _totalSize ;

   _ossMemStatItem()
   {
      _times = 0 ;
      _totalSize = 0 ;
   }

   bool operator< ( const _ossMemStatItem &rhs ) const
   {
      /// reverse
      return _times > rhs._times ? true : false ;
   }
} ;
typedef _ossMemStatItem ossMemStatItem ;

/// Item title and format
#define OSS_MEM_DUMP_ITEM_TITLE \
   "        ID              Address           Size    FillSize                                          File      Line       TID    Time"

#define OSS_MEM_DUMP_ITER_FOMART \
   "%10ld   %18p     %10ld  %10ld    %30s(%10u)    %6u"

#define OSS_MEM_DUMP_ITEM_FORMAT_TID_TIME \
   "    %6u    %s"

#define OSS_MEM_DUMP_ERRORSTR " ****(has error)"

/// Stat title and format

#define OSS_MEM_DUMP_STAT_TITLE \
   "        ID                            File              :   Line ----      Count           TotalSize"

#define OSS_MEM_DUMP_STAT_FORMAT \
   "%10ld    %30s(%10u): %6u ---- %10u    %16ld"

typedef std::set<ossMemTrackItem>               OSS_MEM_TRACKMAP ;
typedef OSS_MEM_TRACKMAP::iterator              OSS_MEM_TRACKMAP_IT ;
typedef std::map<UINT64,ossMemStatItem>         OSS_MEM_STATMAP ;
typedef OSS_MEM_STATMAP::iterator               OSS_MEM_STATMAP_IT ;
typedef OSS_MEM_STATMAP::const_iterator         OSS_MEM_STATMAP_CIT ;
typedef std::multimap<ossMemStatItem, UINT64>   OSS_MEM_SORTMAP ;
typedef OSS_MEM_SORTMAP::iterator               OSS_MEM_SORTMAP_IT ;
typedef OSS_MEM_SORTMAP::const_iterator         OSS_MEM_SORTMAP_CIT ;

#define OSS_MEM_TRACKCB_NAME_SZ     ( 64 )
#define OSS_MEM_TRACEDUMP_TM_BUF    ( 64 )
#define OSSMEMTRACEDUMPBUFSZ        ( 1024 )

/*
   _ossMemTrackCB define
*/
class _ossMemTrackCB
{
   private:
      ossSpinXLatch              _memTrackMutex ;
      OSS_MEM_TRACKMAP           _memTrackMap ;
      OSS_MEM_STATMAP            _memStatMap ;
      CHAR                       _name[ OSS_MEM_TRACKCB_NAME_SZ + 1 ] ;
      UINT64                     _resetTime ;

      /// buf
      CHAR                       _linebuff[ OSSMEMTRACEDUMPBUFSZ + 1 ] ;
      CHAR                       _timebuff[ OSS_MEM_TRACEDUMP_TM_BUF ] ;

      // stat
      UINT64                     _lastDumpTime ;
      UINT64                     _lastTrackNum ;
      UINT64                     _lastTrackSize ;
      UINT64                     _lastStatTimes ;
      UINT64                     _lastStatSize ;

   protected:
      BOOLEAN   verifyItem( const ossMemTrackItem &item ) const
      {
         if ( !ossMemVerify( ( void* )( item._p + OSS_MEM_HEADSZ ) ) )
         {
            return FALSE ;
         }

         /// check size
         ossMemInfoAssit assit( ( const void* )item._p ) ;
         if ( assit._size != item._size  )
         {
#if defined (SDB_ENGINE)
            PD_LOG ( PDSEVERE, "memory size(%lld) is not the same with "
                     "track item(%lld)",
                     assit._size, item._size ) ;
#endif
            return FALSE ;
         }
         else if ( assit._file != item.getFile() ||
                   assit._line != item.getLine() )
         {
#if defined (SDB_ENGINE)
            PD_LOG ( PDSEVERE, "memory file(%d) or line(%d) is not the "
                     "same with track item's file(%d) or line(%d)",
                     assit._file, assit._line,
                     item.getFile(), item.getLine() ) ;
#endif
            return FALSE ;
         }

         return TRUE ;
      }

      UINT64    getFillSize( const ossMemTrackItem &item ) const
      {
         ossMemInfoAssit assit( (const void*)item._p ) ;
         return OSS_MEM_HEADSZ + ( assit._debugSize << 1 ) ;
      }

   protected:
      void memTraceItem( UINT64 index,
                         const ossMemTrackItem &item,
                         ossPrimitiveFileOp &trapFile,
                         BOOLEAN &isError )
      {
         UINT32 len = 0 ;

         if ( ossMemDebugVerify )
         {
            isError = verifyItem( item ) ? FALSE : TRUE ;
         }
         else
         {
            isError = FALSE ;
         }

         len = ossSnprintf( _linebuff, sizeof( _linebuff),
                            OSS_MEM_DUMP_ITER_FOMART,
                            index, item._p, item._size,
                            getFillSize( item ),
                            autoGetFileName( item.getFile() ).c_str(),
                            item.getFile(),
                            item.getLine() ) ;

         if ( 0 != item._tid || 0 != item._time )
         {
            ossTimestamp tm ;
            tm.time = item._time / 1000000 ;
            tm.microtm = item._time % 1000000 ;
            ossTimestampToString( tm, _timebuff ) ;

            len += ossSnprintf( _linebuff + len, sizeof( _linebuff ) - len,
                                OSS_MEM_DUMP_ITEM_FORMAT_TID_TIME,
                                item._tid, _timebuff ) ;
         }

         if ( isError )
         {
            len += ossSnprintf( _linebuff + len, sizeof( _linebuff ) - len,
                                OSS_MEM_DUMP_ERRORSTR ) ;
         }

         len += ossSnprintf( _linebuff + len, sizeof( _linebuff ) - len,
                             "\n" ) ;

         trapFile.Write( _linebuff, len ) ;
      }

      void memTraceStatItem( UINT64 index,
                             UINT64 fileline,
                             const ossMemStatItem &item,
                             ossPrimitiveFileOp &trapFile )
      {
         UINT32 len = 0 ;
         UINT32 file = (UINT32)( fileline >> 32 ) ;
         UINT32 line = (UINT32)fileline ;

         len = ossSnprintf( _linebuff, sizeof( _linebuff ),
                            OSS_MEM_DUMP_STAT_FORMAT"\n",
                            index,
                            autoGetFileName( file ).c_str(),
                            file, line,
                            item._times, item._totalSize ) ;
         trapFile.Write( _linebuff, len ) ;
      }

      void addStat( OSS_MEM_STATMAP &mapStat,
                    UINT64 fileline,
                    UINT64 size )
      {
         try
         {
            ossMemStatItem &item = mapStat[ fileline ] ;
            ++item._times ;
            item._totalSize += size ;
         }
         catch ( ... )
         {
         }
      }

      void memStatMap2SortMap( const OSS_MEM_STATMAP &mapStat,
                               OSS_MEM_SORTMAP &mapSort )
      {
         try
         {
            OSS_MEM_STATMAP_CIT cit = mapStat.begin() ;
            while ( cit != mapStat.end() )
            {
               mapSort.insert( OSS_MEM_SORTMAP::value_type( cit->second,
                                                            cit->first ) ) ;
               ++cit ;
            }
         }
         catch ( ... )
         {
         }
      }

      void memTrace( ossPrimitiveFileOp &trapFile,
                     const OSS_MEM_TRACKMAP &mapTrack,
                     OSS_MEM_STATMAP &mapStat,
                     UINT64 *pTotalSize = NULL )
      {
         OSS_MEM_TRACKMAP_IT it ;
         UINT64 totalSize = 0 ;
         UINT64 index = 0 ;
         UINT64 totalError = 0 ;
         BOOLEAN isError = FALSE ;

         totalError = 0 ;

         /// write begin
         ossSnprintf( _linebuff, sizeof( _linebuff ),
                      "\n\n-------- Memory Information List( Run Time ) --------\n" ) ;
         trapFile.Write( _linebuff ) ;

         /// write title
         trapFile.Write( OSS_MEM_DUMP_ITEM_TITLE ) ;
         trapFile.Write( "\n\n" ) ;

         /// write item
         it = mapTrack.begin() ;
         while ( it != mapTrack.end() )
         {
            const ossMemTrackItem &item = *it ;
            totalSize += item._size ;
            memTraceItem( ++index, item, trapFile, isError ) ;
            if ( isError )
            {
               ++totalError ;
            }
            /// insert to map stat
            addStat( mapStat, item._fileline, item._size ) ;
            ++it ;
         }

         /// write result
         ossSnprintf( _linebuff, sizeof( _linebuff ),
                      "\n\n"
                      "+++++++++++++++++++\n"
                      "TotalSize  : %llu\n"
                      "TotalNum   : %llu\n"
                      "TotalError : %llu\n"
                      "+++++++++++++++++++\n",
                      totalSize, index, totalError ) ;
         trapFile.Write( _linebuff ) ;

         if ( pTotalSize )
         {
            *pTotalSize = totalSize ;
         }
      }

      void memStatTrace( ossPrimitiveFileOp &trapFile,
                         const OSS_MEM_SORTMAP &mapSort,
                         const CHAR *prefix,
                         UINT64 *pTotalSize = NULL,
                         UINT64 *pTotalTimes = NULL )
      {
         OSS_MEM_SORTMAP_CIT cit ;
         UINT64 totalSize = 0 ;
         UINT64 totalTimes = 0 ;
         UINT64 index = 0 ;

         if ( !prefix )
         {
            prefix = "Unknown" ;
         }

         /// write begin
         ossSnprintf( _linebuff, sizeof( _linebuff ),
                      "\n\n-------- Memory Information Stat( %s ) --------\n",
                      prefix ) ;
         trapFile.Write( _linebuff ) ;

         /// write title
         trapFile.Write( OSS_MEM_DUMP_STAT_TITLE ) ;
         trapFile.Write( "\n\n" ) ;

         /// write item
         cit = mapSort.begin() ;
         while ( cit != mapSort.end() )
         {
            totalSize += cit->first._totalSize ;
            totalTimes += cit->first._times ;
            memTraceStatItem( ++index, cit->second, cit->first, trapFile ) ;
            ++cit ;
         }

         /// write result
         ossSnprintf( _linebuff, sizeof( _linebuff ),
                      "\n\n"
                      "+++++++++++++++++++\n"
                      "TotalSize : %llu\n"
                      "TotalNum  : %llu\n"
                      "StatNum   : %llu\n"
                      "+++++++++++++++++++\n",
                      totalSize, totalTimes, index ) ;
         trapFile.Write( _linebuff ) ;

         if ( pTotalSize )
         {
            *pTotalSize = totalSize ;
         }
         if ( pTotalTimes )
         {
            *pTotalTimes = totalTimes ;
         }
      }

   public:
      _ossMemTrackCB ( const CHAR *name )
      {
         ossMemset( _name, 0, sizeof( _name ) ) ;
         ossStrncpy( _name, name, OSS_MEM_TRACKCB_NAME_SZ ) ;

         ossMemset( _linebuff, 0, sizeof( _linebuff ) ) ;
         ossMemset( _timebuff, 0, sizeof( _timebuff ) ) ;
         _resetTime = ossGetCurrentMicroseconds() ;

         _lastDumpTime = _resetTime ;
         _lastTrackNum = 0 ;
         _lastTrackSize = 0 ;
         _lastStatTimes = 0 ;
         _lastStatSize = 0 ;
      }
      ~_ossMemTrackCB ()
      {
      }

      void reset()
      {
         _memTrackMutex.get() ;

         _memTrackMap.clear() ;
         _memStatMap.clear() ;
         _resetTime = ossGetCurrentMicroseconds() ;

         _lastDumpTime = _resetTime ;
         _lastTrackNum = 0 ;
         _lastTrackSize = 0 ;
         _lastStatTimes = 0 ;
         _lastStatSize = 0 ;

         _memTrackMutex.release() ;
      }

      const CHAR* getName() const { return _name ; }

      void memTrack ( void *p, UINT32 file, UINT32 line, UINT64 size )
      {
         ossMemTrackItem item( p, file, line, size ) ;
         item._tid = ossGetCurrentThreadID() ;
         item._time = ossGetCurrentMicroseconds() ;

         _memTrackMutex.get() ;

         /// add stat
         addStat( _memStatMap, item._fileline, item._size ) ;
         /// add to item
         try
         {
            _memTrackMap.insert( item ) ;
         }
         catch ( ... )
         {
         }
         _memTrackMutex.release() ;
      }

      void memUnTrack ( void *p, UINT32 file, UINT32 line, UINT64 size )
      {
         ossMemTrackItem item( p, file, line, size ) ;

         _memTrackMutex.get() ;
         try
         {
            _memTrackMap.erase( item ) ;
         }
         catch ( ... )
         {
         }
         _memTrackMutex.release() ;
      }

      void memTrace( ossPrimitiveFileOp &trapFile )
      {
         UINT32 len = 0 ;
         OSS_MEM_SORTMAP mapSort ;
         OSS_MEM_STATMAP mapLocalStat ;
         UINT64 beginTime = 0 ;
         UINT64 endTime = 0 ;
         ossTimestamp tm ;
         CHAR beginTimebuff[ OSS_MEM_TRACEDUMP_TM_BUF ] = { 0 } ;

         UINT64 trackNum = 0 ;
         UINT64 trackSize = 0 ;
         UINT64 statTimes = 0 ;
         UINT64 statSize = 0 ;

         beginTime = ossGetCurrentMicroseconds() ;

         tm.time = beginTime / 1000000 ;
         tm.microtm = beginTime % 1000000 ;
         ossTimestampToString( tm, beginTimebuff ) ;

         /// header info
         len = ossSnprintf( _linebuff, sizeof( _linebuff ),
                            "====> Memory( %s ) dump begin ====>\n",
                            getName() ) ;
         trapFile.Write( _linebuff, len ) ;

         /// guard
         {
            ossScopedLock lock( &_memTrackMutex ) ;

            tm.time = _resetTime / 1000000 ;
            tm.microtm = _resetTime % 1000000 ;
            ossTimestampToString( tm, _timebuff ) ;

            /// dump global stat info
            memStatMap2SortMap( _memStatMap, mapSort ) ;
            memStatTrace( trapFile, mapSort, "Global", &statSize, &statTimes ) ;
            mapSort.clear() ;

            /// dump item info
            memTrace( trapFile, _memTrackMap, mapLocalStat, &trackSize ) ;

            trackNum = ( UINT64 )_memTrackMap.size() ;
         }

         /// dump local stat info
         memStatMap2SortMap( mapLocalStat, mapSort ) ;
         memStatTrace( trapFile, mapSort, "Run Time" ) ;
         mapSort.clear() ;

         endTime = ossGetCurrentMicroseconds() ;

         /// dump reset time
         len = ossSnprintf( _linebuff, sizeof( _linebuff ),
                            "\n"
                            " Reset Time         : %s\n"
                            " Dump Time          : %s\n"
                            " Dump Interval      : %lld (secs)\n"
                            " Dump Cost Time     : %lld (secs)\n"
                            " Global Count Inc   : %lld\n"
                            " Global Size Inc    : %lld\n"
                            " Run Time Count Inc : %lld\n"
                            " Run Time Size Inc  : %lld\n",
                            _timebuff,
                            beginTimebuff,
                            ( beginTime - _lastDumpTime ) / 1000000L,
                            ( endTime - beginTime ) / 1000000L,
                            ( statTimes - _lastStatTimes ),
                            ( statSize - _lastStatSize ),
                            ( trackNum - _lastTrackNum ),
                            ( trackSize - _lastTrackSize ) ) ;
         trapFile.Write( _linebuff, len ) ;

         /// result info
         len = ossSnprintf( _linebuff, sizeof( _linebuff ),
                            "\n<==== Memory( %s ) dump end <====\n\n",
                            getName() ) ;
         trapFile.Write( _linebuff, len ) ;

         /// set last info
         _lastDumpTime = beginTime ;
         _lastStatTimes = statTimes ;
         _lastStatSize = statSize ;
         _lastTrackNum = trackNum ;
         _lastTrackSize = trackSize ;
      }

} ;
typedef _ossMemTrackCB ossMemTrackCB ;

static ossMemTrackCB gMemTrackCB( "SDB_OSS_MALLOC" ) ;

void ossMemTrack ( void *p )
{
   ossMemInfoAssit assit( p ) ;
   gMemTrackCB.memTrack( p, assit._file, assit._line, assit._size ) ;
}

void ossMemUnTrack ( void *p )
{
   ossMemInfoAssit assit( p ) ;
   gMemTrackCB.memUnTrack( p, assit._file, assit._line, assit._size ) ;
}

// dump memory trace info into memtrace file
INT32 ossMemTrace ( const CHAR *pPath )
{
   ossPrimitiveFileOp trapFile ;
   CHAR fileName [ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
   UINT32 len = 0 ;
   INT32 rc = SDB_OK ;

   // build mem trace file name
   len = ossSnprintf ( fileName, sizeof(fileName), "%s%s%u%s",
                       pPath, OSS_PRIMITIVE_FILE_SEP,
                       ossGetCurrentProcessID(),
                       SDB_OSS_MEMDUMPNAME ) ;
   if ( len >= sizeof( fileName ) )
   {
      rc = SDB_INVALIDPATH ;
      // file path invalid
      goto error ;
   }

   // open memtrace file
   rc = trapFile.Open ( fileName ) ;

   // dump into trap file
   if ( trapFile.isValid () )
   {
      trapFile.seekToEnd () ;
      gMemTrackCB.memTrace( trapFile ) ;
   }

done :
   trapFile.Close () ;
   return rc ;
error :
   goto done ;
}

void ossOnMemConfigChange( BOOLEAN debugEnable,
                           UINT32  memDebugSize,
                           BOOLEAN memDebugVerify )
{
   if ( debugEnable != ossMemDebugEnabled )
   {
      gMemTrackCB.reset() ;
   }

   ossEnableMemDebug( debugEnable, memDebugSize, memDebugVerify ) ;
}

#endif // (__cplusplus) && defined (SDB_ENGINE)

