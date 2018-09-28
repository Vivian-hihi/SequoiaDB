package com.sequoias3.context;

import com.sequoias3.config.ContextConfig;
import com.sequoias3.config.ServiceInfo;
import com.sequoias3.dao.MetaDao;
import org.apache.commons.codec.binary.Hex;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import sun.misc.BASE64Decoder;

import java.util.*;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

@Component
public class ContextManager {
    private static final Logger logger = LoggerFactory.getLogger(ContextManager.class);
    private Lock lock = new ReentrantLock();

    @Autowired
    MetaDao metaDao;

    @Autowired
    ContextConfig contextConfig;

    @Autowired
    ServiceInfo serviceInfo;

    private AtomicLong index = new AtomicLong(0);
    private Map<String, Context> contextMap = new HashMap<>();

    public Context create(long bucketId){
        lock.lock();
        long newIndex = this.index.getAndIncrement();
        String indexString = "B"+bucketId+"P"+serviceInfo.getPort()+"H"+serviceInfo.getHost()+"I"+newIndex+"E";
        String token = new String(Hex.encodeHex(indexString.getBytes()));
        Context context = new Context(token, bucketId);
        context.setLastModified(System.currentTimeMillis());
        contextMap.put(token, context);
        lock.unlock();
        return context;
    }

    public void release(Context context){
        lock.lock();
        if (null != context) {
            metaDao.releaseDBAndCursor(context.getDbCursor());
            contextMap.remove(context.getToken());
        }
        lock.unlock();
    }

    public Context get(String continueToken){
        lock.lock();
        Context context = contextMap.get(continueToken);
        context.setLastModified(System.currentTimeMillis());
        lock.unlock();
        return context;
    }

    public void cleanExpiredContext(){
        logger.debug("before scan. contextMap size:"+contextMap.size());
        long contextMaxLife = contextConfig.getLifecycle() * 60 * 1000;

        lock.lock();
        long nowTime = System.currentTimeMillis();
        Iterator<Map.Entry<String, Context>> it = contextMap.entrySet().iterator();
        while(it.hasNext()){
            Map.Entry<String, Context> entry = it.next();
            long diff = nowTime - entry.getValue().getLastModified();
            if (diff > contextMaxLife){
                logger.info("release context. context{}", entry.getValue().toString());
                metaDao.releaseDBAndCursor(entry.getValue().getDbCursor());
                it.remove();
            }
        }
        lock.unlock();

        logger.debug("after scan. contextMap size:"+contextMap.size());
    }
}
