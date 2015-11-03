package com.sequoiadb.xml2sql;

import java.io.BufferedReader;
import java.io.CharArrayWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.ParseException;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import org.w3c.dom.Node;
import org.xml.sax.InputSource;

public class testFile {
	public static void main(String[] args ) throws ParseException {
		
//		String path = "SdbDoc_En/installation/system_en.html";
//		String dir[] = path.split("/");
//		System.out.println("dir[0] : " + dir[0]);

		
		try {  
			BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(String.valueOf(new File("C:\\Users\\longyang\\Desktop\\network_security.html"))),  "UTF-8"));
			StringBuffer sb = new  StringBuffer();
//			String comm_url = "http://community.sequoiadb.com/";
			String comm_url = "./";
			String str = "cn/index.php?";
//			String image_dir = "./Public/Home/images/docs/";
//			String api_dir = "./Public/Home/document";
			String image_dir = "./index/Public/Home/images/docs/";
			String api_dir = "./index/Public/Home/document";
			String mao_str = "h2 key=\"title\" data-alt=\"alt\"";
			String temp = null;
			
			
			while((temp=br.readLine())!=null){ 
				sb.append(temp);  
			}  
			
			String content = sb.toString();
			Pattern pattern_url = Pattern.compile("href=[\"|\']([^http://|^api|^./].*?)[\"|\']",Pattern.CASE_INSENSITIVE);
			Matcher url = pattern_url.matcher(sb);
			System.out.println("url : " + url);
			while(url.find()){
				String match_url = url.group(1);
				System.out.println("match_url : " + match_url);
				InputSource inputSource = new InputSource(new FileInputStream("C:\\Users\\longyang\\Desktop\\Conf_1.12.xml"));
				XPath xPath = XPathFactory.newInstance().newXPath();
				String expression = "//objlist/obj[cnpath='" + match_url + "' or enpath='" + match_url + "']/id";
				Node node = (Node) xPath.evaluate(expression,inputSource,XPathConstants.NODE);

				String repl = "m=Files&a=index&cat_id=" + node.getTextContent() + "&edition_id=" + 1;
				content = url.replaceFirst("href=\"" + comm_url + str + repl + "\"");
				
				url = pattern_url.matcher(content);
			}
			Pattern pattern_image = Pattern.compile("src=[\"|\']([^\\./].*?)[\"|\']",Pattern.CASE_INSENSITIVE);
//			System.out.println("pattern_image : " + pattern_image);
//			System.out.println("image : " + pattern_image.matcher(content));
			Matcher image = pattern_image.matcher(content);
			while(image.find()){
				String match_image = image.group(1);
				System.out.println("image : " + image);
				System.out.println("match_image : " + match_image);

				content = image.replaceFirst("src=\"" + image_dir + match_image + "\"");
				image = pattern_image.matcher(content);
			}
			Pattern pattern_api = Pattern.compile("href=[\"|\']([api].*?)[\"|\']",Pattern.CASE_INSENSITIVE);
			Matcher api = pattern_api.matcher(content);
			while(api.find()){
				String match_api = api.group(1);

				content = api.replaceFirst("href=\"" + api_dir + "/" + match_api + "\"");
				api = pattern_api.matcher(content);
			}
			Pattern pattern_mao = Pattern.compile("h2.id=[\"|\'](.*?)[\"|\']",Pattern.CASE_INSENSITIVE);
			Matcher mao = pattern_mao.matcher(content);
			while(mao.find()){
				content = mao.replaceFirst(mao_str);
				mao = pattern_mao.matcher(content);
			}
//			Pattern pattern_code = Pattern.compile("lang-javascript",Pattern.CASE_INSENSITIVE);
//			Matcher code = pattern_code.matcher(content);
//			while(code.find()){
//				content = code.replaceFirst("prettyprint lang-javascript");
//				code = pattern_code.matcher(content);
//			}
			br.close();
			System.out.println("content : " + content);
			
		} catch (FileNotFoundException e) {  
		e.printStackTrace();  
		} catch (IOException e) {  
		e.printStackTrace();  
		} 
		catch (XPathExpressionException e) {
			e.printStackTrace();
		}  
	}
}
