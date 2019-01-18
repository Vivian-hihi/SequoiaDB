package com.sequoias3.dao.sequoiadb;

import com.sequoiadb.base.*;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoias3.common.DBParamDefine;
import com.sequoias3.common.RegionParamDefine;
import com.sequoias3.config.SequoiadbConfig;
import com.sequoias3.core.ObjectMeta;
import com.sequoias3.core.Region;
import com.sequoias3.core.RegionSpace;
import com.sequoias3.dao.ConnectionDao;
import com.sequoias3.dao.DaoCollectionDefine;
import com.sequoias3.dao.RegionDao;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Repository;

import java.util.ArrayList;
import java.util.List;

@Repository("RegionDao")
public class SequoiadbRegionDao implements RegionDao {
    private static final Logger logger = LoggerFactory.getLogger(SequoiadbRegionDao.class);

    @Autowired
    SdbDataSourceWrapper sdbDatasourceWrapper;

    @Autowired
    SequoiadbConfig config;

    @Override
    public void insertRegion(ConnectionDao connection, Region regionCon) throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = ((SdbConnectionDao)connection).getConnection();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.REGION_LIST_COLLECTION);

            cl.insert(regionCon.toBson());
        }catch (BaseException e){
            if (e.getErrorType() == SDBError.SDB_IXM_DUP_KEY.name()) {
                throw new S3ServerException(S3Error.DAO_DUPLICATE_KEY, "Duplicate key.");
            } else {
                throw e;
            }
        }catch (Exception e) {
            logger.error("insertRegion failed. errorMessage = " + e.getMessage());
            throw e;
        }
    }

    @Override
    public void updateRegion(ConnectionDao connection, Region regionCon) throws S3ServerException {
        try{
            Sequoiadb sdb = ((SdbConnectionDao)connection).getConnection();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.REGION_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(RegionSpace.REGION_SPACE_NAME, regionCon.getName());

            BSONObject updateData = new BasicBSONObject();
            if (regionCon.getDataCSShardingType() != null) {
                updateData.put(Region.DATA_CS_SHARDINGTYPE, regionCon.getDataCSShardingType());
            }
            if (regionCon.getDataCLShardingType() != null){
                updateData.put(Region.DATA_CL_SHARDINGTYPE, regionCon.getDataCLShardingType());
            }
            BSONObject setUpdate = new BasicBSONObject();
            setUpdate.put(DBParamDefine.MODIFY_SET, updateData);

            cl.update(matcher, setUpdate, null);
        }catch (Exception e){
            logger.error("update region config failed. error:"+e.getMessage());
            throw e;
        }
    }

    @Override
    public Region queryRegion(String regionName) throws S3ServerException {
        Sequoiadb sdb = null;
        try {
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.REGION_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(Region.REGION_NAME, regionName);

            BSONObject queryResult = cl.queryOne(matcher, null, null, null, 0);
            if (null == queryResult){
                return null;
            }
            return convertBsonToRegion(queryResult);
        }catch (Exception e) {
            logger.error("queryRegion failed. errorMessage = " + e.getMessage());
            throw e;
        }finally {
            sdbDatasourceWrapper.releaseSequoiadb(sdb);
        }
    }

    @Override
    public List<String> queryRegionList() throws S3ServerException {
        Sequoiadb sdb = null;
        DBCursor cursor = null;
        try{
            sdb = sdbDatasourceWrapper.getSequoiadb();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.REGION_LIST_COLLECTION);

            BSONObject selector = new BasicBSONObject();
            selector.put(Region.REGION_NAME, 1);

            BSONObject orderBy = new BasicBSONObject();
            orderBy.put(Region.REGION_NAME, 1);

            cursor = cl.query(null, selector, orderBy, null);
            ArrayList<String> regionList = new ArrayList<>();
            while (cursor.hasNext()){
                BSONObject record = cursor.getNext();
                regionList.add(record.get(Region.REGION_NAME).toString());
            }
            return regionList;
        }catch (Exception e) {
            logger.error("queryRegionList failed. errorMessage = " + e.getMessage());
            throw e;
        }finally {
            if (cursor != null){
                cursor.close();
            }
        }
    }

    @Override
    public void detectDomain(ConnectionDao connection, String domain) throws S3ServerException {
        try{
            Sequoiadb sdb = ((SdbConnectionDao)connection).getConnection();

            if (domain != null){
                if (!sdb.isDomainExist(domain)){
                    throw new S3ServerException(S3Error.REGION_INVALID_DOMAIN,
                            "Domain does not exist. domain:"+domain);
                }
            }
        }catch (BaseException e){
            throw e;
        }
    }

    @Override
    public void detectLocation(ConnectionDao connection, String CSName, String CLName, int locationType) throws S3ServerException{
        try{
            Sequoiadb sdb = ((SdbConnectionDao)connection).getConnection();
            CollectionSpace cs = null;
            if(!sdb.isCollectionSpaceExist(CSName)){
                throw new S3ServerException(S3Error.REGION_LOCATION_EXIST,
                        "DataLocation CollectionSpace does not exist. csName:"+CSName);
            }else {
                cs = sdb.getCollectionSpace(CSName);
                if(!cs.isCollectionExist(CLName)){
                    throw new S3ServerException(S3Error.REGION_LOCATION_EXIST,
                            "DataLocation Collection does not exist. csName:"+CSName+", clName:"+CLName);
                }
            }

            if (locationType != RegionParamDefine.LocationType.Data) {
                Boolean findIndex = false;
                DBCollection cl = cs.getCollection(CLName);
                DBCursor cursor = cl.getIndexes();
                while (cursor.hasNext()) {
                    BSONObject record = cursor.getNext();
                    BSONObject indexDef = (BSONObject) record.get("IndexDef");
                    BSONObject key = (BSONObject) indexDef.get("key");
                    Boolean unique = (Boolean) indexDef.get("unique");
                    Boolean enforced = (Boolean) indexDef.get("enforced");
                    if (key.containsField(ObjectMeta.META_KEY_NAME)
                            && key.containsField(ObjectMeta.META_BUCKET_ID)
                            && (locationType == RegionParamDefine.LocationType.Meta
                                || key.containsField(ObjectMeta.META_VERSION_ID))
                            && unique == true
                            && enforced == true) {
                        findIndex = true;
                        break;
                    }
                }

                if (findIndex == false) {
                    BSONObject indexKey = new BasicBSONObject();
                    String indexName = ObjectMeta.META_BUCKET_ID + "+" + ObjectMeta.META_KEY_NAME;
                    indexKey.put(ObjectMeta.META_BUCKET_ID, 1);
                    indexKey.put(ObjectMeta.META_KEY_NAME, 1);
                    if (locationType == RegionParamDefine.LocationType.MetaHis) {
                        indexKey.put(ObjectMeta.META_VERSION_ID, 1);
                        indexName = indexName + "+" + ObjectMeta.META_VERSION_ID;
                    }
                    sdbDatasourceWrapper.createIndex(sdb, CSName, CLName,
                            indexName, indexKey, true, true);
                }
            }

        }catch (BaseException e){
            throw e;
        }
    }

    @Override
    public void deleteRegion(ConnectionDao connection, String regionName)
            throws S3ServerException {
        try {
            Sequoiadb sdb = ((SdbConnectionDao)connection).getConnection();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.REGION_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(Region.REGION_NAME, regionName);

            cl.delete(matcher);
        }catch (BaseException e){
            if (e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                return;
            } else {
                logger.error("deleteRegion failed. "+ ", regionName:" + regionName + ", error:"+e.getMessage());
                throw e;
            }
        }catch (Exception e){
            throw e;
        }
    }

    @Override
    public Region queryForUpdateRegion(ConnectionDao connection, String regionName)
            throws S3ServerException {
        try {
            Sequoiadb sdb = ((SdbConnectionDao) connection).getConnection();
            CollectionSpace cs = sdb.getCollectionSpace(config.getMetaCsName());
            DBCollection cl = cs.getCollection(DaoCollectionDefine.REGION_LIST_COLLECTION);

            BSONObject matcher = new BasicBSONObject();
            matcher.put(Region.REGION_NAME, regionName);

            BSONObject queryResult = cl.queryOne(matcher, null, null, null, DBQuery.FLG_QUERY_FOR_UPDATE);
            if (null == queryResult){
                return null;
            }
            return convertBsonToRegion(queryResult);
        }catch (BaseException e) {
            if (e.getErrorCode() == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
                //no cl ,return null
                return null;
            } else {
                logger.error("queryForUpdateRegion failed. "+ ", regionName:" + regionName + ", error:"+e.getMessage());
                throw e;
            }
        } catch (Exception e) {
            logger.error("queryForUpdateRegion failed. "+ ", regionName:" + regionName + ", error = " + e.getMessage());
            throw e;
        }
    }



    private Region convertBsonToRegion(BSONObject result){
        if (null == result){
            return null;
        }

        Region region = new Region();
        region.setName(result.get(Region.REGION_NAME).toString());
        region.setCreateTime((long)result.get(Region.REGION_CREATERTIME));
        if (result.get(Region.DATA_CS_SHARDINGTYPE) != null){
            region.setDataCSShardingType(result.get(Region.DATA_CS_SHARDINGTYPE).toString());
        }
        if (result.get(Region.DATA_CL_SHARDINGTYPE) != null){
            region.setDataCLShardingType(result.get(Region.DATA_CL_SHARDINGTYPE).toString());
        }
        if (result.get(Region.META_DOMAIN) != null){
            region.setMetaDomain(result.get(Region.META_DOMAIN).toString());
        }
        if (result.get(Region.DATA_DOMAIN) != null){
            region.setDataDomain(result.get(Region.DATA_DOMAIN).toString());
        }
        if (result.get(Region.DATA_LOCATION) != null){
            region.setDataLocation(result.get(Region.DATA_LOCATION).toString());
        }
        if (result.get(Region.META_LOCATION) != null){
            region.setMetaLocation(result.get(Region.META_LOCATION).toString());
        }
        if (result.get(Region.META_HIS_LOCATION) != null){
            region.setMetaHisLocation(result.get(Region.META_HIS_LOCATION).toString());
        }
        if (result.get(Region.DATA_CS_LOCATION) != null){
            region.setDataCSLocation(result.get(Region.DATA_CS_LOCATION).toString());
        }
        if (result.get(Region.DATA_CL_LOCATION) != null){
            region.setDataCLLocation(result.get(Region.DATA_CL_LOCATION).toString());
        }
        if (result.get(Region.META_CS_LOCATION) != null){
            region.setMetaCSLocation(result.get(Region.META_CS_LOCATION).toString());
        }
        if (result.get(Region.META_CL_LOCATION) != null){
            region.setMetaCLLocation(result.get(Region.META_CL_LOCATION).toString());
        }
        if (result.get(Region.META_HIS_CS_LOCATION) != null){
            region.setMetaHisCSLocation(result.get(Region.META_HIS_CS_LOCATION).toString());
        }
        if (result.get(Region.META_HIS_CL_LOCATION) != null){
            region.setMetaHisCLLocation(result.get(Region.META_HIS_CL_LOCATION).toString());
        }

        return region;
    }
}
