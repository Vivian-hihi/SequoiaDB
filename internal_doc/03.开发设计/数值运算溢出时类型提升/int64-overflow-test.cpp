#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cassert>
using namespace std ;

typedef long long INT64 ;
typedef unsigned long long UINT64 ;

//                    1234567890123456
#define      MAX64  0x7fffffffffffffffLL 
#define      MIN64  0x8000000000000000LL 

int loop, count, safeCount, unsafeCount, overflowCount ;
INT64 *aNums, *bNums ;

inline INT64 rand64bit()
{
   int r[2] ;
   r[0] = rand() ;
   r[1] = rand() ;
   return *( (INT64*)r ) ;
}

inline int highestOne( UINT64 a )
{
   int r = 0 ;
   if( a >> 32 )  { r+=32 ; a >>= 32; }
   if( a >> 16 )  { r+=16 ; a >>= 16; }
   if( a >> 8 )   { r+=8 ; a >>= 8; }
   if( a >> 4 )   { r+=4 ; a >>= 4; }
   if( a >> 2 )   { r+=2 ; a >>= 2; }
   if( a >> 1 )   { r+=1 ; a >>= 1; }
   r+=a;
   return r ;
}

inline bool safe( UINT64 a, UINT64 b )
{
   int ha = highestOne(a) ;
   int hb = 63 - ha ;
   return ( hb >= 0 ) && ( b >> hb ) == 0 ;
}

bool safeTest( INT64 a, INT64 b )
{
   UINT64 x = a < 0 ? -a : a ;
   UINT64 y = b < 0 ? -b : b ;
   return safe(x,y) ;
}

bool overflowTest( INT64 a, INT64 b )
{
   if( a > 0 && b > 0 ) return a > MAX64/b ;
   if( a > 0 && b < 0 ) return a > MIN64/b ;
   if( a < 0 && b > 0 ) return a < MIN64/b ;
   if( a < 0 && b < 0 ) return a < MAX64/b ;
   return false ;
}

//                            1234567890123456
#define _MASK(x)   ( UINT64(0xffffffffffffffff) >> ( 64 -(x) ) )
#define MASK(x)    ( x == 0 ? 0 : _MASK(x) )
#define HIGHEST_BIT(x) ( 1LL << ((x)-1) )

void generateData()
{
   aNums = new INT64[count] ;
   bNums = new INT64[count] ;
   
   for( int i=0; i < count; ++i )
   {
      aNums[i] = rand64bit() ;
      bNums[i] = rand64bit() ;
   }
   //generate safe numbers
   for( int i=0; i< safeCount; ++i )
   {
      int ax = rand() % 63 ;//0 ~ 63
      int bx = rand() % ( (63-ax) + 1 ) ;//0 ~ (63-ax)
      aNums[i] &= MASK(ax) ;
      bNums[i] &= MASK(bx) ;
      //for debug
      if( !safeTest(aNums[i],bNums[i]) ) 
      {
         cout << "should be safe : " << ax << "," << bx << " ; "
              << aNums[i] << "," << bNums[i] << " ; "
              << hex << "0x" << aNums[i] << ",0x" << bNums[i] << dec << endl ;
      }
      if( overflowTest(aNums[i],bNums[i]) ) 
      {
         cout << "should be not-overflow : " << ax << "," << bx << " ; "
              << aNums[i] << "," << bNums[i] << " ; "
              << hex << "0x" << aNums[i] << ",0x" << bNums[i] << dec << endl ;
      }
      assert( safeTest(aNums[i],bNums[i]) && !overflowTest(aNums[i],bNums[i]) ) ;
   }
   //generate unsafe numbers
   for( int i = safeCount; i < safeCount+unsafeCount; ++i )
   {
      int ax = rand() % 63 + 1 ;//1 ~ 63
      int bx = 64 - ax ;
      aNums[i] = HIGHEST_BIT(ax) ;
      bNums[i] = HIGHEST_BIT(bx) ;
      if( ax>bx )
         aNums[i] += rand()/2 ;
      else
         bNums[i] += rand()/2 ;

      //for debug
      if( safeTest(aNums[i],bNums[i]) ) 
      {
         cout << "should be unsafe : " << ax << "," << bx << " ; "
              << aNums[i] << "," << bNums[i] << " ; "
              << hex << "0x" << aNums[i] << ",0x" << bNums[i] << dec << endl ;
      }
      if( overflowTest(aNums[i],bNums[i]) ) 
      {
         cout << "should be not-overflow : " << ax << "," << bx << " ; "
              << aNums[i] << "," << bNums[i] << " ; "
              << hex << "0x" << aNums[i] << ",0x" << bNums[i] << dec << endl ;
      }
      assert( !safeTest(aNums[i],bNums[i]) && !overflowTest(aNums[i],bNums[i]) ) ;
   }
   //generate overflow numbers
   for( int i = safeCount+unsafeCount; i < count; ++i )
   {
      int ax = rand() % 62 + 2 ;//2 ~ 63
      int bx = rand() % ( 63 - (65-ax) + 1 ) + (65-ax) ;//(65-ax) ~ 63
      aNums[i] = HIGHEST_BIT(ax) | ( aNums[i] & MASK(ax) ) ;
      bNums[i] = HIGHEST_BIT(bx) | ( bNums[i] & MASK(bx) ) ;
      //for debug
      if( !overflowTest(aNums[i],bNums[i]) ) 
      {
         cout << "should be overflow : " << ax << "," << bx << " ; "
              << aNums[i] << "," << bNums[i] << " ; "
              << hex << "0x" << aNums[i] << ",0x" << bNums[i] << dec << endl ;
      }
      assert( overflowTest(aNums[i],bNums[i]) ) ;
   }

   for( int i=0; i < count; ++i )
   {
      if( rand() % 2 )  aNums[i] = -aNums[i] ;
   }
   for( int i=0; i < count; ++i )
   {
      if( rand() % 2 )  bNums[i] = -bNums[i] ;
   }
}

//only overflow
bool multiply_1( INT64 a, INT64 b )
{
   UINT64 max = ( ( a^b ) < 0 /*不同符号*/ ) ? MIN64 : MAX64 ;
   UINT64 x = a < 0 ? -a : a ;
   UINT64 y = b < 0 ? -b : b ;
   bool willOverflow =( x != 0 && y > max/x ) ;
   return willOverflow ;
}

//safe + overflow
bool multiply_2( INT64 a, INT64 b )
{
   UINT64 x = a < 0 ? -a : a ;
   UINT64 y = b < 0 ? -b : b ;
   bool multiplySafe = safe(x,y) ;
   if( multiplySafe ) 
   {
      return false ;
   }
   else
   {
      UINT64 max = ( ( a^b ) < 0 /*不同符号*/ ) ? MIN64 : MAX64 ;
      bool willOverflow =( x != 0 && y > max/x ) ;
      return willOverflow ;
   }
}
/*
bool multiply_3( INT64 a, INT64 b )
{
   UINT64 max = ( ( a^b ) < 0 ) ? MIN64 : MAX64 ;
   UINT64 x = a < 0 ? -a : a ;
   UINT64 y = b < 0 ? -b : b ;
   UINT64 x1 = x >> 32 ;
   UINT64 x2 = x & 0x00000000ffffffffLL ;
   UINT64 y1 = y >> 32 ;
   UINT64 y2 = y & 0x00000000ffffffffLL ;
   
   if( x1 == 0 && y1 == 0  )
   {
      return false ;
   }
   else if( x1 && y1 == 0 )
   {
      UINT64 x2y2 = x2*y2 ;
      return ( x1*y2 + (x2y2 >> 32) ) > ( max >> 32 ) ;
   }
   else if( y1 && x1 == 0 )
   {
      UINT64 x2y2 = x2*y2 ;
      return ( x2*y1 + (x2y2 >> 32) ) > ( max >> 32 ) ;
   }
   else
   {
      return true ;
   }
}
*/

typedef bool (*multiplyFunc)(INT64,INT64) ;
void test( const char *desc, multiplyFunc func )
{
   int ov = 0 ;
   clock_t beg, end ;
   beg = clock() ;
   for( int i = 0; i < loop; ++i )
      for( int x = 0; x < count; ++x )
         if( func(aNums[x],bNums[x]) )
            ++ov ;
   end = clock() ;
   cout << desc << " : " << endl ;
   cout << (ov/loop) << " overflow, " << (end-beg) << " clock ticks" << endl ;
}


int main()
{
   cout << "input <loop> <safe-count> <unsafe-but-not-overflow-count> <overflow-count>" << endl ;
   cin >> loop >> safeCount >> unsafeCount >> overflowCount ;
   count = safeCount + unsafeCount + overflowCount ;
   generateData() ;
   test( "only overflow", multiply_1 ) ;
   test( "safe+overflow", multiply_2 ) ;
}
