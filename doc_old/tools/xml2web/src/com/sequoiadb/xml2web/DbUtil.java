package com.sequoiadb.xml2web;

import java.text.ParseException;

public class DbUtil {
	public static String addEditionSQL() throws ParseException {           
        String sql = "INSERT INTO tp_edition (id,cat_id,title,`describe`,content,add_time) "
        			+ "VALUES " 
        			+ "(?,?,?,?,null,?)"
        			+ " ON DUPLICATE KEY UPDATE "
        			+ "id = ?, "
        			+ "cat_id = ?, "
        			+ "title = ?, "
        			+ "`describe` = ?, "
        			+ "add_time = ?;";
        return sql;         
    }
	
	public static String addContentSQL(){
		String sql = "INSERT INTO tp_articlecat (cat_id,"
												+ "cat_img,"
												+ "cat_name,"
												+ "cat_en_name,"
												+ "cat_type,"
												+ "keywords,"
												+ "cat_desc,"
												+ "sort_order,"
												+ "parent_id,"
												+ "content,"
												+ "is_open) "
					+ "VALUES " 
					+ "(?,null,?,null,1,'','',?,?,null,?)"
					+ " ON DUPLICATE KEY UPDATE "
					+ "cat_id = ?, "
					+ "cat_name = ?, "
					+ "sort_order = ?, "
					+ "parent_id = ?, "
					+ "is_open = ?;";
		return sql;
	}
	
	public static String addFileSQL(){
    	String sql = "INSERT INTO tp_article (article_id,"
    										+ "cat_id,"
    										+ "title,"
    										+ "filetitle,"
    										+ "fileshort,"
    										+ "content,"
    										+ "keywords,"
    										+ "is_open,"
    										+ "is_recommend,"
    										+ "add_time,"
    										+ "file_url,"
    										+ "link,"
    										+ "description,"
    										+ "sort_order,"
    										+ "short,"
    										+ "original_img,"
    										+ "thumb_img,"
    										+ "label,"
    										+ "is_link,"
    										+ "downocunt,"
    										+ "article_url,"
    										+ "edition) "
    				+ "VALUES "
					+ "(?,?,?,null,'',?,'',?,0,?,'','',null,?,null,null,null,null,1,0,null,?)"
					+ " ON DUPLICATE KEY UPDATE "
					+ "article_id = ?, "
					+ "cat_id = ?, "
					+ "title = ?, "
					+ "content = ?, "
					+ "is_open = ?, "
					+ "add_time = ?, "
					+ "sort_order = ?, "
					+ "edition = ? "
					+ "WHERE edition;";
		return sql;
	}
}
