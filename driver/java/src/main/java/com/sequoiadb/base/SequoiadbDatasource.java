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
import java.util.ListIterator;
import java.util.Timer;
import java.util.TimerTask;

import javax.sql.DataSource;
import javax.swing.text.html.Option;
//import javax.xml.bind.ValidationEvent;

/**
 * @class SequoiadbDatasource
 * @brief SequoiaDB DataSource
 */
public class SequoiadbDatasource {
	// the idle queue
	private volatile LinkedList<Sequoiadb> sequoiadbs = new LinkedList<Sequoiadb>();
	// the busy queue
	private volatile  HashSet<Sequoiadb> used_sequoiadbs = new HashSet<Sequoiadb>();
	// the configuration for datasource
	private SequoiadbOption option;
	private String url = null;
	private String username = null;
	private String password = null;
	private Timer timer = new Timer(true);
	
	/**
	 * @fn SequoiadbDatasource()
	 * @brief constructor method
	 * @param url   the url of jdbc
	 * @param username   the username of sequoiadb
	 * @param password   the password of sequoiadb
	 * @param option     the option of datasource
	 * @throws Exception
	 */
	public SequoiadbDatasource(String url, String username, String password,
			SequoiadbOption option) throws Exception
	{
		this.url = url;
		this.username = username;
		this.password = password;
		this.option = option;
		init(option);
		// after the timer start 500ms, it goes to clean periodically 
		timer.schedule(new CleanConnectionTask(this), option.getRecheckCyclePeriod(), option.getRecheckCyclePeriod());
	}

	/**
	 * @fn void init(SequoiadbOption option)
	 * @brief init SequoiadbDatabase,after you create a instance of SequoiadbDatabase,you must call this method.
	 * @param option  
	 *               this instance of SequoiadbOption
	 * @return void
	 * @exception Exception
	 */
	private void init(SequoiadbOption option) throws Exception
	{
		if (option.getMaxConnectionNum() < 0)
			throw new Exception(
					"datasource maxconnectionnum is less than 0");
		else if (option.getMaxConnectionNum() == 0)
			return ; 
		if (option.getMaxConnectionNum() < option.getInitConnectionNum())
		{
			throw new Exception(
					"datasource maxconnectionnum is less than initconnectionnum");
		}
		if (option.getInitConnectionNum() < 0)
			throw new Exception("this datasoure connection num is  less than 0");
		for (int i = 0; i < option.getInitConnectionNum(); i++)
		{
			Sequoiadb sequoiadb = new Sequoiadb(url, username, password);
			sequoiadbs.add(sequoiadb);
		}
	}

	/**
	 * @fn Sequoiadb getConnection()
	 * @brief  get the connection from this datasource
	 * @throws SQLException
	 * @throws InterruptedException
	 * @return Sequoiadb
	 * @exception SQLException, InterruptedException
	 */
	public synchronized Sequoiadb getConnection() throws SQLException, InterruptedException
	{
		// when we don't want to use datasource, return sequiadb instance directly
		if (option.getMaxConnectionNum() == 0)
		{
			Sequoiadb temp = new Sequoiadb(url, username, password);
			return temp;
		}
		// otherwise
		if ((sequoiadbs.size() > 0) && (used_sequoiadbs.size() < option.getMaxConnectionNum())) 
		{
			Sequoiadb sequoiadb = null;
			// get connection from idle queue
			sequoiadb = sequoiadbs.poll();
			// get a valid instance
			while((sequoiadb != null) && (!sequoiadb.isValid()))
			{
				sequoiadb = sequoiadbs.poll();
			}
			// if no valid instance in idle queue, let't create one return
			if (sequoiadb == null)
				sequoiadb = new Sequoiadb(url, username, password);
			// add to busy queue
			used_sequoiadbs.add(sequoiadb);
			return sequoiadb;
		}
		else
		{
			if (used_sequoiadbs.size() >= option.getMaxConnectionNum())
			{
				wait(option.getTimeout());
				// when wake up, check again
				if (used_sequoiadbs.size() >= option.getMaxConnectionNum())
					throw new SQLException("datasource is full," +
							" can't get database connection exception");
			}
			Sequoiadb temp = new Sequoiadb(url, username, password);
			used_sequoiadbs.add(temp);
			// create a thread to increase connection
			Thread t = new Thread(new CreateConnectionTask(this));
			t.start();
			return temp;
		}
	}
	
	/**
	 * @fn void close(Sequoiadb sequoiadb)
	 * @brief  when you accomplish some actions,you should close the conncetion
	 * @param sequoiadb
	 * @return void
	 * @exception NullPointerException if sequoiadb is null
	 */
	public synchronized void close(Sequoiadb sequoiadb)
	{
		if (sequoiadb == null)
			throw new NullPointerException();
		// when datasource not be used, abandon the connection directly
		if (option.getMaxConnectionNum() == 0)
		{
			sequoiadb.disconnect();
			return;
		}
		// if the busy queue contain this instance
		if (used_sequoiadbs.contains(sequoiadb)) 
		{
			// remove it from busy queue
			used_sequoiadbs.remove(sequoiadb);
			// judge put it in idle queue or not
			long lastTime = sequoiadb.getConnection().getLastUseTime();
			long currentTime = System.currentTimeMillis();
			// check the time keep in instance, don't let it go back to pool
			// when it timeout
			if (currentTime - lastTime >= option.getAbandonTime())
			{
				sequoiadb.disconnect();
			}
			else
			{
				// close all cursor
				sequoiadb.closeAllCursors();
				// release the heap memory or other resource holds in the instance
				sequoiadb.releaseResource();
				// put it back to idle queue
				sequoiadbs.add(sequoiadb);
				notify();	
			}
		}
		// else, abandon it directly
		else
		{
			sequoiadb.disconnect();
		}
	}
	
	/**
	 * @fn void increaseConnetions()
	 * @bref Add another 20 Sequoiadb objects to the datasource. 
	 */
	synchronized void increaseConnetions()
	{
		// when datasource not be used, return directly
		if (option.getMaxConnectionNum() == 0)
			return;
		// otherwise
		if (used_sequoiadbs.size() < option.getMaxConnectionNum() -
                                   option.getDeltaIncCount()) 
		{
			for (int i = 0; i < option.getDeltaIncCount(); i++)
			{
				Sequoiadb sequoiadb = new Sequoiadb(url, username, password);
				sequoiadbs.add(sequoiadb);
			}
		} 
		else 
		{
			for (int i = 0; i < option.getMaxConnectionNum()
					- used_sequoiadbs.size(); i++)
			{
				Sequoiadb sequoiadb = new Sequoiadb(url, username, password);
				sequoiadbs.add(sequoiadb);
			}
		}
	}
	
	/**
	 * @fn void cleanAbandonConnection()
	 * @brief  clean the tiemout connection periodically
	 */
	synchronized void cleanAbandonConnection()
	{
		if (sequoiadbs.size() == 0)
			return ;
		long lastTime = 0;
		long currentTime = System.currentTimeMillis();
		ListIterator<Sequoiadb> list = sequoiadbs.listIterator(0);
		while(list.hasNext())
		{
			Sequoiadb db = list.next();
			lastTime = db.getConnection().getLastUseTime();
			if ( currentTime - lastTime >= option.getAbandonTime())
			{
				db.disconnect () ;
				list.remove();
			}
		}
		for (int i = 0; i < sequoiadbs.size() - option.getMaxIdeNum(); i++)
		{
			Sequoiadb db = sequoiadbs.poll();
			db.disconnect();
			i--;
		}
	}
	
}

class CleanConnectionTask extends TimerTask {
	private SequoiadbDatasource datasource;
	
	public CleanConnectionTask(SequoiadbDatasource ds){
		datasource = ds;
	}
	@Override
	public void run() {
		datasource.cleanAbandonConnection();
	}
	
}

class CreateConnectionTask implements Runnable {
	private SequoiadbDatasource datasource;
	
	public CreateConnectionTask(SequoiadbDatasource ds){
		datasource = ds;
	}
	
	public void run(){
		datasource.increaseConnetions();
	}
	
}