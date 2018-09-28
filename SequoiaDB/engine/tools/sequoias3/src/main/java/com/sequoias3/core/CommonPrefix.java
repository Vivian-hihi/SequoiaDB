package com.sequoias3.core;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;

public class CommonPrefix {
    @JsonProperty("Prefix")
    private String prefix;
    @JsonProperty("Owner")
    private Owner owner;

    public CommonPrefix(String prefix, String encodingType) throws S3ServerException{
        try {
            if (null != encodingType) {
                this.prefix = URLEncoder.encode(prefix, "UTF-8");
            }else {
                this.prefix = prefix;
            }
        }catch (UnsupportedEncodingException e){
            throw new S3ServerException(S3Error.UNKNOWN_ERROR, "URL encode failed", e);
        }
    }

    public void setPrefix(String prefix) {
        this.prefix = prefix;
    }

    public String getPrefix() {
        return prefix;
    }

    public void setOwner(Owner owner) {
        this.owner = owner;
    }

    public Owner getOwner() {
        return owner;
    }

    @Override
    public boolean equals(Object o){
        CommonPrefix inItem = (CommonPrefix) o;
        return prefix.equals(inItem.prefix);
    }

    @Override
    public int hashCode(){
        return prefix.hashCode();
    }
}
