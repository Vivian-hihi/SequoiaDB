package com.sequoias3;

import org.springframework.boot.context.embedded.tomcat.TomcatEmbeddedServletContainerFactory;
import org.springframework.context.annotation.Bean;
import org.springframework.stereotype.Component;

@Component
public class TomcatConfig{

    @Bean
    public TomcatEmbeddedServletContainerFactory containerCustomizer(CustomContextValve contextValve){
        TomcatEmbeddedServletContainerFactory factory = new TomcatEmbeddedServletContainerFactory();
        factory.addContextValves(contextValve);
        return factory;
    }

    @Bean
    public CustomContextValve contextValve(){
        return new CustomContextValve();
    }
}
