package com.sequoiadb.om.plugin;

import com.sequoiadb.om.plugin.config.MySQLConfig;
import com.sequoiadb.om.plugin.config.OmsvcConfig;
import com.sequoiadb.om.plugin.config.PostgreSQLConfig;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.stereotype.Component;

@Component
public class Runner implements CommandLineRunner {

    @Autowired
    private Register register;

    @Autowired
    private OmsvcConfig omConf;

    @Autowired
    private PostgreSQLConfig pgConf;

    @Autowired
    private MySQLConfig mysqlConf;

    @Override
    public void run(String... strings) throws Exception {

        final Logger logger = LoggerFactory.getLogger(Runner.class);

        logger.info("Event: Init");

        pgConf.setName("SequoiaSQL");
        pgConf.setType("sequoiasql-postgresql");

        mysqlConf.setName("SequoiaSQL");
        mysqlConf.setType("sequoiasql-mysql");

        register.start();
    }
}

