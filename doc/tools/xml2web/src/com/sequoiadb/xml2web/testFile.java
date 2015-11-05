package com.sequoiadb.xml2web;

import java.text.ParseException;

public class testFile {
	public static void main(String[] args ) throws ParseException {
		String confFile = "../../../doc/toc.xml";
		new XmlConfigReader(confFile,"Cn");
		System.out.println("The process over!");
	}
}
