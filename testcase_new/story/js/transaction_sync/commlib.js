/* *****************************************************************************
@Description: sdb transaction common function 
@modify list:
   2014-4-1 YiBang Ruan  Init
***************************************************************************** */

import ( "../lib/fulltext_commlib.js" );

function dbNew( db )
{
   try
   {
      db = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
   }
   catch( e )
   {
      println( " new  Sdb failed : " + e ) ;
      throw new Error(e) ;
   }
}

function dbClose( db )
{
   try
   {
      db.close() ;
   }
   catch( e )
   {
      println( " close Sdb failed : " + e ) ;
      throw new Error(e) ;
   }
}

function dbArrayNew( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i] = new Sdb( COORDHOSTNAME, COORDSVCNAME ) ;
      }
   }
   catch( e )
   {
      println( " new the " + i + "st Sdb failed : " + e ) ;
      throw new Error(e) ;
   }
}

function dbArrayClose( db )
{
   try
   {
      for( i = 0; i < CONNECTNUM; ++i )
      {
         db[i].close() ;
      }
   }
   catch( e )
   {
      println( " close the" + i + "st Sdb failed : " + e ) ;
      throw new Error(e) ;
   }
}

/*******************************************************************************
@Description : 比较查询返回的结果（游标）与预期结果(数组)是否一致
@Modify list : 2018-10-15 zhaoyu init
*******************************************************************************/
function checkRec( rc, expRecs )
{				
	//get actual records to array
	var actRecs = [];
   while( rc.next() )
   {
		actRecs.push( rc.current().toObj() );
   }
   
   //check count
	if( actRecs.length !== expRecs.length )
   {
   	//println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));
   	throw new Error("expect count: " + expRecs.length + ",actual count: " + actRecs.length);
   }
   
   //check every records every fields
   for( var i in expRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	for ( var f in expRec )
   	{
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{	
	   		throw new Error("error occurs in " + (parseInt(i)+1) + "th record, in field' " + f + "'expect record: " + JSON.stringify(expRec) + ",actual record: " + JSON.stringify(actRec));
	   	}
   	}
   }
   
   //check every records every fields,actRecs as compare source
   for( var i in actRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	
   	for ( var f in actRec )
   	{
   	   if(f == "_id")
   	   {
   	      continue;
   	   }
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		throw new Error("error occurs in " + (parseInt(i)+1) + "th record, in field' " + f + "'expect record: " + JSON.stringify(expRec) + ",actual record: " + JSON.stringify(actRec));
	   	}
   	}
   }
}

/*****************************************************************
*@Description : check consistency: LSNs consistency of all nodes, ES consistency
@input:         csName
                clName
******************************************************************/
function checkConsistency(csName, clName)
{
   // check LSN consistency
   if(csName == null) { var csName = "UNDEFINED"; }
   if(clName == null) { var clName = "UNDEFINED"; }

   var groups = commGetCLGroups( db, csName + "." + clName );
   
   //the longest waiting time is 600S
   var lsnFlag = false;
   var timeout = 600;
   var doTimes = 0; 
   
   //get primary nodes
   var primaryNodeLSNs = getPrimaryNodeLSNs(groups);
   while(true)
   {
      lsnFlag = checkLSN(groups, primaryNodeLSNs);
      if(!lsnFlag)
      {
         if(doTimes < timeout)
         {
            ++doTimes;
            sleep(1000);
         }
         else
         {
            throw new Error("check lsn time out");
         }     
      }
      else 
      {
         break;
      }
   }

   println("check consistency success!");
}

/*****************************************************************
*@Description: check lsn consistency
@input:         groups
                primaryNodeLSNs
******************************************************************/
function checkLSN(groups, primaryNodeLSNs)
{
   var slaveNodeLSNs = getSlaveNodeLSNs(groups);
 
   var checkLSN = true;
   //比较主备节点lsn
   for(var i = 0; i < slaveNodeLSNs.length; ++i)
   {
      for(var j = 0; j < slaveNodeLSNs[i].length; ++j)
      {
         if(primaryNodeLSNs[i][0] > slaveNodeLSNs[i][j])
         {
            checkLSN = false;
            return checkLSN;
         }
      }
   }
   
   return checkLSN;
}

/*****************************************************************
*@Description: get lsn of primary node
@input:        groups
******************************************************************/
function getPrimaryNodeLSNs(groups)
{
   
   var datas = getNodesInGroups(groups);
   
   var LSNs = new Array();
   for(var i = 0; i < datas.length; ++i)
   {
      var nodesInGroup = datas[i];
      LSNs[i] = Array();
      for(var j = 0; j < nodesInGroup.length; ++j)
      { 
         var getSnapshot6 = eval( "(" + nodesInGroup[j].snapshot(6).toArray()[0] + ")" );
         
         var completeLSN = getSnapshot6.CompleteLSN;
         var isPrimary = getSnapshot6.IsPrimary;
         if(isPrimary)
         {
            LSNs[i][0] = completeLSN;
            break;
         }   
      }
   }
   
   return LSNs;
}

/*****************************************************************
*@Description: get lsn of slave node
@input:        groups
******************************************************************/
function getSlaveNodeLSNs(groups)
{
   var datas = getNodesInGroups(groups);
   
   var LSNs = new Array();
   for(var i = 0; i < datas.length; ++i)
   {
      var nodesInGroup = datas[i];
      LSNs[i] = Array();
      var f = 0;
      for(var j = 0; j < nodesInGroup.length; ++j)
      { 
         var getSnapshot6 = eval( "(" + nodesInGroup[j].snapshot(6).toArray()[0] + ")" );
         
         var completeLSN = getSnapshot6.CompleteLSN;
         var isPrimary = getSnapshot6.IsPrimary;
         if(!isPrimary)
         {
            LSNs[i][f++] = completeLSN;
         }   
      }
   }
   
   return LSNs;
}

/*****************************************************************
*@Description: get all nodes of all groups
@input:        groups
******************************************************************/
function getNodesInGroups(groups)
{
   var datas = new Array();
   
   //standalone
   if(true === commIsStandalone(db))
   {
      datas[0] = Array();
      datas[0][0] = db;
   }else{
      for (var i = 0 ; i < groups.length; ++i)
      {
         datas[i] = Array();
         
         var rg = db.getRG(groups[i]);
         var rgDetail = eval( "(" + rg.getDetail().toArray()[0] + ")");
         var nodesInGroup = rgDetail.Group;
         for(var j = 0; j < nodesInGroup.length; ++j)
         {
            var hostName = nodesInGroup[j].HostName;
            var serviceName = nodesInGroup[j].Service[0].Name;
            datas[i][j] = new Sdb(hostName, serviceName);                                                                                                                                 
         }
         
      }
   }
   return datas;
}

/*****************************************************************
@description:   sort object in array, rg:
                array.sort(compare("key1", compare("key2", compare("key3"))));       
@input:         key
                o: object1's key
                p: object2's key					 
******************************************************************/
function compare(name, minor) {
   return function (o, p) {
      var a, b;
      if (o && p && typeof o === 'object' && typeof p === 'object') {
         a = o[name];
         b = p[name];
         if (a === b) {
            return typeof minor === 'function' ? minor(o, p) : 0;
         }
         if (typeof a === typeof b) {
            return a < b ? -1 : 1;
         }
         return typeof a < typeof b ? -1 : 1;
      } else {
         throw new Error("error");
      }
   }
}

/*****************************************************************
@description:   check result        
@input:         expectResult
                actResult
******************************************************************/
function checkResult(expectResult, actResult)
{
   if(expectResult.length !== actResult.length)
   {
      throw new Error("record length failed, expect record count: " + expectResult.length + ", actual record count: " + actResult.length);
   }

   // compare array  
   for( var i in expectResult )
   {
      var actRec = actResult[i];
      var expRec = expectResult[i];
   	
      for ( var f in expRec )
      {
         if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) ) 
         {
            throw new Error("check record failed, expect record: " + JSON.stringify(expRec) + ", actual record: " + JSON.stringify(actRec));
         }
      }
   }
   
   for( var j in actResult )
   {
      var actRec = actResult[j];
      var expRec = expectResult[j];
   	
      for ( var f in actRec )
      {
         if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
         {
            throw new Error("check record failed, expect record: " + JSON.stringify(expRec) + ", actual record: " + JSON.stringify(actRec));
         }
      }
   }
	
   println("check results success!");
}

/*****************************************************************
@description:   check index in elasticsearch not exist       
@input:         csName
                clName
                textIndexName
******************************************************************/
function checkIndexNotExistInES(csName, clName, esIndexNames)
{
   // check indexnames in ES not exist
   for(var i in esIndexNames)
   {
      if(esOpr.isDropIndexInES(esIndexNames[i]))
      {
         throw new Error("index name exists in ES");
      }
   }
}

/******************************************************************************
*@Description : remove duplicate items in array
@input:         array
******************************************************************************/
function removeDuplicateItems(array)
{
   var uniqueArray = new Array();
   for (var i in array){
      if (uniqueArray.indexOf(array[i]) == -1){
         uniqueArray.push(array[i]);
      }
   }
   return uniqueArray;
}

/************************************
*@Description: insert data
*@author:      wuyan
*@createDate:  2018.1.22
**************************************/
function insertData( dbcl, number)
{
   if( undefined == this.number ){ this.number = 1000 ; }
   println("---Begin to insert data " );   
   var docs = [];
   for( var i = 0; i < number; ++i )
   {      
      var no = i;
      var a = i;
      var user = "test"+i;
      var phone = 13700000000+i;
      var time = new Date().getTime(); 
      var doc = {no:no, a:a,customerName:user, phone:phone, openDate:time};      
      //data example: {"no":5, customerName:"test5", "phone":13700000005, "openDate":1402990912105
      
      docs.push( doc );
   }	
   dbcl.insert( docs ); 
}

function beginTrans( db )
{
   db.transBegin(); 
}


function commitTrans( db )
{
   db.transCommit() ;
}

function rollbackTrans()
{
   db.transRollback() ;
}

/************************************
*@Description: check the new cl name 
*@author:      wuyan
*@createDate:  2018.10.12
**************************************/
function checkRenameCLResult( csName, oldCLName, newCLName)
{   
   var clFullName = csName + "." + newCLName; 
   var getNewCLName = db.snapshot(SDB_SNAP_COLLECTIONS ,{"Name": clFullName }).current().toObj().Name;     
   if( getNewCLName !== clFullName  )
   {
      throw new Error("check the new cl name, old cl name: " + clFullName + ", new cl name: " + getNewCLName);
   }   
   
   //check the old cl is not exist
   try
   {
	   db.getCS(csName).getCL( oldCLName );
	   throw new Error("need throw error");
   }
   catch ( e )
   { 
	   if ( e !== -23  )
	   {		      
		   throw new Error(e);
	   }		
   }  
}

/************************************
*@Description: check the new cs name 
*@author:      luweikang
*@createDate:  2018.10.13
**************************************/
function checkRenameCSResult( oldCSName, newCSName, clNum)
{   
   var newCSObj = db.snapshot(SDB_SNAP_COLLECTIONSPACES ,{"Name": newCSName }).current().toObj();     
   var getNewCSName = newCSObj.Name;
   if( getNewCSName !== newCSName  )
   {
      throw new Error("check the new cs name, expect cs name: " + newCSName + ", actual cs name: " + getNewCSName);
   }
   
   var clArray = newCSObj.Collection;
   
   if(clNum != clArray.length){
      throw new Error("check cl num, expect cl num: " + clNum + ",actual cl num: " + clArray.length);
   }
   
   for( i = 0; i< clArray.length; i++)
   {
      var csname = clArray[i].Name.split(".")[0];
      if( csname !== newCSName  )
      {
         throw new Error("expect cs name:" + newCSName + ",actual cs name: " + csname);
      }
   }
   
   //check the old cl is not exist
   try
   {
	   db.getCS(oldCSName);
	   throw new Error("CS_IS_EXIT");
   }
   catch ( e )
   { 
	   if ( e !== -34  )
	   {		      
		   throw new Error(e);
	   }		
   } 
}