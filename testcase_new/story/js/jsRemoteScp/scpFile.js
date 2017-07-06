/************************************
*@Description: 
*@author:      zhaoyu
*@createdate:  2017.4.14
*@testlinkCase:seqDB-11338/seqDB-11339/seqDB-11340/seqDB-11346
**************************************/
function main(){
   var localhost = toolGetLocalhost();
   println("localhost:" + localhost);
   var localFileName = "/tmp/test_11338";
   localFile = new File(localFileName , 0777);
   
   var remotehost = toolGetRemotehost();
   println( "remotehost:" + remotehost );
   var remote = new Remote( remotehost, CMSVCNAME );
   var remoteFile = remote.getFile();
   
   if(!localFile.exist(WORKDIR)){
      localFile.mkdir(WORKDIR, 0777); 
   }
   
   var localCmd = new Cmd();
   var localInstallPath = commGetInstallPath();
   var localSrcFileName = localInstallPath + "/bin/sdbdmsdump";
   println( "local source file name :" + localhost + ":" + localSrcFileName );
   var localDstFileName = WORKDIR + "/dstFile_11338";
   
   if(!remoteFile.exist(WORKDIR)){
      remoteFile.mkdir(WORKDIR, 0777); 
   }
   
   var remoteCmd = remote.getCmd();
   var remoteInstallPath = commGetInstallPath();
   var remoteSrcFileName = remoteInstallPath + "/bin/sdbdmsdump";
   println( "remote source file name :" + remotehost + ":" + remoteSrcFileName );
   var remoteDstFileName = WORKDIR + "/dstFile_11338";
   
   //seqDB-11338
   scpTest(localSrcFileName, localDstFileName, localFile, localFile);
   
   scpTest(remotehost + ":" + CMSVCNAME + "@" + remoteSrcFileName, 
           remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName, 
           remoteFile, remoteFile);
           
   scpTest(remotehost + ":" + CMSVCNAME + "@" + remoteSrcFileName, 
           localDstFileName, 
           remoteFile, localFile);
           
   scpTest(localSrcFileName, 
           remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName, 
           localFile, remoteFile);
   println("check scp from source to destination success");
   
   //seqDB-11339
   //remote to remote
   try{
      replaceFile = remote.getFile(remoteDstFileName);
      File.scp(remotehost + ":" + CMSVCNAME + "@" + remoteSrcFileName, 
               remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName,
               false);
      throw "EXPECT GET AN ERROR";
   }catch(e){
      if( e !== -5){
         throw buildException("scp()", e, e, -5, e);
      }
   }
   replaceFile.remove(remoteDstFileName);
   
   //local to remote
   try{
	  replaceFile = remote.getFile(remoteDstFileName);
	  File.scp(localSrcFileName, 
			   remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName,
			   false);
	  throw "EXPECT GET AN ERROR";
	}catch(e){
	  if( e !== -5){
		 throw buildException("scp()", e, e, -5, e);
	  }
	}
	replaceFile.remove(remoteDstFileName);
   
   //remote to local
   try{
      replaceFile = new File(localDstFileName);
	  File.scp(remotehost + ":" + CMSVCNAME + "@" + remoteSrcFileName, 
			   localDstFileName,
			   false);
	  throw "EXPECT GET AN ERROR";
	}catch(e){
	  if( e !== -5){
		 throw buildException("scp()", e, e, -5, e);
	  }
	}
	replaceFile.remove(localDstFileName);
   
   //local to local
   try{
	  replaceFile = new File(localDstFileName);
	  File.scp(localSrcFileName, 
			   localDstFileName,
			   false);
	  throw "EXPECT GET AN ERROR";
	}catch(e){
	  if( e !== -5){
		 throw buildException("scp()", e, e, -5, e);
	  }
	}
	replaceFile.remove(localDstFileName);
	println("check replace set false success");

	try{
	  if(remoteFile.exist(remoteDstFileName)){
		 remoteFile.remove(remoteDstFileName);
	  }
	  var srcMode = 447;
	  replaceFile = remote.getFile(remoteDstFileName, srcMode);
	  File.scp(remotehost + ":" + CMSVCNAME + "@" + remoteSrcFileName, 
			   remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName,
			   true, 0711);
	  
	  //check size         
	  var expectMd5 = remoteFile.md5(remoteSrcFileName);
	  var actualMd5 = remoteFile.md5(remoteDstFileName);
	  if( expectMd5 !== actualMd5){
		 throw "REPLACE_SET_TRUE_NOT_OK";
	  }
	  
	  //check mode
	  var umask = remoteFile.getUmask();
	  var expectMode = srcMode&~umask;
	  var actualMode = remoteFile._getPermission(remoteDstFileName);
	  if(expectMode !== actualMode){
		 throw "MODE_NOT_OK";
	  }
	}catch(e){
	  throw buildException("scp()", e, e, expectMode, actualMode);
	  
	}
	remoteFile.remove(remoteDstFileName);
	println("check replace set true success");

	//seqDB-11340
	scpTest(localSrcFileName, 
		   remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName,
		   localFile, remoteFile, 448)
	println("check set mode copy success");

	//seqDB-11346        
	checkArgumentScp(localInstallPath, remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName,
					remoteFile, 0755, true);
					
	checkArgumentScp(remotehost + ":" + CMSVCNAME + remoteSrcFileName, remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName,
					remoteFile, 0755, true);

	checkArgumentScp("", remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName, remoteFile, 0755, true);
					
	checkArgumentScp(localSrcFileName, remotehost + CMSVCNAME + "@" + remoteDstFileName,
					remoteFile, 0755, true);
				 
	checkArgumentScp(localSrcFileName, WORKDIR, remoteFile, 0755, true);

	checkArgumentScp(localSrcFileName, "", remoteFile, 0755, true);

	//SEQUOIADBMAINSTREAM-2378
	/*checkArgumentScp(localSrcFileName, 
					remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName, 0800, true);
					
	checkArgumentScp(localSrcFileName, 
					remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName, 512, true);*/

	scpTest(localSrcFileName, 
		   remotehost + ":" + CMSVCNAME + "@" + remoteDstFileName, 
		   localFile, remoteFile, 0755, "a");
		 
	File.remove(localFileName);
}
main();

function scpTest(srcFileName, dstFileName, srcFile, dstFile, mode, isReplace){
   try{
      File.scp(srcFileName, dstFileName, isReplace, mode);
      
      srcIndex = srcFileName.indexOf("@");
      dstIndex = dstFileName.indexOf("@");
      if( srcIndex > 0 ){
         md5SrcFileName = srcFileName.substring(srcIndex+1);
      }else{
         md5SrcFileName = srcFileName;
      }
      if( dstIndex > 0 ){
         md5DstFileName = dstFileName.substring(dstIndex+1);
      }else{
         md5DstFileName = dstFileName;
      }
      
      var expectMd5 = srcFile.md5(md5SrcFileName);
      var actualMd5 = dstFile.md5(md5DstFileName);
      
      var srcMode = srcFile._getPermission(md5SrcFileName);
      var dstMode = dstFile._getPermission(md5DstFileName);
      if ( typeof(mode) == "undefined" ) { mode = srcMode; }
     
      var umask = dstFile.getUmask() ;
      mode = mode & ~umask ; 
      dstFile.remove(md5DstFileName);
      
      if(expectMd5 !== actualMd5 || mode !== dstMode){
         throw "MD5_MODE_NOT_SAME";
      }
   }catch(e){
      throw buildException("scp()", e, e, "expectMd5:" + expectMd5 + ",mode:" + mode, ",actualMd5:" + actualMd5 + ",dstMode:" + dstMode);
   }  
}

function checkArgumentScp(srcFileName, dstFileName, dstFile, mode, isReplace){
   try{
      File.scp(srcFileName, dstFileName, isReplace, mode);
      throw "EXPECT GET AN ERROR";
   }catch(e){
      if(e !== -6 && e !== -1 && e !== -4 && e !== -3){
         throw buildException("checkArgumentScp()", e, e, "FAILED", "SUCCESS");
      }
   }
   
   dstIndex = dstFileName.indexOf("@");
   if( dstIndex > 0 ){
      md5DstFileName = dstFileName.substring(dstIndex+1);
   }else{
      md5DstFileName = dstFileName;
   }
   
   //check
   //println("md5DstFileName:" + md5DstFileName);
   try{
      if(dstFile.exist(md5DstFileName) && dstFile.isFile(md5DstFileName)){
         throw "FILE_EXIST"
      }
   }catch(e){
      throw buildException("checkArgumentScp()", e, e, "FAILED", "SUCCESS");
   }
}
