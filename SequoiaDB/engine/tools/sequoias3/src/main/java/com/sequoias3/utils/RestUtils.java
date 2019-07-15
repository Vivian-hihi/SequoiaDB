package com.sequoias3.utils;

import com.sequoias3.common.InitAdminUserDefine;
import com.sequoias3.common.RestParamDefine;
import com.sequoias3.config.AuthorizationConfig;
import com.sequoias3.core.Range;
import com.sequoias3.core.User;
import com.sequoias3.dao.UserDao;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import javax.servlet.http.HttpServletRequest;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.Enumeration;
import java.util.Map;

@Component
public class RestUtils {
    private static final Logger logger = LoggerFactory.getLogger(RestUtils.class);

    @Autowired
    UserDao userDao;

    @Autowired
    AuthorizationConfig authConfig;

    public User getOperatorByAuthorization(String authorization) throws S3ServerException {
        //       1.get access key id
        String accessKeyId = null;
        if (authConfig.isCheck()) {
            if (authorization == null){
                throw new S3ServerException(S3Error.INVALID_AUTHORIZATION, "authorization is null");
            }

            int beginIndex = authorization.indexOf(RestParamDefine.REST_CREDENTIAL);
            if (-1 != beginIndex) {
                int endIndex = authorization.indexOf(RestParamDefine.REST_DELIMITER, beginIndex);
                if (endIndex != -1) {
                    accessKeyId = authorization.substring(beginIndex + RestParamDefine.REST_CREDENTIAL.length(), endIndex);
                } else {
                    throw new S3ServerException(S3Error.INVALID_AUTHORIZATION, "authorization is invalid. authorization="+authorization);
                }
            }else {
                int beginIndexV2 = authorization.indexOf(RestParamDefine.REST_AWS);
                if (-1 == beginIndexV2){
                    throw new S3ServerException(S3Error.INVALID_AUTHORIZATION, "authorization is invalid. authorization = " + authorization);
                }

                String accessKeys = authorization.substring(beginIndexV2+RestParamDefine.REST_AWS.length());

                String[] keys = accessKeys.split(":");

                if (keys.length < 2){
                    throw new S3ServerException(S3Error.INVALID_AUTHORIZATION, "no aws accesskeyId. authorization:"+authorization);
                }

                accessKeyId = keys[0].trim();
            }
        }else {
            return userDao.getUserByName(InitAdminUserDefine.ADMIN_NAME);
        }

        //       2.check access key
        User user = userDao.getUserByAccessKeyID(accessKeyId);
        if (null == user) {
            throw new S3ServerException(S3Error.INVALID_ACCESSKEYID,
                    "Invalid accessKeyId. accessKeyId = " + accessKeyId);
        }

        //       3.check signature

        return user;
    }

    public String getObjectNameByURI(String url) throws S3ServerException {
        String decodeUrl;
        try {
            decodeUrl = URLDecoder.decode(url, "UTF-8");
        }catch (UnsupportedEncodingException e){
            throw new S3ServerException(S3Error.OBJECT_INVALID_KEY, "Invalid key. url = " + url);
        }

        int beginObject = decodeUrl.indexOf(RestParamDefine.REST_DELIMITER, 1);
        if (beginObject == -1) {
            throw new S3ServerException(S3Error.OBJECT_INVALID_KEY, "Invalid key. url = " + url);
        }

        return decodeUrl.substring(beginObject+1);
    }

    public Range getRange(String rangeHeader){
        try {
            return new Range(rangeHeader);
        }catch (S3ServerException e){
            return null;
        }
    }

    public void getHeaders(HttpServletRequest httpServletRequest,
                           Map<String, String> requestHeaders, Map<String, String> xMeta ){
        Enumeration names = httpServletRequest.getHeaderNames();
        while (names.hasMoreElements()){
            String name = names.nextElement().toString();
            if (name.startsWith(RestParamDefine.PutObjectHeader.X_AMZ_META_PREFIX)){
                xMeta.put(name,httpServletRequest.getHeader(name));
            }else {
                requestHeaders.put(name, httpServletRequest.getHeader(name));
            }
        }
    }
}
