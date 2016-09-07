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

//                           1234567890123456
const INT64 FIRST_2_BITS = 0xc000000000000000LL ;
//#define FIRST_2_BITS (0xcLL << 60) 

int loop, count, safeCount, unsafeCount, overflowCount ;
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

bool addSafeTest( INT64 a, INT64 b )
{
   if( (a^b) < 0 )  
   { 
      return true ; 
   }
   else if( a < 0 )    
   { 
      return ( ( a & b & FIRST_2_BITS ) == FIRST_2_BITS ) ;
   }
   else           
   {
      return 0 == ( (a|b) & FIRST_2_BITS ) ; 
   }
}

bool addOverflowTest( INT64 a, INT64 b )
{
   if(b<0)  
   {
      return a < MIN64 - b ;
   }
   else  
   {
      return a > (MAX64 - b) ;
   }
}

void generateData()
{
   // safe
   for( int i = 0; i < safeCount; ++i )
   {
      int r = rand() % 3 ;
      INT64 a, b ;
      if(0==r)
      {
         // a >= 0 && b >= 0
         a = rand64() ;
         b = rand64() ;
         a &= ( ~(FIRST_2_BITS) ) ;
         b &= ( ~(FIRST_2_BITS) ) ;
      }
      else if(1==r)
      {
         a = rand64() ;
         b = -rand64() ;
      }
      else
      {
         a = -rand64() ;
         b = -rand64() ;
         a |= FIRST_2_BITS ;
         b |= FIRST_2_BITS ;
      }
      
      if( !addSafeTest(a,b) )
      {
         cout << "should be safe ; "
              << i << " ; " << a << "," << b << " ; "
              << hex << "0x" << a << ",0x" << b << dec
              << endl ;
      }
      assert( addSafeTest(a,b) ) ;
      aNums[i] = a ;
      bNums[i] = b ;
   }
   // unsafe but not overflow
   for( int i = safeCount; i < safeCount+unsafeCount; ++i )
   {
      int r = rand() % 2 ;
      INT64 a, b ;
      if(0==r)
      {
         a = rand64() ;
         b = rand64() ;
         a >>= 4 ;
         b >>= 4 ;
         //     1234567890123456
         a |= 0x4000000000000000LL ;
         b |= 0x2000000000000000LL ;
      }
      else
      {
         a = rand64() ;
         b = rand64() ;
         a >>= 4 ;
         b >>= 4 ;
         //     1234567890123456
         a |= 0xb000000000000000LL ;
         b |= 0xe000000000000000LL ;
      }

      if( addSafeTest(a,b) )
      {
         cout << "should be unsafe ; "
              << i << " ; " << a << "," << b << " ; "
              << hex << "0x" << a << ",0x" << b << dec
              << endl ;
      }
      if( addOverflowTest(a,b) )
      {
         cout << "should not be overflow ; "
              << i << " ; " << a << "," << b << " ; "
              << hex << "0x" << a << ",0x" << b << dec
              << endl ;
      }
      assert( !addSafeTest(a,b) && !addOverflowTest(a,b) ) ;
      aNums[i] = a ;
      bNums[i] = b ;
   }
   for( int i = safeCount+unsafeCount; i < count; ++i )
   {
      INT64 a, b ;
      if(0==rand()%2)
      {
         a = rand64() ;
         b = rand64() ;
         //     1234567890123456
         a |= 0x4000000000000000LL ;
         b |= 0x4000000000000000LL ;
      }
      else
      {
         a = -rand64() ;
         b = -rand64() ;
         //     1234567890123456
         a &= 0xbfffffffffffffffLL ;
         b &= 0xbfffffffffffffffLL ;
      }

      if( addSafeTest(a,b) )
      {
         cout << "should be unsafe ; "
              << i << " ; " << a << "," << b << " ; "
              << hex << "0x" << a << ",0x" << b << dec
              << endl ;
      }
      if( !addOverflowTest(a,b) )
      {
         cout << "should be overflow ; "
              << i << " ; " << a << "," << b << "," <<(a+b)<< " ; "
              << hex << "0x" << a << ",0x" << b << ",0x" << (a+b) << dec
              << endl ;
      }
      assert( !addSafeTest(a,b) && addOverflowTest(a,b) ) ;
      aNums[i] = a ;
      bNums[i] = b ;
   }
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

// add-safe + add-overflow
bool add_2( INT64 a, INT64 b )
{
   if( (a^b) < 0 ) 
   {
      return false ;
   }
/*
   if( b > 0 ) 
   {
      return a > MAX64 - b ;
   }
   else
   {
      return a < MIN64 - b ;
   }
*/
   else if( a >= 0 )
   {
      if( (a>>62) == 0 && (b>>62) == 0 )
      {
         return false ;
      }
      else // not safe
      {
         return b > MAX64 - a ;
      }
   }
   else // a < 0
   {
      if( (UINT64(a)>>62) == 3 && (UINT64(b)>>62) == 3 )
      {
         return false ;
      }
      else // not safe
      {
         return b < MIN64 - a ;
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
   cout << "input <loop> <safe-count> <unsafe-but-not-overflow-count> <overflow-count>" << endl ;
   cin >> loop >> safeCount >> unsafeCount >> overflowCount ;
   count = safeCount + unsafeCount + overflowCount ;
   aNums = new INT64[count] ;
   bNums = new INT64[count] ;
   generateData() ;
   addTest( "only overflow", add_1 ) ;
   addTest( "safe+overflow", add_2 ) ;
}
