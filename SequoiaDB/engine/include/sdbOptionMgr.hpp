//#define SDB_HELP_ONLY       -1
//#define SDB_VERSION_ONLY    -2

#define SDB_POSITIONAL_OPTIONS_DESCRIPTION                        \
      destd.add ( "shell" , -1 );

#define SDB_ADD_PARAM_OPTIONS_BEGIN(desc)                         \
           desc.add_options()

#define SDB_COMMANDS_OPTIONS                                      \
      ("help,h", "help")                                          \
      ("version,v", "version")                                    \
      ("file,f", po::value< string >(), "script file")            \
      ("eval,e", po::value< string >(),  "variable(format: \"var varname=\'varvalue\'\")")              \
      ("shell,s", po::value< string >(),  "shell file")


#define SDB_ADD_PARAM_OPTIONS_END ;
