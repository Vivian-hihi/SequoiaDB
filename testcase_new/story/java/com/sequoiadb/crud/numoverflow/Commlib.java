package com.sequoiadb.crud.numoverflow;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BSONDecimal;
import org.bson.types.BSONTimestamp;
import org.bson.types.BasicBSONList;
import org.bson.util.JSON;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBQuery;
import com.sequoiadb.base.Node;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;


public class Commlib {
    
    /**
     * Judge the mode
     * @param sdb
     * @return true/false, true is standalone, false is cluster
     */
    public boolean isStandAlone(Sequoiadb sdb){
        try{
            sdb.listReplicaGroups();
        }catch(BaseException e){
            if( e.getErrorCode() == -159 ){  //-159:The operation is for coord node only
                //System.out.printf("The mode is standalone.");
                return true;
            }
        }
        return false;
    }
    
    public static List<String> getDataRgNames(Sequoiadb sdb) {
        List<String> rgNames = new ArrayList<String>();
        try {
            rgNames = sdb.getReplicaGroupNames();
            rgNames.remove("SYSCatalogGroup");
            rgNames.remove("SYSCoord");
            rgNames.remove("SYSSpare");
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
        return rgNames;
    }
    
    public static DBCollection createCL(CollectionSpace cs, String clName, String option) {
        DBCollection cl = null;
        BSONObject options = (BSONObject) JSON.parse(option);
        try {        	
            if (cs.isCollectionExist(clName)) {
                cs.dropCollection(clName);
            }
            
            cl = cs.createCollection(clName, options);
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
        return cl;
    }
    
    public static DBCollection createCL(CollectionSpace cs, String clName) {
        DBCollection cl = null;        
        try {        	
            if (cs.isCollectionExist(clName)) {
                cs.dropCollection(clName);
            }
            
            cl = cs.createCollection(clName);
        } catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
        return cl;
    }
    
    
    public static String getGroupIPByGroupName(Sequoiadb sdb, String groupName) {
        String dataUrl = "";
        try{
            ReplicaGroup replicaGroup = sdb.getReplicaGroup(groupName);
            Node master = replicaGroup.getMaster();
            dataUrl = master.getNodeName();
        }catch (BaseException e) {
            Assert.fail(e.getMessage());
        }
        return dataUrl;
    }
    
    
    public static void insert(DBCollection cl,String []records){
		try{			
			for( int i = 0; i < records.length;i++){								
				cl.insert(records[i]);
			}			
		}catch(BaseException e){
			Assert.fail("insert datas failed, errMsg:" + e.getMessage()+ e.getErrorCode());
		}
	}
    
    /**
     * arithmetic operation as a selector,and check the overflow value type
     * @param: cl:rg:"db.cs.cl"
     *         matcherValue:  used to match a record
     *         sValue:  selector operator,rg:{"$abs":1}
     *         selectorName:    field name to participate in an operation,rg:{a:1},the selectorName is "a"
     *         isVerifyDataType: if true validation data type as required,if false not validation data type 
     */
    public static void selectorOper(DBCollection cl,int matcherValue,BSONObject sValue,String selectorName, String []expRecords){
		try{			
			DBQuery query = new DBQuery();
			BSONObject selector = new BasicBSONObject();			
			BSONObject sValue1 = new BasicBSONObject();
			BSONObject matcher = new BasicBSONObject();				
			matcher.put("test",matcherValue );			
			sValue1.put("$include",0);	
			selector.put(selectorName, sValue);
			selector.put("_id", sValue1);
			System.out.println("selector="+selector.toString());
			query.setSelector(selector);	
			query.setMatcher(matcher);		
			DBCursor cursor = cl.query(query);			
			List<BSONObject> actualList= new ArrayList<BSONObject>(); 			
			
	        while( cursor.hasNext() ) {
	            BSONObject object = cursor.getNext();	            
	            actualList.add(object);
	        }  
	        System.out.println("actualList"+actualList.toString());
	        cursor.close(); 
	        	        
	        List<BSONObject> expectedList= new ArrayList<BSONObject>();
			for (int i = 0; i < expRecords.length; i++) {	
				BSONObject expRecord =(BSONObject) JSON.parse(expRecords[i]);
				expectedList.add(expRecord); 
				System.out.println("exp=="+expectedList.toString());
            }			
			Assert.assertEquals(actualList, expectedList,"the actual query datas is error");  	              		
		}catch(BaseException e){			
			Assert.assertTrue(false,"operator is used as selector oper failed,"+e.getMessage());
		}		
	}
    
    /**
     * Check strict data control mode
     * @param: cl
     *         operSymbol:      inculde $abs/$add/$substract/$multiply/$divide
     *         selectorName:    field name to participate in an operation,rg:{a:1},the selectorName is "a"
     *         arithmeticValue: the value involved in the operation,rg:a+1,and the 1 is arithmeticValue
     */
    public static void isStrictDataTypeOper(DBCollection cl, BSONObject selector ){
		try{
			DBQuery query = new DBQuery();			
			BSONObject matcher = new BasicBSONObject();	
			query.setSelector(selector);
			query.setMatcher(matcher);			
			DBCursor cursor = cl.query(query);
	        while( cursor.hasNext() ) {
	            BSONObject object = cursor.getNext();  
	            System.out.println("actRecs==="+object.toString());	            
	        }	        
	        cursor.close();	        
	        Assert.fail("the operation must be error!" );
		}catch(BaseException e){			
			if( e.getErrorCode() != -318 ){ 
				Assert.assertTrue(false,"oper should be failed,"+e.getErrorCode()+e.getMessage());
            }			
		}		
	}
    
    public static void multipleFieldOper(DBCollection cl,String selector, String[] expRecords){
		try{			
	        long skipRows   = 0;
	        long returnRows = -1;       
            DBCursor cursor =  cl.query(null, selector, null, null, skipRows, returnRows);
            
			List<BSONObject> actualList= new ArrayList<BSONObject>(); 			
	        while( cursor.hasNext() ) {
	            BSONObject object = cursor.getNext();	        
	            actualList.add(object);
	        }  
	        System.out.println("actualList"+actualList.toString());
	        cursor.close(); 	        
	        
	        List<BSONObject> expectedList= new ArrayList<BSONObject>();
			for (int i = 0; i < expRecords.length; i++) {	
				BSONObject expRecord =(BSONObject) JSON.parse(expRecords[i]);
				expectedList.add(expRecord);                
            }
			
			Assert.assertEquals(actualList, expectedList,"the actual query datas is error");   
		}catch(BaseException e){			
			Assert.assertTrue(false,"perform arithmetic error,"+e.getErrorCode()+e.getMessage());
		}
    }
		
        
    public static void checkDataType(DBCollection cl,BSONObject sValue,int matcherValue,String selectorName, 
    		String expTypeToSdb,Boolean isVerifyTypeToJava,String expTypeToJava) throws Exception{
		try{
			if(!selectorName.contains("$")){
				DBQuery query = new DBQuery();
				DBQuery query1 = new DBQuery();
				BSONObject selector = new BasicBSONObject();
				BSONObject selector1 = new BasicBSONObject();
				BSONObject matcher = new BasicBSONObject();			
				matcher.put("test",matcherValue );			
				selector1.put(selectorName, new BasicBSONObject(sValue.toMap()));			
				sValue.put("$type", 2);
				selector.put(selectorName, sValue);			
				query.setSelector(selector);
				query.setMatcher(matcher);		
				DBCursor cursor = cl.query(query);						
			
				String type = "";	
				BSONObject object = null;
		        while( cursor.hasNext() ) {
		            object = cursor.getNext();
		            BasicBSONObject obj = (BasicBSONObject) object;	          
		            type = getType(selectorName,obj);	
		            
		        } 	        
		        cursor.close(); 
		        Assert.assertEquals(type, expTypeToSdb,"the numtype is error");	  
		        
		        //check the data type from java client
		        if(isVerifyTypeToJava){
	            	String typeOfJava = "";
	            	BSONObject object1 = null;
	            	query1.setSelector(selector1);            	
	    			query1.setMatcher(matcher);		
	    			DBCursor cursor1 = cl.query(query1);
	    	        while( cursor1.hasNext() ) {
	    	            object1 = cursor1.getNext();	    	            
	    	            typeOfJava = object1.get(selectorName).getClass().toString(); 	    	            
	    	        } 	        
	    	        cursor1.close();             	
	            	Assert.assertEquals(typeOfJava, expTypeToJava,"the numtype from java is error");
	            }
			}
			
		}catch(BaseException e){			
			Assert.assertTrue(false,"abs is used as selector oper failed,"+e.getMessage());
		}		
	}

	public static String getType(String inField,BasicBSONObject object) throws Exception{
		
		String[] tmps = inField.split("\\.");		
		String type = null;
		if ( tmps.length == 1 ){
			if ( object.containsField( tmps[0])){
				type = ((BasicBSONObject) object).getString( tmps[0]);
			}
		}else {
			BSONObject subobj = object;
			for ( int i = 0; i< tmps.length - 1; ++i){
				if ( subobj.containsField(tmps[i])){
					subobj = (BSONObject)subobj.get( tmps[i]);					
				}else{
					throw new Exception( object.toString() + "not contain:" + tmps[i]);
				}
			}
			if( tmps[tmps.length -1].contains("$[0]")){
				type = (String) ((BasicBSONList)subobj).get(0);
			}else{
				if( subobj.containsField(tmps[tmps.length -1])){
					type = ((BasicBSONObject)subobj).getString(tmps[tmps.length -1]);			
				}else{
					throw new Exception( object.toString() + "not contain:" + tmps[tmps.length -1]);
				}
			}
		}
		
		return type;
		
	}


    
}
