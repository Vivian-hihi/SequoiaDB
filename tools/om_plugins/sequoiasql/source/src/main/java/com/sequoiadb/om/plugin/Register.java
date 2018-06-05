package com.sequoiadb.om.plugin;

import com.sequoiadb.om.plugin.config.MySQLConfig;
import com.sequoiadb.om.plugin.config.OmsvcConfig;
import com.sequoiadb.om.plugin.config.PluginConfig;
import com.sequoiadb.om.plugin.config.PostgreSQLConfig;
import org.json.JSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.stereotype.Component;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.web.client.RestTemplate;

import java.util.Collections;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

@Component
public class Register extends TimerTask {

    private final Logger logger = LoggerFactory.getLogger(Register.class);
    private Timer timer = new Timer();

    @Autowired
    private OmsvcConfig omConf;

    @Autowired
    private PostgreSQLConfig pgConfig;

    @Autowired
    private MySQLConfig mysqlConfig;

    public void start() {
        timer.schedule(this, 0, 1000);
    }

    public synchronized boolean register(boolean isInit) {

        if (isInit) {
            pgConfig.setIsRegister(false);
            mysqlConfig.setIsRegister(false);
        }

        boolean rc1 = registerPlugin(pgConfig);
        boolean rc2 = registerPlugin(mysqlConfig);
        if (rc1 && rc2) {
            logger.info("Event: timer cancel");
            timer.cancel();
        }
        return rc1 && rc2;
    }

    public synchronized boolean isTimeout() {
        long currentTime = new Date().getTime() / 1000;
        return currentTime - omConf.getRegisterTime() >= omConf.getLeaseTime();
    }

    public void run() {
        register(false);
    }

    private boolean registerPlugin(PluginConfig config) {

        if (config.getIsRegister()) {
            return true;
        }

        String url = "http://" + omConf.getHostName() + ":" + omConf.getHttpname();

        String pluginPublicKey = Crypto.randomGeneratePublicKey();

        LinkedMultiValueMap<String, String> para = new LinkedMultiValueMap<String, String>();
        para.put("cmd", Collections.singletonList("register plugin"));
        para.put("Name", Collections.singletonList(config.getName()));
        para.put("HostName", Collections.singletonList(config.getHostName()));
        para.put("ServiceName", Collections.singletonList(config.getSvcname()));
        para.put("Role", Collections.singletonList(config.getRole()));
        para.put("Type", Collections.singletonList(config.getType()));
        para.put("PublicKey", Collections.singletonList(pluginPublicKey));

        HttpHeaders httpHeaders = new HttpHeaders();
        httpHeaders.setContentType(MediaType.APPLICATION_FORM_URLENCODED);

        HttpEntity<LinkedMultiValueMap<String, String>> httpEntity = new HttpEntity<LinkedMultiValueMap<String, String>>(para, httpHeaders);

        try {
            RestTemplate restTemp = new RestTemplate();
            String response = restTemp.postForObject(url, httpEntity, String.class);
            omConf.setRegisterTime(new Date().getTime() / 1000);
            JSONObject result = new JSONObject(response);
            int errno = result.getInt("errno");
            if (errno != 0) {
                logger.error("Failed to register with om svc, detail: " + result.getString("detail"));
            } else {
                omConf.setSvcname(result.getString("svcname"));
                omConf.setUser(Crypto.decrypt(pluginPublicKey, result.getString("user")));
                omConf.setPasswd(Crypto.decrypt(pluginPublicKey, result.getString("passwd")));
                omConf.setLeaseTime(result.getLong("leaseTime"));
                logger.info("Event: " + config.getType() + " plugin success");
                //logger.info("%s %s %s %d %d\r\n", omSvcname, omUser, omPasswd, registerTime, leaseTime);
                config.setIsRegister(true);
                return true;
            }
        } catch (Exception e) {
            logger.error("Failed to access om svc, detail: " + e);
        }
        return false;
    }
}