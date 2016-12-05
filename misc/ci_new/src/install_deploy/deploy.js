/******************************************************
@decription:   deploy cluster in CI

@input:        mode: STANDALONE | G1D3 | G3D3
               hostList: e.g.['host1','host2','host3'], G1D3 and G3D3 need 3 hosts
               installDir: default "/opt/sequoiadb"
               diagLevel: 0 1 2 3 4 5, default 4
@author:       Ting YU 2016-04-26   
******************************************************/
if ( typeof(mode) === "undefined" ) 
{ 
   throw "invalid para: mode, can not be null"; 
}
if ( mode !== "STANDALONE" && mode !== "G1D3" && mode !== "G3D3" ) 
{ 
   throw "invalid para: mode, should be STANDALONE | G1D3 | G3D3"; 
}
if ( typeof(hostList) === "undefined" ) 
{ 
   throw "invalid para: hostList, can not be null"; 
}
if ( hostList.constructor !== Array ) 
{ 
   throw "invalid para: hostList, should be array"; 
}
if ( typeof(installDir) === "undefined" ) installDir = "/opt/sequoiadb";
if ( typeof(diagLevel) === "undefined" ) diagLevel = 4;

var cmPort         = 11790;
var tmpCoordPort   = 18800;
var cataPort       = 11800;
var coordPort      = 11810;
var dataPort1      = 21100;
var dataPort2      = 22100;
var dataPort3      = 23100;
var STANDALONEPort = 11810;
var databaseDir    = installDir + "/database";
var cmd = new Cmd();
var osArch = cmd.run('arch');
if ( osArch === "x86_64\n" ) 
{ 
   var fapValue = "fapmongo";
}
else
{
   var fapValue = "";
}

switch( mode )
{
   case "STANDALONE":
      deployStandalone();
      break;
           
   case "G1D3":
      deployClster( mode );
      break;
      
   case "G3D3":
      deployClster( mode );
      break;      
   
}

function deployStandalone()
{
   println("------deploy mode: STANDALONE");
   
   for( var i in hostList )
   {
      var hostname = hostList[i]; 
      println( "-----begin to create node in " + hostname );
      
      //create node
      var oma = new Oma( hostname, cmPort );
      oma.createData( STANDALONEPort, 
                      databaseDir+"/STANDALONE/"+STANDALONEPort,
                      {diaglevel:diagLevel, fap:fapValue} );
      oma.startNode( STANDALONEPort );     
   }
   
   println("------succed to deploy");
}

function deployClster( mode )
{
   println("------deploy mode: " + mode );   
   switch( mode )
   {              
      case "G1D3":
         var datargNum = 1;
         break;         
      case "G3D3":
         var datargNum = 3;
         break;            
   }   
   var controlHost = hostList[0];
   
   //1 create tmp coord
   println( "-----begin to create temp coord" );
   var oma = new Oma( controlHost, cmPort );
   oma.createCoord( tmpCoordPort, databaseDir+"/coord/"+tmpCoordPort );
   oma.startNode( tmpCoordPort );
   
   println("-----begin to link temp coord");
   var db = new Sdb( controlHost, tmpCoordPort );
   
   //2 create cata group
   println("-----begin to create cata group");
   var config = { diaglevel:diagLevel,
                  sharingbreak:30000,
                  diagnum:30,
                  optimeout:60000,
                  fap:fapValue,
                  transactionon:true
                };
   db.createCataRG( controlHost, cataPort, 
                    databaseDir+"/cata/"+cataPort,
                    config  ); 
                                     
   for(var i = 0; i < 600; i++ )  //wait for cata group to select primary node 
   {  
      try
      {
         sleep(100); 
         var cataRG = db.getRG("SYSCatalogGroup"); 
         break;       
      } 
      catch(e)
      {
         if( e !== -71 ) throw e;         
      }   
   }                                              
   
   var node1 = cataRG.createNode( hostList[1], cataPort, 
                                  databaseDir+"/cata/"+cataPort,
                                  config );
   var node2 = cataRG.createNode( hostList[2], cataPort, 
                                  databaseDir+"/cata/"+cataPort,
                                  config );
   node1.start();
   node2.start();
   
   //3 create coord group
   println("-----begin to create coord group");
   var coordRG = db.createCoordRG();
   for( var i in hostList )
   {
      var config = {  diaglevel:diagLevel,                      
                      diagnum:30,
                      optimeout:60000,
                      fap:fapValue 
                   };
      coordRG.createNode( hostList[i], coordPort, 
                          databaseDir+"/coord/"+coordPort,
                          config );
   }
   coordRG.start();  
   
   //4 create data groups
   for( var n = 1; n <= datargNum; n++ )
   {
      var datargName = "group" + n;
      var dataPort = eval( "dataPort" + n );
      println( "-----begin to create data group: " + datargName );
      
      var dataRG = db.createRG( datargName );
      
      // random array
      var tmpHostList = hostList.sort(function(){return 0.5-Math.random()});
      
      for( var i in tmpHostList )
      {
         var config = { diaglevel:diagLevel,
                        sharingbreak:30000,
                        diagnum:30,
                        optimeout:60000,
                        fap:fapValue
                      };
         dataRG.createNode( tmpHostList[i], dataPort, 
                            databaseDir+"/data/"+dataPort,
                            config );
      }
      dataRG.start();
   } 
   
   println("-----begin to remove temp coord");
   oma.removeCoord(18800); 
   
   println("------succed to deploy");
}
