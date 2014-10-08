//#define SDB_HELP_ONLY       -1
//#define SDB_VERSION_ONLY    -2

#define SDB_POSITIONAL_OPTIONS_DESCRIPTION                        \
      destd.add ( "shell" , -1 );

#define SDB_ADD_PARAM_OPTIONS_BEGIN(desc)                         \
           desc.add_options()

#define SDB_COMMANDS_OPTIONS                                      \
      ("help,h", "help")                                          \
      ("version,v", "version")                                    \
      ("file,f", po::value< string >(),                           \
       "if the -f option is present, then commands are read from "\
       "the file specified by <string>")                          \
      ("eval,e", po::value< string >(),                           \
       "predefined variables(format: "                            \
       "\"var varname=\'varvalue\'\")")                           \
      ("shell,s", po::value< string >(),                          \
       "if the -s option is present, "                            \
       "then commands are read from <string>")


#define SDB_ADD_PARAM_OPTIONS_END ;
