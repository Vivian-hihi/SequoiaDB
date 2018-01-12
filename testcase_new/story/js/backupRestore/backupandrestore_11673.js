/*******************************************************************************
*@Description : Backup all and restore for cluster
*@Modify list :
*               2018-1-10  wenjing Wang Init
*******************************************************************************/
function backupTestCase11673()
{
    
}

backupTestCase11673.prototype = new backupTestCase( db ) ;
backupTestCase11673.prototype.constructor = backupTestCase11673 ;
backupTestCase11673.prototype.csName = COMMCSNAME + "_11673" ;
backupTestCase11673.prototype.reInit=
function()
{
   
}

backupTestCase11673.prototype.createShardingCL =
function()
{
   this.groupNames = [];
   this.groups = commGetGroups( this.db ) ;
   // 整个集群备份，任意挑一个组恢复
   var pos = Math.floor( Math.random() * this.groups.length ) ;
   this.group = this.groups[pos] ;
   
   for ( var i = 0; i < this.groups.length; ++i )
   {
      this.groupNames.push( this.groups[i][0].GroupName );
   }
   
   this.domainName = "backup11673" ;
   try
   {
      commDropCS( this.db, this.csName, true, "dropCS in the beginning") ; 
      this.db.getDomain( this.domainName ) ;
      this.db.dropDomain( this.domainName ) ;
   }
   catch( e )
   {
      if ( e !== -214)
         throw new Error( "getDomain(" + this.domainName +"),err=" + e ) ;
   }
   
   try
   {
      this.db.createDomain( this.domainName, this.groupNames, {AutoSplit:true} ) ;
   }
   catch(e)
   {
      throw new Error( "createDomain(" + domainName +"),err=" + e ) ;
   }
   
   var csOpt = {Domain:this.domainName, LobPageSize:4096 } ;
   commCreateCS( this.db, this.csName, false, "create cs in begin", csOpt ) ;
   var clOpt = {ShardingType:'hash',ShardingKey:{no:1}, ReplSize:-1}
   this.cl = commCreateCLByOption( this.db, this.csName, this.clName, clOpt,  true, false, 
                   "Create collection in the beginning" ) ;
}

backupTestCase11673.prototype.init=
function()
{
   this.createShardingCL() ;
   var primaryPos = this.group[0].PrimaryPos ;
   var hostName = this.group[primaryPos].HostName ;
   var svcName = this.group[primaryPos].svcname ;
   var dbPath = this.group[primaryPos].dbpath ;
   this.nodeinfo = new nodeInfo( this.group[0].GroupName, hostName, svcName, dbPath);
   this.cmd = getCmdByHostName( this.localCmd, hostName )  ;
   return true ;
}

backupTestCase11673.prototype.execTest=
function(backupName, path)
{
   var bakInfo = new backUpInfo( backupName, this.nodeinfo.dbPath + "bakfile" ) ;
   this.groupNames.push( "SYSCatalogGroup" ) ;
   this.docs = bakInsertData( this.cl ) ;
   println( "write docs: " + this.docs.length );
   this.oids.push( sdbPutLob( this.cl, path ) );
   println( "putLob: " + this.oids[0] ) ;
   
   // 全量备份
   bakBackup( this.db , { "Name": backupName, CompressionType: "zlib"} );
   this.checkBackupRes( bakInfo, 1, this.groupNames ) ;
   
   this.docs = bakInsertData( this.cl ) ;
   println( "write docs: " + this.docs.length );
   this.oids.push( sdbPutLob( this.cl, path ) );
   println( "putLob: " + this.oids[0] ) ;
   
   for (var i = 0; i < 1000; ++i )
   {
      commCreateCL( this.db, this.csName, this.clName+i, -1, true, true, false,
                            "Create collection for backup" ) ;
   }
   
   bakBackup( this.db , { "Name": backupName, EnsureInc: true, CompressionType: "zlib" } );
   this.checkBackupRes( bakInfo, 2, this.groupNames ) ;
   if ( this.group !== undefined )
   {
      println( "backup on " + this.group[0].GroupName ) ;
      this.removeNodeExceptPrimary() ;
   }
   sdbRestore( this.sdb, this.cmd, bakInfo, this.nodeinfo ) ;
   this.checkResult( 2 ) ;
}

backupTestCase11673.prototype.tearDown=
function()
{
   bakRemoveBackups( this.db, CHANGEDPREFIX, true ) ;
   commDropCS( this.db, this.csName, true, "dropCS in the end")
   try
   {
      this.db.getDomain( this.domainName ) ;
      this.db.dropDomain( this.domainName ) ;
   }
   catch( e )
   {
      if ( e !== -214)
         throw new Error( "getDomain(" + this.domainName +"),err=" + e ) ;
   }
   
}  
   
function main( db )
{
   if ( commIsStandalone(db) )
   {
      return ;
   }
   
   try
   {
      var testBackUp = new backupTestCase11673();
      if ( testBackUp.setUp() )
      {
         testBackUp.test() ;
      }
      testBackUp.tearDown() ;
   }
   catch( e )
   {  
      var tmp = new Error( "tmp" )
      if ( e.constructor === tmp.constructor)
      {
         println( e.fileName + ":" + e.lineNumber + " throw " + e.message );
         println( e.stack ) ;
      }
      throw e;
   }
}

main( db )
