/*******************************************************************
* @Description : test case for NumberLong
*                seqDB-14672:使用valueOf获取NumberLong值        
* @author      : Liang XueWang
*                2018-03-12
*******************************************************************/
main( db ) ;

function main( db )
{
   var number = 2147483648 ;
   var numberLong = NumberLong( number ) ;
   if( numberLong.valueOf() !== number )
   {
      throw buildException( "main", null, "check valueOf", 
            number, numberLong.valueOf() ) ;
   }
}