package com.sequoias3.user;

import org.json.JSONObject;
import org.json.XML;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.HttpClientErrorException;

import com.sequoias3.testcommon.S3TestBase;

public class UserUtils extends S3TestBase {
    public final static String accessKeyId = "ABCDEFGHIJKLMNOPQRST";

    public static JSONObject createUser(String name, String type, String accessKeyId) throws HttpClientErrorException {
	TestRest rest = new TestRest();
	ResponseEntity<?> response = rest.setApi("/users/" + name + "?role=" + type).setRequestMethod(HttpMethod.POST).setRequestHeaders(UserCommDefind.authorization, accessKeyId).setResponseType(String.class).exec();
	String body = response.getBody().toString();
	org.json.JSONObject json = XML.toJSONObject(body);
	return json;
    }

    public static String deleteUser(String name, String accessKeyId) throws HttpClientErrorException {
	TestRest rest = new TestRest();
	ResponseEntity<?> response = rest.setApi("/users/" + name).setRequestMethod(HttpMethod.DELETE)
		.setRequestHeaders(UserCommDefind.authorization, accessKeyId).setResponseType(String.class).exec();
	String headers = response.getHeaders().toString();
	return headers;
    }

    public static String deleteUser(String name, String accessKeyId, boolean force) throws HttpClientErrorException {
	TestRest rest = new TestRest();
	ResponseEntity<?> response = rest.setApi("/users/" + name + "?force").setRequestMethod(HttpMethod.DELETE)
		.setRequestHeaders(UserCommDefind.authorization, accessKeyId).setResponseType(String.class).exec();
	String headers = response.getHeaders().toString();
	return headers;
    }

    public static JSONObject updateUser(String name, String accessKeyId) throws HttpClientErrorException {
	TestRest rest = new TestRest();
	ResponseEntity<?> response = rest.setApi("/users/" + name).setRequestMethod(HttpMethod.PUT)
		.setRequestHeaders(UserCommDefind.authorization, accessKeyId).setResponseType(String.class).exec();
	String body = response.getBody().toString();
	org.json.JSONObject json = XML.toJSONObject(body);
	return json;
    }

    public static JSONObject getUser(String name, String accessKeyId) throws HttpClientErrorException {
	TestRest rest = new TestRest();
	ResponseEntity<?> response = rest.setApi("/users/" + name).setRequestMethod(HttpMethod.GET)
		.setRequestHeaders(UserCommDefind.authorization, accessKeyId).setResponseType(String.class).exec();
	String body = response.getBody().toString();
	org.json.JSONObject json = XML.toJSONObject(body);
	return json;
    }
}
