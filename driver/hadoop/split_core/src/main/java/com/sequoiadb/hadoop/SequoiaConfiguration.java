/* Copyright (c) 2012 - 2013 SequoiaDB */

package com.sequoiadb.hadoop;

import java.io.DataInput;
import java.io.IOException;

import org.apache.commons.logging.*;
import org.apache.hadoop.conf.Configuration;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;

/**
 * A container for SequoiaDB configuration
 */

public class SequoiaConfiguration {
	private static final Log _log = LogFactory
			.getLog(SequoiaConfiguration.class);
	private static Configuration _conf;

	public SequoiaConfiguration(Configuration conf) {
		_conf = conf;
		BSONObject dummy = new BasicBSONObject();
		if (getInputFields() == null)
			setInputFields(dummy);
		if (getInputHint() == null)
			setInputHint(dummy);
		if (getInputQuery() == null)
			setInputQuery(dummy);
		if (getInputSort() == null)
			setInputSort(dummy);
	}

	public SequoiaConfiguration(DataInput in) throws IOException {
		_conf = new Configuration();
		_conf.readFields(in);
	}

	public Configuration getConfiguration() {
		return _conf;
	}

	public void setInputConn(String inputConn) {
		SequoiaConfigUtil.setInputConn(_conf, inputConn);
	}

	public String getInputConn() {
		return SequoiaConfigUtil.getInputConn(_conf);
	}

	public void setOutputConn(String outPutConn) {
		SequoiaConfigUtil.setOutputConn(_conf, outPutConn);
	}

	public String getOutputConn() {
		return SequoiaConfigUtil.getOutputConn(_conf);
	}

	public void setInputUsername(String username) {
		SequoiaConfigUtil.setInputUsername(_conf, username);
	}

	public String getInputUsername() {
		return SequoiaConfigUtil.getInputUsername(_conf);
	}

	public void setInputPassword(String password) {
		SequoiaConfigUtil.setInputPassword(_conf, password);
	}

	public String getInputPassword() {
		return SequoiaConfigUtil.getInputPassword(_conf);
	}
	
	
	
	public void setOutputUsername(String username) {
		SequoiaConfigUtil.setOutputUsername(_conf, username);
	}

	public String getOutputUsername() {
		return SequoiaConfigUtil.getOutputUsername(_conf);
	}

	public void setOutputPassword(String password) {
		SequoiaConfigUtil.setOutputPassword(_conf, password);
	}

	public String getOutputPassword() {
		return SequoiaConfigUtil.getOutputPassword(_conf);
	}
	
	
	
	

	public void setInputCollectionspace(String csName) {
		SequoiaConfigUtil.setInputCollectionspace(_conf, csName);
	}

	public String getInputCollectionspace() {
		return SequoiaConfigUtil.getInputCollectionspace(_conf);
	}

	public void setOutputCollectionspace(String csName) {
		SequoiaConfigUtil.setOutputCollectionspace(_conf, csName);
	}

	public String getOutputCollectionspace() {
		return SequoiaConfigUtil.getOutputCollectionspace(_conf);
	}

	public void setInputCollection(String collection) {
		SequoiaConfigUtil.setInputCollection(_conf, collection);
	}

	public String getInputCollection() {
		return SequoiaConfigUtil.getInputCollection(_conf);
	}

	public void setOutputCollection(String collection) {
		SequoiaConfigUtil.setOutputCollection(_conf, collection);
	}

	public String getOutputCollection() {
		return SequoiaConfigUtil.getOutputCollection(_conf);
	}

	public void setInputKey(String key) {
		SequoiaConfigUtil.setInputKey(_conf, key);
	}

	public String getInputKey() {
		return SequoiaConfigUtil.getInputKey(_conf);
	}

	/**
	 * public void setJSON (String key, String value) { try { final BSONObject
	 * obj = SDBMessageHelper.fromJson (value) ; _conf.set (key, obj.toString())
	 * ; } catch ( final Exception e ) { _log.error ( "Failed to parse JSON: ",
	 * e ) ; throw new IllegalArgumentException ( "" ); } }
	 */

	public void setInputQuery(BSONObject query) {
		SequoiaConfigUtil.setInputQuery(_conf, query);
	}

	public BSONObject getInputQuery() {
		return SequoiaConfigUtil.getInputQuery(_conf);
	}

	public void setInputFields(BSONObject fields) {
		SequoiaConfigUtil.setInputFields(_conf, fields);
	}

	public BSONObject getInputFields() {
		return SequoiaConfigUtil.getInputFields(_conf);
	}

	public void setInputSort(BSONObject sort) {
		SequoiaConfigUtil.setInputSort(_conf, sort);
	}

	public BSONObject getInputSort() {
		return SequoiaConfigUtil.getInputSort(_conf);
	}

	public void setInputHint(BSONObject hint) {
		SequoiaConfigUtil.setInputHint(_conf, hint);
	}

	public BSONObject getInputHint() {
		return SequoiaConfigUtil.getInputHint(_conf);
	}

	public void setInputSkip(long skip) {
		SequoiaConfigUtil.setInputSkip(_conf, skip);
	}

	public long getInputSkip() {
		return SequoiaConfigUtil.getInputSkip(_conf);
	}

	public void setInputReturn(long re) {
		SequoiaConfigUtil.setInputReturn(_conf, re);
	}

	public long getInputReturn() {
		return SequoiaConfigUtil.getInputReturn(_conf);
	}

	public void setInputLimit(String limit) {
		SequoiaConfigUtil.setInputLimit(_conf, limit);
	}

	public String getInputLimit() {
		return SequoiaConfigUtil.getInputLimit(_conf);
	}

}