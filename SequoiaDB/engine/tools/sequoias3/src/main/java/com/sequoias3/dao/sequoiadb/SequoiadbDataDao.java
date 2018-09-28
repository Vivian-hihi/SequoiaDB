package com.sequoias3.dao.sequoiadb;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBLob;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoias3.common.DBParamDefine;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.core.InsertResult;
import com.sequoias3.core.ObjectMeta;
import com.sequoias3.core.Range;
import com.sequoias3.dao.DaoCollectionDefine;
import com.sequoias3.dao.DataDao;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import org.apache.commons.codec.binary.Hex;
import org.bson.types.ObjectId;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;

import java.io.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Date;

@Repository("DataDao")
public class SequoiadbDataDao implements DataDao {
    private static final Logger logger = LoggerFactory.getLogger(SequoiadbDataDao.class);

    private static int ONCE_WRITE_BYTES  = 1024 * 1024;       //1MB

    @Autowired
    SdbDataSourceWrapper sdbDatasourceWrapper;

    @Autowired
    SequoiadbConfig config;

    @Override
    public InsertResult insertObjectData(String csName, String clName, InputStream data)
            throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb               = sdbDatasourceWrapper.getSequoiadb();
            DBLob dbLob       = createLobWithCsCl(sdb, csName, clName);
            MessageDigest MD5 = MessageDigest.getInstance("MD5");

            byte[] buffer    = new byte[ONCE_WRITE_BYTES];

            int size = data.read(buffer, 0, buffer.length);
            while (size > 0){
                //md5
                MD5.update(buffer, 0, size);

                //write lob
                dbLob.write(buffer, 0, size);

                //get bytes
                size = data.read(buffer, 0, buffer.length);
            }

            //record md5 lobId size
            InsertResult result = new InsertResult();
            result.seteTag(new String(Hex.encodeHex(MD5.digest())));
            result.setSize(dbLob.getSize());
            result.setLobId(dbLob.getID());

            //close lob
            dbLob.close();
            return result;
        } catch (S3ServerException e) {
            throw e;
        } catch (IOException e){
            throw new S3ServerException(S3Error.UNKNOWN_ERROR, "IOException", e);
        } catch (NoSuchAlgorithmException e){
            throw new S3ServerException(S3Error.UNKNOWN_ERROR, "NoSuchAlgorithmException", e);
        } catch (Exception e) {
            throw e;
        } finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    private DBLob createLobWithCsCl(Sequoiadb sdb, String csName, String clName)
            throws S3ServerException{
        try {
            return createLob(sdb, csName, clName);
        } catch (BaseException e) {
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode()) {
                sdbDatasourceWrapper.createCS(sdb, csName, null);
                sdbDatasourceWrapper.createCL(sdb, csName, clName, null);
                return createLob(sdb, csName, clName);
            } else if (e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                if (!sdb.isCollectionSpaceExist(csName)) {
                    sdbDatasourceWrapper.createCS(sdb, csName, null);
                }
                sdbDatasourceWrapper.createCL(sdb, csName, clName, null);
                return createLob(sdb, csName, clName);
            } else {
                logger.error("create lob failed. error:", e);
                throw e;
            }
        }
    }

    private DBLob createLob(Sequoiadb sdb, String csName, String clName){
        try{
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            DBCollection cl = cs.getCollection(clName);

            return cl.createLob();
        }catch (Exception e){
            logger.error("create lob failed. error:",e);
            throw e;
        }
    }

    @Override
    public void getObjectDataByLobId(String csName, String clName, ObjectId lobId, Range range,
                                     OutputStream outputStream) throws S3ServerException {
        Sequoiadb sdb = null;
        DBLob dbLob = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            DBCollection cl = cs.getCollection(clName);

            try {
                dbLob = cl.openLob(lobId);
                long contentLength = dbLob.getSize();
                if (null == range){
                    dbLob.read(outputStream);
                    return;
                }

                if (range.getStart() >= contentLength){
                    throw new S3ServerException(S3Error.OBJECT_RANGE_INVALID,
                            "start > contentlength. start:" + range.getStart() +
                                    ", contentlength：" + contentLength);
                }

                //final bytes
                if (range.getStart() == -1){
                    if(range.getEnd() < contentLength) {
                        range.setStart(contentLength - range.getEnd());
                        range.setEnd(contentLength-1);
                    }else {
                        range.setStart(0);
                        range.setEnd(contentLength-1);
                    }
                }

                //from start to the final of Lob
                if (range.getEnd() == -1 || range.getEnd() >= contentLength){
                    range.setEnd(contentLength - 1);
                }

                //from 0 - final of Lob
                if (range.getStart() == 0 && range.getEnd() == contentLength - 1){
                    dbLob.read(outputStream);
                    range.setStart(contentLength);
                    return;
                }
                byte[] buffer    = new byte[ONCE_WRITE_BYTES];
                long readOffset  = range.getStart();
                long readLength  = range.getEnd() - range.getStart() + 1;
                range.setContentLength(readLength);
                int writeOffset = 0;

                dbLob.seek(readOffset, DBLob.SDB_LOB_SEEK_SET);
                int size = dbLob.read(buffer, 0,
                        readLength > ONCE_WRITE_BYTES ? ONCE_WRITE_BYTES: (int)readLength);
                while (size > 0) {
                    outputStream.write(buffer, writeOffset, size);
                    writeOffset  += size;
                    readOffset   += size;

                    readLength -= size;
                    size = dbLob.read(buffer, 0,
                            readLength > ONCE_WRITE_BYTES ? ONCE_WRITE_BYTES : (int)readLength);
                }
                return;
            }finally {
                closeLob(dbLob);
            }
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode()
                    || e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                //no cs or no cl
                throw new S3ServerException(S3Error.OBJECT_NO_SUCH_KEY,
                        "cs:" + csName + ", cl:" + clName + ", error message:"+e.toString());
            } else if(e.getErrorCode() == SDBError.SDB_FNE.getErrorCode()){
                //no lob
                throw new S3ServerException(S3Error.DAO_LOB_FNE,
                        "cs:" + csName + ", cl:" + clName + ", error message:"+e.toString());
            } else{
                logger.error("get lob failed. error:");
                throw e;
            }
        } catch (IOException e){
            logger.error("get lob failed.");
            throw new S3ServerException(S3Error.UNKNOWN_ERROR, "IOException. error:"+e.getMessage(), e);
        } catch (Exception e){
            logger.error("get lob failed.");
            throw e;
        } finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public void deleteObjectDataByLobId(String csName, String clName, ObjectId lobId)
            throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(csName);
            DBCollection cl = cs.getCollection(clName);

            cl.removeLob(lobId);
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode()
                    || e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()
                    || e.getErrorCode() == SDBError.SDB_FNE.getErrorCode()) {
                //no cs or no cl or no lob ,return null
                return;
            }else if (e.getErrorCode() == SDBError.SDB_LOB_IS_IN_USE.getErrorCode()){
                throw new S3ServerException(S3Error.OBJECT_IS_IN_USE,
                        "lob is in use. cs" + csName +", cl:" + clName +", LobId:" +lobId.toString());
            }else {
                logger.error("delete lob failed. error:",e);
                throw e;
            }
        } catch (Exception e){
            logger.error("delete lob failed.",e);
            throw e;
        } finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    private void closeLob(DBLob lob){
        if (null == lob) {
            return;
        }
        try {
            lob.close();
        }catch (Exception e){
            logger.error("lob close failed.", e);
        }
    }

    @Override
    public String getDataCSName( String region, Date date){
        StringBuilder csName = new StringBuilder();

        if (null != region){
            csName.append(region);
            csName.append(DBParamDefine.CS_DATA);
        }else {
            csName.append(config.getDataCsName());
        }

        return csName.toString();
    }

    @Override
    public String getDataClName(){
        return DaoCollectionDefine.OBJECT_DATA_LIST;
    }
}
