/**
 *      Copyright (C) 2012 SequoiaDB Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
/**
 * @package com.sequoiadb.base;
 * @brief SequoiaDB DataSource
 * @author gaosj
 */
package com.sequoiadb.base;

import java.io.PrintWriter;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;

import javax.sql.DataSource;
import javax.swing.text.html.Option;
//import javax.xml.bind.ValidationEvent;

/**
 * @class SequoiadbDatasource
 * @brief SequoiaDB DataSource
 */
public class SequoiadbDatasource {
	private long lastClearTime = System.currentTimeMillis();// 上一次清理链接的时间
	private volatile LinkedList<Sequoiadb> sequoiadbs;// 可用的连接
	private volatile  HashSet<Sequoiadb> used_sequoiadbs = new HashSet<Sequoiadb>();// 已用的连接
	private SequoiadbOption option;// 连接池配置文件
	private String url = null;
	private String username = null;
	private String password = null;
	
	/**
	 * @fn SequoiadbDatasource()
	 * @brief constructor method
	 * @param url   the url of jdbc
	 * @param username   the username of sequoiadb
	 * @param password   the password of sequoiadb
	 * @param option     the option of datasouce
	 * @throws Exception
	 */
	public SequoiadbDatasource(String url, String username, String password,
			SequoiadbOption option) throws Exception {
		this.url = url;
		this.username = username;
		this.password = password;
		this.option = option;
		init(option);
	}

	/**
	 * 
	 * @fn void init()
	 * @brief init SequoiadbDatabase,after you create a instance of SequoiadbDatabase,you must call this method.
	 * @param option  
	 *               this instance of SequoiadbOption
	 * @throws Exception
	 * @return void
	 * @exception
	 * @since 1.0.0
	 */
	private void init(SequoiadbOption option) throws Exception {
		sequoiadbs = new LinkedList<Sequoiadb>();
		if (option.getMaxConnectionNum() < option.getInitConnectionNum()) {
			throw new Exception(
					"datasource maxconnectionnum is less than initconnectionnum");
		}
		if (option.getInitConnectionNum() < 0)
			throw new Exception("this datasoure connection num is  less than 0");
		for (int i = 0; i < option.getInitConnectionNum(); i++) {
			Sequoiadb sequoiadb = new Sequoiadb(url, username, password);
			sequoiadbs.add(sequoiadb);
		}
	}

	/**
	 * 
	 * @fn Sequoiadb getConnection()
	 * @brief  get the connection from this datasouce
	 * @throws SQLException
	 * @throws InterruptedException
	 * @return Sequoiadb
	 * @exception
	 * @since 1.0.0
	 */
	public synchronized Sequoiadb getConnection() throws SQLException,
			InterruptedException {
		clearClosedConnection();// 关闭多余的连接
		if (sequoiadbs.size() > 0) {
			return getSequoiadb();
		} else {
			increaseConnetions();
			if (sequoiadbs.size() > 0) {
				return getSequoiadb();
			} else {
				wait(option.getTimeout());
				if (sequoiadbs.size() > 0) {
					return getSequoiadb();
				}
				throw new SQLException(
						"can't get database connection exception");
			}
		}

	}

	/**
	 * 
	 * increaseConnetions(添加连接)
	 * 
	 * @return void
	 * @exception
	 * @since 1.0.0
	 */
	private void increaseConnetions() {
		if (used_sequoiadbs.size() < option.getMaxConnectionNum() -
                                   option.getDeltaIncCount()) {
			for (int i = 0; i < option.getDeltaIncCount(); i++) {
				Sequoiadb sequoiadb = new Sequoiadb(url, username, password);
				sequoiadbs.add(sequoiadb);
			}
		} else {
			for (int i = 0; i < option.getMaxConnectionNum()
					- used_sequoiadbs.size(); i++) {
				Sequoiadb sequoiadb = new Sequoiadb(url, username, password);
				sequoiadbs.add(sequoiadb);
			}
		}
	}

	private synchronized Sequoiadb getSequoiadb() {
		Sequoiadb sequoiadb = sequoiadbs.poll();
		used_sequoiadbs.add(sequoiadb);
		return sequoiadb;
	}

	/**
	 * 
	 * @fn void close(Sequoiadb sequoiadb)
	 * @brief  when you accomplish some actions,you should close the conncetion
	 * @param sequoiadb
	 * @return void
	 * @exception
	 * @since 1.0.0
	 */
	public synchronized void close(Sequoiadb sequoiadb) {
		if (used_sequoiadbs.contains(sequoiadb)) {
			used_sequoiadbs.remove(sequoiadb);
			sequoiadbs.add(sequoiadb);
			notify();
		} 
	}

	private void clearClosedConnection() {
		long time = System.currentTimeMillis();
		if (time - lastClearTime >= option.getRecheckCyclePeriod()) {
			if (sequoiadbs.size() > option.getMaxIdeNum()) {
				this.lastClearTime = System.currentTimeMillis();
				for (int i = 0; i < sequoiadbs.size() - option.getMaxIdeNum(); i++) {
               Sequoiadb db = sequoiadbs.get(i) ;
               db.disconnect () ;
               sequoiadbs.remove ( db ) ;
               i-- ;
				}
			}
		}
	}
}
