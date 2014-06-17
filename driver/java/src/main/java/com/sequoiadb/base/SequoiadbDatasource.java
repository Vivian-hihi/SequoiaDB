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

import java.sql.SQLException;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Timer;
import java.util.TimerTask;

import com.sequoiadb.exception.BaseException;

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
	private final double MULTIPLE = 1.5;
	
	/**
	 * @fn SequoiadbDatasource()
	 * @brief constructor method
	 * @param url   the url of coord address
	 * @param username   the username of sequoiadb
	 * @param password   the password of sequoiadb
	 * @param option     the option of datasource
	 * @exception com.sequoiadb.exception.BaseException
	 */
	public SequoiadbDatasource(String url, String username, String password,
			SequoiadbOption option) throws BaseException
	{
		if (url == null || username == null || password == null || option == null)
			throw new BaseException("SDB_INVALIDARG", url, username, password, option);
		this.url = url;
		this.username = username;
		this.password = password;
		this.option = option;
		try{
		init(option);
		}catch(BaseException e){
			throw e;
		}
		// after the timer start 500ms, it goes to clean periodically 
		timer.schedule(new CleanConnectionTask(this), option.getRecheckCyclePeriod(), option.getRecheckCyclePeriod());
	}

	/**
	 * @fn void init(SequoiadbOption option)
	 * @brief init SequoiadbDatabase,after you create a instance of SequoiadbDatabase,you must call this method.
	 * @param option  
	 *               this instance of SequoiadbOption
	 * @exception com.sequoiadb.Exception.BaseException              
	 */
	private void init(SequoiadbOption option) throws BaseException
	{
		// check option
		if (option.getMaxConnectionNum() < 0)
			throw new BaseException("SDB_INVALIDARG",
					                "maxConnectionNum is negative: " + option.getMaxConnectionNum());
		// if no need to init, return directly
		if (option.getMaxConnectionNum() == 0)
			return ;
		// check option
		if (option.getMaxConnectionNum() < option.getInitConnectionNum())
			throw new BaseException("SDB_INVALIDARG", "maxConnectionNum is less then initConnectionNum, maxConnectionNum is " +
					            option.getMaxConnectionNum() + ", initConnectionNum is " + option.getInitConnectionNum());
		if (option.getInitConnectionNum() < 0)
			throw new BaseException("SDB_INVALIDARG", 
					            "initConnectionNum is negative: " + option.getInitConnectionNum());
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
	 * @exception InterruptedException
	 * 			  BaseException  throws BaseException with the error type SDB_DRIVER_DS_RUNOUT when datasource had run out
	 */
	public synchronized Sequoiadb getConnection() throws BaseException, InterruptedException
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
					throw new BaseException("SDB_DRIVER_DS_RUNOUT");
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
	 * @brief Put the connection back to datasource, can't be null
	 * @param sequoiadb connection get from current datasource
	 * @exception com.sequoiadb.Exception.BaseException
	 *            when param sequoiadb is null, throws BaseException with the type of "SDB_INVALIDARG"
	 */
	public synchronized void close(Sequoiadb sequoiadb) throws BaseException
	{
		if (sequoiadb == null)
			throw new BaseException("SDB_INVALIDARG", sequoiadb);
		// check option
		if (option.getAbandonTime() <= 0)
			throw new BaseException("SDB_INVALIDARG", "abandonTime is negative: " + option.getAbandonTime());
		if (option.getRecheckCyclePeriod() <= 0)
			throw new BaseException("SDB_INVALIDARG", "recheckCyclePeriod is negative: " + option.getRecheckCyclePeriod());
		if (option.getRecheckCyclePeriod() >= option.getAbandonTime())
			throw new BaseException("SDB_INVALIDARG", "recheckCyclePeriod is not less then abandonTime, recheckCyclePeriod is " +
					option.getRecheckCyclePeriod() + ", abandonTime is " + option.getAbandonTime());
		// if the busy queue contain this instance
		if (used_sequoiadbs.contains(sequoiadb)) 
		{
			// remove it from busy queue
			used_sequoiadbs.remove(sequoiadb);
			// when datasource not be used, abandon the connection directly
			if (option.getMaxConnectionNum() == 0)
			{
				sequoiadb.disconnect();
				return;
			}
			// judge put it in idle queue or not
			long lastTime = sequoiadb.getConnection().getLastUseTime();
			long currentTime = System.currentTimeMillis();
			// check the time keep in instance, don't let it go back to pool
			// when it timeout
			if (currentTime - lastTime + MULTIPLE*option.getRecheckCyclePeriod() >= option.getAbandonTime())
			{
				sequoiadb.disconnect();
				return;
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
			return;
		}
	}
	
	/**
	 * @fn void increaseConnetions()
	 * @bref Add another 20 Sequoiadb objects to the datasource. 
	 */
	synchronized void increaseConnetions() throws BaseException
	{
		// when datasource not be used, return directly
		if (option.getMaxConnectionNum() == 0)
			return;
		// if the number of connection in the pool is less than SequoiadbOption::maxIdeNum
		// we are going to increase. we don't want to let every request to increate the count of 
		// connecton, if we don't limit at here, every backgroup thread created in getConnection()
		// will create a lot of connecton. and as a result, the system's socket resource will run out easily
		// the max number of the connection in datasource is maxConnectionNum + (maxIdeNum-1) + deltaIncCount
		if (sequoiadbs.size() >= option.getMaxIdeNum())
			return;
		if (option.getDeltaIncCount() < 0)
				throw new BaseException("SDB_INVALIDARG", 
						                "deltaIncCount is negative: " + option.getDeltaIncCount());
		if (option.getMaxConnectionNum() < option.getDeltaIncCount())
			throw new BaseException("SDB_INVALIDARG",
					                "deltaIncCount is greater then maxConnectionNum, deltaIncCount is " +
					option.getDeltaIncCount() + ", maxConnectionNum is " + option.getMaxConnectionNum());
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
	synchronized void cleanAbandonConnection() throws BaseException
	{
		// when no need to clean
		if (sequoiadbs.size() == 0)
			return ;
		// check option
		if (option.getAbandonTime() <= 0)
			throw new BaseException("SDB_INVALIDARG", "abandonTime is negative: " + option.getAbandonTime());
		if (option.getRecheckCyclePeriod() <= 0)
			throw new BaseException("SDB_INVALIDARG", "recheckCyclePeriod is negative: " + option.getRecheckCyclePeriod());
		if (option.getRecheckCyclePeriod() >= option.getAbandonTime())
			throw new BaseException("SDB_INVALIDARG", "recheckCyclePeriod is not less then abandonTime, recheckCyclePeriod is " +
					option.getRecheckCyclePeriod() + ", abandonTime is " + option.getAbandonTime());
		long lastTime = 0;
		long currentTime = System.currentTimeMillis();
		ListIterator<Sequoiadb> list = sequoiadbs.listIterator(0);
		while(list.hasNext())
		{
			Sequoiadb db = list.next();
			lastTime = db.getConnection().getLastUseTime();
			if ( currentTime - lastTime + MULTIPLE*option.getRecheckCyclePeriod() >= option.getAbandonTime())
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