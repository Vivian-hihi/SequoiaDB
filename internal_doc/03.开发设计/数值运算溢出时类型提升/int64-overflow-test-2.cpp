#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cassert>
using namespace std ;

typedef long long INT64 ;
typedef unsigned long long UINT64 ;

//                    1234567890123456
const INT64 MAX64 = 0x7fffffffffffffffLL ;
const INT64 MIN64 = 0x8000000000000000LL ;

const INT64 MAX32 = 0x000000007fffffffLL ;

int loop, count, safeCount, unsafeCount ;
INT64 *aNums, *bNums ;

INT64 rand64()
{
   int r[4] ;
   r[0] = rand() ;
   r[1] = rand() ;
   r[2] = 0 ;
   r[3] = rand() ;
   int i = rand() % 3 ;
   return *( (INT64*)(r+i) ) ;
}

inline bool safe( UINT64 a, UINT64 b )
{
   return a <= MAX32 && b <= MAX32 ;
}

bool safeTest( INT64 a, INT64 b )
{
   UINT64 x = a < 0 ? -a : a ;
   UINT64 y = b < 0 ? -b : b ;
   return safe(x,y) ;
}

void generateData()
{
   aNums = new INT64[count] ;
   bNums = new INT64[count] ;
   for( int i = 0; i < safeCount; ++i )
   {
      aNums[i] = rand() ;
      bNums[i] = rand() ;
      assert( safeTest(aNums[i],bNums[i]) ) ;
   }
   for( int i= safeCount; i < count; ++i )
   {
      aNums[i] = MAX32 + rand()/2 + 1 ;
      bNums[i] = rand64() ;
      assert( !safeTest(aNums[i],bNums[i]) ) ;
   }
   for( int i = 0; i < count; ++i )
      if( rand() % 2 )
         aNums[i] = -aNums[i] ;
   for( int i = 0; i < count; ++i )
      if( rand() % 2 )
         bNums[i] = -bNums[i] ;
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
   srand(time(0)) ;
   cout << "input <loop> <safe-count> <unsafe-count>" << endl ;
   cin >> loop >> safeCount >> unsafeCount ;
   count = safeCount + unsafeCount ;
   generateData() ;
   test( "only overflow", multiply_1 ) ;
   test( "safe+overflow", multiply_2 ) ;
}

