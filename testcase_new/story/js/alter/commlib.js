/*******************************************************************************
@Description : alter common functions
@Modify list :
               2018-4-25  wuyan  Init
*******************************************************************************/

//inspect the alter field is success or not.
function checkAlterResult(clName, fieldName, expFieldValue)
{
   var clFullName = COMMCSNAME + "." + clName;   
   var cur = db.snapshot(8,{"Name":clFullName});
   var actualFieldValue = cur.current().toObj()[fieldName];
   
   if ( typeof(expFieldValue) === "object" )
   {      
      if (JSON.stringify(expFieldValue) !== JSON.stringify(actualFieldValue))
      {
         throw buildException("test fieldvalue1", "check field", "value is wrong", JSON.stringify(expFieldValue), JSON.stringify(actualFieldValue));
      }
      
   }
   else
   {
      if (expFieldValue  !== actualFieldValue)
      {
         throw buildException("test fieldvalue2", "check field", "value is wrong", expFieldValue, actualFieldValue);
      }
   }
   
}

//inspect the alter cs field is success or not.
function checkAlterCSResult(csName, fieldName, expFieldValue)
{
   try
	{	   
      var rg = db.getRG("SYSCatalogGroup"); 
      var dbca = new Sdb(rg.getMaster());
      var cur = dbca.SYSCAT.SYSCOLLECTIONSPACES.find({"Name":csName});       
      while( cur.next() )
      {        
         var tempinfo = cur.current().toObj();
         var actFieldValue = tempinfo[fieldName];
      }      
      
      if (expFieldValue  !== actFieldValue)
      {
         
         println("---"+expFieldValue);
         throw buildException("test fieldvalue", "check field", "", expFieldValue, actFieldValue);
      }
      
	} 
   catch( e )
   {
      throw buildException( "check alter cs result:", e);      
   }
   finally
   {
      if( dbca != null )
      {
         dbca.close()
      }
   }       
}


/************************************
*@Description: check snapshot 
*@author:      luweikang
*@createDate:  2018.4.25
**************************************/
function checkSnapshot( db, snapType, csName, clName, field, expFieldValue)
{
   var clFullName = csName + "." + clName;
   var cursor = db.snapshot(snapType, {'Name': clFullName});
   var Obj = cursor.current().toObj();
   var actualFieldValue = Obj[field];
   println("expFieldValue   : " + expFieldValue);
   println("actualFieldValue: "+ actualFieldValue);
   if ( typeof(expFieldValue) === "object" )
   {      
      if (JSON.stringify(expFieldValue) !== JSON.stringify(actualFieldValue))
      {
         throw buildException("test fieldvalue1", "check field", "value is wrong", JSON.stringify(expFieldValue), JSON.stringify(actualFieldValue));
      }
      
   }
   else
   {
      if (expFieldValue  !== actualFieldValue)
      {
         throw buildException("test fieldvalue2", "check field", "value is wrong", expFieldValue, actualFieldValue);
      }
   }
   
}

/************************************
*@Description: get all groupName
*@author:      luweikang
*@createDate:  2018.4.25
**************************************/
function getGroupName(db)
{
   var groupArr = db.listReplicaGroups();
   var dataRGNames = new Array();
   while( groupArr.next() )
   {
      var groupObj = groupArr.current().toObj();
      var groupID = groupObj.GroupID;
      if(groupID >= 1000 )
      {
         dataRGNames.push(groupObj.GroupName);
      }
   }
   return dataRGNames;
}

/************************************
*@Description: get Split Group
*@author:      luweikang
*@createDate:  2018.4.25
**************************************/
function getSplitGroup( db, csName, clName )
{
   var clFullName = csName + "." + clName;
   var arr = getGroupName( db );
   var srcGroup = commGetCLGroups( db, clFullName )[0];
   var tarGroup = null;
   for(i in arr)
   {
      if( arr[i] !== srcGroup )
      {
         tarGroup = arr[i];
         break;
      }
   }
   var splitGroup = {};
   splitGroup.srcGroup = srcGroup;
   splitGroup.tarGroup = tarGroup;
   return splitGroup;
}

/************************************
*@Description: alter cl 
*@author:      luweikang
*@createDate:  2018.4.25
**************************************/
function clSetAttributes( cl, options )
{
   try
   {
      cl.setAttributes( options );
      throw "ALTER_SHOULD_ERR";
   }
   catch( e )
   {
      if( e !== -32 )
      {
         throw e;
      }
   }
}

/* *****************************************************************************
@discription: delete domain
@author: wuyan
@parameter
	dmName:the domain to be deleted
   ignoreExisted: default = true, value: true/false
   message: user define message, default:""
***************************************************************************** */
function dropDomain( db, dmName, ignoreNotExist, message )
{
   if ( ignoreNotExist == undefined ) { ignoreNotExist = true ; }
   if ( message == undefined ) { message = "" ; }
   try
   {
      db.dropDomain( dmName ) ;
   }
   catch( e )
   {
      if ( e === -214 && ignoreNotExist )
      {
         // think right
      }
      else
      {
      	throw buildException("drop domain",e,message,"dropDomain sucessfully","dropDomain fail");
      }
   }
}