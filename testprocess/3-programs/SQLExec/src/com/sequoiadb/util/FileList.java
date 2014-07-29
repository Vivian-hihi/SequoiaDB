package com.sequoiadb.util;

import java.io.IOException;
import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedList;
import java.util.Random;

/*
 * author: dps
 * testcase dir
 */

public class FileList {
		private ArrayList Filelist = null;
		private ArrayList Dirlist 	= null;
		private String fileArg = null;
		private Random random;
		private boolean caseRandom;
		public boolean SwitchDirFlag = false;
		
	public FileList(String filePath, boolean caseRandom) throws IOException {
		Filelist	=	new ArrayList();
		Dirlist		=	new ArrayList();
		
		fileArg		= filePath;
		File file 	=	new File(filePath);
		random = new Random(System.currentTimeMillis());
		this.caseRandom = caseRandom;
		
		if(file.isDirectory())
		{
			Dirlist = new ArrayList();
			Dirlist.add(file);
			Collections.sort(Dirlist);
		}
		else
		{
			Filelist.add(file);
			Collections.sort(Filelist);
		}
		SwitchDirFlag = false;
	}
	
// 取得一个目录的处理
// 参数
//	initFilelistFlag：boolean  为true表示将现有filelist清空
	public boolean getNextDir(boolean initFilelistFlag){
		File tmp	=	null;
		File file[];
		int i;
		
		// 如果需要初始化Filelist，那么将Filelist清空
		if(initFilelistFlag)
			Filelist.clear();
		
		do{
		// 如果Dirlist为空，那么直接返回
			if(Dirlist.isEmpty())
				return false;
			tmp = (File) Dirlist.remove(caseRandom ? random.nextInt(Dirlist.size()) : 0);
//			System.out.println(tmp.getName());
		}while(tmp.getAbsolutePath().toLowerCase().endsWith(".svn"));	//ignore the .svn dir
			
		
		file = tmp.listFiles();
		for(i = 0; i < file.length; i++)
		{
			if(!file[i].isDirectory())
			{
				Filelist.add(file[i]);
				file[i] = null;
			}
		}
		for(i = file.length - 1; i >= 0; i--)
		{
			if(file[i] != null)
				Dirlist.add(file[i]);
		}
		
		SwitchDirFlag = true;
		
		Collections.sort(Filelist);
		
		return true;
	}
	
//	 取得一个文件。若没有文件，则返回null
	public File getNextFile(){
		File tmp	=	null;
		
		while(Filelist.isEmpty()&&!DirlistIsEmp())
			getNextDir(false);
		
		if(!Filelist.isEmpty())
			tmp = (File) Filelist.remove(caseRandom ? random.nextInt(Filelist.size()) : 0);
		
		return tmp;
	}
	
//	 打印所有文件列表
	public void showfiles(){
		File tmp	= null;
		
		do
		{
			tmp	= getNextFile();
			if(tmp == null)
				break;
			System.out.println(tmp.getAbsolutePath());
		}while(true);
	}
	
//	返回文件列表是否为空
	public boolean FilelistIsEmp() {
		return Filelist.isEmpty();
	}

//  返回目录列表是否为空
	public boolean DirlistIsEmp() {
		return Dirlist.isEmpty();
	}
}