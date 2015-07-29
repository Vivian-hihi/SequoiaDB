package com.sequoiadb.bean;

import java.util.Map;

import org.bson.*;

public class BsonBean {

	protected BSONObject bson;
	
	public BsonBean(){
		
		bson=new BasicBSONObject();
	}
	
	public BsonBean( BSONObject doc ){
    	this();
        setBson(doc);
    }

    public BSONObject getBson() {
		return bson;
	}

	public void setBson(BSONObject bson) {
		this.bson = bson;
	}
	public Map toMap(){
        return bson.toMap();
    }
}
