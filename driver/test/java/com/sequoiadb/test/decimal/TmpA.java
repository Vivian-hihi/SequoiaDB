package com.sequoiadb.test.decimal;

import java.util.ArrayList;

import org.bson.BSONObject;
import org.bson.types.BSONDecimal;
import org.bson.types.BSONTimestamp;
import org.bson.types.BasicBSONList;
import org.bson.types.ObjectId;


public class TmpA {

	private int fieldA;
	private String fieldB;
	private ObjectId fieldC;
	private BSONTimestamp fieldD;
	private BSONDecimal fieldE;
	private ArrayList fieldF = new ArrayList();
	
	public ArrayList getFieldF() {
		return fieldF;
	}

	public void setFieldF(ArrayList fieldF) {
		this.fieldF = fieldF;
	}

	public TmpA() {}
	
	public BSONTimestamp getFieldD() {
		return fieldD;
	}

	public void setFieldD(BSONTimestamp fieldD) {
		this.fieldD = fieldD;
	}

	public BSONDecimal getFieldE() {
		return fieldE;
	}

	public void setFieldE(BSONDecimal fieldE) {
		this.fieldE = fieldE;
	}

	public int getFieldA() {
		return fieldA;
	}
	public void setFieldA(int fieldA) {
		this.fieldA = fieldA;
	}
	public String getFieldB() {
		return fieldB;
	}
	public void setFieldB(String fieldB) {
		this.fieldB = fieldB;
	}
	
	public String toString() {
		return "fieldA: " + fieldA + ", fieldB: " + fieldB +
				", fieldC: " + fieldC.toString() + ", fieldD: " + 
				fieldD.toString() + ", fieldE: " + fieldE.toString() +
				", fieldF: " + fieldF.toString();
	}
	
	public ObjectId getFieldC() {
		return fieldC;
	}

	public void setFieldC(ObjectId fieldC) {
		this.fieldC = fieldC;
	}
}