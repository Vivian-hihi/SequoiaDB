package com.sequoiadb.xml2sql;

import java.util.ArrayList;

public class XmlConfig {
    private String editionId = "";
    private ArrayList<String> editionValues = new ArrayList<String>();
    private ArrayList<SDBDocument> docs = new ArrayList<SDBDocument>();

	public void addDocument(SDBDocument doc){
		docs.add(doc);
	}	
	public SDBDocument getDocument(int num){
		SDBDocument doc = docs.get(num);
		return doc;
	} 
	public ArrayList<SDBDocument> getDocuments(){
		return docs;
	}
    public void addEditionID(int editionId){
    	if(this.editionId.compareTo("") == 0){
    		this.editionId = String.valueOf(editionId);
    	}else{
    		this.editionId = editionId + "," + this.editionId;
    	}
    }
    public String getEditionID(){
    	return editionId;
    }
	public String getEditionValue(int num) {
		return editionValues.get(num);
	}
	public void addEditionValue(int editionId,String editionValue) {
		editionValues.add(String.valueOf(editionId));
		editionValues.add(editionValue);
	}
	public boolean idExist(int id){
		int flag = editionId.indexOf(String.valueOf(id));
		if(-1 == flag){
			return false;
		}
		return true;
	}
}
