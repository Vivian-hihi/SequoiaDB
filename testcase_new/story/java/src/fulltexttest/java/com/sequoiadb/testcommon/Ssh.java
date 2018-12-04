package com.sequoiadb.testcommon;

import com.jcraft.jsch.*;
import com.sequoiadb.exception.FaultException;
import com.sequoiadb.exception.ReliabilityException;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Logger;

public class Ssh {

	private static final int CHANNEL_CONNECT_TIMEOUT = 60 * 1000;
	private String host;
	private String username;
	private String password;
	private int port;
	private Session session = null;
	private int exitStatus;
	private String stdout;
	private String stderr;

	// ssh建立的后台命令集合（key：Channel id ，value：Channel）
	private Map<Integer, Channel> backgroundCMD = new HashMap<Integer, Channel>();

	/**
	 * 使用给定参数及22端口创建ssh对象
	 *
	 * @param host
	 * @param username
	 * @param password
	 * @throws ReliabilityException
	 */
	public Ssh(String host, String username, String password) throws ReliabilityException {
		this(host, username, password, 22);
	}

	/**
	 * 使用给定参数创建ssh对象
	 *
	 * @param host
	 * @param username
	 * @param password
	 * @param port
	 * @throws ReliabilityException
	 */
	public Ssh(String host, String username, String password, int port) throws ReliabilityException {
		super();
		this.host = host;
		this.username = username;
		this.password = password;
		this.port = port;
		System.out
				.println("host : " + host + " username : " + username + " password : " + password + " port : " + port);
		JSch jsch = new JSch();
		try {
			session = jsch.getSession(username, host, port);
			session.setPassword(password);
			session.setConfig("StrictHostKeyChecking", "no");
			session.connect(CHANNEL_CONNECT_TIMEOUT);
		} catch (JSchException e) {
			if (session != null) {
				session.disconnect();
			}
			throw new FaultException(e);
		}
	}

	/**
	 * 在远程主机上执行命令，并等待其执行结果，标准输出存入stdout，标准出错存入stderr,返回值存入exitStatus(注意：
	 * 每一次调用exec都将覆盖上一次的执行结果,返回值不为零将抛出异常)
	 *
	 * @param command
	 * @return
	 * @throws ReliabilityException
	 */
	public void exec(String command) throws ReliabilityException {
		Channel channel = null;
		try {
			channel = session.openChannel("exec");
			((ChannelExec) channel).setCommand(command);
			channel.setInputStream(null);
			getResult(channel, CHANNEL_CONNECT_TIMEOUT);
			if (exitStatus != 0) {
				throw new ReliabilityException("ssh failed to execute commond '" + command + "',stderr:" + stderr
						+ " ,stdout:" + stdout + ",errcode: " + exitStatus);
			}
		} catch (IOException | JSchException e) {
			throw new FaultException(e);
		} finally {
			if (channel != null) {
				channel.disconnect();
			}
		}
	}

	public String getSdbInstallDir() throws ReliabilityException {
		Ssh ssh = new Ssh(host, username, password);
		String dir = null;
		try {
			ssh.exec("cat /etc/default/sequoiadb |grep INSTALL_DIR");
			String str = ssh.getStdout();
			if (str.length() <= 0) {
				throw new ReliabilityException(
						"exec command:cat /etc/default/sequoiadb |grep INSTALL_DIR can not find sequoiadb install dir");
			}
			dir = str.substring(str.indexOf("=") + 1, str.length() - 1);
		} finally {
			ssh.disconnect();
		}
		return dir;

	}

	/**
	 * 关闭Session，关闭backgroundCMD中的Channel（但这些未结束的后台命令可能仍会在远程主机正常执行）
	 */
	public void disconnect() {
		for (Channel channel : backgroundCMD.values()) {
			channel.disconnect();
		}
		if (this.session != null) {
			this.session.disconnect();
		}
	}

	public String getStdout() {
		return stdout;
	}

	private void getResult(Channel channel, long timeOut) throws IOException, JSchException {
		StringBuffer stdoutBf = new StringBuffer();
		StringBuffer stderrBf = new StringBuffer();
		InputStream er = ((ChannelExec) channel).getErrStream();
		InputStream in = channel.getInputStream();
		byte[] tmp = new byte[1024];
		long timer = System.currentTimeMillis();
		channel.connect(CHANNEL_CONNECT_TIMEOUT);
		while (true) {
			while (in.available() > 0) {
				int i = in.read(tmp, 0, 1024);
				if (i < 0) {
					break;
				}
				stdoutBf.append(new String(tmp, 0, i));

				if (System.currentTimeMillis() - timer > timeOut * 1000) {
					break;
				}
			}
			while (er.available() > 0) {
				int i = er.read(tmp, 0, 1024);
				if (i < 0)
					break;
				stderrBf.append(new String(tmp, 0, i));
				if (System.currentTimeMillis() - timer > timeOut * 1000) {
					break;
				}
			}

			if (channel.isClosed()) {
				if (in.available() > 0 || er.available() > 0) {
					continue;
				}
				break;
			}

			try {
				Thread.sleep(200);
			} catch (Exception e) {
				// ignore
			}

			if (System.currentTimeMillis() - timer > timeOut * 1000) {
				break;
			}
		}

		stdout = stdoutBf.toString();
		stderr = stderrBf.toString();
		exitStatus = channel.getExitStatus();
	}
}
