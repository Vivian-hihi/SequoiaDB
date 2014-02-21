#ifndef SDBCM_HPP__
#define SDBCM_HPP__

#include "pmdOptions.h"

#define SDBCM_OPTION_PREFIX         "--"
#define SDBCM_DFT_PORT              11790

// sdbcm configure file
#define SDBCM_CONF_PATH             ".." OSS_FILE_SEP "conf" OSS_FILE_SEP "sdbcm.conf"
#define SDBCM_CONF_DFTPORT          "defaultPort"
#define SDBCM_CONF_PORT             "_Port"
#define SDBCM_RESTART_COUNT         "RestartCount"
#define SDBCM_RESTART_INTERVAL      "RestartInterval"       // minute
#define SDBCM_AUTO_START            "AutoStart"

#define SDBCM_LOG_PATH              ".." OSS_FILE_SEP "conf" OSS_FILE_SEP "log" \
                                    OSS_FILE_SEP "sdbcm.log"
#define PMD_DFT_CONF_PATH           ".." OSS_FILE_SEP "conf" OSS_FILE_SEP "local"

// remote operation code
#define SDBSTART              1
#define SDBSTOP               2
#define SDBADD                3
#define SDBMODIFY             4
#define SDBRM                 5

// stop status
#define STOPFAIL              1
#define STOPPART              3



#endif /* SDBCM_HPP__ */
