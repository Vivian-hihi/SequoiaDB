package com.sequoias3.testcommon;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;

public class TestResidualData extends S3TestBase{
    Sequoiadb db = null;
	int errorCount = 0;
    String printInfo = "";

	@BeforeClass
	private void setUp() {}

	@Test
	private void printResidualData() throws Exception{
    	List<String> csNames = new ArrayList<String>();
    	List<String> s3CSNames = new ArrayList<String>();
        db =  new Sequoiadb(S3TestBase.coordUrl, "", "");
        csNames = db.getCollectionSpaceNames();
        
        for(String csName : csNames){
        	if(csName.startsWith("S3_")){
        		s3CSNames.add(csName);
        	}
        }
        
        for(String csName : s3CSNames){
        	CollectionSpace cs = db.getCollectionSpace(csName);
        	List<DBCollection> clList = new ArrayList<DBCollection>();
            List<String> clNameList = cs.getCollectionNames();
            for(String csclName : clNameList){
                String clname = csclName.substring(cs.getName().length()+1);
                clList.add(cs.getCollection(clname));
            }
            printCLResidualData(cs, clList);
        }
        
        writeToFile();
    }
	
	private void printCLResidualData(CollectionSpace cs, List<DBCollection> clList){
        DBCursor cursor = null;
        try{
		    for(DBCollection cl : clList) {
		        cursor = cl.query();
		        if(cursor.hasNext()) {
		        	printInfo+="\n===============begin print " + cs.getName() +"."+ cl.getName() + " data============\n";
		            while(cursor.hasNext()){
		                if(cursor.getNext().containsField("Name")){
		                    if(!cursor.getCurrent().get("Name").equals(S3TestBase.s3UserName)&&!cursor.getCurrent().get("Name").equals(S3TestBase.bucketName)&&!cursor.getCurrent().get("Name").equals(S3TestBase.enableVerBucketName)){
		                    	printInfo+=cursor.getCurrent().toString()+"\n";
		                        errorCount++;
		                    }
		                }else{
		                	printInfo+=cursor.getCurrent().toString()+"\n";
		                    errorCount++;
		                }
		            }
		            printInfo+="===============end print " + cs.getName() +"."+ cl.getName() + " data==============\n";
		        }
		    }
		    cursor.close();
        }finally{
        	if(cursor != null){
                cursor.close();
            }
        }
	}
	
	private void writeToFile() throws Exception{
	    File file = new File(S3TestBase.installPath+"/tools/sequoias3/log/residualdata.log");
	    if(!file.exists()) {
	        file.createNewFile();
	    }
	    
	    FileWriter fw = new FileWriter(file.getAbsoluteFile(),false);
	    BufferedWriter bw = new BufferedWriter(fw);
	    bw.write(printInfo);
	    bw.close();
	    
	    if(errorCount != 0){
	        throw new Exception("There is data residue problem");
	    }
	}
	@AfterClass
	private void tearDown() throws Exception {
    	if(db != null){
            db.close();
        }
	}
}
