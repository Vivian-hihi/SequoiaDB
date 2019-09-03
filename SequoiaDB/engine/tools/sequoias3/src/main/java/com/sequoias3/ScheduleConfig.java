package com.sequoias3;

import org.springframework.boot.autoconfigure.batch.BatchProperties;
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.scheduling.annotation.SchedulingConfigurer;
import org.springframework.scheduling.config.ScheduledTaskRegistrar;

import java.lang.reflect.Method;
import java.util.concurrent.Executors;

@Configuration
public class ScheduleConfig implements SchedulingConfigurer {
    @Override
    public void configureTasks(ScheduledTaskRegistrar scheduledTaskRegistrar) {
        Method[] methods = BatchProperties.Job.class.getMethods();
        int scheduleSize = 5;
        int newScheduleSize = 0;
        if (methods != null && methods.length > 0){
            for (Method method : methods) {
                Scheduled annotation = method.getAnnotation(Scheduled.class);
                if (annotation != null){
                    newScheduleSize++;
                }
            }
        }
        if (scheduleSize > newScheduleSize){
            newScheduleSize = scheduleSize;
        }
        scheduledTaskRegistrar.setScheduler(Executors.newScheduledThreadPool(newScheduleSize));
    }
}
