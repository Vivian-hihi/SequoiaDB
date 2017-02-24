/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:DiskFull.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.fault ;

import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.Test;

import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.CommException;
import com.sequoiadb.exception.ReliabilityException ;

public class DiskFull extends Fault {
	private String hostName;
	private String user;
	private String passwd;
	private String path;
	private int fileID = 0;
	private List<String> padFileList = new ArrayList<String>();
	private int hashCode;

	@Test
	public static void test() {
	    try{
	        DiskFull df = new DiskFull("192.168.31.31", "root", "sequoiadb", "/tmp");
	        df.make();
	        System.out.println("make over");
	        df.restore();
	    }catch(ReliabilityException e){
	        
	    }
	}

	public DiskFull(String hostName, String user, String passwd, String path) {
		super("diskFull");
		this.hostName = hostName;
		this.user = user;
		this.passwd = passwd;
		this.path = path;
		this.hashCode = this.hashCode();
	}

    public void make() throws ReliabilityException{

		fillUpDisk(100);
		/*
		 * Ssh ssh = new Ssh(hostName, user, passwd); ssh.exec(getAvailCMD);
		 * String stdOut = ssh.getStdout(); if (stdOut.length() <= 0) {
		 * ssh.close(); throw new CommException("can not find this path:" +
		 * path); }
		 * 
		 * int padSize; try { padSize = Integer.parseInt(stdOut.substring(0,
		 * stdOut.length() - 1)); } catch (NumberFormatException e) {
		 * ssh.close(); throw e; }
		 * 
		 * if (padSize == 0) { return; } int padCount = padSize / (1024 * 100);
		 * if (padSize % (1024 * 100) > 0) { padCount += 1; } ssh.exec(
		 * "dd if=/dev/zero of=" + path + "/" + padCount * 100 +
		 * "MPad.tmp bs=100M count=" + padCount); padFileName = path + "/" +
		 * padCount * 100 + "MPad.tmp"; ssh.close();
		 */
	}

	public boolean checkMakeResult() throws ReliabilityException{
		Ssh ssh = new Ssh(hostName, user, passwd);
		ssh.exec("df "+path+" | sed '1d' |awk '{print $4}'");
		ssh.close();
		String stdOut = ssh.getStdout();
		if (stdOut.length() <= 0) {
			throw new CommException("can not find this path:" + path);
		}
		int padSize = Integer.parseInt(stdOut.substring(0, stdOut.length() - 1));
		if (padSize > 0) {
			return false;
		} else {
			return true;
		}
	}

	public void restore() throws ReliabilityException{
		Ssh ssh = new Ssh(hostName, user, passwd);
		for (int i = 0; i < padFileList.size(); i++) {
			ssh.exec("rm -rf " + padFileList.get(i));
		}
		ssh.close();
	}

	public boolean checkRestoreResult() {
		// TODO:
		return true;
	}

	public void fillUpDisk(float percent) throws ReliabilityException{
		Ssh ssh = null;
		String getAvailCMD = "df -l " + path + " |sed '1d' | awk '{print $3,$4,$5}'";
		ssh = new Ssh(hostName, user, passwd);
		ssh.exec(getAvailCMD);
		String stdOut = ssh.getStdout();
		if (stdOut.length() <= 0) {
			ssh.close();
			throw new CommException("can not find this filesystem:" + path);
		}
		String[] res = stdOut.substring(0, stdOut.length() - 1).split(" ");
		int used = Integer.parseInt(res[0]);
		int available = Integer.parseInt(res[1]);
		int usedPercent = Integer.parseInt(res[2].substring(0, res[2].length() - 1));

		if (percent <= usedPercent) {
			return;
		}
		if (percent > 100) {
			percent = 100;
		}
		int total = available + used;
		int padSize = (int) (total * percent / 100 - used);
		int padCount = padSize / (1024 * 100);
		if (padSize % (1024 * 100) > 0) {
			padCount += 1;
		}
		ssh.exec("dd if=/dev/zero of=" + path + "/" + padCount * 100 + "MPad_hashCode_" + hashCode + "_id_" + fileID
				+ ".tmp bs=100M count=" + padCount);
		padFileList.add(path + "/" + padCount * 100 + "MPad_hashCode_" + hashCode + "_id_" + fileID + ".tmp");
		fileID++;
		ssh.close();
	}

}
