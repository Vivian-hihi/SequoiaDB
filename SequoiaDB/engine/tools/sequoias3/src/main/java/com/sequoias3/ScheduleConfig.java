package com.sequoias3;

import org.springframework.boot.autoconfigure.batch.BatchProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.scheduling.annotation.SchedulingConfigurer;
import org.springframework.scheduling.config.ScheduledTaskRegistrar;

import java.lang.reflect.Method;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Configuration
@EnableScheduling
public class ScheduleConfig implements SchedulingConfigurer {
    @Override
    public void configureTasks(ScheduledTaskRegistrar scheduledTaskRegistrar) {
        scheduledTaskRegistrar.setScheduler(taskExecutor());
    }

    @Bean(destroyMethod = "shutdown")
    public ExecutorService taskExecutor() {
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
        return Executors.newScheduledThreadPool(newScheduleSize);
    }
}
