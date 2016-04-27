package com.sequoiadb.datasource;

import java.net.InterfaceAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.Random;

import com.sequoiadb.exception.BaseException;

public class ConcreteLocalStrategy extends AbstractStrategy{
	private Random _rand = new Random(47);
	private List<String> _localAddrs = new ArrayList<String>();
	private List<String> _localIPs = new ArrayList<String>();
	
	@Override
	public void init(List<String> addresses, List<Pair> _idleConnPairs, List<Pair> _usedConnPairs) {
		super.init(addresses, _idleConnPairs, _usedConnPairs);
		_localIPs = getNetCardIPs();
		_localAddrs = getLocalCoordIPs(_addrs, _localIPs);
	}

	@Override
	public String getAddress() {
		String addr = null;
		_lockForAddr.lock();
		try {
			if (1 <= _localAddrs.size()) {
				addr = _localAddrs.get(_rand.nextInt(_localAddrs.size()));
			} else {
				if (1 <= _addrs.size()) {
					addr = _addrs.get(_rand.nextInt(_addrs.size()));
				}
			}
		} finally {
			_lockForAddr.unlock();
		}
		return addr;
	}
	
	public void addAddress(String addr) {
		super.addAddress(addr);
		_lockForAddr.lock();
		try {
			if (isLocalAddress(addr, _localIPs)) {
				_localAddrs.add(addr);
			}
		} finally {
			_lockForAddr.unlock();
		}
	}
	
	public List<ConnItem> removeAddress(String addr) {
		List<ConnItem> list = super.removeAddress(addr);
		_lockForAddr.lock();
		try {
			if (isLocalAddress(addr, _localIPs)) {
				_localAddrs.remove(addr);
			}
		} finally {
			_lockForAddr.unlock();
		}
		return list;
	}
	
	static List<String> getNetCardIPs() {
		List<String> localIPs = new ArrayList<String>();
		localIPs.add("127.0.0.1");
		try {
			Enumeration<NetworkInterface> netcards = NetworkInterface.getNetworkInterfaces();
			if (null == netcards) {
				return localIPs;
			}
	        for (NetworkInterface netcard : Collections.list(netcards)) {
	            if (null != netcard.getHardwareAddress()) {
	                List<InterfaceAddress> list = netcard.getInterfaceAddresses();
	                for (InterfaceAddress interfaceAddress : list) {
	                    String addr=interfaceAddress.getAddress().toString();
	                    if (addr.indexOf("/") >= 0) {// TODO: check in linux
	                    	localIPs.add(addr.split("/")[1]);
	                    }
	                }
	            }
	        }
		} catch (SocketException e) {
			throw new BaseException("SDB_SYS", "failed to get local ip address");
		}
		return localIPs;
	}
	
	static List<String> getLocalCoordIPs(List<String> urls, List<String> localIPs) {
		List<String> localAddrs = new ArrayList<String>();
		if (localIPs.size() > 0) {
			for(String url : urls) {
				String ip = url.split(":")[0].trim();
				if (localIPs.contains(ip))
					localAddrs.add(url);
			}
		}
		return localAddrs;
	}
	
	/**
	 * @fn boolean isLocalAddress(String url)
	 * @bref Judge a coord address is in local or not 
	 * @return true or false
	 */
	static boolean isLocalAddress(String url, List<String> localIPs) {
	    return localIPs.contains(url.split(":")[0].trim());
	}
	
//	private void pickLocalAddresses2(List<String> urls, 
//			List<String> localIPs, 
//			List<String> localAddrs) throws BaseException {
//		try {
//			Enumeration<NetworkInterface> netcards = NetworkInterface.getNetworkInterfaces();
//			if (null == netcards) {
//				return ;
//			}
//	        for (NetworkInterface netcard : Collections.list(netcards)) {
//	            if (null != netcard.getHardwareAddress()) {
//	                List<InterfaceAddress> list = netcard.getInterfaceAddresses();
//	                for (InterfaceAddress interfaceAddress : list) {
//	                    String addr=interfaceAddress.getAddress().toString();
//	                    if (addr.indexOf("/") >= 0) {
//	                    	localIPs.add(addr.split("/")[1]);
//	                    }
//	                }
//	            }
//	        }
//		} catch (SocketException e) {
//			throw new BaseException("SDB_SYS", "failed to get local coord addresses");
//		}
//		if (localIPs.size() > 0) {
//			for(String ip : urls) {
//				for(String localIP : localIPs) {
//					if (ip == localIP) {
//						localAddrs.add(ip);
//					}
//				}
//			}
//		}
//	}

}
