CSPREFIX_CS = CSPREFIX+"foo";
CSPREFIX_CL1 = CSPREFIX+"company";
CSPREFIX_CL2 = CSPREFIX+"department";
CSPREFIX_CL3 = CSPREFIX+"staff";

function createRand()
{
   var number = rand();
   number = parseInt(number*100000);	
   return number;
	
}
var db = new SecureSdb(COORDHOSTNAME,COORDSVCNAME)
try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   if ( e != -34)
   {
      println( "unexpected err happened when clear cs:" + e ) ;
      throw e ;
   }
}

try{
var claSize = new RSize( CSPREFIX_CS );

var varCS = db.createCS(CSPREFIX_CS);
// claSize.ReplSize()
var varCL1 = varCS.createCL(CSPREFIX_CL1,{ReplSize:claSize.ReplSize(),Compressed:true});
var varCL2 = varCS.createCL(CSPREFIX_CL2,{ReplSize:claSize.ReplSize(),Compressed:true});
var varCL3 = varCS.createCL(CSPREFIX_CL3,{ReplSize:claSize.ReplSize(),Compressed:true});
}catch( e ){
    throw e;
}

try{
	 var num;
	 num = createRand();
   varCL1.insert({company:"A",comID:1,number:num});
   num = createRand();
   varCL1.insert({company:"B",comID:2,number:num});
   num = createRand();
   varCL1.insert({company:"C",comID:3,number:num});
   num = createRand();
   varCL1.insert({department:"AA",dep_staff_num:100,depID:1,comID:1,number:num})
   num = createRand();
   varCL1.insert({department:"BB",dep_staff_num:200,depID:2,comID:1,number:num})
   num = createRand();
   varCL1.insert({department:"CC",dep_staff_num:300,depID:3,comID:2,number:num})
   num = createRand();
   varCL1.insert({department:"DD",dep_staff_num:400,depID:4,comID:2,number:num})
   num = createRand();
   varCL1.insert({department:"EE",dep_staff_num:500,depID:5,comID:3,number:num})
   num = createRand();
   varCL1.insert({department:"FF",dep_staff_num:600,depID:6,comID:3,number:num})
   for(var i=1;i<=100;i++){
   	  num = createRand();
      varCL1.insert({name:"Staff"+i,staffID:i,depID:1,comID:1,number:num});
   }
   println("insert into varCL1 depID:1,comID:1 complete \n");
   for(var i=101;i<=200;i++){
   	  num = createRand();
      varCL1.insert({name:"Staff"+i,staffID:i,depID:2,comID:1,number:num});	
   }
   println("insert into varCL1 depID:2,comID:1 complete \n");
   for(var i=201;i<=300;i++){
   	  num = createRand();
      varCL1.insert({name:"Staff"+i,staffID:i,depID:3,comID:2,number:num});	
   }
   println("insert into varCL1 depID:3,comID:2 complete \n");
   for(var i=301;i<=400;i++){
   	  num = createRand();
      varCL1.insert({name:"Staff"+i,staffID:i,depID:4,comID:2,number:num});	
   }
   println("insert into varCL1 depID:4,comID:2 complete \n");
   for(var i=401;i<=500;i++){
   	  num = createRand();
      varCL1.insert({name:"Staff"+i,staffID:i,depID:5,comID:3,number:num});	
   }
   println("insert into varCL1 depID:5,comID:3 complete \n");
   for(var i=501;i<=600;i++){
   	  num = createRand();
      varCL1.insert({name:"Staff"+i,staffID:i,depID:6,comID:3,number:num});	
   }
   println("insert into varCL1 depID:6,comID:3 complete \n");
   
   
   varCL2.insert({department:"AA",dep_staff_num:100,depID:1,comID:1,number:num})
   varCL2.insert({department:"BB",dep_staff_num:200,depID:2,comID:1,number:num})
   varCL2.insert({department:"CC",dep_staff_num:300,depID:3,comID:2,number:num})
   varCL2.insert({department:"DD",dep_staff_num:400,depID:4,comID:2,number:num})
   varCL2.insert({department:"EE",dep_staff_num:500,depID:5,comID:3,number:num})
   varCL2.insert({department:"FF",dep_staff_num:600,depID:6,comID:3,number:num})
   
   for(var i=1;i<=100;i++){
   	  num = createRand();
      varCL3.insert({name:"Staff"+i,staffID:i,depID:1,comID:1,number:num});
   }
   println("insert into varCL3 depID:1,comID:1 complete \n");
   for(var i=101;i<=200;i++){
   	  num = createRand();
      varCL3.insert({name:"Staff"+i,staffID:i,depID:2,comID:1,number:num});	
   }
   println("insert into varCL3 depID:2,comID:1 complete \n");
   for(var i=201;i<=300;i++){
   	  num = createRand();
      varCL3.insert({name:"Staff"+i,staffID:i,depID:3,comID:2,number:num});	
   }
   println("insert into varCL3 depID:3,comID:2 complete \n");
   for(var i=301;i<=400;i++){
   	  num = createRand();
      varCL3.insert({name:"Staff"+i,staffID:i,depID:4,comID:2,number:num});	
   }
   println("insert into varCL3 depID:4,comID:2 complete \n");
   for(var i=401;i<=500;i++){
   	  num = createRand();
      varCL3.insert({name:"Staff"+i,staffID:i,depID:5,comID:3,number:num});	
   }
   println("insert into varCL3 depID:5,comID:3 complete \n");
   for(var i=501;i<=600;i++){
   	  num = createRand();
      varCL3.insert({name:"Staff"+i,staffID:i,depID:6,comID:3,number:num});	
   }
   println("insert into varCL3 depID:6,comID:3 complete \n");
   
   
   
}catch( e ){
   println("insert data failed!!!");
   throw e;

}
var testi=50;
try{
	var com = varCL1.find({$and:[{company:{$exists:1},comID:{$exists:1},$not:[{depID:{$exists:1}}]}]},{company:null,comID:null}).sort({number:1});
	 println("com length is " + com.count() );
	 if( 3 != com.count() )
	    throw "com.count is not equals 3";
	 println("###############################################################");
	 println("find by three Collection");
	 println("###############################################################");
	 while( com.next() ){
	    var comID = com.current().toObj()["comID"];	
	    var dep = varCL2.find({$and:[{dep_staff_num:{$exists:1},depID:{$exists:1},comID:{$exists:1},comID:comID}]},{department:null,depID:null}).sort({number:1});
	    println("dep length is "+dep.count());
	    if( 2 != dep.count() )
	       throw "dep.count is not equals 2";
	    while(dep.next()){
	       var depID = dep.current().toObj()["depID"];
	       var i = 0;
	       var sta = varCL3.find({$and:[{name:{$exists:1},depID:depID,staffID:{$exists:1}}]}).sort({number:-1});	
	       println("staff length is "+sta.count());
	       if( 100 != sta.count() )
	          throw "sta.coutn is not equals 100";
	       while(sta.next()){
	       	 i++;
	       	 if( i == testi ){
	       	    try{
	       	       db.chenfoo.dropCL("jasflaksjfdlkasjlkfjals");
	       	       db.dropCS("sfjakljfdlajflkjaslfj");
	       	  }catch( e ){}
	       	 }
	       	 /*
		       println(com.current().toObj()["company"]
		       + "     "
		       + dep.current().toObj()["department"]
		       + "     "
		       + sta.current().toObj()["name"])
		       */
	       }
	    }
	    println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	    println(" one department is over ");
	    println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	    
	    
	    
	 }
	 var com = varCL1.find({$and:[{company:{$exists:1},comID:{$exists:1},$not:[{depID:{$exists:1}}]}]},{company:null,comID:null}).sort({number:1});
	 println("com length is " + com.count() );
	 if( 3 != com.count() )
	    throw "com.count is not equals 3";
	 println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	 println("###############################################################");
	 println("find by one Collection");
	 println("###############################################################");
	 while( com.next() ){
	    var comID = com.current().toObj()["comID"];	
	    var dep = varCL1.find({$and:[{dep_staff_num:{$exists:1},depID:{$exists:1},comID:{$exists:1},comID:comID}]},{department:null,depID:null}).sort({number:1});
	    println("dep length is "+dep.count());
	    if( 2 != dep.count() )
	       throw "dep.count is not equals 2";
	    while(dep.next()){
	       var depID = dep.current().toObj()["depID"];
	       var i = 0;
	       var sta = varCL1.find({$and:[{name:{$exists:1},depID:depID,staffID:{$exists:1}}]}).sort({number:1});	
	       println("staff length is "+sta.count());
	       if( 100 != sta.count() )
	          throw "sta.coutn is not equals 100";
	       while(sta.next()){
	       	 i++;
	       	 if( i == testi ){
	       	    try{
	       	       db.chenfoo.dropCL("jasflaksjfdlkasjlkfjals");
	       	       db.dropCS("sfjakljfdlajflkjaslfj");
	       	  }catch( e ){}
	       	 }
	       	 /*
		       println(com.current().toObj()["company"]
		       + "     "
		       + dep.current().toObj()["department"]
		       + "     "
		       + sta.current().toObj()["name"])
		       */
	       }
	    }
	    println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	    println(" one department is over ");
	    println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	 }
}catch( e ){
   println("find the value by cursor failed on three CL!!!");
   throw e;	
}


try
{
   db.dropCS( CSPREFIX_CS ) ;
}
catch ( e )
{
   println( "failed to drop cs, rc= " + e ) ;
   throw e ;
}

