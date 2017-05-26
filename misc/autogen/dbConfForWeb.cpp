#include "dbConfForWeb.h"
#include "core.hpp"
#include "ossUtil.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>

// optlist.xml elements for web page
#define OPTLISTTAG            "optlist"
#define LONGTAG               "long"
#define SHORTTAG              "short"
#define TYPEOFWEBTAG          "typeofweb"
#define HIDDENTAG             "hidden"
#define DETAILTAG             "detail"

// optOtherInfoForWeb.xml elements for web page
#define OPTOTHERINFOFORWEBTAG "optOtherInfoForWeb"
#define TITLETAG              "title"
#define SUBTITLETAG           "subtitle"
#define STHEADTAG             "sthead"
#define STENTRY_NAMETAG       "stentry_name"
#define STENTRY_ACRONYMTAG    "stentry_acronym"
#define STENTRY_TYPETAG       "stentry_type"
#define STENTRY_DESTTAG       "stentry_dest"
#define NOTETAG               "note"
#define NOTE_FIRSTTAG         "first"
#define NOTE_SECONDTAG        "second"

using namespace boost::property_tree;
using namespace std;

const CHAR *pLanguage[] = { "en", "cn" } ;

static string& replace_all ( string &str, const string& old_value, const string &new_value )
{
   for ( string::size_type pos(0) ; pos != string::npos; pos += new_value.length() )
   {
      if ( ( pos = str.find ( old_value, pos ) ) != string::npos )
         str.replace ( pos, old_value.length(), new_value ) ;
      else break ;
   }
   return str ;
}

OptGenForWeb::OptGenForWeb ( const char* lang ) : language( lang )
{
    loadFromXML () ;
    loadOtherInfoFromXML () ;
}

OptGenForWeb::~OptGenForWeb ()
{
    vector<OptEle*>::iterator it ;
    vector<OptOtherInfoEle*>::iterator ite ;
    for ( it = optlist.begin(); it != optlist.end(); it++ )
    {
        delete *it ;
    }
    optlist.clear() ;
    for ( ite = optOtherInfo.begin(); ite != optOtherInfo.end(); ite++ )
    {
        delete *ite ;
    }
    optOtherInfo.clear() ;
}

void OptGenForWeb::loadOtherInfoFromXML ()
{
    ptree pt ;
    try
    {
       read_xml ( OPTOTHERINFOFORWEBFILE, pt ) ;
    }
    catch ( std::exception &e )
    {
        cout << "Can not read xml file, not exist or wrong directory forOptGenForWeb: "
             << e.what() << endl ;
        exit ( 0 ) ;
    }

    OptOtherInfoEle *newele = new OptOtherInfoEle() ;
    if ( !newele )
    {
        cout << "Failed to allocate memory for OptOtherInfoEle!" << endl ;
        exit ( 0 ) ;
    }

    try
    {
        BOOST_FOREACH ( ptree::value_type &v, pt.get_child( OPTOTHERINFOFORWEBTAG ) )
        {
            if ( TITLETAG == v.first )
            {
                // get the title tag
                try
                {
                    newele->titletag = v.second.get<string>(language) ;
                }
                catch ( std::exception &e )
                {
                    cout << "Wrong to get the title tag: " << e.what()
                         << endl ;
                    continue ;
                }
            }
            else if ( SUBTITLETAG == v.first )
            {
                // subtile tag
                try
                {
                    newele->subtitletag = v.second.get<string>(language) ;
                }
                catch ( std::exception &e )
                {
                    cout << "Wrong to get the subtitle tag: " << e.what()
                         << endl ;
                    continue ;
                }
            }
            else if ( STHEADTAG == v.first )
            {
                // sthead tag
                try
                {
                    BOOST_FOREACH( ptree::value_type &v1, v.second )
                    {
                        if ( STENTRY_NAMETAG == v1.first )
                        {
                            newele->stentry_nametag = v1.second
                                           .get<string>(language) ;
                        }
                        else if ( STENTRY_ACRONYMTAG == v1.first )
                        {
                            newele->stentry_acronymtag = v1.second
                                           .get<string>(language) ;
                        }
                        else if ( STENTRY_TYPETAG == v1.first )
                        {
                            newele->stentry_typetag = v1.second
                                           .get<string>(language) ;
                        }
                        else if ( STENTRY_DESTTAG == v1.first )
                        {
                            newele->stentry_desttag = v1.second
                                           .get<string>(language) ;
                        }
                    }
                }
                catch ( std::exception &e )
                {
                    cout << "Wrong to get the stentry tags: " << e.what()
                         << endl ;
                    continue ;
                }
            }
            else if ( NOTETAG == v.first )
            {
                // note tag
                try
                {
                    BOOST_FOREACH( ptree::value_type &v2, v.second )
                    {
                        if ( NOTE_FIRSTTAG == v2.first )
                        {
                            newele->firsttag = v2.second
                                    .get<string>(language) ;
                        }
                        else if ( NOTE_SECONDTAG == v2.first )
                        {
                            newele->secondtag = v2.second
                                    .get<string>(language) ;
                        }
                    }
                }
                catch ( std::exception &e )
                {
                    cout << "Wrong to get the note tags: " << e.what()
                         << endl ;
                    continue ;
                }
            }
        }
    }
    catch ( std::exception& e )
    {
        cout << "XML format error: " << e.what()
             << ", unknown node name or description language,please check!"
             << endl ;
        exit(0) ;
    }
    optOtherInfo.push_back ( newele ) ;
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
        cout << "Can not read src xml file, not exist or wrong directory for OptGenForWeb: "
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
            // we don't need the notes whose hidden tag is true
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
                (void)e ;
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
                (void)e ;
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
                (void)e ;
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
    vector<OptEle*>::iterator it ;
    vector<OptOtherInfoEle*>::iterator ite ;
    ostringstream oss ;

    ite = optOtherInfo.begin() ;
    if ( optOtherInfo.end() == ite )
    {
        cout << "Nothing in 'optOtherInfo'." << endl ;
        exit( 0 ) ;
    }

    oss << "##" << (*ite)->subtitletag << "##" << endl ;
    oss << endl ;

    oss << "|" << (*ite)->stentry_nametag
        << "|" << (*ite)->stentry_acronymtag
        << "|" << (*ite)->stentry_typetag
        << "|" << (*ite)->stentry_desttag
        << "|" << endl ;
    oss << "|---|---|---|---|" << endl ;

    for ( it = optlist.begin(); it != optlist.end(); it++ )
    {
        string detail = replace_all( (*it)->detailtag, "\n", "<br/>" );
        detail = replace_all( detail, "|", "\\|" );

        oss << "|" << (*it)->longtag
            << "|" << (*it)->shorttag
            << "|" << (*it)->typeofwebtag
            << "|" << detail
            << "|" << endl ;
    }
    oss << endl ;

    if( !(*ite)->firsttag.empty() )
    {
        oss << ">**Note:**  " << endl ;
        oss << ">1. " << (*ite)->firsttag << "  " << endl ;
        if( !(*ite)->secondtag.empty() )
        {
            oss << ">2. " << (*ite)->secondtag << "  " << endl ;
        }
    }
    oss << endl ;
    return oss.str() ;
}

string OptGenForWeb::genSupplement()
{
    string str ;

    if ( 0 == strcmp( pLanguage[0], language ) )
    {
    }
    else if ( 0 == strcmp( pLanguage[1], language ) )
    {
        ifstream fin( OPT_SUPPLEMENTFILE ) ;
        str = string( istreambuf_iterator< char >( fin ),
                      istreambuf_iterator< char >() ) ;
    }
    else
    {
        cout << "The language is not support: " << language << endl ;
    }

    return str ;
}

void OptGenForWeb::gendoc()
{
    string optStr ;
    string suppleStr ;
    string fileName ;

    if ( 0 == strcmp( pLanguage[0], language ) )
    {
        fileName = string( OPT_MDPATH ) ;
    }
    else if ( 0 == strcmp( pLanguage[1], language ) )
    {
        fileName = string( OPT_MDPATH ) ;
    }
    else
    {
        cout << "The language is not support: " << language << endl ;
    }

    optStr = genOptions() ;
    if ( "" == optStr )
    {
        cout << "Failed to generate database configuration options." << endl ;
        exit ( 0 ) ;
    }

    suppleStr = genSupplement() ;
    ofstream fout( fileName.c_str() ) ;

    fout << optStr << suppleStr << endl ;
}

void OptGenForWeb::run ()
{
   gendoc () ;
}

