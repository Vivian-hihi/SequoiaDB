package com.sequoias3.taskmanager;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import javax.servlet.ServletOutputStream;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

@Component
@EnableScheduling
public class OutStreamFlushQueue {
    private static final Logger logger = LoggerFactory.getLogger(OutStreamFlushQueue.class);
    public final static long TWENTY_SECONDS = 20 * 1000;
    private static Lock lock = new ReentrantLock();
    private static Map<Long, ServletOutputStream> outputStreamHashMap = new HashMap<>();

    public static void add(Long uploadId, ServletOutputStream outputStream){
        lock.lock();
        try{
            outputStreamHashMap.put(uploadId, outputStream);
        }finally {
            lock.unlock();
        }
    }

    public static void remove(Long uploadId, ServletOutputStream outputStream){
        lock.lock();
        try{
            if (outputStreamHashMap.get(uploadId) != null
                    && outputStreamHashMap.get(uploadId) == outputStream){
                outputStreamHashMap.remove(uploadId);
            }
        }finally {
            lock.unlock();
        }
    }

    @Scheduled(initialDelay = 1000 * 10,fixedDelay = TWENTY_SECONDS)
    public void flushOutputStream(){
        logger.debug("scan begin, outputStreamHashMap size:" + outputStreamHashMap.size());
        String whiteSpace = " ";
        byte[] whitebyte = whiteSpace.getBytes();
        lock.lock();
        try{
            Iterator<Map.Entry<Long, ServletOutputStream>> it = outputStreamHashMap.entrySet().iterator();
            while (it.hasNext()) {
                Map.Entry<Long, ServletOutputStream> entry = it.next();
                try {
                    entry.getValue().write(whitebyte);
                    entry.getValue().flush();
                }catch (Exception e){
                    it.remove();
                }
            }
        }finally {
            lock.unlock();
        }
        logger.debug("scan end, outputStreamHashMap size:" + outputStreamHashMap.size());
    }
}
