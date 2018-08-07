package com.sequoias3.config;

import java.nio.charset.Charset;
import java.util.List;

import com.fasterxml.jackson.dataformat.xml.XmlMapper;
import com.sequoias3.core.AccessKeys;
import com.sequoias3.core.serial.ErrorSerializer;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.converter.HttpMessageConverter;
import org.springframework.http.converter.json.MappingJackson2HttpMessageConverter;
import org.springframework.http.converter.xml.MappingJackson2XmlHttpMessageConverter;
import org.springframework.web.filter.HttpPutFormContentFilter;
import org.springframework.web.servlet.config.annotation.ContentNegotiationConfigurer;
import org.springframework.web.servlet.config.annotation.EnableWebMvc;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurerAdapter;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.module.SimpleModule;
import com.sequoias3.core.serial.UserAuthKeySerializer;
import com.sequoias3.core.Error;

@Configuration
@EnableWebMvc
public class WebMvcConfig extends WebMvcConfigurerAdapter {

    @Bean
    public HttpPutFormContentFilter httpPutFormContentFilter() {
        return new HttpPutFormContentFilter();
    }

    @Override
    public void configureContentNegotiation(ContentNegotiationConfigurer configurer) {
        super.configureContentNegotiation(configurer);
        configurer.favorPathExtension(false);
    }

    @Override
    public void configureMessageConverters(List<HttpMessageConverter<?>> converters) {
        super.configureMessageConverters(converters);

        //AccessKeys
        MappingJackson2HttpMessageConverter converter = new MappingJackson2HttpMessageConverter();
        converter.setDefaultCharset(Charset.forName("UTF-8"));
        converters.add(converter);

        ObjectMapper mapper = converter.getObjectMapper();
        SimpleModule module = new SimpleModule();
        module.addSerializer(AccessKeys.class, new UserAuthKeySerializer());
        module.addSerializer(Error.class, new ErrorSerializer());
        mapper.registerModule(module);

        XmlMapper mapperXml = new XmlMapper();
        SimpleModule moduleXml = new SimpleModule();
        moduleXml.addSerializer(AccessKeys.class, new UserAuthKeySerializer());
        moduleXml.addSerializer(Error.class, new ErrorSerializer());
        mapperXml.registerModule(moduleXml);

        MappingJackson2XmlHttpMessageConverter converterXml = new MappingJackson2XmlHttpMessageConverter(mapperXml);
        converterXml.setDefaultCharset(Charset.forName("UTF-8"));
        converters.add(converterXml);
    }
}