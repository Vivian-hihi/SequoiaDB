/* *****************************************************************************
@discretion: 切分表执行事务操作，创建唯一索引插入相同记录
@author：2015-11-21 wuyan  Init
***************************************************************************** */

main();
function main()
{		
	try
	{
	   var clName = CHANGEDPREFIX + "_transaction6005";
      if( !commIsTransEnabled(db) )
      {
         println( "transaction is disabled" ) ;   
      }
      
      var cl = splitCl(COMMCSNAME, clName ); 
      
      transOperation(cl)  
      
      //@ clean end
		commDropCL( db, COMMCSNAME, clName, false, false,"drop CL in the beginning" );
   }
   catch( e )
   {
      throw e;
   }
   finally
   {
      if ( undefined !== db )
      {
         db.close();
      }
   }
}

function splitCl(csName,clName)
{
   var allGroupInfo = commGetGroups(db, true) 
   if( !(1 < allGroupInfo.length) )
   {
      println("least two groups");
      return;
   }
   var cl = commCreateCLByOption(db, COMMCSNAME, clName, 
                  {ShardingKey:{no:1}, ShardingType:"range",ReplSize:0}, true, true);
                  
   var CLName = csName + "." + clName
   var srcGroupName = commGetCLGroups( db, CLName );  
   println("srcGroupName="+srcGroupName)   
   for(var i=0; i != allGroupInfo.length; ++i)
   {        
      if(  srcGroupName != allGroupInfo[i][0].GroupName )
    	{

         try
         {                                     
            cl.split( srcGroupName.toString(),allGroupInfo[i][0].GroupName,{"no":0},{"no":100});                   
            break;                    
         }
         catch(e)
         {
            throw e;
         }
      }          
   }
   
   commCreateIndex( cl, 'idxtest', {no:-1}, true, false)  
   return cl;
   println("--end split")  
}

function transOperation(cl)
{
   var dataNum = 1000; 
   var insert = new insertData( cl, dataNum );       
   //remove data and left some datas,then commit transaction
   var removeLeftNum = 100;
   var remove = new removeData( cl , removeLeftNum );
   execTransaction(beginTrans,insert);
   checkResult( cl, true, insert ); 
   execTransaction(remove);
   checkResult( cl, true, remove ); 
   
   //insert  the same datas
   try
   {
      execTransaction(insert) ;
   }
   catch( e )   
   {
      if ( e == "insertData.exec() unknown error expect: -38" )
      {
         // think right
      }
      else  
      {
         throw buildException("execTransaction(insert)", e )
      }
   }
   
   //commit transaction after autoRollback 
   try
   {
      execTransaction(commitTrans) ;
   }
   catch( e )
   {
      if ( e == "commitTrans() unknown error expect: -196" )
      {
         // think right
      }
      else
      {
         throw buildException("execTransaction(commitTrans)", e )
      }
   }
      
   checkResult( cl, false, insert ) ;    
}


