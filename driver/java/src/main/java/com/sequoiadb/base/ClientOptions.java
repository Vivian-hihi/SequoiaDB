package com.sequoiadb.base;

public class ClientOptions {
	private boolean enableCache;
	// define in milliseconds
	private long cacheInterval;
	
	public ClientOptions() {
		enableCache = true;
		cacheInterval = 300 * 1000;
	}
	
	/**
	 * @fn boolean getEnableCache()
	 * @brief Get the value of "enableCache".
	 * @return The value of "enableCache".
	 */
	public boolean getEnableCache() {
		return enableCache;
	}
	
	/**
	 * @fn void setEnableCache(boolean enable)
	 * @brief Set caching the name of collection space and collection in client or not.
	 * @param enable true or false.
	 * @return void
	 */
	public void setEnableCache(boolean enable) {
		enableCache = enable;
	}
	
	/**
	 * @fn long getCacheInterval()
	 * @brief Get the caching interval. 
	 * @return The value of caching interval.
	 */
	public long getCacheInterval() {
		return cacheInterval;
	}
	
	/**
	 * @fn void setCacheInterval(long interval)
	 * @brief Set the interval for caching the name of collection space 
	 *        and collection in client in milliseconds.
	 *        This value should not be less than 0,  
	 *        or it will be set to the default value, 
	 *        default to be 300*1000ms.
	 * @param interval The interval in milliseconds.
	 * @return void
	 */
	public void setCacheInterval(long interval) {
		cacheInterval = interval;
	}
	
}
