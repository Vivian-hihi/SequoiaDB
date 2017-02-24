/**
 * Copyright (c) 2017, SequoiaDB Ltd.
 * File Name:BrokenNetwork.java
 * 
 *
 *  @author wenjingwang
 * Date:2017-2-21下午4:54:48
 *  @version 1.00
 */
package com.sequoiadb.fault;
import org.testng.annotations.Test;

import com.sequoiadb.commlib.Ssh;
import com.sequoiadb.exception.CommException;

public class BrokenNetwork extends Fault {

    private String hostName ;
    private String user ;
    private String passwd ;
    private int duration ;

	@Test
	public static void test() {
		BrokenNetwork bn = new BrokenNetwork("192.168.31.31", "root", "sequoiadb", 3);
		for (int i = 0; i < 10; i++)
			bn.ping();
	}

	public BrokenNetwork(String hostName, String user, String passwd, int duration) {
		super("brokenNetwork");
		this.hostName = hostName;
		this.user = user;
		this.passwd = passwd;
		this.duration = duration;
	}

	public void make() {
		// TODO: 这个脚本好像有问题
		/**
		 * #!/bin/bash ifconfig eth0 down sleep $1 ifconfig eth0 up
		 * /etc/init.d/networking restart
		 */
		String shell = "#!/bin/bash\nifconfig eth0 down\nsleep \\$1\nifconfig eth0 up\n/etc/init.d/networking restart";
		Ssh ssh = new Ssh(hostName, user, passwd);
		ssh.exec("echo -e \"" + shell + "\" > /tmp/cutnet.sh");
		ssh.exec("chmod 777 /tmp/cutnet.sh");
		ssh.execBackground("/tmp/cutnet.sh " + duration);
		ssh.close();
	}

	public boolean checkMakeResult() {
		// TODO:checkTime
		int checkTime = 3;
		for (int i = 0; i < checkTime; i++) {
			if (!ping()) {
				return true;
			}
		}
		return false;
	}

	public void restore() {
		// denpend on cunet.sh,sleep???
		try {
			Thread.sleep(duration * 1000);
		} catch (InterruptedException e) {
			// TODO ignore??
		}
	}

	public boolean checkRestoreResult() {
		// TODO:checkTime
		int checkTime = 3;
		for (int i = 0; i < checkTime; i++) {
			if (ping()) {
				return true;
			}
		}
		return false;
	}

	public boolean ping() {
		String os = System.getProperties().getProperty("os.name");
		String cmd;
		if (os.startsWith("win") || os.startsWith("Win")) {
			cmd = "ping " + hostName + " -n 1 -w 1";
		} else {
			cmd = "ping " + hostName + " -c 1 -w 1";
		}
		Runtime rt = Runtime.getRuntime();
		try {
			Process pr = rt.exec(cmd);
			pr.waitFor();
			int exitcode = pr.exitValue();
			pr.destroy();
			if (exitcode == 0) {
				return true;
			} else {
				return false;
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			throw new CommException(e);
		}
	}

}
