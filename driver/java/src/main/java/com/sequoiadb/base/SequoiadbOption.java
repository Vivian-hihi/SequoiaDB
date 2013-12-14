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
	public static final  long CLOSE_CONNECT_TIME=4*60*60*1000;
	private int initConnectionNum=10;
	private int maxConnectionNum=500;
	private int maxIdeNum=10;
	private int timeout=5000;
	
	public int getTimeout() {
		return timeout;
	}
	public void setTimeout(int timeout) {
		this.timeout = timeout;
	}
	public int getInitConnectionNum() {
		return initConnectionNum;
	}
	
	/**
	 * @fn void setInitConnectionNum()
	 * @brief set the inittial number of connection
	 * @param  initConnectionNum
	 * 
	 */
	public void setInitConnectionNum(int initConnectionNum) {
		this.initConnectionNum = initConnectionNum;
	}
	public int getMaxConnectionNum() {
		return maxConnectionNum;
	}
	
	/**
	 * @fn void setMaxConnectionNum()
	 * @brief set the max number of connection
	 * @param  maxConnectionNum
	 * 
	 */
	public void setMaxConnectionNum(int maxConnectionNum) {
		this.maxConnectionNum = maxConnectionNum;
	}
	public int getMaxIdeNum() {
		return maxIdeNum;
	}
	
	/**
	 * @fn void setMaxIdeNum()
	 * @brief how much time to close  the free connection
	 * @param maxIdeNum
	 */
	public void setMaxIdeNum(int maxIdeNum) {
		this.maxIdeNum = maxIdeNum;
	}
}
