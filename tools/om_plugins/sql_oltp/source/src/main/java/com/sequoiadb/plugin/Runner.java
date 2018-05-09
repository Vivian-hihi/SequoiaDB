package com.sequoiadb.plugin;

import com.sequoiadb.plugin.dao.SqlOperations;
import com.sequoiadb.plugin.config.OmsvcConfig;
import com.sequoiadb.plugin.config.PluginConfig;
import org.bson.BSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.stereotype.Component;

import java.util.List;

@Component
public class Runner implements CommandLineRunner {

    @Autowired
    private Register register;

    @Autowired
    private OmsvcConfig omConf;

    @Autowired
    private PluginConfig pluginConf;

    @Override
    public void run(String... strings) throws Exception {

        final Logger logger = LoggerFactory.getLogger(Runner.class);

        logger.info("Event: Init");

        pluginConf.setName("SequoiaPostgreSQL");
        pluginConf.setType("sequoiapostgresql");

        register.start();
    }
}

