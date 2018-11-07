package com.sequoias3.testcommon;

import java.net.URLDecoder;
import java.util.List;



import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.apache.http.NameValuePair;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpDelete;
import org.apache.http.client.methods.HttpEntityEnclosingRequestBase;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpPut;
import org.apache.http.client.methods.HttpRequestBase;
import org.apache.http.conn.ConnectionPoolTimeoutException;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.impl.conn.PoolingHttpClientConnectionManager;
import org.apache.http.util.EntityUtils;
import org.bson.BSONObject;
import org.bson.util.JSON;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public final class RestClient {   

    private static final Logger logger = LoggerFactory.getLogger(RestClient.class);
    private static final String AUTHORIZATION = "x-auth-token";
    private static final String ERROR_ATTRIBUTE = "X-SCM-ERROR";
    private static final String NO_AUTH_SESSION_ID = "-1";
    private static final String CHARSET_UTF8 = "utf-8";
    private static CloseableHttpClient client;

    private RestClient() {
    	
    }
    
    public static void sendRequest(CloseableHttpClient client, String sessionId,
            HttpRequestBase request) throws Exception {
        CloseableHttpResponse response = sendRequestWithHttpResponse(client, sessionId, request);
        closeResp(response);
    }

    public static void sendRequest(CloseableHttpClient client, String sessionId,
            HttpEntityEnclosingRequestBase request, List<NameValuePair> params) throws Exception {
        CloseableHttpResponse response = sendRequestWithHttpResponse(client, sessionId, request,
                params);
        closeResp(response);
    }

    public static String sendRequestWithHeaderResponse(CloseableHttpClient client,
            String sessionId, HttpRequestBase request, String keyName) throws Exception {
        CloseableHttpResponse response = sendRequestWithHttpResponse(client, sessionId, request);
        try {
            String value = response.getFirstHeader(keyName).getValue();
            return URLDecoder.decode(value, CHARSET_UTF8);
        }
        finally {
            closeResp(response);
        }
    }

    public static String sendRequestWithHeaderResponse(CloseableHttpClient client,
            String sessionId, HttpEntityEnclosingRequestBase request, List<NameValuePair> params,
            String keyName) throws Exception {
        CloseableHttpResponse response = sendRequestWithHttpResponse(client, sessionId, request,
                params);
        try {
            String value = response.getFirstHeader(keyName).getValue();
            return URLDecoder.decode(value, CHARSET_UTF8);
        }
        finally {
            closeResp(response);
        }
    }

    public static BSONObject sendRequestWithJsonResponse(CloseableHttpClient client,
            String sessionId, HttpRequestBase request) throws Exception {
        CloseableHttpResponse response = sendRequestWithHttpResponse(client, sessionId, request);
        try {
            String resp = EntityUtils.toString(response.getEntity(), CHARSET_UTF8);
            return (BSONObject) JSON.parse(resp);
        }
        finally {
            closeResp(response);
        }
    }

    public static BSONObject sendRequestWithJsonResponse(CloseableHttpClient client,
            String sessionId, HttpEntityEnclosingRequestBase request, List<NameValuePair> params)
                    throws Exception {
        CloseableHttpResponse response = sendRequestWithHttpResponse(client, sessionId, request,
                params);
        try {
            String resp = EntityUtils.toString(response.getEntity(), CHARSET_UTF8);
            return (BSONObject) JSON.parse(resp);
        }
        finally {
            closeResp(response);
        }
    }

    public static CloseableHttpResponse sendRequestWithHttpResponse(CloseableHttpClient client,
            String sessionId, HttpRequestBase request) throws Exception {
        if (!NO_AUTH_SESSION_ID.equals(sessionId)) {
            request.setHeader(AUTHORIZATION, sessionId);
        }

        return sendRequest(client, request);
    }

    public static CloseableHttpResponse sendRequestWithHttpResponse(CloseableHttpClient client,
            String sessionId, HttpEntityEnclosingRequestBase request, List<NameValuePair> params)
                    throws Exception {
        if (!NO_AUTH_SESSION_ID.equals(sessionId)) {
            request.setHeader(AUTHORIZATION, sessionId);
        }

        if (params != null) {
            HttpEntity entity;
            entity = new UrlEncodedFormEntity(params, CHARSET_UTF8);
            request.setEntity(entity);
        }

        return sendRequest(client, request);
    }

    public static CloseableHttpResponse sendRequest(CloseableHttpClient client,
            HttpRequestBase request) throws Exception {
        try {
            CloseableHttpResponse response = client.execute(request);
            handleException(response);
            return response;
        }
        catch (ConnectionPoolTimeoutException e) {
            logger.warn("lease connection timeout, create a temp http client to send this request",
                    e);
            return resendRequest(request);
        }
    }

    private static CloseableHttpResponse resendRequest(HttpRequestBase request) throws Exception {
        CloseableHttpClient c = HttpClients.createDefault();
        logger.debug("create tempHttpClient:" + c);
        CloseableHttpResponse resp = c.execute(request);
        CloseableHttpResponseWrapper respWrapper = new CloseableHttpResponseWrapper(resp, c);
        handleException(respWrapper);
        return respWrapper;
    }

    private static void handleException(CloseableHttpResponse response) throws Exception {
        int httpStatusCode = response.getStatusLine().getStatusCode();

        // 2xx Success
        if (httpStatusCode >= 200 && httpStatusCode < 300) {
            return;
        }

        try {
            int errcode = httpStatusCode;
            String resp = getErrorResponse(response);

            throw new Exception("errcode=" + errcode + ",resp=" + resp);
        }
        finally {
            closeResp(response);
        }
    }

    private static String getErrorResponse(CloseableHttpResponse response) throws Exception {
        String error = null;

        HttpEntity entity = response.getEntity();
        if (null != entity) {
            error = EntityUtils.toString(entity);
        }
        else {
            Header header = response.getFirstHeader(ERROR_ATTRIBUTE);
            if (header != null) {
                error = header.getValue();
            }
        }

        return error;
    }

    public static void closeResp(CloseableHttpResponse response) {
        if (response != null) {
            try {
                response.close();
            }
            catch (Exception e) {
                logger.warn("close HttpResponse failed:resp={}", response, e);
            }
        }
    }  
    
    public static  CloseableHttpClient createHttpClient() {
        PoolingHttpClientConnectionManager connMgr = new PoolingHttpClientConnectionManager();
        connMgr.setMaxTotal(2);
        connMgr.setDefaultMaxPerRoute(2);

        RequestConfig reqConf = RequestConfig.custom().setConnectionRequestTimeout(1).build();

        CloseableHttpClient client = HttpClients.custom().setConnectionManager(connMgr)
        		.setDefaultRequestConfig(reqConf)
                .build();
        return client;
    } 
    
    
    /**
	 * create user
	 * @param userName,role	
	 * @return accessKeyID, secretAccessKey
	 */
    public static String[] createUser(String userName, String role) throws Exception{
		HttpPost request = new HttpPost(S3TestBase.s3ClientUrl + "/users/" 
					+ userName + "?role=" + role);		
        request.setHeader("Authorization", "Credential=ABCDEFGHIJKLMNOPQRST"); 
        client = createHttpClient();
        CloseableHttpResponse resp = RestClient.sendRequest(client, request);
        String respStr = EntityUtils.toString(resp.getEntity(), "utf-8");
//        System.out.println("----createuserrespstr="+respStr);
//        org.json.JSONObject json = XML.toJSONObject(respStr); 
//        
//        System.out.println("---json="+ json.toString());        
        String accessKeyID = getTotalMidValue( respStr, "<AccessKeyID>", "</AccessKeyID>");
        String secretAccessKey = getTotalMidValue( respStr, "<SecretAccessKey>", "</SecretAccessKey>");
        String[] accessKeys = { accessKeyID, secretAccessKey};
        return accessKeys;
    }   
    
    /**
   	 * intercept string
   	 * @param source
   	 * @param startStr
   	 * @param endStr	
   	 * @return accessKeyID, secretAccessKey
   	 */
    public static String getTotalMidValue( String source,String startStr, String endStr){
    	if( source == null )
    		return null;
    	int iFirst = source.indexOf(startStr);
    	int iLast  = source.indexOf(endStr);
    	if( iFirst < 0 || iLast < 0)
    		return null;
    	int beginIndex = iFirst + startStr.length();
    	return source.substring(beginIndex,iLast);
    }
    
    /**
	 * delete user
	 * @param userName
	 */
    public static void deleteUser(String userName) throws Exception{
        HttpDelete request  = new HttpDelete(S3TestBase.s3ClientUrl + "/users/" + userName);
        request.setHeader("Authorization", "Credential=ABCDEFGHIJKLMNOPQRST"); 
        client = createHttpClient();
        CloseableHttpResponse resp = RestClient.sendRequest(client, request);
        EntityUtils.toString(resp.getEntity(), "utf-8");
        
    }
    
    public static void getUser(String userName) throws Exception{
        HttpGet request = new HttpGet(S3TestBase.s3ClientUrl + "/users/" + userName);
        request.setHeader("Authorization", "Credential=ABCDEFGHIJKLMNOPQRST");
        client = createHttpClient();
        CloseableHttpResponse resp = RestClient.sendRequest(client, request);
        String respStr = EntityUtils.toString(resp.getEntity(), "utf-8");
        System.out.println("----getuser="+respStr);       
    }
    
    public static void createBucket() throws Exception{
        HttpPut request = new HttpPut(S3TestBase.s3ClientUrl + "/testbucket_.1001");
        request.setHeader("Authorization", "Credential=ABCDEFGHIJKLMNOPQRST");
        client = createHttpClient();
        CloseableHttpResponse resp = RestClient.sendRequest(client, request);
        String respStr = EntityUtils.toString(resp.getEntity(), "utf-8");
        System.out.println("----getuser="+respStr);       
    }


	
}