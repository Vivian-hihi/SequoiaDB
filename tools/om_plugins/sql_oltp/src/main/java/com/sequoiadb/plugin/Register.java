package com.sequoiadb.plugin;

import com.sequoiadb.plugin.config.OmsvcConfig;
import com.sequoiadb.plugin.config.PluginConfig;
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
    private PluginConfig pluginConf;

    public void start() {
        timer.schedule(this, 0, 1000);
    }

    public synchronized boolean register(){
        boolean rc = registerPlugin();
        if ( rc ) {
            timer.cancel();
        }
        return rc;
    }

    public synchronized boolean isTimeout() {
        long currentTime = new Date().getTime() / 1000;
        return currentTime - omConf.getRegisterTime() >= omConf.getLeaseTime();
    }

    public void run() {
        register();
    }

    private boolean registerPlugin() {

        String url = "http://" + omConf.getHostName() + ":" + omConf.getHttpname();

        String pluginPublicKey = Crypto.randomGeneratePublicKey();

        LinkedMultiValueMap<String, String> para = new LinkedMultiValueMap<String, String>();
        para.put("cmd", Collections.singletonList("register plugin"));
        para.put("Name", Collections.singletonList(pluginConf.getName()));
        para.put("HostName", Collections.singletonList(pluginConf.getHostName()));
        para.put("ServiceName", Collections.singletonList(pluginConf.getSvcname()));
        para.put("Role", Collections.singletonList(pluginConf.getRole()));
        para.put("Type", Collections.singletonList(pluginConf.getType()));
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
                logger.info("Event: register plugin success");
                //logger.info("%s %s %s %d %d\r\n", omSvcname, omUser, omPasswd, registerTime, leaseTime);
                return true;
            }
        } catch (Exception e) {
            logger.error("Failed to access om svc, detail: " + e);
        }
        return false;
    }
}