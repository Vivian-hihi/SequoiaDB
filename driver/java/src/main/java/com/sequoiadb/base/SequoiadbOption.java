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
 * @brief  this opton of SequoiadbDatasource 
 * @author gaosj
 */
package com.sequoiadb.base;

/**
 * @class SequoiadbOption
 * @brief the option of SequoiadbDatasource
 */
public class SequoiadbOption {
	// max connction count, if maxConnectionNum == 0,
	// it means the datasource doesn't work
	private int maxConnectionNum = 500;
	// while maxConnectionNum != 0, increase 20 connection to pool
    private int deltaIncCount = 10 ;
    // initialize 10 connection in pool
	private int initConnectionNum = 10;
	// no use
	private int maxIdeNum = 10;
	// when busy queue is full, request can't get connection,
	// they will wait 5s and wake up
	private int timeout = 5 * 1000;
	// clean the abandon connection period
    private int recheckCyclePeriod = 1 * 60 * 1000;
    // when the last service time of a connection longer then this,
    // we will abandon it.
    private int abandonTime = 10 * 60 * 1000;

    
	/**
	 * @fn void setMaxConnectionNum(int maxConnectionNum)
	 * @brief Set the max number of connection. when maxConnectionNum is 0,
	 *        the datasource doesn't work. when get connecton, it build a connection and return it
	 *        directly. when a connection go back to pool, it disconnect the connection directly and
	 *        would not put it in pool. 
	 * @param  maxConnectionNum default to be 500 
	 * @exception Exception 1.when maxConnectionNum != 0 and maxConnectionNum is less then deltaIncCount.
	 * 2.maxConnectionNum is negative.
	 * 
	 */
	public void setMaxConnectionNum(int maxConnectionNum) throws Exception {
		if (maxConnectionNum < 0)
			throw new Exception("maxConnectionNum is less then 0");
		if ((maxConnectionNum) != 0 && (maxConnectionNum < deltaIncCount))
			throw new Exception("maxConnectionNum is less then deltaIncCount");
		this.maxConnectionNum = maxConnectionNum;
	}
	/**
	 * @fn void getMaxConnectionNum()
	 * @brief Get the max number of connection.
	 */
	public int getMaxConnectionNum() {
		return maxConnectionNum;
	}
	   
	/**
	 * @fn void setDeltaIncCount(int deltaIncCount)
	 * @brief Set the number of new connections to open once running out the
	 *        connection pool.
	 * @param deltaIncCount default to be 10
	 * @exception Exception 1.when maxConnectionNum != 0 and maxConnectionNum is less then deltaIncCount 
	 * 2. when deltaIncCount is negative.
	 */
	public void setDeltaIncCount ( int deltaIncCount ) throws Exception {
		if (deltaIncCount < 0)
			throw new Exception("deltaIncCount is less then 0");
		if ((maxConnectionNum) != 0 && (maxConnectionNum < deltaIncCount))
			throw new Exception("maxConnectionNum is less then deltaIncCount");
		this.deltaIncCount = deltaIncCount ;
	}
	/**
	 * @fn void getDeltaIncCount()
	 * @brief Get the number of new connections to open once running out the
	 *        connection pool.
	 * @setDeltaIncCount
	 */
	public int getDeltaIncCount () {
		return deltaIncCount ;
	}
	
	/**
	 * @fn void setInitConnectionNum(int initConnectionNum)
	 * @brief Set the initial number of connection.
	 * @param  initConnectionNum default to be 10
	 * @throws Exception 1.when maxConnectionNum != 0 and maxConnectionNum is less then initConnectionNum 
	 * 2.when initConnectionNum is negative.
	 */
	public void setInitConnectionNum(int initConnectionNum) throws Exception {
		if (initConnectionNum < 0)
			throw new Exception("initConnectionNum is less then 0");
		if ((maxConnectionNum) != 0 && (maxConnectionNum < initConnectionNum))
			throw new Exception("maxConnectionNum is less then initConnectionNum");
		this.initConnectionNum = initConnectionNum;
	}
	/**
	 * @fn void getInitConnectionNum()
	 * @brief Get the initial number of connection.
	 */
	public int getInitConnectionNum() {
		return initConnectionNum;
	}
	
	/**
	 * @fn void setMaxIdeNum(int maxIdeNum)
	 * @brief Set the max number of the free connection left in datasource
	 *        after periodically cleaning.
	 * @param maxIdeNum default to be 10
	 * @throws Exception 1.when maxConnectionNum != 0 and maxConnectionNum is less then maxIdeNum
	 * 2.when maxIdeNum is less then 0
	 */
	public void setMaxIdeNum(int maxIdeNum) throws Exception {
		if (maxIdeNum < 0)
			throw new Exception("maxIdeNum is less then 0");
		if ((maxConnectionNum) != 0 && (maxConnectionNum < maxIdeNum))
			throw new Exception("maxConnectionNum is less then maxIdeNum");
		this.maxIdeNum = maxIdeNum;
	}
	/**
	 * @fn void getMaxIdeNum()
	 * @brief Get the max number of the free connection.
	 * @see setMaxIdeNum
	 */
	public int getMaxIdeNum() {
		return maxIdeNum;
	}
	
   /**
    * @fn void setRecheckCyclePeriod(int recheckCyclePeriod)
    * @brief Set the recheck cycle in milliseconds. In each cycle
    *        datasource cleans all the discardable connection,
    *        and keep the number of valid connection not more then maxIdeNum. 
    * @param recheckCyclePeriod default to be 1 * 60 * 1000ms
    * @throws Exception when setRecheckCyclePeriod <= 0
    */
	public void setRecheckCyclePeriod ( int recheckCyclePeriod ) throws Exception {
		if (recheckCyclePeriod <= 0)
			throw new Exception("recheckCyclePeriod is invalid");
		this.recheckCyclePeriod = recheckCyclePeriod ;
	}
   /**
    * @fn void getRecheckCyclePeriod(int recheckCyclePeriod)
    * @brief get the recheck cycle
    * @see setRecheckCyclePeriod
    */
	public int getRecheckCyclePeriod () {
	   return recheckCyclePeriod ;
	}

   /**
    * @fn void setTimeout(int timeout)
    * @brief Set the wait time in milliseconds. If the number of connection reaches
    *        maxConnectionNum, datasource can't offer connection immediately, the
    *        requests will be blocked to wait for a moment. When timeout, and there is
    *        still no available connection, datasource throws exception  
    * @param timeout defalt to be 5 * 1000ms
    * @throws Exception when timeout < 0
    */
	public void setTimeout(int timeout) throws Exception {
		if (timeout < 0)
			throw new Exception("timeout is less then 0");
		if (timeout == 0)
			timeout = 1;
		this.timeout = timeout;
	}
	   /**
	    * @fn void getTimeout()
	    * @brief get the wait time. 
	    * @see setTimeout
	    */
	public int getTimeout() {
		return timeout;
	}

   /**
    * @fn void setAbandonTime(int abandonTime)
    * @brief Set the time in milliseconds for abandoning discardable connection.
    *        If a connection has not be used for a long time(longer then abandonTime),
    *        datasource would not let it come back to pool. And it will clean this kind of
    *        connections in the pool periodically. 
    * @param abandonTime defalt to be 10 * 60 * 1000ms
    * @throws Exception when abandonTime <= 0
    */
	public void setAbandonTime(int abandonTime) throws Exception {
		if (abandonTime <= 0)
			throw new Exception("abandonTime is less then 0");
		this.abandonTime = abandonTime;
	}
   /**
    * @fn void getAbandonTime()
    * @brief Get the setup time for abandoning discardable connection.
    * @setAbandonTime
    */
	public int getAbandonTime() {
		return abandonTime;
	}

}
