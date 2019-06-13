package com.sequoiadb.fulltext;

import org.springframework.core.io.InputStreamResource;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;
import org.springframework.http.client.HttpComponentsClientHttpRequestFactory;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpServerErrorException;
import org.springframework.web.client.RestTemplate;

import com.sequoiadb.commlib.SdbTestBase;

/**
 * @FileName FullTextRest.java
 * @Author luweikang
 * @Date 2019年5月30日
 */
public class FullTextRest {
    private HttpHeaders requestHeaders;
    private String addr = "http://" + SdbTestBase.esHostName + ":" + SdbTestBase.esServiceName;
    private String url;
    private HttpMethod requestMethod;
    private HttpEntity<?> requestEntity;
    private static RestTemplate rest;
    private Class<?> responseType;
    private Object uriVariables[];
    private String api;
    private Object resquestBody = null;
    private MultiValueMap<Object, Object> param;
    private InputStreamResource resource = null;

    static {
        HttpComponentsClientHttpRequestFactory factory = new HttpComponentsClientHttpRequestFactory();
        factory.setConnectionRequestTimeout(10000);
        factory.setConnectTimeout(10000);
        factory.setBufferRequestBody(false);
        factory.setReadTimeout(180000);
        rest = new RestTemplate(factory);
    }

    public FullTextRest() {
        super();
        this.requestHeaders = new HttpHeaders();
        this.param = new LinkedMultiValueMap<>();
    }

    public FullTextRest reset() {
        this.url = null;
        this.requestHeaders = null;
        return this;
    }

    public FullTextRest setRequestMethod(HttpMethod method) {
        this.requestMethod = method;
        return this;
    }

    public FullTextRest setRequestBody(Object body) {
        this.resquestBody = body;
        return this;
    }

    public Object getRequestBody() {
        return this.resquestBody;
    }

    public FullTextRest setApi(String api) {
        this.api = api;
        return this;
    }

    private String setUrl(String api) {
        this.url = addr + api;
        return url;
    }

    private HttpHeaders getRequestHeaders() {
        return this.requestHeaders;
    }

    public FullTextRest setResponseType(Class<?> responseType) {
        this.responseType = responseType;
        return this;
    }

    public FullTextRest setRequestHeaders(String headerName, String headerValue) {
        requestHeaders.add(headerName, headerValue);
        return this;
    }

    public FullTextRest setParameter(Object key, Object value) {
        param.set(key, value);
        return this;
    }

    public FullTextRest setUriVariables(Object[] uriVariables) {
        this.uriVariables = uriVariables;
        return this;
    }

    private String getUrl(String api) {
        setUrl(api);
        return url;
    }

    public ResponseEntity<?> exec() {
        ResponseEntity<?> response = null;
        try {
            if (null != this.resource) {
                requestEntity = new HttpEntity<>(this.resource, this.getRequestHeaders());
            } else {
                if (resquestBody != null) {
                    requestEntity = new HttpEntity<>(this.getRequestBody(), this.getRequestHeaders());
                } else {
                    requestEntity = new HttpEntity<>(this.param, this.getRequestHeaders());
                }
            }
            if (this.uriVariables != null) {
                response = rest.exchange(this.getUrl(this.api), this.requestMethod, this.requestEntity,
                        this.responseType, this.uriVariables);
            } else {
                response = rest.exchange(this.getUrl(this.api), this.requestMethod, this.requestEntity,
                        this.responseType);
            }
        } catch (HttpClientErrorException e) {
            throw e;
        } catch (HttpServerErrorException e) {
            throw e;
        } catch (Exception e) {
            throw e;
        } finally {
            this.reset();
        }
        return response;
    }

}
