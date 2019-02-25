package com.sequoiadb.auth;

import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.util.SdbDecrypt;
import com.sequoiadb.util.SdbDecryptUserInfo;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import java.io.File;
import java.util.Random;

/**
 * @Description: seqDB-17880:parseCipherFile参数检验,此用例密码文件中包含多个用户
 * @author fanyu
 * @Date:2019年02月22日
 * @version:1.0
 */
public class ParseCipherFile17880 extends SdbTestBase {
    private Sequoiadb sdb;
    private String[] usernames = {/*"user@17880","user:17880",*/"  user17880", "测试17880", "token17880A", "token17880B", "token17880C"};
    private String[] passwords = {/*"user@17880","user:17880",*/"  user17880", "测试17880", "token17880A", "token17880B", "token17880C"};
    private String passwdFileName = "/password17880";
    private String passwordFilePath;

    @BeforeClass(alwaysRun = true)
    private void setUp() throws Exception {
        sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
        if (!Util.isCluster(sdb)) {
            throw new SkipException("skip StandAlone");
        }
    }

    @DataProvider(name = "range-provider")
    public Object[][] generateRangData() throws Exception {
        for (int i = 0; i < usernames.length; i++) {
            try {
                sdb.removeUser(usernames[i], passwords[i]);
            } catch (BaseException e) {
                if (e.getErrorCode() != -300) {
                    throw e;
                }
            }
            sdb.createUser(usernames[i], passwords[i]);
        }
        Util.createPasswdFile(usernames[0], passwords[0], passwdFileName);
        Util.createPasswdFile(usernames[1], passwords[1], passwdFileName);
        String token1 = getRandomString(257);
        String token2 = " ";
        String token3 = "测试";
        Util.createPasswdFile(usernames[2], passwords[2], passwdFileName, token1);
        Util.createPasswdFile(usernames[3], passwords[3], passwdFileName, token2);
        Util.createPasswdFile(usernames[4], passwords[4], passwdFileName, token3);
        String toolsPath = Util.getSdbInstallDir() + "/bin/";
        Util.downLoadFileToLocal(SdbTestBase.workDir, toolsPath + passwdFileName);
        passwordFilePath = SdbTestBase.workDir + passwdFileName;
        return new Object[][]{
                {usernames[0], passwords[0], null},
                {usernames[1], passwords[1], null},
                {usernames[2], passwords[2], token1},
                {usernames[3], passwords[3], token2},
                {usernames[4], passwords[4], token3}
        };
    }

    @DataProvider(name = "Invalidrange-provider")
    public Object[][] generateInvalidRangData() throws Exception {
        return new Object[][]{
                {null, new File(SdbTestBase.workDir)},
                {usernames[0], null},
                {usernames[0], new File(SdbTestBase.workDir + "/inexistence")},
                {usernames[0], new File(SdbTestBase.workDir)}
        };
    }

    @Test(dataProvider = "Invalidrange-provider")
    private void testInvalid(String username, File passwordFile) throws Exception {
        SdbDecrypt sdbDecrypt = new SdbDecrypt();
        try {
            sdbDecrypt.parseCipherFile(username, passwordFile);
            Assert.fail("exp fail but act success,username = " + username + ",passwordFilePath = " + passwordFilePath);
        } catch (BaseException e) {
            if (e.getErrorCode() != -6) {
                throw e;
            }
        }
    }

    @Test(dataProvider = "range-provider")
    private void testParseCipherFile(String username, String password, String token) throws Exception {
        SdbDecrypt sdbDecrypt = new SdbDecrypt();
        SdbDecryptUserInfo info = sdbDecrypt.parseCipherFile(username, token, new File(passwordFilePath));
        //check
        Assert.assertEquals(info.getUserName(), username, info.toString());
        Assert.assertEquals(info.getPasswd(), password);
        Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, info.getUserName(), info.getPasswd());
        db.disconnect();
    }

    @AfterClass(alwaysRun = true)
    private void tearDown() throws Exception {
        try {
            for (int i = 0; i < usernames.length; i++) {
                try {
                    sdb.removeUser(usernames[i], passwords[i]);
                } catch (BaseException e) {
                    if (e.getErrorCode() != -300) {
                        throw e;
                    }
                }
            }
            new File(passwordFilePath).deleteOnExit();
            Util.removePasswdFile(Util.getSdbInstallDir() + "/bin"+ passwdFileName);
        } finally {
            if (sdb != null) {
                sdb.disconnect();
            }
        }
    }

    private String getRandomString(int length) {
        String str = "adcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        Random random = new Random();
        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < length; i++) {
            int number = random.nextInt(str.length());
            sb.append(str.charAt(number));
        }
        return sb.toString();
    }
}


