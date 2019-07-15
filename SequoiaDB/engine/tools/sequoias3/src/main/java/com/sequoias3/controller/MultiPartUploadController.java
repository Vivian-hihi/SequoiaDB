package com.sequoias3.controller;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.xml.XmlMapper;
import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.S3InputStreamReaderChunk;
import com.sequoias3.core.User;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.model.*;
import com.sequoias3.service.ObjectService;
import com.sequoias3.utils.RestUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import javax.servlet.ServletInputStream;
import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

@RestController
public class MultiPartUploadController {
    private final Logger logger = LoggerFactory.getLogger(MultiPartUploadController.class);

    @Autowired
    RestUtils restUtils;

    @Autowired
    ObjectService objectService;

    @PostMapping(value="/{bucketname:.+}/**", params = RestParamDefine.UPLOADS, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity initMultiPartUploadObject(@PathVariable("bucketname") String bucketName,
                                                    @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                                    HttpServletRequest httpServletRequest)
            throws S3ServerException {
        User operator = restUtils.getOperatorByAuthorization(authorization);

        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.info("initMultiPartUploadObject. bucketName={}, objectName={}", bucketName, objectName);

        Map<String, String> requestHeaders = new HashMap<>();
        Map<String, String> xMeta          = new HashMap<>();
        restUtils.getHeaders(httpServletRequest, requestHeaders, xMeta);

        InitiateMultipartUploadResult result = objectService.initMultipartUpload(operator.getUserId(),
                bucketName,
                objectName,
                requestHeaders,
                xMeta);

        logger.info("initMultiPartUploadObject success. bucketName={}, objectName={}", bucketName, objectName);
        return ResponseEntity.ok()
                .body(result);
    }

    @PutMapping(value="/{bucketname:.+}/**", params = RestParamDefine.PARTNUMBER, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity uploadPart(@PathVariable("bucketname") String bucketName,
                                     @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                     @RequestHeader(name = RestParamDefine.PutObjectHeader.CONTENT_MD5, required = false) String contentMD5,
                                     @RequestParam(RestParamDefine.PARTNUMBER) int partNumber,
                                     @RequestParam(RestParamDefine.UPLOADID) long uploadId,
                                     HttpServletRequest httpServletRequest)
            throws S3ServerException, IOException {
        User operator = restUtils.getOperatorByAuthorization(authorization);

        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.info("uploadpart. bucketName={}, objectName={}, uploadId={}, partNumber={}", bucketName, objectName, uploadId, partNumber);

        InputStream body =  httpServletRequest.getInputStream();
        Long realContenLength = null;
        if (httpServletRequest.getHeader("x-amz-decoded-content-length") != null){
            body = new S3InputStreamReaderChunk(httpServletRequest.getInputStream());
            realContenLength = Long.parseLong(httpServletRequest.getHeader("x-amz-decoded-content-length"));
        }else {
            if (httpServletRequest.getHeader("content-length") != null) {
                realContenLength = Long.parseLong(httpServletRequest.getHeader("content-length"));
            }
        }
        String eTag = objectService.uploadPart(operator.getUserId(),
                bucketName,
                objectName,
                uploadId,
                partNumber,
                contentMD5,
                body,
                realContenLength);

        HttpHeaders headers = new HttpHeaders();
        if (eTag != null){
            headers.add(RestParamDefine.PutObjectResultHeader.ETAG,
                    "\""+eTag+"\"");
        }

        logger.info("uploadpart success. bucketName={}, objectName={}, uploadId={}, " +
                        "partNumber={}, eTag={}, realContenLength={}",
                bucketName, objectName, uploadId, partNumber, eTag, realContenLength);
        return ResponseEntity.ok()
                .headers(headers)
                .build();
    }

    @PostMapping(value="/{bucketname:.+}/**", params = RestParamDefine.UPLOADID, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity completeMultiPart(@PathVariable("bucketname") String bucketName,
                                            @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                            @RequestParam(RestParamDefine.UPLOADID) long uploadId,
                                            HttpServletRequest httpServletRequest,
                                            HttpServletResponse httpServletResponse)
            throws S3ServerException, IOException {
        User operator = restUtils.getOperatorByAuthorization(authorization);

        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.info("completeMultiPart. bucketName={}, objectName={}, uploadId:{}", bucketName, objectName, uploadId);

        ServletOutputStream outputStream = httpServletResponse.getOutputStream();

        CompleteMultipartUpload completeMultipartUpload = getCompleteMap(httpServletRequest);

        CompleteMultipartUploadResult result = objectService.completeUpload(operator.getUserId(),
                bucketName,
                objectName,
                uploadId,
                completeMultipartUpload.getPart(),
                outputStream);

        result.setLocation(httpServletRequest.getRequestURI());

        logger.info("completeMultiPart success. bucketName={}, objectName={}, uploadId={}",
                bucketName, objectName, uploadId);

        HttpHeaders headers = new HttpHeaders();
        if (result.getVersionId() != null){
            headers.add(RestParamDefine.PutObjectResultHeader.VERSION_ID,
                    result.getVersionId().toString());
        }

        return ResponseEntity.ok()
                .headers(headers)
                .body(result);
    }

    @GetMapping(value="/{bucketname:.+}/**", params = RestParamDefine.UPLOADID, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listParts(@PathVariable("bucketname") String bucketName,
                                    @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                    @RequestParam(RestParamDefine.UPLOADID) long uploadId,
                                    @RequestParam(value = RestParamDefine.ListPartsPara.PART_NUMBER_MARKER, required = false) Integer partNumberMarker,
                                    @RequestParam(value = RestParamDefine.ListPartsPara.MAX_PARTS, required = false, defaultValue = "1000") Integer maxParts,
                                    @RequestParam(value = RestParamDefine.ListPartsPara.ENCODING_TYPE, required = false) String encodingType,
                                    HttpServletRequest httpServletRequest)
            throws S3ServerException {
        User operator = restUtils.getOperatorByAuthorization(authorization);

        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.info("listParts. bucketName={}, objectName={}, uploadId:{}", bucketName, objectName, uploadId);

        ListPartsResult result = objectService.listParts(operator.getUserId(),
                bucketName,
                objectName,
                uploadId,
                partNumberMarker,
                maxParts,
                encodingType);

        logger.info("listParts success. bucketName={}, objectName={}, uploadId={}, part.size={}",
                bucketName, objectName, uploadId, result.getPartList().size());
        return ResponseEntity.ok()
                .body(result);
    }

    @DeleteMapping(value="/{bucketname:.+}/**", params = RestParamDefine.UPLOADID, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity abortUpload(@PathVariable("bucketname") String bucketName,
                                      @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                      @RequestParam(RestParamDefine.UPLOADID) long uploadId,
                                      HttpServletRequest httpServletRequest) throws S3ServerException{
        User operator = restUtils.getOperatorByAuthorization(authorization);

        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.info("abortUpload. bucketName={}, objectName={}, uploadId:{}", bucketName, objectName, uploadId);

        objectService.abortUpload(operator.getUserId(), bucketName, objectName, uploadId);

        logger.info("abortUpload success. bucketName={}, objectName={}, uploadId={}", bucketName, objectName, uploadId);
        return ResponseEntity.ok()
                .build();
    }

    @GetMapping(value="/{bucketname:.+}", params = RestParamDefine.UPLOADS, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listUploads(@PathVariable("bucketname") String bucketName,
                                      @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                      @RequestParam(value = RestParamDefine.ListUploadsPara.PREFIX, required = false) String prefix,
                                      @RequestParam(value = RestParamDefine.ListUploadsPara.DELIMITER, required = false) String delimiter,
                                      @RequestParam(value = RestParamDefine.ListUploadsPara.KEY_MARKER, required = false) String keyMarker,
                                      @RequestParam(value = RestParamDefine.ListUploadsPara.UPLOAD_ID_MARKER, required = false) Long uploadIdMarker,
                                      @RequestParam(value = RestParamDefine.ListUploadsPara.ENCODING_TYPE, required = false) String encodingType,
                                      @RequestParam(value = RestParamDefine.ListUploadsPara.MAX_UPLOADS, required = false, defaultValue = "1000") Integer maxUploads)
            throws S3ServerException {
        User operator = restUtils.getOperatorByAuthorization(authorization);

        logger.info("listUploads. bucketName={}, prefix={}, delimiter={}, keymarker={}, uploadidmarker={}",
                bucketName, prefix, delimiter, keyMarker, uploadIdMarker);

        ListMultipartUploadsResult result = objectService.listUploadLists(operator.getUserId(),
                bucketName,
                prefix,
                delimiter,
                keyMarker,
                uploadIdMarker,
                maxUploads,
                encodingType);

        logger.info("listUploads success. bucketName={}, upload.size={}, commonPrefix.size={}",
                bucketName, result.getUploadList().size(), result.getCommonPrefixList().size());
        return ResponseEntity.ok()
                .body(result);
    }

    private CompleteMultipartUpload getCompleteMap(HttpServletRequest httpServletRequest)
            throws S3ServerException{
        int ONCE_READ_BYTES  = 1024;
        try {
            ServletInputStream inputStream = httpServletRequest.getInputStream();
            StringBuilder stringBuilder = new StringBuilder();
            byte[] b = new byte[ONCE_READ_BYTES];
            int len = inputStream.read(b , 0, ONCE_READ_BYTES);
            while(len > 0){
                stringBuilder.append(new String(b,0, len));
                len = inputStream.read(b , 0, ONCE_READ_BYTES);
            }
            String content = stringBuilder.toString();
            if (content.length() > 0) {
                ObjectMapper objectMapper = new XmlMapper();
                return objectMapper.readValue(content, CompleteMultipartUpload.class);
            }else {
                return null;
            }
        }catch (Exception e){
            throw new S3ServerException(S3Error.OBJECT_PUT_fAILED, "get completeMultipartUpload failed", e);
        }
    }
}
