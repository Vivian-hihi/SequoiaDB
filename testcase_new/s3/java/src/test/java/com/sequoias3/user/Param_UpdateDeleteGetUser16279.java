package com.sequoias3.user;

import org.json.XML;
import org.springframework.http.HttpStatus;
import org.springframework.web.client.HttpClientErrorException;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoias3.testcommon.S3TestBase;
/**
 * @Description: seqDB-16256 :: 管理员更新自身
 * @author fanyu
 * @Date:2018年10月29日
 * @version:1.0
 */

public class Param_UpdateDeleteGetUser16279 extends S3TestBase {
    private String name = "16279";

    @BeforeClass
    private void setUp() {
	try {
	    UserUtils.deleteUser(name, UserUtils.accessKeyId, true);
	} catch (HttpClientErrorException e) {
	    if (e.getStatusCode() != (HttpStatus.NOT_FOUND)) {
		e.printStackTrace();
		Assert.fail(e.getMessage());
	    }
	}
	UserUtils.createUser(name, UserCommDefind.normal, UserUtils.accessKeyId);
    }

    @Test
    private void testGetUser() {
	try {
	    // create
	    UserUtils.getUser(name, "123456");
	    Assert.fail("exp fail but act success");
	} catch (HttpClientErrorException e) {
	    String errorMsg = e.getResponseBodyAsString();
	    org.json.JSONObject json = XML.toJSONObject(errorMsg);
	    if (!json.getJSONObject(UserCommDefind.error).getString(UserCommDefind.errorCode)
		    .contains("InvalidAccessKeyId")) {
		e.printStackTrace();
		Assert.fail(e.getMessage());
	    }
	}
    }

    @Test
    private void testUpdateUser() {
	try {
	    // create
	    UserUtils.updateUser(name, "1" + UserUtils.accessKeyId);
	    Assert.fail("exp fail but act success");
	} catch (HttpClientErrorException e) {
	    String errorMsg = e.getResponseBodyAsString();
	    org.json.JSONObject json = XML.toJSONObject(errorMsg);
	    if (!json.getJSONObject(UserCommDefind.error).getString(UserCommDefind.errorCode)
		    .contains("InvalidAccessKeyId")) {
		e.printStackTrace();
		Assert.fail(e.getMessage());
	    }
	}
    }

    @Test
    private void testDeleteUser() {
	try {
	    // create
	    UserUtils.deleteUser(name, UserUtils.accessKeyId + "1");
	    Assert.fail("exp fail but act success");
	} catch (HttpClientErrorException e) {
	    String errorMsg = e.getResponseBodyAsString();
	    org.json.JSONObject json = XML.toJSONObject(errorMsg);
	    if (!json.getJSONObject(UserCommDefind.error).getString(UserCommDefind.errorCode)
		    .contains("InvalidAccessKeyId")) {
		e.printStackTrace();
		Assert.fail(e.getMessage());
	    }
	}
    }

    @AfterClass
    private void tearDown() {
	UserUtils.deleteUser(name, UserUtils.accessKeyId, true);
    }
}
