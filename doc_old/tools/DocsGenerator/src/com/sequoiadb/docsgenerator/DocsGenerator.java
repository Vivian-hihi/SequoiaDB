package com.sequoiadb.docsgenerator;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.Iterator;
import java.util.Properties;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;

/**
 * 
 * 文件生成类
 * 
 * @author : Lichunhao
 * @since: 2016-03-11
 * 
 */

public class DocsGenerator {
	@SuppressWarnings("rawtypes")
	public static void main(String[] args) throws Exception {

		// 读取properties
		Properties properties = new Properties();
		FileInputStream inputStream = new FileInputStream(
				"conf/conf.properties");
		properties.load(inputStream);
		String mdPath = properties.getProperty("mdPath"); // md文件-配置文件路径
		String outputMD = properties.getProperty("outputMD"); // 整合的md文件
		String outputPDF = properties.getProperty("outputPDF");
		String outputDocx = properties.getProperty("outputDocx");
		String doxygenPath = properties.getProperty("doxygenPath");
		inputStream.close();

		File tmpFile = new File(outputMD);
		if (tmpFile.exists()) {
			tmpFile.delete();
			tmpFile.createNewFile();
		} else
			tmpFile.createNewFile();

		// 读取mdPath.xml，合并md文件
		SAXReader reader = new SAXReader();
		Document document = reader.read(new File(mdPath));
		Element root = document.getRootElement();
		Iterator it = root.elementIterator();
		while (it.hasNext()) {
			Element element = (Element) it.next();
			String isFolder = element.elementText("isfolder");
			String path = element.elementText("path");
			if ("".equals(path) || path == null)
				continue;
			if ("true".equals(isFolder))
				FileUtil.mergeFiles(tmpFile, path);
			else
				FileUtil.mergeFile(tmpFile, path);
		}

		// 生成pdf和word docx
		String cmdGenPDF = "pandoc --template=conf/template.tex " + outputMD
				+ " --latex-engine=xelate --toc -o " + outputPDF;
		String cmdGenDoc = "pandoc -s -S " + outputMD + " --toc -o "
				+ outputDocx;
		Runtime rt = Runtime.getRuntime();
		rt.exec(cmdGenPDF);
		rt.exec(cmdGenDoc);

		SAXReader doxReader = new SAXReader();
		Document doxDocument = doxReader.read(new File(doxygenPath));
		Element doxRoot = doxDocument.getRootElement();
		Iterator doxIt = doxRoot.elementIterator();
		try {
			while (doxIt.hasNext()) {
				Element element = (Element) doxIt.next();
				String doxygenConf = element.elementText("conf");
				String latexPath = element.elementText("latex");
				String name = element.elementText("name");
				File shFile = new File("genAPI.sh");
				if (shFile.exists()) {
					shFile.delete();
					shFile.createNewFile();
				} else
					shFile.createNewFile();
				FileUtil.append(shFile, "#!/bin/sh\n" + "doxygen "
						+ doxygenConf + ";\n" + "cd " + latexPath + ";\n"
						+ "make;\n" + "mv refman.pdf ../../" + name + ";");
				Process ps = rt.exec(new String[] { "/bin/sh", "-c",
						"sh genAPI.sh" });
				BufferedReader br = new BufferedReader(new InputStreamReader(
						ps.getInputStream()));
				// StringBuffer sb = new StringBuffer();
				// String line;
				while (br.readLine() != null) {
					// sb.append(line).append("\n");
				}
				// String result = sb.toString();
				// System.out.println(result);
				ps.waitFor();
			}
		} catch (Exception e) {
			e.printStackTrace();
		}

	}

}