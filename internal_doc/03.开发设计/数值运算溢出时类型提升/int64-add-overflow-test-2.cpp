#include <iostream>
#include <cassert>
#include <cstdlib>
#include <ctime>

using namespace std ;

typedef long long INT64 ;
typedef unsigned long long UINT64 ;

//                    1234567890123456
const INT64 MAX64 = 0x7fffffffffffffffLL ;
const INT64 MIN64 = 0x8000000000000000LL ;

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

// only add-overflow checking
bool add_1( INT64 a, INT64 b )
{
   if( b > 0 )
   {
      return a > MAX64 - b ;
   }
   else
   {
      return a < MIN64 - b ;
   }
}

// safe + add-overflow checking
bool add_2( INT64 a, INT64 b )
{
   if( (a^b) < 0 )
   {
      return false ;
   }
   if( b > 0 )
   {
      return a > MAX64 - b ;
   }
   else
   {
      return a < MIN64 - b ;
   }
}

void generateData()
{
   for( int i = 0; i < safeCount; ++i )
   {
      aNums[i] = rand64() ;
      bNums[i] = -rand64() ;
   }
   for( int i = safeCount; i < count; ++i )
   {
      if(rand()%2)
      {
         aNums[i] = rand64() ;
         bNums[i] = rand64() ;
      }
      else
      {
         aNums[i] = -rand64() ;
         bNums[i] = -rand64() ;
      }
   }
}

typedef bool (*addFunc)(INT64,INT64) ;
void addTest( const char *desc, addFunc func )
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
   aNums = new INT64[count] ;
   bNums = new INT64[count] ;
   generateData() ;
   addTest( "only overflow", add_1 ) ;
   addTest( "safe+overflow", add_2 ) ;
}


