/*******************************************************************
* @Description : list groups in domain
*                seqDB-14669:枚举域中的数据组         
* @author      : Liang XueWang
*                2018-03-12
*******************************************************************/
main( db ) ;

function main( db )
{
   if( commIsStandalone( db ) )
   {
      println( "Run mode is standalone" ) ;
      return ;
   }
   
   var groups = getDataGroups( db ) ;
   if( groups.length === 0 )
   {
      println( "No groups" ) ;
      return ;
   }
   
   var domainName = "testDomain14669" ;
   var domain = db.createDomain( domainName, groups ) ;
   
   var obj = domain.listGroups().next().toObj() ;
   
   // check domain name
   var name = obj["Name"] ;
   if( name !== domainName )
   {
      throw buildException( "main", null, "check domain name", domainName, name ) ;
   }
   
   // check group num
   var groupArr = obj["Groups"] ;
   if( groupArr.length !== groups.length )
   {
      throw buildException( "main", null, "check group num", groups.length, 
            groupArr.length ) ;
   }
   
   // check groups name
   for( var i = 0;i < groups.length;i++ )
   {
      if( groupArr[i]["GroupName"] !== groups[i] )
      {
         throw buildException( "main", null, "check group name", groups[i],
               groupArr[i]["GroupName"] ) ;
      }
   }
}