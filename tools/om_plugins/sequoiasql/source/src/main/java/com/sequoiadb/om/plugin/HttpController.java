package com.sequoiadb.om.plugin;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;

import com.sequoiadb.om.plugin.dao.DbOperations;
import com.sequoiadb.om.plugin.dao.MySQLOperations;
import com.sequoiadb.om.plugin.dao.PostgreSQLOperations;
import com.sequoiadb.om.plugin.dao.SequoiaSQLOperations;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

@RestController
public class HttpController {

    @Autowired
    private DbOperations dbo;

    @Autowired
    private PostgreSQLOperations pgsqlo;

    @Autowired
    private MySQLOperations mysqlo;

    @RequestMapping(value = "/postgresql")
    public String exec_pgsql(HttpServletRequest request, HttpServletResponse response) {
        return exec_sql(request, response, pgsqlo);
    }

    @RequestMapping(value = "/mysql")
    public String exec_mysql(HttpServletRequest request, HttpServletResponse response) {
        return exec_sql(request, response, mysqlo);
    }

    @RequestMapping(value = "/sql")
    public String exec_sql(HttpServletRequest request, HttpServletResponse response) {
        String type = request.getParameter("Type");

        if (type == "mysql") {
            return exec_sql(request, response, mysqlo);
        } else {
            return exec_sql(request, response, pgsqlo);
        }
    }

    @RequestMapping(value = "*")
    public void defaultPage(HttpServletRequest request, HttpServletResponse response) throws IOException {
        response.sendError(HttpServletResponse.SC_NOT_FOUND);
    }

    private String exec_sql(HttpServletRequest request, HttpServletResponse response, SequoiaSQLOperations ssqlo) {
        List<BSONObject> content = new ArrayList<BSONObject>();

        String ClusterName = request.getHeader("SdbClusterName");
        String BusinessName = request.getHeader("SdbBusinessName");
        String Sql = request.getParameter("Sql");
        String DbName = request.getParameter("DbName");
        if (ClusterName == null || ClusterName.trim().length() == 0) {
            return outputResult(-1, "SdbClusterName is NULL", "", content);
        }
        if (BusinessName == null || BusinessName.trim().length() == 0) {
            return outputResult(-1, "SdbBusinessName is NULL", "", content);
        }
        if (Sql == null || Sql.trim().length() == 0) {
            return outputResult(-1, "Sql is NULL", "", content);
        }

        Sql = Sql.trim();

        StringBuilder sqlHostName = new StringBuilder();
        StringBuilder sqlSvcname = new StringBuilder();

        try {
            dbo.getSsqlInfo(ClusterName, BusinessName, sqlHostName, sqlSvcname);
        } catch (BaseException e) {
            return outputResult(e.getErrorCode(), "Failed to get " + BusinessName + " service info", e.getMessage(), content);
        }

        if (sqlHostName.length() == 0 || sqlSvcname.length() == 0) {
            return outputResult(SDBError.SDB_SYS.getErrorCode(), "Failed to get " + BusinessName + " service info", SDBError.SDB_SYS.getErrorDescription(), content);
        }

        StringBuilder sqlUser = new StringBuilder();
        StringBuilder sqlPasswd = new StringBuilder();
        StringBuilder sqlDbName = new StringBuilder();

        try {
            dbo.getSsqlAccountInfo(ClusterName, BusinessName, sqlUser, sqlPasswd, sqlDbName);
        } catch (BaseException e) {
            return outputResult(e.getErrorCode(), "Failed to get " + BusinessName + " auth info", e.getMessage(), content);
        }

        if (DbName == null || DbName.trim().length() == 0) {
            if (sqlDbName.toString().length() > 0) {
                DbName = sqlDbName.toString();
            }
        }

        try {
            content = ssqlo.query(sqlHostName.toString(), sqlSvcname.toString(),
                    sqlUser.toString(), sqlPasswd.toString(), DbName, Sql);
        } catch (Exception e) {
            return outputResult(-1, e.getMessage(), "", content);
        }

        return outputResult(0, "", "Succeed", content);
    }

    private String outputResult(int rc, String detail, String description, List<BSONObject> content) {

        BSONObject result = new BasicBSONObject();
        result.put("errno", rc);
        result.put("detail", detail);
        result.put("description", description);

        content.add(0, result);

        return content.toString();
    }
}
