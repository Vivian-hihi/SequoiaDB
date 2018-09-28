package com.sequoias3.dao;

import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoias3.core.InsertResult;
import com.sequoias3.core.ObjectMeta;
import com.sequoias3.core.Range;
import com.sequoias3.exception.S3ServerException;
import org.bson.types.ObjectId;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Date;

public interface DataDao {
    InsertResult insertObjectData(String csName, String clName, InputStream data) throws S3ServerException;

    void getObjectDataByLobId(String csName, String clName, ObjectId lobId, Range range, OutputStream outputStream) throws S3ServerException;

    void deleteObjectDataByLobId(String csName, String clName, ObjectId lobId) throws S3ServerException;

    String getDataCSName( String region, Date date);

    String getDataClName();
}
