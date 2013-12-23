#include "dbConfForWeb.h"
#include "core.hpp"
#include "ossUtil.h"
#include <fstream>
#include <iostream>
#include <sstream>

#define CN                 "cn"
#define EN                 "en"

// optlist.xml elements for web page
#define OPTLISTTAG         "optlist"
#define LONGTAG            "long"
#define SHORTTAG           "short"
#define TYPEOFWEBTAG       "typeofweb"
#define HIDDENTAG          "hidden"
#define DETAILTAG          "detail"

#define TOPICTAG           "topic"
#define TOPIC_ATTRTAG      "topic.<xmlattr>.id"
#define TOPIC_ATTR         "administration_database_runtime"
#define STEMTRYTAG         "stentry"
#define XML_COMMENTTAG      "<xmlcomment>"
#define STHEADTAG          "sthead"
#define STROWTAG           "strow"
#define TITLETAG           "title"
#define BODYTAG            "body"
#define SIMPLETABLETAG     "simpletable"
#define SECTIONTAG         "section"
#define NOTETAG            "note"
#define PTAG               "p"

#define DBCONF             "数据库配置"
#define DBCONF_EN          "Database Runtime Configurtion"
#define PARADESC           "参数说明"
#define PARADESC_EN        "Parameter Instruction"
#define PARANAME           "参数名"
#define PARANAME_EN        "Parameter Name"
#define ACRONYM            "缩写"
#define ACRONYM_EN         "Acronym"
#define TYPE               "类型"
#define TYPE_EN            "Type"
#define DESC               "说明"
#define DESC_EN            "Description"

#define XMLDECLARATION     "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
#define XMLDTD             "<!DOCTYPE topic PUBLIC \"-//OASIS//DTD DITA Topic//EN\" \"topic.dtd\">"
#define XMLCOMMENT         "id=\"runtime_table\" frame=\"all\" relcolwidth=\"1.43* 1.0*1.18*11.41*\""

#define NOTEINFO1          "SequoiaDB支持命令行方式及配置文件方式。\
当两种方式并存时，命令行参数将会覆盖配置文件中的相同的配置项。"
#define NOTEINFO1_EN       "SequoiaDB supports setting configuration \
with command line, configuration file, or both of them. When both of \
them are used, parameters in command line will overwrite those in \
configuration file."

#define NOTEINFO2          "同步日志的总大小（logfilesz * logfilenum）\
决定了在同步过程中的容错能力。日志越大则进行全量恢复的可能性越小。"
#define NOTEINFO2_EN       "The total size of synch log (logfilesz * logfilenum)\
determines the fault tolerance in the process of sync. \
If log is bigger, the possiblity of full sync is lower."


using namespace boost::property_tree;
using std::cout;
using std::endl;
using std::size_t;
using std::setw;
using std::string;
using std::vector;
using std::ofstream;
using std::ostringstream;

OptGenForWeb::OptGenForWeb ( const char* lang ) : language( lang )
{
    loadFromXML () ;
}
OptGenForWeb::~OptGenForWeb ()
{
    vector<OptEle*>::iterator it ;
    for ( it = optlist.begin(); it != optlist.end(); it++ )
    {
        delete *it ;
    }
    optlist.clear() ;
}
void OptGenForWeb::loadFromXML ()
{
    ptree pt ;
    try
    {
        read_xml ( OPTXMLSRCFILE, pt ) ;
    }
    catch ( std::exception &e )
    {
        cout << "Can not read xml file, not exist or wrong directory forOptGenForWeb: "
             << e.what() << endl ;
        exit ( 0 ) ;
    }
    try
    {
        BOOST_FOREACH ( ptree::value_type &v, pt.get_child( OPTLISTTAG ) )
        {
            BOOLEAN ishidden = FALSE ;
            OptEle *newele = new OptEle() ;
            if ( !newele )
            {
                cout << "Failed to allocate memory for OptEle!" << endl ;
                exit ( 0 ) ;
            }
            try
            {
                ossStrToBoolean ( v.second.get<string>(HIDDENTAG).c_str(),
                                 &ishidden ) ;
            }
            catch ( std::exception & )
            {
                ishidden = FALSE ;
            }
            // we don't need the notes which hidden tag is true
            if ( ishidden )
            {
                continue ;
            }

            // long tag could not be null
            try
            {
                // long
                newele->longtag += v.second.get<string>(LONGTAG) ;
            }
            catch ( std::exception &e )
            {
                cout << "Long tag is requird: " << e.what()
                     << endl ;
                continue ;
            }
            // if no detail tag, we don't need this note
            try
            {
                // detail
                newele->detailtag = v.second.get_child(DETAILTAG
                                           ).get<string>(language) ;
            }
            catch ( std::exception &e )
            {
                continue ;
            }
            // short tag can be null
            try
            {
                // short
                //newele->shorttag += v.second.get<string>(SHORTTAG).c_str[0] ;
                newele->shorttag += v.second.get<string>(SHORTTAG) ;
            }
            catch ( std::exception &e )
            {
                newele->shorttag += "-" ;
            }
            // type tag can be null
            try
            {
                // type
                newele->typeofwebtag = v.second.get<string>(TYPEOFWEBTAG) ;
            }
            catch ( std::exception &e )
            {
                newele->typeofwebtag = "--" ;
            }

            optlist.push_back ( newele ) ;
        }
    }
    catch ( std::exception& )
    {
        cout << "XML format error, unknown node name \
or description language,please check!"<<endl ;
        exit(0) ;
    }
}
string OptGenForWeb::genOptions ()
{
    ptree root_node ;
    ptree topic ;
    ptree s_body ;
    ptree s_s_section ;
    ptree s_s_s_simpletable ;
    ptree s_s_s_note ;
    ptree s_s_s_s_sthead ;
    ptree s_s_s_s_strow ;
    vector<OptEle*>::iterator it ;
    ofstream fout ;
    ostringstream oss ;

    // s_s_s_s_sthead and s_s_s_s_strow are int the 4th sub level
    // build s_s_s_s_sthead
    if ( 0 == strcmp( EN, language ) )
    {
        s_s_s_s_sthead.add ( STEMTRYTAG, PARANAME_EN ) ;
        s_s_s_s_sthead.add ( STEMTRYTAG, ACRONYM_EN ) ;
        s_s_s_s_sthead.add ( STEMTRYTAG, TYPE_EN ) ;
        s_s_s_s_sthead.add ( STEMTRYTAG, DESC_EN ) ;
    }
    else if ( 0 == strcmp( CN, language ) )
    {
        s_s_s_s_sthead.add ( STEMTRYTAG, PARANAME ) ;
        s_s_s_s_sthead.add ( STEMTRYTAG, ACRONYM ) ;
        s_s_s_s_sthead.add ( STEMTRYTAG, TYPE ) ;
        s_s_s_s_sthead.add ( STEMTRYTAG, DESC ) ;
    }
    else
    {
        cout << "Wrong language: " << language << endl ;
        exit ( 0 ) ;
    }
    // s_s_s_note and s_s_s_simpletable are int the 3rd sub level
    // build s_s_s_simpletabl
    s_s_s_simpletable.add ( XML_COMMENTTAG, XMLCOMMENT ) ;
    s_s_s_simpletable.add_child ( STHEADTAG, s_s_s_s_sthead ) ;
    // build s_s_s_s_strow
    for ( it = optlist.begin(); it != optlist.end(); it++ )
    {
        s_s_s_s_strow.clear () ;
        s_s_s_s_strow.add ( STEMTRYTAG, (*it)->longtag ) ;
        s_s_s_s_strow.add ( STEMTRYTAG, (*it)->shorttag ) ;
        s_s_s_s_strow.add ( STEMTRYTAG, (*it)->typeofwebtag ) ;
        s_s_s_s_strow.add ( STEMTRYTAG, (*it)->detailtag ) ;
        // add s_s_s_s_strow to s_s_s_simpletable
        s_s_s_simpletable.add_child ( STROWTAG, s_s_s_s_strow ) ;
    }
    // build s_s_s_note
    if ( 0 == strcmp( EN, language ) )
    {
        s_s_s_note.add ( PTAG, NOTEINFO1_EN ) ;
        s_s_s_note.add ( PTAG, NOTEINFO2_EN ) ;
    }
    else if ( 0 == strcmp( CN, language ) )
    {
        s_s_s_note.add ( PTAG, NOTEINFO1 ) ;
        s_s_s_note.add ( PTAG, NOTEINFO2 ) ;
    }
    else
    {
        cout << "Wrong language: " << language << endl ;
        exit ( 0 ) ;
    }
    // s_s_sectionthe is in the 2nd sub level
    // build s_s_section
    if ( 0 == strcmp( EN, language ) )
    {
        s_s_section.add ( TITLETAG, PARADESC_EN ) ;
    }
    else if ( 0 == strcmp( CN, language ) )
    {
        s_s_section.add ( TITLETAG, PARADESC ) ;
    }
    else
    {
        cout << "Wrong language: " << language << endl ;
        exit ( 0 ) ;
    }
    s_s_section.add_child ( SIMPLETABLETAG, s_s_s_simpletable ) ;
    s_s_section.add_child ( NOTETAG, s_s_s_note ) ;
    // s_body is in the 1st sub level
    // build the body node
    s_body.add_child ( SECTIONTAG, s_s_section ) ;
    // the top level, build the topic node
    if ( 0 == strcmp( EN, language ) )
    {
        topic.add ( TITLETAG, DBCONF_EN ) ;
    }
    else if ( 0 == strcmp( CN, language ) )
    {
        topic.add ( TITLETAG, DBCONF ) ;
    }
    else
    {
        cout << "Wrong language: " << language << endl ;
        exit ( 0 ) ;
    }
    topic.add_child ( BODYTAG, s_body ) ;
    // add attribute, then finish building the root_nodea
    root_node.add_child ( TOPICTAG, topic ) ;
    root_node.add ( TOPIC_ATTRTAG, TOPIC_ATTR ) ;
    // write to stream
    xml_writer_settings<char> settings('\t', 1) ;
    write_xml ( oss, root_node, settings ) ;
    return oss.str () ;
}
void OptGenForWeb::gendoc()
{
    string str ;
    string subStr ;
    size_t found ;
    string fileName ;

//    string fileName = string( OPTGENFORWEBPATH ) + string( languge )
//                      + string( FILESUFFIX ) ;
    if ( 0 == strcmp( EN, language ) )
        fileName = string( DBCONFFORWEBPATH ) + string( "_en" ) + string( FILESUFFIX ) ;
    else
        fileName = string( DBCONFFORWEBPATH ) + string( FILESUFFIX ) ;

    str = genOptions() ;
    if ( "" == str.c_str() )
    {
        cout << "Failed to generate database configuration options." << endl ;
        exit ( 0 ) ;
    }

    ofstream fout( fileName.c_str() ) ;
    if ( NULL == fout )
    {
        cout << "Can not open file: " << fileName << endl;
        exit(0);
    }
    // add DTD
    found = str.find_first_of ( '>' ) ;
    if ( string::npos != found )
    {
        subStr = str.substr ( 0, found+1 ) ;
        fout << subStr.c_str() << '\n' ;
        fout << XMLDTD ;
        fout << str.substr ( found+1, string::npos ) ;
    }
    else
    {
        cout << "Can not add DTD, "<<"\"found\" is: " << found << endl ;
        exit ( 0 ) ;
    }
}
void OptGenForWeb::run ()
{
   gendoc () ;
}

