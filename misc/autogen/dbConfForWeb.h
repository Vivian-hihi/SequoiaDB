#ifndef DBConfForWeb_H
#define DBConfForWeb_H

#include "core.hpp"
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

// xml source file
#define OPTXMLSRCFILE "optlist.xml"
//#define OPTXMLSRCFILE "newoptlist.xml"

// output file path
#define DBCONFFORWEBPATH "../../doc/administration/database/topics/runtime_configuration"
#define FILESUFFIX ".dita"

class OptEle
{
public :
    std::string longtag ;
    std::string shorttag ;
    std::string typeofwebtag ;
    std::string detailtag ;
    BOOLEAN hiddentag ;
    OptEle()
    {
       longtag = "--" ;
       shorttag = "-" ;
       hiddentag = FALSE ;
    }
} ;
class OptGenForWeb
{
    const char* language ;
    std::vector<OptEle*> optlist ;
    void loadFromXML () ;
    std::string genOptions () ;
    void gendoc () ;

public:
    OptGenForWeb ( const char* lang ) ;
    ~OptGenForWeb () ;
    void run () ;
};

#endif
