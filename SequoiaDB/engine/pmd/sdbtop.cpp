#include "../client/client.hpp"
#include <sys/select.h>
#include <termios.h>
#include <string.h>
#include "ncurses.h"
#include <time.h>
#include <string>
#include <vector>

#include "ossUtil.hpp"
#include "ossMem.hpp"

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/typeof/typeof.hpp>



using namespace sdbclient;
using namespace bson;
using namespace std;
using namespace boost::property_tree;
namespace po = boost::program_options;

#define STDIN 0

#define SDBTOP_VERSION "sdbtop 1.0"
#define SDBTOP_DEFAULT_CONFPATH "../conf/samples/sdbtop.xml"
#define SDBTOP_DEFAULT_HOSTNAME "localhost"
#define SDBTOP_DEFAULT_SERVICENAME "50000"

#define NULLSTRING ""
#define STRING_NULL "NULL"

#define OUTPUT_FORMATTING "%.3f"

#define LENGTH_OF_RESULTBUFFER 256

//zoomMode
#define ZOOM_MODE_ALL "ZOOM_MODE_ALL"
#define ZOOM_MODE_NONE "ZOOM_MODE_NONE"

#define ZOOM_MODE_POS "ZOOM_MODE_POS"
#define ZOOM_MODE_ROW_POS "ZOOM_MODE_ROW_POS"
#define ZOOM_MODE_COL_POS "ZOOM_MODE_COL_POS"

#define ZOOM_MODE_POS_X "ZOOM_MODE_POS_X"
#define ZOOM_MODE_ROW_POS_X "ZOOM_MODE_ROW_POS_X"
#define ZOOM_MODE_COL_POS_X "ZOOM_MODE_COL_POS_X"

#define ZOOM_MODE_POS_Y "ZOOM_MODE_POS_Y"
#define ZOOM_MODE_ROW_POS_Y "ZOOM_MODE_ROW_POS_Y"
#define ZOOM_MODE_COL_POS_Y "ZOOM_MODE_COL_POS_Y"

#define ZOOM_MODE_ROW_COL "ZOOM_MODE_ROW_COL"
#define ZOOM_MODE_COL "ZOOM_MODE_COL"
#define ZOOM_MODE_ROW "ZOOM_MODE_ROW"

//occupyMode
#define OCCUPY_MODE_NONE "OCCUPY_MODE_NONE"
#define OCCUPY_MODE_WINDOW_BELOW "OCCUPY_MODE_WINDOW_BELOW"

//jumpType
#define JUMPTYPE_PANEL "JUMPTYPE_PANEL"
#define JUMPTYPE_FUNC "JUMPTYPE_FUNC"
#define JUMPTYPE_FIXED "JUMPTYPE_FIXED"
#define JUMPTYPE_GLOBAL "JUMPTYPE_GLOBAL"
#define JUMPTYPE_GROUP "JUMPTYPE_GROUP"
#define JUMPTYPE_NODE "JUMPTYPE_NODE"
#define JUMPTYPE_ASC "JUMPTYPE_ASC"
#define JUMPTYPE_DESC "JUMPTYPE_DESC"
#define JUMPTYPE_FILTER_CONDITION "JUMPTYPE_FILTER_CONDITION"
#define JUMPTYPE_NO_FILTER_CONDITION "JUMPTYPE_NO_FILTER_CONDITION"
#define JUMPTYPE_FILTER_NUMBER "JUMPTYPE_FILTER_NUMBER"
#define JUMPTYPE_NO_FILTER_NUMBER "JUMPTYPE_NO_FILTER_NUMBER"


//sortingWay
#define SORTINGWAY_ASC "1"
#define SORTINGWAY_DESC "-1"

//bodyPanelType
#define BODYTYPE_MAIN "BODYTYPE_MAIN"
#define BODYTYPE_NORMAL "BODYTYPE_NORMAL"
#define BODYTYPE_HELP_DYNAMIC "BODYTYPE_HELP_DYNAMIC"


//string globalStyle;// TABLE OR LIST
//string groupStyle;// TABLE OR LIST
//string nodeStyle;// TABLE OR LIST

#define TABLE "TABLE"
#define LIST "LIST"


//displayType
#define DISPLAYTYPE_NULL "DISPLAYTYPE_NULL"
#define DISPLAYTYPE_STATICTEXT_HELP_Header "DISPLAYTYPE_STATICTEXT_HELP_Header"
#define DISPLAYTYPE_STATICTEXT_LICENSE "DISPLAYTYPE_STATICTEXT_LICENSE"
#define DISPLAYTYPE_STATICTEXT_MAIN "DISPLAYTYPE_STATICTEXT_MAIN"
#define DISPLAYTYPE_DYNAMIC_HELP "DISPLAYTYPE_DYNAMIC_HELP"
#define DISPLAYTYPE_DYNAMIC_EXPRESSION "DISPLAYTYPE_DYNAMIC_EXPRESSION"
#define DISPLAYTYPE_DYNAMIC_SNAPSHOT "DISPLAYTYPE_DYNAMIC_SNAPSHOT"

//DISPLAYTYPE_DYNAMIC_SNAPSHOT
#define SERIALNUMBER_LENGTH 4

//expressionType
#define STATIC_EXPRESSION "STATIC_EXPRESSION"
#define DYNAMIC_EXPRESSION "DYNAMIC_EXPRESSION"

//expression
#define EXPRESSION_BODY_LABELNAME "EXPRESSION_BODY_LABELNAME"
#define EXPRESSION_VERSION "EXPRESSION_VERSION"
#define EXPRESSION_REFRESH_QUIT_HELP "EXPRESSION_REFRESH_QUIT_HELP"
#define EXPRESSION_REFRESH_TIME "EXPRESSION_REFRESH_TIME"
#define EXPRESSION_HOSTNAME "EXPRESSION_HOSTNAME"
#define EXPRESSION_SERVICENAME "EXPRESSION_SERVICENAME"
#define EXPRESSION_USRNAME "EXPRESSION_USRNAME"
#define EXPRESSION_DISPLAYMODE "EXPRESSION_DISPLAYMODE"
#define EXPRESSION_SNAPSHOTMODE "EXPRESSION_SNAPSHOTMODE"
#define EXPRESSION_FILTER_NUMBER "EXPRESSION_FILTER_NUMBER"
#define EXPRESSION_SORTINGWAY "EXPRESSION_SORTINGWAY"
#define EXPRESSION_SORTINGFIELD "EXPRESSION_SORTINGFIELD"
#define EXPRESSION_SNAPSHOTMODE_INPUTNAME "EXPRESSION_SNAPSHOTMODE_INPUTNAME"

//AutoSetType
#define UPPER_LEFT "UPPER_LEFT"
#define MIDDLE_LEFT "MIDDLE_LEFT"
#define LOWER_LEFT "LOWER_LEFT"
#define UPPER_MIDDLE "UPPER_MIDDLE"
#define MIDDLE "MIDDLE"
#define LOWER_MIDDLE "LOWER_MIDDLE"
#define UPPER_RIGHT "UPPER_RIGHT"
#define MIDDLE_RIGHT "MIDDLE_RIGHT"
#define LOWER_RIGHT "LOWER_RIGHT"

//alignment
#define LEFT "LEFT"
#define CENTER "CENTER"
#define RIGHT "RIGHT"

//sourceSnapShot
#define SDB_SNAP_NULL "SDB_SNAP_NULL"
#define SDB_SNAP_CONTEXTS_TOP "SDB_SNAP_CONTEXTS_TOP"
#define SDB_SNAP_CONTEXTS_CURRENT_TOP "SDB_SNAP_CONTEXTS_CURRENT_TOP"
#define SDB_SNAP_SESSIONS_TOP "SDB_SNAP_SESSIONS_TOP"
#define SDB_SNAP_SESSIONS_CURRENT_TOP "SDB_SNAP_SESSIONS_CURRENT_TOP"
#define SDB_SNAP_COLLECTIONS_TOP "SDB_SNAP_COLLECTIONS_TOP"
#define SDB_SNAP_COLLECTIONSPACES_TOP "SDB_SNAP_COLLECTIONSPACES_TOP"
#define SDB_SNAP_DATABASE_TOP "SDB_SNAP_DATABASE_TOP"
#define SDB_SNAP_SYSTEM_TOP "SDB_SNAP_SYSTEM_TOP"
#define SDB_SNAP_CATALOG_TOP "SDB_SNAP_CATALOG_TOP"


//displayModeChooser // DELTA or ABSOLUTE or AVERAGE
#define DELTA "DELTA"
#define ABSOLUTE "ABSOLUTE"
#define AVERAGE "AVERAGE"
const INT32 DISPLAYMODENUMBER = 3;
const string DISPLAYMODECHOOSER[DISPLAYMODENUMBER] = { ABSOLUTE, DELTA, AVERAGE };

#define ANYVALUE 0

//snapshotModeChooser // GLOBAL or GROUP or NODE 
#define GLOBAL "GLOBAL"
#define GROUP "GROUP"
#define NODE "NODE"

#define HEADER_NULL -1
#define FOOTER_NULL -1

#define SDB_OK 0
#define SDB_SDBTOP_DONE 1
#define SDB_ERROR -1

//forcedToRefresh_Local 
//forcedToRefresh_Global
#define REFRESH 0
#define NOTREFRESH 1

//foreGroundColor
//backGroundColor
//all had included in ncurses.h , don't redefine
/*#define COLOR_BLACK   0
#define COLOR_RED 1
#define COLOR_GREEN  2
#define COLOR_YELLOW 3
#define COLOR_BLUE   4
#define COLOR_MAGENTA   5
#define COLOR_CYAN   6
#define COLOR_WHITE  7*/

//if keypad( stdscr, TRUE ) ;
//#define BUTTON_LEFT 4476699
//#define BUTTON_RIGHT 4411163
//if keypad( stdscr, TRUE ) ;
#define BUTTON_LEFT 4479771
#define BUTTON_RIGHT 4414235
#define BUTTON_TAB 9
#define BUTTON_ENTER 13
#define BUTTON_ESC 27
#define BUTTON_H_LOWER 'h'
#define BUTTON_Q_LOWER 'q'
#define BUTTON_F5 542058306331

CHAR* HELP_Header = "[Help for SDBTOP]"; //DISPLAYTYPE_STATICTEXT_HELP_Header outputText
CHAR* LICENSE_Footer = 
   "Licensed Materials - Property of SequoiaDB\nCopyright SequoiaDB Corp. 2013-2014 All Rights Reserved. "; //DISPLAYTYPE_STATICTEXT_HELP_Header outputText
CHAR* Hello_Body = 
" #####  ######  ######  ####### ####### ######   For help type h or ...\n"
"#       #     # #     #    #    #     # #     #  sdbtop -h: usage\n"
"#       #     # #     #    #    #     # #     #\n"
" #####  #     # ######     #    #     # ######\n"
"      # #     # #     #    #    #     # #\n"
"      # #     # #     #    #    #     # #\n"
" #####  ######  ######     #    ####### #\n"
"\n"
"SDB Interactive Snapshot Monitor V2.0\n"
"Use these keys to navigate:\n";

const INT32 errStrLength = 1024 ;
CHAR *errStr = NULL ;
CHAR *errStrBuf = NULL ;
string confPath = SDBTOP_DEFAULT_CONFPATH ;
string hostname  = SDBTOP_DEFAULT_HOSTNAME ;
string serviceName = SDBTOP_DEFAULT_SERVICENAME ;
string usrName = NULLSTRING ;
string password = NULLSTRING ;
INT32 headerLengthFromConf = 0 ;
INT32 bodyLengthFromConf = 0 ;
INT32 footerLengthFromConf = 0 ;
INT32 keySuiteLengthFromConf = 0 ;

#define BUFFERSIZE         256
#define OPTION_HELP        "help"
#define OPTION_CONFPATH      "confpath"
#define OPTION_HOSTNAME      "hostname"
#define OPTION_SERVICENAME      "servicename"
#define OPTION_USRNAME      "usrname"
#define OPTION_PASSWORD      "password"


#define ADD_PARAM_OPTIONS_BEGIN( desc )\
        desc.add_options()

#define ADD_PARAM_OPTIONS_END ;

#define COMMANDS_STRING( a, b ) (string(a) +string( b)).c_str()
#define COMMANDS_OPTIONS \
       ( COMMANDS_STRING(OPTION_HELP, ",h"), "help" )\
       ( COMMANDS_STRING(OPTION_CONFPATH, ",c"), boost::program_options::value<string>(), "configuration file path" ) \
       ( COMMANDS_STRING(OPTION_HOSTNAME, ",i"), boost::program_options::value<string>(), "hostname" ) \
       ( COMMANDS_STRING(OPTION_SERVICENAME, ",s"), boost::program_options::value<string>(), "servicename" ) \
       ( COMMANDS_STRING(OPTION_USRNAME, ",u"), boost::program_options::value<string>(), "usrname" ) \
       ( COMMANDS_STRING(OPTION_PASSWORD, ",p"), boost::program_options::value<string>(), "password" ) 

struct Colours
{
   INT32 foreGroundColor ;
   INT32 backGroundColor ;
};

struct StaticTextOutPut
{
   CHAR *outputText ;
   string   autoSetType ;
   Colours colour ;
};

struct ExpValueStruct
{
   string text ;
   string expression ;
};

struct ExpressionContent
{
   string expressionType ;
   INT32 expressionLength ;
   ExpValueStruct expressionValue ;
   string alignment ;
   Colours colour ;
   INT32 rowLocation ;
};

struct DynamicExpressionOutPut
{
   ExpressionContent *content ;
   string autoSetType ;
   INT32 expressionNumber ;
   INT32 rowNumber ;
};

struct FiledWarningValue
{
   INT64 absoluteMaxLimitValue ;
   INT64 absoluteMinLimitValue ;
   
   INT64 deltaMaxLimitValue ;
   INT64 deltaMinLimitValue ;
   
   INT64 averageMaxLimitValue ;
   INT64 averageMinLimitValue ;
};

struct FieldStruct
{
   string deltaName ;
   string absoluteName ;
   string averageName ;
   string sourceField ;
   INT32 contentLength ;
   string alignment ;
   Colours deltaColour ;
   Colours absoluteColour ;
   Colours averageColour ;
   BOOLEAN canSwitch ;
   FiledWarningValue warningValue ;
};

struct DynamicSnapshotOutPut
{
   FieldStruct* fixedField ;
   FieldStruct* mobileField ;
   INT32 actualFixedFieldLength ;
   INT32 actualMobileFieldLength ;
   INT32 fieldLength ; // fieldLength should longer than actualFixedFieldLength + actualMobileFieldLength
   string globalAutoSetType ;
   string groupAutoSetType ;
   string nodeAutoSetType ;
   string baseField ;
   INT32 tableCellLength ;
   string globalStyle ;// TABLE OR LIST
   string groupStyle ;// TABLE OR LIST
   string nodeStyle ;// TABLE OR LIST
   INT32 globalRow ;
   INT32 globalCol ;
   INT32 groupRow ;
   INT32 groupCol ;
   INT32 nodeRow ;
   INT32 nodeCol ;
};
struct DynamicHelp
{
   INT32 tableRow ;
   INT32 tableColumn ;
   INT32 cellLength ;
   Colours prefixColour ;
   Colours contentColour ;
   string autoSetType ;
};
struct DisplayContent
{
   StaticTextOutPut staticTextOutPut ;
   DynamicExpressionOutPut dyExOutPut ;
   DynamicSnapshotOutPut dySnapshotOutPut ;
   DynamicHelp dynamicHelp ;
};

struct Position
{
   INT32 referUpperLeft_X ;
   INT32 referUpperLeft_Y ;
   INT32 length_X ;
   INT32 length_Y ;
};

struct NodeWindow
{
   INT32 actualWindowMinRow ;
   INT32 actualWindowMinColumn ;
   string zoomMode ;
   string displayType ;
   DisplayContent displayContent ;
   Position position ;
   string occupyMode;
};

struct Panel
{
   NodeWindow* subWindow ;
   INT32 numOfSubWindow ;
   INT32 numOfSubWindowFromConf ; // need it to free memory
};

struct HotKey
{
   INT64 button ;
   string jumpType ;
   string jumpName ;
};

struct KeySuite
{
   INT64 mark ;
   INT32 hotKeyLength ;
   HotKey *hotKey ;
};

struct HeadTailMap
{
   INT32 key ;
   Panel value ;
};

struct BodyMap
{
   INT32 headerKey ;
   Panel value ;
   INT32 footerKey ;
   string labelName ;
   INT64 hotKeySuiteType ;
   string sourceSnapShot ;
   string bodyPanelType ;
   string   helpPanelType ;
};

struct InputPanel
{
   INT32 displayModeChooser ; // ABSOLUTE or DELTA or AVERAGE .......pos of  DISPLAYMODECHOOSER[]
   string snapshotModeChooser; // GLOBAL or GROUP or NODE 
   string groupName ;
   string nodeName ;
   INT32 fieldPosition ;
   BOOLEAN isFirstGetSnapshot ;
   vector<BSONObj> last_Snapshot ;
   vector<BSONObj> cur_Snapshot ;
   BOOLEAN isFirstGetAbsolute ;
   BOOLEAN isFirstGetDelta ;
   BOOLEAN isFirstGetAverage ;
   map<string, FLOAT64> last_absoluteMap ;
   map<string, FLOAT64> last_deltaMap ;
   map<string, FLOAT64> last_averageMap ;
   map<string, FLOAT64> cur_absoluteMap ;
   map<string, FLOAT64> cur_deltaMap ;
   map<string, FLOAT64> cur_averageMap ;
   struct timeval startTime ;
   string confPath ;
   string hostname ;
   string serviceName ;
   string usrName ;
   string password ;
   INT32 refreshInterval ;
   BOOLEAN forcedToRefresh_Local ;
   BOOLEAN forcedToRefresh_Global ;
   BodyMap* activatedPanel ;
   Colours colourOfTheMax ;
   Colours colourOfTheMin ;
   Colours colourOfTheChange ;
   string sortingWay ;
   string sortingField ;
   string filterCondition ;
   INT32 filterNumber ;
};

struct RootWindow
{
   INT32 referWindowRow ;
   INT32 referWindowColumn ;
   INT32 actualWindowMinRow ;
   INT32 actualWindowMinColumn ;
   HeadTailMap *header ;
   INT32 headerLength ;
   BodyMap *body ;
   INT32 bodyLength ;
   HeadTailMap *footer ;
   INT32 footerLength ;
   InputPanel input ;
   KeySuite *keySuite ;
   INT32 keySuiteLength ;
};

class Event //: public SDBObject
{
public: // features
   RootWindow root ;
   sdb coord ;
public://consturct function
   Event() ;
   ~Event() ;
public: // operation
   INT32 readConfiguration( ) ;
   INT32 assignActivatedPanelByLabelName( BodyMap **activatedPanel, string labelName ) ;
   INT32 assignActivatedPanel( BodyMap **activatedPanel, string bodyPanelType ) ;
   INT32 getActivatedHeadTailMap(  BodyMap *activatedPanel, HeadTailMap **header, HeadTailMap **footer ) ;
   INT32 getActualLength( INT32 &actualLength, INT32 &referLength ) ;
   INT32 getActualPosition( Position &actualPosition, Position &referPosition,
                            const string zoomMode, const string occupyMode ) ;
   INT32 getActivatedKeySuite( KeySuite **keySuite ) ;
   INT32 getTopKey_TOP( INT64 *keyBuffer, INT32 bufLength, INT64 &key ) ;
   INT32 SDBTOP_strTOnum( const CHAR *str, INT32 &number ) ;
   INT32 SDBTOP_MEMSET( INT64 *pBuffer, INT64 c, INT32 setLength ) ;
   INT32 SDBTOP_MEMSET( CHAR *pBuffer, CHAR c, INT32 setLength ) ;
   INT32 SDBTOP_FORMATTING_OUTPUT( CHAR *pBuffer, INT32 &printfLength, const CHAR *PSrc ) ;
   INT32 MVPRINTW_TOP( string &expression, INT32 expressionLength,
                       string alignment, INT32 start_row, INT32 start_col ) ;
   void getPairNumber_ColourOfTheMax( INT32 &colourPairNumber ) ;
   void getPairNumber_ColourOfTheMin( INT32 &colourPairNumber ) ;
   void getPairNumber_ColourOfTheChange( INT32 &colourPairNumber ) ;
   INT32 getResultFromBSONObj( const BSONObj &bsonobj, const string &sourceField,
                               const string &displayMode, string &result, BOOLEAN canSwitch,
                               const string& baseField, const FiledWarningValue& waringValue,
                               INT32 &colourPairNumber ) ;
   INT32 getExpression( string& expression, string& result ) ;
   INT32 getCurSnapshot() ;
   INT32 fixedOutputLocation( INT32 start_row, INT32 start_col, INT32 &fixed_row,
                              INT32 &fixed_col, INT32 referRowLength,INT32 referColLength,
                              string displayType, string autoSetType ) ;
   INT32 getFieldStructNameAndColour( const FieldStruct &fieldStruct,
                                      const string &displayMode, string &fieldName, Colours &fieldColour ) ;
   INT32 refresh_DISPLAYTYPE_DYNAMIC_HELP( DisplayContent &displayContent,
                                                 string displayType, Position &actualPosition ) ;
   INT32 refresh_DISPLAYTYPE_DYNAMIC_EXPRESSION( DisplayContent &displayContent,
                                                 string displayType, Position &actualPosition ) ;
   INT32 refresh_DISPLAYTYPE_DYNAMIC_SNAPSHOT( DisplayContent &displayContent,
                                               string displayType, Position &actualPosition ) ;
   INT32 refreshDisplayContent( DisplayContent &displayContent,
                                string displayType, Position &actualPosition ) ;
   INT32 refreshNodeWindow( NodeWindow &window ) ;
   INT32 refreshHeadTail( HeadTailMap *headtail ) ;
   INT32 refreshBody( BodyMap *body ) ;
   void initAllColourPairs() ;
   INT32 addFixedHotKey() ;
   INT32 findSourceFieldByDisplayName( const string DisplayName ) ;
   INT32 buttonManagement( INT64 key ,BOOLEAN isFirstStart ) ;
   INT32 runSDBTOP( ) ;
};


static inline std::string &ltrim ( std::string &s )
{
   s.erase ( s.begin(), std::find_if ( s.begin(), s.end(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace)))) ;
   return s ;
}

static inline std::string &rtrim ( std::string &s )
{
   s.erase ( std::find_if ( s.rbegin(), s.rend(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace))).base(),
             s.end() ) ;
   return s ;
}

static inline std::string &trim ( std::string &s )
{
   return ltrim ( rtrim ( s ) ) ;
}

INT32 readPosition( ptree pt_position, Position& position )
{
   INT32 rc = SDB_OK ;
   try
   {
      position.referUpperLeft_X = pt_position.get<INT32>( "referUpperLeft_X" ) ;
      position.referUpperLeft_Y = pt_position.get<INT32>( "referUpperLeft_Y" ) ;
      position.length_X= pt_position.get<INT32>( "length_X" ) ;
      position.length_Y= pt_position.get<INT32>( "length_Y" ) ;
   }
   catch( std::exception &e )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s readPosition failed, e.what():%s\n", errStrBuf, e.what() ) ;
      goto error ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}


INT32 readDisplayContent( ptree pt_displayContent, DisplayContent& display, string displayType )
{
   INT32 rc = SDB_OK ;
   if( displayType == DISPLAYTYPE_STATICTEXT_HELP_Header )
   {
      try
      {
         display.staticTextOutPut.autoSetType = pt_displayContent.get<string>( "autoSetType" ) ;
         display.staticTextOutPut.colour.foreGroundColor = pt_displayContent.get<INT32>( "colour.foreGroundColor" ) ;
         display.staticTextOutPut.colour.backGroundColor = pt_displayContent.get<INT32>( "colour.backGroundColor" ) ;
         display.staticTextOutPut.outputText = HELP_Header ;
      }
      catch( std::exception &e )
      {
         snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
         snprintf( errStr, errStrLength,"%s readDisplayContent failed"
                   "(displayType == DISPLAYTYPE_STATICTEXT_HELP_Header), e.what():%s\n",
                   errStrBuf, e.what() ) ;
         goto error ;
      }
   }
   else if( displayType == DISPLAYTYPE_STATICTEXT_LICENSE )
   {
      try
      {
         display.staticTextOutPut.autoSetType = pt_displayContent.get<string>( "autoSetType" ) ;
         display.staticTextOutPut.colour.foreGroundColor = pt_displayContent.get<INT32>( "colour.foreGroundColor" ) ;
         display.staticTextOutPut.colour.backGroundColor = pt_displayContent.get<INT32>( "colour.backGroundColor" ) ;
         display.staticTextOutPut.outputText = LICENSE_Footer ;
      }
      catch( std::exception &e )
      {
         snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
         snprintf( errStr, errStrLength,"%s readDisplayContent failed"
                   "(displayType == DISPLAYTYPE_STATICTEXT_LICENSE), e.what():%s\n",
                   errStrBuf, e.what() ) ;
         goto error ;
      }
   }
   else if( displayType == DISPLAYTYPE_STATICTEXT_MAIN )
   {
      try
      {
         display.staticTextOutPut.autoSetType = pt_displayContent.get<string>( "autoSetType" ) ;
         display.staticTextOutPut.colour.foreGroundColor = pt_displayContent.get<INT32>( "colour.foreGroundColor" ) ;
         display.staticTextOutPut.colour.backGroundColor = pt_displayContent.get<INT32>( "colour.backGroundColor" ) ;
         display.staticTextOutPut.outputText = Hello_Body ;
      }
      catch( std::exception &e )
      {
         snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
         snprintf( errStr, errStrLength,"%s readDisplayContent failed"
                   "(displayType == DISPLAYTYPE_STATICTEXT_MAIN), e.what():%s\n",
                   errStrBuf, e.what() ) ;
         goto error;
      }
   }
   else if( displayType == DISPLAYTYPE_DYNAMIC_HELP )
   {
      try
      {
         display.dynamicHelp.autoSetType = pt_displayContent.get<string>( "autoSetType" ) ;
         display.dynamicHelp.prefixColour.foreGroundColor = pt_displayContent.get<INT32>( "prefixColour.foreGroundColor" ) ;
         display.dynamicHelp.prefixColour.backGroundColor = pt_displayContent.get<INT32>( "prefixColour.backGroundColor" ) ;
         display.dynamicHelp.contentColour.foreGroundColor = pt_displayContent.get<INT32>( "contentColour.foreGroundColor" ) ;
         display.dynamicHelp.contentColour.backGroundColor = pt_displayContent.get<INT32>( "contentColour.backGroundColor" ) ;
         display.dynamicHelp.tableRow = pt_displayContent.get<INT32>( "tableRow" ) ;
         display.dynamicHelp.tableColumn= pt_displayContent.get<INT32>( "tableColumn" ) ;
         display.dynamicHelp.cellLength= pt_displayContent.get<INT32>( "cellLength" ) ;
      }
      catch( std::exception &e )
      {
         snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
         snprintf( errStr, errStrLength,"%s readDisplayContent failed"
                   "(displayType == DISPLAYTYPE_DYNAMIC_HELP), e.what():%s\n",
                   errStrBuf, e.what() ) ;
         goto error ;
      }
   }
   else if( displayType == DISPLAYTYPE_DYNAMIC_EXPRESSION )
   {
      try
      {
         display.dyExOutPut.autoSetType = pt_displayContent.get<string>( "autoSetType" ) ;
         display.dyExOutPut.expressionNumber = pt_displayContent.get<INT32>( "expressionNumber" ) ;
         display.dyExOutPut.rowNumber= pt_displayContent.get<INT32>( "rowNumber" ) ;
         display.dyExOutPut.content = new ExpressionContent[display.dyExOutPut.expressionNumber] ;
         INT32 expressionNumber = 0 ;
         for( BOOST_AUTO( child_displayContent, pt_displayContent.begin() ); child_displayContent != pt_displayContent.end(); ++child_displayContent )
         {
            if( child_displayContent->first == "ExpressionContent" )
            {
               if( expressionNumber>= display.dyExOutPut.expressionNumber )
               {
                  goto error;
               }
               display.dyExOutPut.content[expressionNumber].alignment = child_displayContent->second.get<string>( "alignment" ) ;
               display.dyExOutPut.content[expressionNumber].colour.foreGroundColor = child_displayContent->second.get<INT32>( "colour.foreGroundColor" ) ;
               display.dyExOutPut.content[expressionNumber].colour.backGroundColor = child_displayContent->second.get<INT32>( "colour.backGroundColor" ) ;
               display.dyExOutPut.content[expressionNumber].rowLocation = child_displayContent->second.get<INT32>( "rowLocation" ) ;
               display.dyExOutPut.content[expressionNumber].expressionType = child_displayContent->second.get<string>( "expressionType" ) ;
               display.dyExOutPut.content[expressionNumber].expressionLength = child_displayContent->second.get<INT32>( "expressionLength" ) ;
               if( display.dyExOutPut.content[expressionNumber].expressionType == DYNAMIC_EXPRESSION )
               {
                  display.dyExOutPut.content[expressionNumber].expressionValue.expression = child_displayContent->second.get<string>( "expressionValue.expression" ) ;
               }
               else if( display.dyExOutPut.content[expressionNumber].expressionType == STATIC_EXPRESSION )
               {
                  display.dyExOutPut.content[expressionNumber].expressionValue.text = child_displayContent->second.get<string>( "expressionValue.text" ) ;
               }
               else
               {
                  snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                  snprintf( errStr, errStrLength,"%s readDisplayContent failed"
                            "(displayType == DISPLAYTYPE_DYNAMIC_EXPRESSION), expressionType == %s\n",
                            errStrBuf, display.dyExOutPut.content[expressionNumber].expressionType.c_str() ) ;
                  goto error ;
               }
               ++expressionNumber ;
            }
         }
         display.dyExOutPut.expressionNumber = expressionNumber ;
      }
      catch( std::exception &e )
      {
         snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
         snprintf( errStr, errStrLength,"%s readDisplayContent failed"
                   "(displayType == DISPLAYTYPE_DYNAMIC_EXPRESSION), e.what():%s\n",
                   errStrBuf, e.what() ) ;
         goto error ;
      }
   }
   else if( displayType == DISPLAYTYPE_DYNAMIC_SNAPSHOT )
   {
      try
      {
         display.dySnapshotOutPut.globalAutoSetType = pt_displayContent.get<string>( "globalAutoSetType" ) ;
         display.dySnapshotOutPut.groupAutoSetType = pt_displayContent.get<string>( "groupAutoSetType" ) ;
         display.dySnapshotOutPut.nodeAutoSetType = pt_displayContent.get<string>( "nodeAutoSetType" ) ;
         
         display.dySnapshotOutPut.globalStyle = pt_displayContent.get<string>( "globalStyle" ) ;
         display.dySnapshotOutPut.groupStyle = pt_displayContent.get<string>( "groupStyle" ) ;
         display.dySnapshotOutPut.nodeStyle = pt_displayContent.get<string>( "nodeStyle" ) ;
         if( TABLE == display.dySnapshotOutPut.globalStyle )
         {
            display.dySnapshotOutPut.globalRow = pt_displayContent.get<INT32>( "globalRow" ) ;
            display.dySnapshotOutPut.globalCol = pt_displayContent.get<INT32>( "globalCol" ) ;
            display.dySnapshotOutPut.tableCellLength = pt_displayContent.get<INT32>( "tableCellLength" ) ;
         }
         
         if( TABLE == display.dySnapshotOutPut.groupStyle )
         {
            display.dySnapshotOutPut.groupRow = pt_displayContent.get<INT32>( "groupRow" ) ;
            display.dySnapshotOutPut.groupCol = pt_displayContent.get<INT32>( "groupCol" ) ;
            display.dySnapshotOutPut.tableCellLength = pt_displayContent.get<INT32>( "tableCellLength" ) ;
         }

         if( TABLE == display.dySnapshotOutPut.nodeStyle )
         {
            display.dySnapshotOutPut.nodeRow = pt_displayContent.get<INT32>( "nodeRow" ) ;
            display.dySnapshotOutPut.nodeCol = pt_displayContent.get<INT32>( "nodeCol" ) ;
            display.dySnapshotOutPut.tableCellLength = pt_displayContent.get<INT32>( "tableCellLength" ) ;
         }
         
         display.dySnapshotOutPut.baseField = pt_displayContent.get<string>( "baseField" ) ;
         display.dySnapshotOutPut.fieldLength = pt_displayContent.get<INT32>( "fieldLength" ) ;
         display.dySnapshotOutPut.fixedField = new FieldStruct[display.dySnapshotOutPut.fieldLength] ;
         display.dySnapshotOutPut.mobileField = new FieldStruct[display.dySnapshotOutPut.fieldLength] ;

      }
      catch( std::exception &e )
      {
         snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
         snprintf( errStr, errStrLength,"%s readDisplayContent failed"
                   "(displayType == DISPLAYTYPE_DYNAMIC_SNAPSHOT), e.what():%s\n",
                   errStrBuf, e.what() ) ;
         goto error;
      }
      
      INT32 actualFixedFieldNum = 0;
      INT32 actualMobileFieldNum = 0;
      for( BOOST_AUTO( child_displayContent, pt_displayContent.begin() ); child_displayContent != pt_displayContent.end(); ++child_displayContent )
      {
         if( child_displayContent->first == "fixed" )
         {
            for( BOOST_AUTO( child_pFixed, child_displayContent->second.begin() ); child_pFixed != child_displayContent->second.end(); ++child_pFixed )
            {
               if( child_pFixed->first == "FieldStruct" )
               {
                  if( actualFixedFieldNum >= display.dySnapshotOutPut.fieldLength )
                  {
                     break ;
                  }
                  display.dySnapshotOutPut.fixedField[actualFixedFieldNum].absoluteName = child_pFixed->second.get<string>( "absoluteName" ) ;  
                  display.dySnapshotOutPut.fixedField[actualFixedFieldNum].sourceField = child_pFixed->second.get<string>( "sourceField" ) ;
                  display.dySnapshotOutPut.fixedField[actualFixedFieldNum].contentLength = child_pFixed->second.get<INT32>( "contentLength" ) ;
                  display.dySnapshotOutPut.fixedField[actualFixedFieldNum].alignment = child_pFixed->second.get<string>( "alignment" ) ;

                  display.dySnapshotOutPut.fixedField[actualFixedFieldNum].absoluteColour.foreGroundColor = child_pFixed->second.get<INT32>( "absoluteColour.foreGroundColor" ) ;
                  display.dySnapshotOutPut.fixedField[actualFixedFieldNum].absoluteColour.backGroundColor = child_pFixed->second.get<INT32>( "absoluteColour.backGroundColor" ) ;

                  try
                  {
                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.absoluteMaxLimitValue = child_pFixed->second.get<INT64>( "warningValue.absoluteMaxLimitValue" ) ;
                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.absoluteMinLimitValue = child_pFixed->second.get<INT64>( "warningValue.absoluteMinLimitValue" ) ;
                  }
                  catch( std::exception &e )
                  {
                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.absoluteMaxLimitValue = 0 ;
                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.absoluteMinLimitValue = 0 ;
                  }

                  display.dySnapshotOutPut.fixedField[actualFixedFieldNum].canSwitch= child_pFixed->second.get<INT32>( "canSwitch" ) ;
                  if( 1 == display.dySnapshotOutPut.fixedField[actualFixedFieldNum].canSwitch )
                  {
                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].deltaName = child_pFixed->second.get<string>( "deltaName" ) ;
                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].averageName = child_pFixed->second.get<string>( "averageName" ) ;
                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].deltaColour.foreGroundColor = child_pFixed->second.get<INT32>( "deltaColour.foreGroundColor" ) ;
                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].deltaColour.backGroundColor = child_pFixed->second.get<INT32>( "deltaColour.backGroundColor" ) ;

                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].averageColour.foreGroundColor = child_pFixed->second.get<INT32>( "averageColour.foreGroundColor" ) ;
                     display.dySnapshotOutPut.fixedField[actualFixedFieldNum].averageColour.backGroundColor = child_pFixed->second.get<INT32>( "averageColour.backGroundColor" ) ;
                     try
                     {                 
                        display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.deltaMaxLimitValue = child_pFixed->second.get<INT64>( "warningValue.deltaMaxLimitValue" ) ;
                        display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.deltaMinLimitValue = child_pFixed->second.get<INT64>( "warningValue.deltaMinLimitValue" ) ;
                           
                        display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.averageMaxLimitValue = child_pFixed->second.get<INT64>( "warningValue.averageMaxLimitValue" ) ;
                        display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.averageMinLimitValue = child_pFixed->second.get<INT64>( "warningValue.averageMinLimitValue" ) ;
                     }
                     catch( std::exception &e )
                     {
                        display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.deltaMaxLimitValue= 0 ;
                        display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.deltaMinLimitValue= 0 ;
                        
                        display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.averageMaxLimitValue= 0 ;
                        display.dySnapshotOutPut.fixedField[actualFixedFieldNum].warningValue.averageMinLimitValue= 0 ;

                     }

                  }
               }
               ++actualFixedFieldNum ;
            }
         }
         else if( child_displayContent->first == "mobile" )
         {
            for( BOOST_AUTO( child_pMobile, child_displayContent->second.begin() ); child_pMobile != child_displayContent->second.end(); ++child_pMobile )
            {
               if( child_pMobile->first == "FieldStruct" )
               {
                  if( actualMobileFieldNum >= display.dySnapshotOutPut.fieldLength )
                  {
                     break ;
                  }
                  display.dySnapshotOutPut.mobileField[actualMobileFieldNum].absoluteName = child_pMobile->second.get<string>( "absoluteName" ) ;
                     
                  display.dySnapshotOutPut.mobileField[actualMobileFieldNum].sourceField = child_pMobile->second.get<string>( "sourceField" ) ;
                  display.dySnapshotOutPut.mobileField[actualMobileFieldNum].contentLength = child_pMobile->second.get<INT32>( "contentLength" ) ;
                  display.dySnapshotOutPut.mobileField[actualMobileFieldNum].alignment = child_pMobile->second.get<string>( "alignment" ) ;

                  display.dySnapshotOutPut.mobileField[actualMobileFieldNum].absoluteColour.foreGroundColor = child_pMobile->second.get<INT32>( "absoluteColour.foreGroundColor" ) ;
                  display.dySnapshotOutPut.mobileField[actualMobileFieldNum].absoluteColour.backGroundColor = child_pMobile->second.get<INT32>( "absoluteColour.backGroundColor" ) ;
                     
                  try
                  {
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.absoluteMaxLimitValue = child_pMobile->second.get<INT64>( "warningValue.absoluteMaxLimitValue" ) ;
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.absoluteMinLimitValue = child_pMobile->second.get<INT64>( "warningValue.absoluteMinLimitValue" ) ;
                  }
                  catch( std::exception &e )
                  {
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.absoluteMaxLimitValue = 0 ;
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.absoluteMinLimitValue = 0 ;
                  }

                  display.dySnapshotOutPut.mobileField[actualMobileFieldNum].canSwitch= child_pMobile->second.get<INT32>( "canSwitch" ) ;
                  if( 1 == display.dySnapshotOutPut.mobileField[actualMobileFieldNum].canSwitch )
                  {
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].deltaName = child_pMobile->second.get<string>( "deltaName" ) ;
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].averageName = child_pMobile->second.get<string>( "averageName" ) ;
                     
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].deltaColour.foreGroundColor = child_pMobile->second.get<INT32>( "deltaColour.foreGroundColor" ) ;
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].deltaColour.backGroundColor = child_pMobile->second.get<INT32>( "deltaColour.backGroundColor" ) ;
                     
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].averageColour.foreGroundColor = child_pMobile->second.get<INT32>( "averageColour.foreGroundColor" ) ;
                     display.dySnapshotOutPut.mobileField[actualMobileFieldNum].averageColour.backGroundColor = child_pMobile->second.get<INT32>( "averageColour.backGroundColor" ) ;

                     try
                     {  
                        display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.deltaMaxLimitValue = child_pMobile->second.get<INT64>( "warningValue.deltaMaxLimitValue" ) ;
                        display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.deltaMinLimitValue = child_pMobile->second.get<INT64>( "warningValue.deltaMinLimitValue" ) ;
                           
                        display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.averageMaxLimitValue = child_pMobile->second.get<INT64>( "warningValue.averageMaxLimitValue" ) ;
                        display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.averageMinLimitValue = child_pMobile->second.get<INT64>( "warningValue.averageMinLimitValue" ) ;
                     }
                     catch( std::exception &e )
                     {            
                        display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.deltaMaxLimitValue= 0 ;
                        display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.deltaMinLimitValue= 0 ;
                        
                        display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.averageMaxLimitValue= 0 ;
                        display.dySnapshotOutPut.mobileField[actualMobileFieldNum].warningValue.averageMinLimitValue= 0 ;
                     }
                  }
               }
                  ++actualMobileFieldNum ;
            }
         }
      }
      display.dySnapshotOutPut.actualFixedFieldLength= actualFixedFieldNum ;
      display.dySnapshotOutPut.actualMobileFieldLength= actualMobileFieldNum ;
   }
   else if( displayType == DISPLAYTYPE_NULL )
   {
      
   }
   else
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,"%s readDisplayContent failed,"
                " displayType is wrong\n", errStrBuf ) ;
      goto error ;
   }
done :
   return rc ;
error :
   if( SDB_OK == rc )
   {
      rc = SDB_ERROR ;
   }
   goto done ;
}

INT32 readPanelValue( ptree pt_value, Panel& value )
{
   INT32 rc = SDB_OK ;
   try
   {
      value.numOfSubWindow = pt_value.get<INT32>( "numOfSubWindow" ) ;
      value.numOfSubWindowFromConf = pt_value.get<INT32>( "numOfSubWindow" ) ;
      value.subWindow = new NodeWindow[value.numOfSubWindow] ;
      INT32 numOfSubWindow = 0 ;
      for( BOOST_AUTO( child_value, pt_value.begin() ); child_value != pt_value.end(); ++child_value )
      {
         if( child_value->first == "NodeWindow" )
         {
            if( numOfSubWindow >= value.numOfSubWindow )
            {
               snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
               snprintf( errStr, errStrLength,"%s readPanelValue failed,"
                         " numOfSubWindow >= value.numOfSubWindow\n", errStrBuf ) ;
               goto error ;
            }
            value.subWindow[numOfSubWindow].actualWindowMinRow = child_value->second.get<INT32>( "actualWindowMinRow" ) ;
            value.subWindow[numOfSubWindow].actualWindowMinColumn = child_value->second.get<INT32>( "actualWindowMinColumn" ) ;
            value.subWindow[numOfSubWindow].zoomMode = child_value->second.get<string>( "zoomMode" ) ;
            value.subWindow[numOfSubWindow].displayType = child_value->second.get<string>( "displayType" ) ;
            try
            {
               value.subWindow[numOfSubWindow].occupyMode = child_value->second.get<string>( "occupyMode" ) ;
            }
            catch( std::exception &e )
            {
               value.subWindow[numOfSubWindow].occupyMode = OCCUPY_MODE_NONE;
            }
            for( BOOST_AUTO( child_nodewindow, child_value->second.begin() ); child_nodewindow != child_value->second.end(); ++child_nodewindow )
            {
               if( child_nodewindow->first == "displayContent" )
               {
                  rc = readDisplayContent( child_nodewindow->second, value.subWindow[numOfSubWindow].displayContent,
                                 value.subWindow[numOfSubWindow].displayType ) ;
                  if( rc)
                  {
                     snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                     snprintf( errStr, errStrLength,"%s readPanelValue failed, can't readDisplayContent\n", errStrBuf ) ;
                     goto error ;
                  }
               }
               else if( child_nodewindow->first == "position" )
               {
                  rc = readPosition( child_nodewindow->second, value.subWindow[numOfSubWindow].position ) ;
                  if( rc )
                  {
                     snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                     snprintf( errStr, errStrLength,"%s readPanelValue failed,"
                               " can't readPosition\n", errStrBuf ) ;
                     goto error ;
                  }
               }
            }
            ++numOfSubWindow ;
         }
      }
      value.numOfSubWindow = numOfSubWindow ;
   }
   catch( std::exception &e )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,"%s readPanelValue failed,"
                " e.what():%s\n", errStrBuf, e.what() ) ;
      goto error ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

Event::Event()
{
   root.input.activatedPanel = NULL ;
   root.input.displayModeChooser = 0 ;
   root.input.sortingWay = NULLSTRING;
   root.input.sortingField = NULLSTRING ;
   root.input.filterCondition = NULLSTRING ;
    root.input.filterNumber = 0 ;
   root.input.snapshotModeChooser = GLOBAL ;
   root.input.forcedToRefresh_Global = NOTREFRESH ;
   root.input.forcedToRefresh_Global = NOTREFRESH ;
   root.input.fieldPosition = 0 ;
   root.input.groupName = NULLSTRING ;
   root.input.nodeName = NULLSTRING ;
   root.input.confPath = SDBTOP_DEFAULT_CONFPATH ;
   root.input.hostname = SDBTOP_DEFAULT_HOSTNAME ;
   root.input.serviceName = SDBTOP_DEFAULT_SERVICENAME ;
   root.input.refreshInterval = 0;
   root.input.isFirstGetSnapshot = TRUE ;
   root.input.isFirstGetAbsolute = TRUE ;
   root.input.isFirstGetDelta = TRUE ;
   root.input.isFirstGetAverage = TRUE ;
   root.input.last_Snapshot.clear() ;
   root.input.cur_Snapshot.clear() ;
   root.input.last_absoluteMap.clear() ;
   root.input.last_averageMap.clear() ;
   root.input.last_deltaMap.clear() ;
   root.input.cur_absoluteMap.clear() ;
   root.input.cur_averageMap.clear() ;
   root.input.cur_deltaMap.clear() ;
   root.input.colourOfTheMax.foreGroundColor = COLOR_BLACK ;
   root.input.colourOfTheMax.backGroundColor = COLOR_WHITE ;
   root.input.colourOfTheMin.foreGroundColor = COLOR_BLACK ;
   root.input.colourOfTheMin.backGroundColor = COLOR_WHITE ;
   root.keySuiteLength = 0 ;
   root.headerLength = 0 ;
   root.bodyLength = 0 ;
   root.footerLength = 0;
}

Event::~Event()
{
   INT32 headerLength = root.headerLength ;
   INT32 bodyLength = root.bodyLength ;
   INT32 footerLength = root.footerLength ;
   INT32 keySuiteLength = root.keySuiteLength;
   INT32 numOfSubWindow = 0 ;

   while( keySuiteLength ) // free the memory root.keySuite
   {
      delete []root.keySuite[keySuiteLength -1].hotKey ;
      --keySuiteLength ;
   }
   if( root.keySuiteLength )
      delete []root.keySuite ;

   while( headerLength ) // free the memory root.header
   {
      numOfSubWindow = root.header[headerLength - 1].value.numOfSubWindow;
      while( numOfSubWindow )
      {
         if( DISPLAYTYPE_DYNAMIC_EXPRESSION == root.header[headerLength - 1].value.subWindow[numOfSubWindow - 1].displayType )
         {
            delete []root.header[headerLength - 1].value.subWindow[numOfSubWindow - 1].displayContent.dyExOutPut.content ;
         }
         else if( DISPLAYTYPE_DYNAMIC_SNAPSHOT == root.header[headerLength - 1].value.subWindow[numOfSubWindow - 1].displayType )
         {
            delete []root.header[headerLength - 1].value.subWindow[numOfSubWindow - 1].displayContent.dySnapshotOutPut.fixedField ;
            delete []root.header[headerLength - 1].value.subWindow[numOfSubWindow - 1].displayContent.dySnapshotOutPut.mobileField ;
         }
         --numOfSubWindow ;
      }
      delete []root.header[headerLength - 1].value.subWindow ;
      --headerLength ;
   }
   if( root.headerLength )
      delete []root.header;

   while( bodyLength ) // free the memory root.body
   {
      numOfSubWindow = root.body[bodyLength - 1].value.numOfSubWindow ;
      while( numOfSubWindow )
      {
         if( DISPLAYTYPE_DYNAMIC_EXPRESSION == root.body[bodyLength - 1].value.subWindow[numOfSubWindow - 1].displayType )
         {
            delete []root.body[bodyLength - 1].value.subWindow[numOfSubWindow - 1].displayContent.dyExOutPut.content ;
         }
         else if( DISPLAYTYPE_DYNAMIC_SNAPSHOT == root.body[bodyLength - 1].value.subWindow[numOfSubWindow - 1].displayType )
         {
            delete []root.body[bodyLength - 1].value.subWindow[numOfSubWindow - 1].displayContent.dySnapshotOutPut.fixedField ;
            delete []root.body[bodyLength - 1].value.subWindow[numOfSubWindow - 1].displayContent.dySnapshotOutPut.mobileField ;
         }
         --numOfSubWindow ;
      }
      delete []root.body[bodyLength - 1].value.subWindow ;
      --bodyLength ;
   }
   if( root.bodyLength )
      delete []root.body ;

   while( footerLength ) // free the memory root.footer
   {
      numOfSubWindow = root.footer[footerLength - 1].value.numOfSubWindow ;
      while( numOfSubWindow )
      {
         if( DISPLAYTYPE_DYNAMIC_EXPRESSION == root.footer[footerLength - 1].value.subWindow[numOfSubWindow - 1].displayType )
         {
            delete []root.footer[footerLength - 1].value.subWindow[numOfSubWindow - 1].displayContent.dyExOutPut.content ;
         }
         else if( DISPLAYTYPE_DYNAMIC_SNAPSHOT == root.footer[footerLength - 1].value.subWindow[numOfSubWindow - 1].displayType )
         {
            delete []root.footer[footerLength - 1].value.subWindow[numOfSubWindow - 1].displayContent.dySnapshotOutPut.fixedField ;
            delete []root.footer[footerLength - 1].value.subWindow[numOfSubWindow - 1].displayContent.dySnapshotOutPut.mobileField ;
         }
         --numOfSubWindow ;
      }
      delete []root.footer[footerLength - 1].value.subWindow ;
      --footerLength ;
   }
   if( root.footerLength )
      delete []root.footer ;
}


INT32 Event::readConfiguration( )
{
   INT32 rc = SDB_OK ;
   INT32 otherTree = 0 ;
   ptree pt_sdbtopXML ;
   INT32 keySuiteLength = 0 ;
   INT32 hotKeyLength = 0 ;
   INT32 headerLength = 0 ;
   INT32 bodyLength = 0 ;
   INT32 footerLength = 0 ;
   ptree pt_Event ;
   root.input.confPath = confPath ;
   try
   {
      read_xml( root.input.confPath, pt_sdbtopXML ) ;
   }
   catch( std::exception &e )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                " check configuration file %s is exist, e.what():%s\n",
                errStrBuf, root.input.confPath.c_str(), e.what() ) ;
      goto error ;
   }
   pt_Event = pt_sdbtopXML.get_child( "Event" ) ;
   for( BOOST_AUTO( child_event, pt_Event.begin() ); child_event != pt_Event.end(); ++child_event )
   {
      if( child_event->first == "RootWindow" )
      {
         try
         {
            root.referWindowRow = child_event->second.get<INT32>( "referWindowRow" ) ;
            root.referWindowColumn = child_event->second.get<INT32>( "referWindowColumn" ) ;
            root.actualWindowMinRow = child_event->second.get<INT32>( "actualWindowMinRow" );
            root.actualWindowMinColumn = child_event->second.get<INT32>( "actualWindowMinColumn" ) ;
            root.input.refreshInterval = child_event->second.get<INT32>( "refreshInterval" ) ;
            
            root.input.colourOfTheChange.foreGroundColor = child_event->second.get<INT32>( "colourOfTheChange.foreGroundColor" ) ;
            root.input.colourOfTheChange.backGroundColor = child_event->second.get<INT32>( "colourOfTheChange.backGroundColor" ) ;
            
            root.input.colourOfTheMax.foreGroundColor = child_event->second.get<INT32>( "colourOfTheMax.foreGroundColor" ) ;
            root.input.colourOfTheMax.backGroundColor = child_event->second.get<INT32>( "colourOfTheMax.backGroundColor" ) ;
            
            root.input.colourOfTheMin.foreGroundColor = child_event->second.get<INT32>( "colourOfTheMin.foreGroundColor" ) ;
            root.input.colourOfTheMin.backGroundColor = child_event->second.get<INT32>( "colourOfTheMin.backGroundColor" ) ;
         }
         catch( std::exception &e )
         {
            snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
            snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                      " scope: child_event->first ==..... ,e.what():%s\n",
                      errStrBuf, e.what() ) ;
            goto error ;
         }
         for( BOOST_AUTO( child_root, child_event->second.begin() ); child_root != child_event->second.end(); ++child_root )
         {
            if( child_root->first == "KeySuites" )
            {
               try
               {
                  root.keySuiteLength = child_root->second.get<INT32>( "keySuiteLength" ) ;
                  keySuiteLengthFromConf = child_root->second.get<INT32>( "keySuiteLength" ) ;
                  root.keySuite = new KeySuite[root.keySuiteLength] ;
               }
               catch( std::exception &e )
               {
                  snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                  snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                            " scope: child_root->first ==..... ,e.what():%s\n",
                            errStrBuf, e.what() ) ;
                  goto error ;
               }
               keySuiteLength = 0 ;
               for( BOOST_AUTO( child_keysuites, child_root->second.begin() ); child_keysuites != child_root->second.end(); ++child_keysuites )
               {
                  if( child_keysuites->first == "KeySuite" )
                  {
                     if( keySuiteLength >= root.keySuiteLength )
                     {
                        snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                        snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                                  " keySuiteLength >= root.keySuiteLength\n", errStrBuf ) ;
                        goto error ;
                     }
                     try
                     {
                        root.keySuite[keySuiteLength].mark = child_keysuites->second.get<INT64>( "mark" ) ;
                        root.keySuite[keySuiteLength].hotKeyLength = child_keysuites->second.get<INT32>( "hotKeyLength" ) ;
                        root.keySuite[keySuiteLength].hotKey = new HotKey[root.keySuite[keySuiteLength].hotKeyLength ] ;
                     }
                     catch( std::exception &e)
                     {
                        snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                        snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                                  " scope: child_keysuites->first ==..... ,e.what():%s\n",
                                  errStrBuf, e.what() ) ;
                        goto error ;
                     }
                     hotKeyLength = 0 ;
                     for( BOOST_AUTO( child_keysuite, child_keysuites->second.begin() ); child_keysuite != child_keysuites->second.end(); ++child_keysuite )
                     {
                        if( child_keysuite->first == "HotKey" )
                        {
                           if( hotKeyLength >= root.keySuite[keySuiteLength].hotKeyLength )
                           {
                              snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                              snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                                        " hotKeyLength >= root.keySuite[keySuiteLength].hotKeyLength\n", errStrBuf ) ;
                              goto error ;
                           }
                           try
                           {
                              root.keySuite[keySuiteLength].hotKey[hotKeyLength].button = child_keysuite->second.get<CHAR>( "button" ) ;
                              root.keySuite[keySuiteLength].hotKey[hotKeyLength].jumpType = child_keysuite->second.get<string>( "jumpType" ) ;
                              root.keySuite[keySuiteLength].hotKey[hotKeyLength].jumpName = child_keysuite->second.get<string>( "jumpName" ) ;
                           }
                           catch( std::exception &e )
                           {
                              snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                              snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                                        " scope: child_keysuite->first ==..... ,e.what():%s\n",
                                        errStrBuf, e.what() ) ;
                              goto error ;
                           }
                           ++hotKeyLength ;
                        }
                     }
                     root.keySuite[keySuiteLength].hotKeyLength = hotKeyLength ;
                     ++keySuiteLength ;
                  }
               }
               root.keySuiteLength = keySuiteLength ;
            }
            else if( child_root->first == "Headers" )
            {
               try
               {
                  root.headerLength = child_root->second.get<INT32>( "headerLength" ) ;
                  headerLengthFromConf = child_root->second.get<INT32>( "headerLength" ) ;
                  root.header = new HeadTailMap[root.headerLength] ;
               }
               catch( std::exception &e )
               {
                  snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                  snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                            " scope: child_root->first ==..... (Headers),e.what():%s\n", errStrBuf, e.what() ) ;
                  goto error ;
               }
               headerLength = 0 ;
               for( BOOST_AUTO( child_headers, child_root->second.begin() ); child_headers != child_root->second.end(); ++child_headers )
               {
                  if( child_headers->first == "HeadTailMap" )
                  {
                     if( headerLength >= root.headerLength )
                     {
                        snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                        snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                                  " scope: headerLength >= root.headerLength\n", errStrBuf ) ;
                        goto error ;
                     }
                     try
                     {
                        root.header[headerLength].key = child_headers->second.get<INT32>( "key" ) ;
                     }
                     catch( std::exception &e )
                     {
                        snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                        snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                                  " scope: child_headers->first ==..... ,e.what():%s\n",
                                  errStrBuf, e.what() ) ;
                        goto error ;
                     }
                     for( BOOST_AUTO( child_headtailmap, child_headers->second.begin() ); child_headtailmap != child_headers->second.end(); ++child_headtailmap )
                     {
                        if( child_headtailmap->first == "value" )
                        {
                           rc = readPanelValue( child_headtailmap->second, root.header[headerLength].value ) ;
                           if( rc )
                           {
                              goto error ;
                           }
                        }
                     }
                     ++headerLength ;
                  }
               }
               root.headerLength = headerLength ;
            }
            else if( child_root->first == "Bodies" )
            {
               try
               {
                  root.bodyLength = child_root->second.get<INT32>( "bodyLength" ) ;
                  bodyLengthFromConf = child_root->second.get<INT32>( "bodyLength" ) ;
                  root.body = new BodyMap[root.bodyLength] ;
               }
               catch( std::exception &e )
               {
                  snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                  snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                            " scope: child_root->first ==..... (Bodys),e.what():%s\n", errStrBuf, e.what() ) ;
                  goto error ;
               }
               bodyLength = 0 ;
               for( BOOST_AUTO( child_bodys, child_root->second.begin() ); child_bodys != child_root->second.end(); ++child_bodys )
               {
                  if( child_bodys->first == "BodyMap" )
                  {
                     if( bodyLength >= root.bodyLength )
                     {
                        snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                        snprintf( errStr, errStrLength,"%s readConfiguration failed, scope: bodyLength >= root.bodyLength\n", errStrBuf ) ;
                        goto error ;
                     }
                     try
                     {
                        root.body[bodyLength].headerKey = child_bodys->second.get<INT32>( "headerKey" ) ;
                        root.body[bodyLength].footerKey = child_bodys->second.get<INT32>( "footerKey" ) ;
                        root.body[bodyLength].labelName = child_bodys->second.get<string>( "labelName" ) ;          
                        root.body[bodyLength].bodyPanelType = child_bodys->second.get<string>( "bodyPanelType" ) ;
                        if( root.body[bodyLength].bodyPanelType == BODYTYPE_MAIN ||
                            root.body[bodyLength].bodyPanelType == BODYTYPE_NORMAL )
                        {
                           root.body[bodyLength].hotKeySuiteType = child_bodys->second.get<INT64>( "hotKeySuiteType" ) ;
                           root.body[bodyLength].helpPanelType = child_bodys->second.get<string>( "helpPanelType" ) ;
                           if( root.body[bodyLength].bodyPanelType == BODYTYPE_NORMAL )
                           {
                              root.body[bodyLength].sourceSnapShot = child_bodys->second.get<string>( "sourceSnapShot" ) ;
                           }
                           else
                           {
                              root.body[bodyLength].sourceSnapShot = SDB_SNAP_NULL ;
                           }
                        }
                     }
                     catch( std::exception &e )
                     {
                        snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                        snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                                  " scope: child_bodys->first ==..... ,e.what():%s\n", errStrBuf, e.what() ) ;
                        goto error ;
                     }
                     for( BOOST_AUTO( child_bodymap, child_bodys->second.begin() ); child_bodymap != child_bodys->second.end(); ++child_bodymap )
                     {
                        if( child_bodymap->first == "value" )
                        {
                           rc = readPanelValue( child_bodymap->second, root.body[bodyLength].value ) ;
                           if( rc )
                           {
                              goto error ;
                           }
                        }
                     }
                     ++bodyLength ;
                  }
               }
               root.bodyLength = bodyLength ;
            }
            else if( child_root->first == "Footers" )
            {
               try
               {
                  root.footerLength = child_root->second.get<INT32>( "footerLength" ) ;
                  footerLengthFromConf = child_root->second.get<INT32>( "footerLength" ) ;
                  root.footer = new HeadTailMap[root.footerLength] ;
               }
               catch( std::exception &e )
               {
                  snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                  snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                            " scope: child_root->first ==..... (Footers),e.what():%s\n",
                            errStrBuf, e.what() ) ;
                  goto error ;
               }
               footerLength = 0 ;
               for( BOOST_AUTO( child_footers, child_root->second.begin() ); child_footers != child_root->second.end(); ++child_footers )
               {
                  if( child_footers->first == "HeadTailMap" )
                  {
                     if( footerLength >= root.footerLength )
                     {
                        snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                        snprintf( errStr, errStrLength,"%s readConfiguration failed,"
                                  " scope: footerLength >= root.footerLength\n", errStrBuf ) ;
                        goto error ;
                     }
                     root.footer[footerLength].key = child_footers->second.get<INT32>( "key" ) ;
                     for( BOOST_AUTO( child_headtailmap, child_footers->second.begin() ); child_headtailmap != child_footers->second.end(); ++child_headtailmap )
                     {
                        if( child_headtailmap->first == "value" )
                        {
                           rc = readPanelValue( child_headtailmap->second, root.footer[footerLength].value ) ;
                           if( rc )
                           {
                              goto error;
                           }
                        }
                     }
                     ++footerLength;
                  }
               }
               root.footerLength = footerLength;
            }
         }
      }
      else 
      {
         ++otherTree ;
      }
   }
   
done :
   return rc ;
error :
   if( SDB_OK == rc )
   {
      rc = SDB_ERROR ;
   }
   goto done ;
}

INT32 Event::assignActivatedPanel( BodyMap **activatedPanel, string bodyPanelType )
{
   INT32 rc = SDB_OK ;
   INT32 i = 0 ;
   *activatedPanel = NULL ;
   for( i = 0; i < root.bodyLength; ++i )
   {
      if( root.body[i].bodyPanelType == bodyPanelType )
      {
         *activatedPanel = &root.body[i] ;
         break ;
      }
   }
   if( NULL == *activatedPanel )
   {
      goto error ;
   }
done :
   return rc ;
error :
   if( SDB_OK == rc )
   {
      rc = SDB_ERROR ;
   }
   goto done ;
}

INT32 Event::assignActivatedPanelByLabelName( BodyMap **activatedPanel, string labelName )
{
   INT32 rc = SDB_OK ;
   INT32 i = 0 ;
   for( i = 0; i < root.bodyLength; ++i )
   {
      if( root.body[i].labelName== labelName )
      {
         *activatedPanel = &root.body[i] ;
         break ;
      }
   }
   if( NULL == *activatedPanel )
   {
      goto error ;
   }
done :
   root.input.fieldPosition = 0 ;
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::getActivatedHeadTailMap( BodyMap *activatedPanel, HeadTailMap **header, HeadTailMap **footer )
{
   INT32 rc = SDB_OK ;
   *header = NULL ;
   *footer = NULL ;
   INT32 index_header = 0 ;
   INT32 index_footer = 0 ;
   for( index_header = 0; index_header < root.headerLength; ++index_header )
   {
      if( root.header[index_header].key == activatedPanel->headerKey )
      {
         *header = &root.header[index_header] ;
         break ;
      }
   }
   for( index_footer = 0; index_footer < root.footerLength; ++index_footer )
   {
      if( root.footer[index_footer].key == activatedPanel->footerKey)
      {
         *footer = &root.footer[index_footer] ;
         break ;
      }  
   }
   if( NULL == *header )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getActivatedHeadTailMap faild, SDB_HEADER_NULL\n",
                errStrBuf ) ;
      goto error ;
   }
   if( NULL == *footer )
   {
      if( FOOTER_NULL != activatedPanel->footerKey )
      {
         if( NULL == *header )
         {
            snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
            snprintf( errStr, errStrLength, "%s getActivatedHeadTailMap faild,"
                      " SDB_HEADER_FOOTER_NULL\n", errStrBuf ) ;
            goto error ;
         }
         else
         {
            snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
            snprintf( errStr, errStrLength, "%s getActivatedHeadTailMap faild,"
                      " SDB_FOOTER_NULL\n", errStrBuf ) ;
            goto error ;
         }
      }
   }
done :
   return rc ;
error :
   rc = SDB_ERROR;
   goto done ;
}

INT32 Event::getActualLength( INT32 &actualLength, INT32 &referLength )
{
   INT32 rc = SDB_OK ;
   INT32 row =0 ;
   INT32 col = 0 ;
   FLOAT32 SCALE_COLUMN = 0.0 ;
   getmaxyx( stdscr, row, col ) ;
   SCALE_COLUMN = (FLOAT32)col / (FLOAT32)root.referWindowColumn ;
   actualLength = (INT32)( referLength * SCALE_COLUMN ) ;
done :
   return rc ;
}

INT32 Event::getActualPosition( Position &actualPosition, Position &referPosition,
                                const string zoomMode, const string occupyMode )
{
   INT32 rc = SDB_OK ;
   INT32 row =0 ;
   INT32 col = 0 ;
   FLOAT32 SCALE_ROW = 0.0 ;
   getmaxyx( stdscr, row, col ) ;
   if( row < root.actualWindowMinRow || col < root.actualWindowMinColumn )
   {
      rc = SDB_ERROR ;
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength, "%s Minimum window size: %dx%d, found %dx%d\n", 
                errStrBuf, root.actualWindowMinRow, root.actualWindowMinColumn, row, col ) ;
      goto error ;
   }
   SCALE_ROW = (FLOAT32)row / (FLOAT32)root.referWindowRow ;
   if( zoomMode == ZOOM_MODE_ALL )
   {
      actualPosition.length_X = ( INT32 )( referPosition.length_X * (FLOAT32)col /
                                ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.length_Y = ( INT32 )( referPosition.length_Y * (FLOAT32)row /
                                ( FLOAT32 )root.referWindowRow ) ;
      actualPosition.referUpperLeft_X = ( INT32 )( referPosition.referUpperLeft_X * ( FLOAT32 )col /
                                        ( FLOAT32 )root.referWindowColumn) ;
      actualPosition.referUpperLeft_Y = ( INT32 )( referPosition.referUpperLeft_Y * ( FLOAT32 )row /
                                        ( FLOAT32)root.referWindowRow ) ;
   }
   else if( zoomMode == ZOOM_MODE_NONE )
   {
      actualPosition.length_X = referPosition.length_X ;
      actualPosition.length_Y = referPosition.length_Y ;
      actualPosition.referUpperLeft_X = referPosition.referUpperLeft_X ;
      actualPosition.referUpperLeft_Y = referPosition.referUpperLeft_Y ;
   }
   else if( zoomMode == ZOOM_MODE_POS )
   {
      actualPosition.length_X = referPosition.length_X ;
      actualPosition.length_Y = referPosition.length_Y ;
      actualPosition.referUpperLeft_X = ( INT32 )( referPosition.referUpperLeft_X * ( FLOAT32 )col /
                                       ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.referUpperLeft_Y = ( INT32 )( referPosition.referUpperLeft_Y * ( FLOAT32 )row /
                                        ( FLOAT32 )root.referWindowRow ) ;
   }
   else if( zoomMode == ZOOM_MODE_ROW_POS )
   {
      actualPosition.length_X = referPosition.length_X ;
      actualPosition.length_Y = ( INT32 )( referPosition.length_Y * SCALE_ROW ) ;
      actualPosition.referUpperLeft_X = ( INT32 )( referPosition.referUpperLeft_X * ( FLOAT32 )col /
                                        ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.referUpperLeft_Y = ( INT32 )( referPosition.referUpperLeft_Y * ( FLOAT32 )row /
                                        ( FLOAT32)root.referWindowRow ) ;
   }
   else if( zoomMode == ZOOM_MODE_COL_POS )
   {
      actualPosition.length_X = ( INT32 )( referPosition.length_X * ( FLOAT32 )col /
                                ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.length_Y = referPosition.length_Y ;
      actualPosition.referUpperLeft_X= ( INT32 )( referPosition.referUpperLeft_X * ( FLOAT32 )col /
                                       ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.referUpperLeft_Y= ( INT32 )( referPosition.referUpperLeft_Y * ( FLOAT32 )row /
                                       ( FLOAT32 )root.referWindowRow ) ;
   }
   else if( zoomMode == ZOOM_MODE_POS_X )
   {
      actualPosition.length_X = referPosition.length_X ;
      actualPosition.length_Y = referPosition.length_Y ;
      actualPosition.referUpperLeft_X= ( INT32 )( referPosition.referUpperLeft_X * ( FLOAT32 )col /
                                       (FLOAT32)root.referWindowColumn ) ;
      actualPosition.referUpperLeft_Y= referPosition.referUpperLeft_Y ;
   }
   else if( zoomMode == ZOOM_MODE_ROW_POS_X )
   {
      actualPosition.length_X = referPosition.length_X ;
      actualPosition.length_Y = ( INT32 )( referPosition.length_Y * SCALE_ROW ) ;
      actualPosition.referUpperLeft_X= ( INT32 )( referPosition.referUpperLeft_X * ( FLOAT32 )col /
                                       ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.referUpperLeft_Y= referPosition.referUpperLeft_Y ;
   }
   else if( zoomMode == ZOOM_MODE_COL_POS_X )
   {
      actualPosition.length_X = ( INT32 )( referPosition.length_X * ( FLOAT32 )col /
                                ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.length_Y = referPosition.length_Y ;
      actualPosition.referUpperLeft_X = ( INT32 )( referPosition.referUpperLeft_X * ( FLOAT32 )col /
                                       ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.referUpperLeft_Y = referPosition.referUpperLeft_Y ;
   }
   else if( zoomMode == ZOOM_MODE_POS_Y )
   {
      actualPosition.length_X = referPosition.length_X ;
      actualPosition.length_Y = referPosition.length_Y ;
      actualPosition.referUpperLeft_X= referPosition.referUpperLeft_X ;
      actualPosition.referUpperLeft_Y= ( INT32 )( referPosition.referUpperLeft_Y * ( FLOAT32 )row /
                                       ( FLOAT32 )root.referWindowRow ) ;
   }
   else if( zoomMode == ZOOM_MODE_ROW_POS_Y )
   {
      actualPosition.length_X = referPosition.length_X ;
      actualPosition.length_Y = ( INT32 )( referPosition.length_Y * SCALE_ROW ) ;
      actualPosition.referUpperLeft_X= referPosition.referUpperLeft_X ;
      actualPosition.referUpperLeft_Y= ( INT32 )( referPosition.referUpperLeft_Y * ( FLOAT32 )row /
                                       ( FLOAT32 )root.referWindowRow ) ;
   }
   else if( zoomMode == ZOOM_MODE_COL_POS_Y )
   {
      actualPosition.length_X = ( INT32 )( referPosition.length_X * (FLOAT32)col /
                                ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.length_Y = referPosition.length_Y ;
      actualPosition.referUpperLeft_X = referPosition.referUpperLeft_X ;
      actualPosition.referUpperLeft_Y = ( INT32 )( referPosition.referUpperLeft_Y * ( FLOAT32 )row /
                                        ( FLOAT32 )root.referWindowRow ) ;
   }
   else if( zoomMode == ZOOM_MODE_ROW_COL )
   {
      actualPosition.length_X = ( INT32 )( referPosition.length_X * ( FLOAT32 )col /
                                ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.length_Y = ( INT32 )( referPosition.length_Y * ( FLOAT32 )row /
                                ( FLOAT32 )root.referWindowRow ) ;
      actualPosition.referUpperLeft_X = referPosition.referUpperLeft_X ;
      actualPosition.referUpperLeft_Y = referPosition.referUpperLeft_Y ;
   }
   else if( zoomMode == ZOOM_MODE_COL )
   {
      actualPosition.length_X = ( INT32 )( referPosition.length_X * ( FLOAT32 )col / ( FLOAT32 )root.referWindowColumn ) ;
      actualPosition.length_Y = referPosition.length_Y ;
      actualPosition.referUpperLeft_X = referPosition.referUpperLeft_X ;
      actualPosition.referUpperLeft_Y = referPosition.referUpperLeft_Y ;
   }
   else if( zoomMode == ZOOM_MODE_ROW )
   {
      actualPosition.length_X = referPosition.length_X ;
      actualPosition.length_Y = ( INT32 )( referPosition.length_Y * ( FLOAT32 )row /
                                ( FLOAT32 )root.referWindowRow ) ;
      actualPosition.referUpperLeft_X = referPosition.referUpperLeft_X ;
      actualPosition.referUpperLeft_Y = referPosition.referUpperLeft_Y ;
   }
   else 
   {
      rc = SDB_ERROR ;
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getActualPosition faild: wrong zoomMode:%s\n",
                errStrBuf, zoomMode.c_str() ) ;
      goto error ;
   }
   if( occupyMode != OCCUPY_MODE_NONE )
   {
      if( occupyMode == OCCUPY_MODE_WINDOW_BELOW )
      {
         actualPosition.length_Y = row - actualPosition.referUpperLeft_Y ;
      }
      else 
      {
         rc = SDB_ERROR ;
         snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
         snprintf( errStr, errStrLength,
                   "%s getActualPosition faild: wrong occupyMode:%s\n",
                   errStrBuf, occupyMode.c_str() ) ;
         goto error ;
      }
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::getActivatedKeySuite( KeySuite **keySuite )
{
   INT32 rc = SDB_OK ;
   INT32 i = 0 ;
   try
   {
      for( i = 0; i < root.keySuiteLength; ++i )
      {
         if( root.keySuite[i].mark == root.input.activatedPanel->hotKeySuiteType )
         {
            break ;
         }
      }
      if( i != root.keySuiteLength )
      {
         *keySuite = root.keySuite ;
      }
      else
      {
         *keySuite = NULL ;
      }
   }
   catch( std::exception &e )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getActivatedKeySuite failed, e.what():%s\n",
                errStrBuf, e.what() ) ;
      goto error ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::getTopKey_TOP( INT64 *keyBuffer, INT32 bufLength, INT64 &key )
{
   INT32 rc = SDB_OK ;
   INT32 i = 0 ;
   key = 0 ;
   try
   {
      for( i = 0; i < bufLength; ++i )
      {
         if( 0 == keyBuffer[i] )
         {
            break ;
         }
      }
      if( i != bufLength )
      {
         key = keyBuffer[i-1] ;
         keyBuffer[i-1] = 0 ;
      }
      else
      {
         key = 0 ;
      }
   }
   catch( std::exception &e )
   {
      snprintf( errStrBuf, errStrLength, "%s", errStr ) ;
      snprintf( errStr, errStrLength, 
                "%s getTopKey_TOP failed, e.what():%s\n",
                errStrBuf, e.what() ) ;
      goto error ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::SDBTOP_strTOnum( const CHAR *str, INT32 &number )
{
   INT32 rc = SDB_OK ;
   INT32 pos = 0 ;
   number = 0 ;
   while( str[pos] )
   {
      if( '0' > str[pos] ||'9' < str[pos] )
      {
         number = 0;
         goto error;
      }
      number *= 10 ;
      number = number + str[pos] -'0' ;
      ++pos;
   }
done:
   return rc;
error :
   rc = SDB_ERROR ;
   goto done;
}

INT32 Event::SDBTOP_MEMSET( INT64 *pBuffer, INT64 c, INT32 setLength )
{
   INT32 rc = SDB_OK ;
   INT32 i = 0 ;
   try
   {
      for( i = 0; i < setLength; ++i )
      {
         pBuffer[i] = c ;
      }
   }
   catch( std::exception &e )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getTopKey_TOP failed, e.what():%s\n",
                errStrBuf, e.what() ) ;
      goto error ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}
INT32 Event::SDBTOP_MEMSET( CHAR *pBuffer, CHAR c, INT32 setLength )
{
   INT32 rc = SDB_OK ;
   INT32 i = 0 ;
   try
   {
      for( i = 0; i < setLength; ++i )
      {
         pBuffer[i] = c ;
      }
   }
   catch( std::exception &e )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getTopKey_TOP failed, e.what():%s\n",
                errStrBuf, e.what() ) ;
      goto error ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}
INT32 Event::SDBTOP_FORMATTING_OUTPUT( CHAR *pBuffer, INT32 &printfLength, const CHAR *pSrc )
{
   INT32 rc = SDB_OK ;
   INT32 i = 0 ;
   try
   {
      for( i = 0; i < printfLength && pSrc[i] != '\0'; ++i )
      {
         pBuffer[i] = pSrc[i] ;
      }
      pBuffer[i] = '\0' ;
      printfLength = i ;
   }
   catch( std::exception &e )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s SNPRINTF_TOP failed, e.what():%s\n",
                errStrBuf, e.what() ) ;
      goto error ;
   }
done :
   return rc ;
error :
   if( SDB_OK == rc )
   {
      rc = SDB_ERROR ;
   }
   goto done ;
}
INT32 Event::MVPRINTW_TOP( string &expression, INT32 expressionLength,
                           string alignment, INT32 start_row, INT32 start_col )
{
   INT32 rc = SDB_OK ;
   INT32 col = 0 ;
   INT32 printfLength = expressionLength ;
   CHAR* printf_str = NULL ;
   printf_str = ( CHAR * )malloc( printfLength * sizeof( CHAR ) ) ;
   if( !printf_str )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                   "%s MVPRINTW_TOP faild, can't malloc memory for printf_str :%d\n", errStrBuf, expressionLength ) ;
      goto error ;
   }
   if( expressionLength <= 0 )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                   "%s MVPRINTW_TOP faild, wrong expressionLength:%d\n", errStrBuf, expressionLength ) ;
      goto error ;
   }
   rc = SDBTOP_FORMATTING_OUTPUT( printf_str, printfLength, expression.c_str() ) ;
   if( rc )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                   "%s MVPRINTW_TOP faild, SNPRINTF_TOP faild\n", errStrBuf ) ;
      goto error ; 
   }
   if( LEFT == alignment )
   {
      mvprintw( start_row, start_col, printf_str ) ;
   }
   else if( RIGHT == alignment )
   {
      col = expressionLength - printfLength ;
      start_col += col ;
      mvprintw( start_row, start_col, printf_str ) ;
   }
   else if( CENTER == alignment )
   {
      col = ( expressionLength - printfLength ) / 2 ;
      start_col += col ;
      mvprintw( start_row, start_col, printf_str ) ;
   }
   else
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                   "%s MVPRINTW_TOP wrong alignment:%s\n", errStrBuf, alignment.c_str() ) ;
      goto error ; 
   }
done :
   free( printf_str ) ;
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

void Event::getPairNumber_ColourOfTheMax( INT32& colourPairNumber )
{
   colourPairNumber = root.input.colourOfTheMax.foreGroundColor +
                                     root.input.colourOfTheMax.backGroundColor * 8 ;

}

void Event::getPairNumber_ColourOfTheMin( INT32& colourPairNumber )
{
   colourPairNumber = root.input.colourOfTheMin.foreGroundColor +
                                     root.input.colourOfTheMin.backGroundColor * 8 ;

}

void Event::getPairNumber_ColourOfTheChange( INT32& colourPairNumber )
{
   colourPairNumber = root.input.colourOfTheChange.foreGroundColor +
                                     root.input.colourOfTheChange.backGroundColor * 8 ;

}


INT32 Event::getResultFromBSONObj( const BSONObj &bsonobj, const string &sourceField,
                                   const string &displayMode, string &result, BOOLEAN canSwitch,
                                   const string &baseField, const FiledWarningValue &waringValue, INT32 &colourPairNumber )
{
   INT32 rc = SDB_OK ;
   INT32 pos_last = 0 ;
   FLOAT64 delta = 0.0 ; // when displayMode is DELTA , use it as resulte buffer
   FLOAT64 elementNumber = 0.0 ; // when displayMode is ABSOLUTE , use it as resulte buffer
   FLOAT64 average = 0.0 ; // when displayMode is AVERAGE , use it as resulte buffer
   BSONElement element ; // get it with sourceField from cur bsonobj
   BSONElement last_element ; // get it with sourceField from last bsonobj
   BSONElement baseElement  ; // get it with baseField from cur bsonobj
   BSONElement baseElement_last ; // compare to baseElement
   string new_ = NULLSTRING ;
   string old_ = NULLSTRING ; // compare to new_, check out the same BSONobj
   CHAR *resultBuf = ( CHAR * )malloc( LENGTH_OF_RESULTBUFFER * sizeof( CHAR ) ) ;
   if( !resultBuf )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getResultFromBSONobj failed, can't malloc memory for resultBuf : %d\n",
                errStrBuf, LENGTH_OF_RESULTBUFFER ) ;
      goto error ;
   }
   try
   {
      element = bsonobj.getField( sourceField ) ;
      
      baseElement = bsonobj.getField( baseField ) ;
      baseElement_last ;
      new_ = baseElement.toString( FALSE ) ;
      if( !canSwitch ||NULLSTRING == baseField )
      {
         result = element.toString( FALSE ) ;
         if( element.isNumber() )
         {
            elementNumber = element.Number() ;
            if( waringValue.absoluteMaxLimitValue != 0 &&
                elementNumber > waringValue.absoluteMaxLimitValue )
            {
               getPairNumber_ColourOfTheMax( colourPairNumber ) ;
            }
            else if( waringValue.absoluteMinLimitValue != 0 &&
                     elementNumber < waringValue.absoluteMinLimitValue )
            {
               getPairNumber_ColourOfTheMin( colourPairNumber ) ;
            }
            root.input.cur_absoluteMap[new_+sourceField] = elementNumber ;
         }
         goto done ;
      }
      while( pos_last < root.input.last_Snapshot.size() ) //find the match bsonj
      {
         baseElement_last = root.input.last_Snapshot[pos_last].getField( baseField ) ;
         old_ = baseElement_last.toString( FALSE ) ;
         if( new_ == old_ )
            break ;
         ++pos_last ;
      }
      if( pos_last == root.input.last_Snapshot.size() )
      {
         if( DELTA == displayMode || AVERAGE== displayMode )
         {
            snprintf( resultBuf, LENGTH_OF_RESULTBUFFER, OUTPUT_FORMATTING, 0.0 ) ;
            result = resultBuf ;
            root.input.cur_deltaMap[new_+sourceField] = 0.0 ;
            root.input.cur_averageMap[new_+sourceField] = 0.0 ;
         }
         else if( ABSOLUTE == displayMode )
         {
            if( element.isNumber() )
            {
               elementNumber = element.Number() ;
               snprintf( resultBuf, LENGTH_OF_RESULTBUFFER, OUTPUT_FORMATTING, elementNumber ) ;
               result = resultBuf ;
               if( waringValue.absoluteMaxLimitValue != 0 &&
                   elementNumber > waringValue.absoluteMaxLimitValue )
               {
                  getPairNumber_ColourOfTheMax( colourPairNumber ) ;
               }
               else if( waringValue.absoluteMinLimitValue != 0 && elementNumber < waringValue.absoluteMinLimitValue )
               {
                  getPairNumber_ColourOfTheMin( colourPairNumber ) ;
               }
               root.input.cur_absoluteMap[new_+sourceField] = elementNumber ;
            }
            else
            {
               result = element.toString( FALSE ) ;
            }
         }
         else
         {
            snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
            snprintf( errStr, errStrLength,
                      "%s getResultFromBSONobj failed, displayMode = %s\n", 
                      errStrBuf, displayMode.c_str() ) ;
            goto error ;
         }
      }
      else
      {
         last_element = root.input.last_Snapshot[pos_last].getField( sourceField ) ;
         if( DELTA == displayMode )
         {
            if( element.isNumber() )
            {
               delta = element.Number() - last_element.Number() ;
               snprintf( resultBuf, LENGTH_OF_RESULTBUFFER, OUTPUT_FORMATTING, delta ) ;
               result = resultBuf ;
               if( waringValue.deltaMaxLimitValue != 0 &&
                   delta > waringValue.deltaMaxLimitValue )
               {
                  getPairNumber_ColourOfTheMax( colourPairNumber ) ;
               }
               else if( waringValue.deltaMinLimitValue!= 0 &&
                        delta < waringValue.deltaMinLimitValue )
               {
                  getPairNumber_ColourOfTheMin( colourPairNumber ) ;
               }
               else if( delta != root.input.last_deltaMap[new_+sourceField] )
               {
                  getPairNumber_ColourOfTheChange( colourPairNumber ) ;
               }
               root.input.cur_deltaMap[new_+sourceField] = delta ;
            }
            else
            {
               root.input.cur_deltaMap[new_+sourceField] = 0.0 ;
               result = NULLSTRING ;
            }
            goto done ;
         }
         else if( ABSOLUTE== displayMode )
         {
            if( element.isNumber() )
            {
               elementNumber = element.Number();
               snprintf( resultBuf, LENGTH_OF_RESULTBUFFER, OUTPUT_FORMATTING, elementNumber );
               result = resultBuf;
               if( waringValue.absoluteMaxLimitValue != 0 &&
                   elementNumber > waringValue.absoluteMaxLimitValue )
               {
                  getPairNumber_ColourOfTheMax( colourPairNumber ) ;
               }
               else if( waringValue.absoluteMinLimitValue != 0 &&
                        elementNumber < waringValue.absoluteMinLimitValue )
               {
                  getPairNumber_ColourOfTheMin( colourPairNumber ) ;
               }
               else if( elementNumber != root.input.last_absoluteMap[new_+sourceField] )
               {
                  getPairNumber_ColourOfTheChange( colourPairNumber ) ;
               }
               root.input.cur_absoluteMap[new_+sourceField] = elementNumber ;
            }
            else
            {
               result = element.toString( FALSE );
            }
            goto done ;
         }
         else if( AVERAGE== displayMode )
         {
            if( element.isNumber() )
            {
               average = ( element.Number() - last_element.Number() ) /
                                 root.input.refreshInterval ;
               snprintf( resultBuf, LENGTH_OF_RESULTBUFFER, OUTPUT_FORMATTING, average ) ;
               result = resultBuf ;
               if( waringValue.averageMaxLimitValue!= 0 &&
                   average > waringValue.averageMaxLimitValue )
               {
                  getPairNumber_ColourOfTheMax( colourPairNumber ) ;
               }
               else if( waringValue.averageMinLimitValue!= 0 &&
                        average < waringValue.averageMinLimitValue )
               {
                  getPairNumber_ColourOfTheMin( colourPairNumber ) ;
               }
               else if( average != root.input.last_averageMap[new_+sourceField] )
               {
                  getPairNumber_ColourOfTheChange( colourPairNumber ) ;
               }
               root.input.cur_averageMap[new_+sourceField] = average ;
            }
            else
            {
               root.input.cur_deltaMap[new_+sourceField] = 0.0 ;
               result = NULLSTRING ;
            }
            goto done ;
         }
         else
         {
            snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
            snprintf( errStr, errStrLength,
                         "%s getResultFromBSONobj failed, displayMode = %s\n",
                         errStrBuf, displayMode.c_str() ) ;
            goto error ;
         }
         result = element.toString( FALSE ) ;
      }
   }
   catch( std::exception &e )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                   "%s getResultFromBSONobj failed, e.what():%s ,sourceField = %s\n", 
                   errStrBuf, e.what(), sourceField.c_str() ) ;
      goto error ;
   }
   
done :
   free( resultBuf ) ;
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::getExpression( string& expression, string& result )
{
   INT32 rc = SDB_OK ;
   CHAR *buf = ( CHAR * )malloc( 9 * sizeof( CHAR ) ) ;
   if( !buf )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                   "%s getExpression faild, can't malloc memory for buf :%d\n", errStrBuf, 9 ) ;
      goto error ;
   }
   if( EXPRESSION_BODY_LABELNAME == expression )
   {
      result = root.input.activatedPanel->labelName ;
   }
   else if( EXPRESSION_REFRESH_QUIT_HELP == expression )
   {
      result = "Refresh: F5, Quit: q, Help: h" ;
   }
   else if( EXPRESSION_VERSION == expression )
   {
      result = SDBTOP_VERSION ;
   }
   else if( EXPRESSION_REFRESH_TIME == expression )
   {
      snprintf( buf, 9,"%d", root.input.refreshInterval ) ;
      result = buf ;
   }
   else if( EXPRESSION_HOSTNAME == expression )
   {
      result = root.input.hostname ;
   }
   else if( EXPRESSION_SERVICENAME == expression )
   {
      result = root.input.serviceName ;
   }
   else if( EXPRESSION_USRNAME == expression )
   {
      if( NULLSTRING == root.input.usrName )
      {
         result = STRING_NULL ;
      }
      else
      {
         result = root.input.usrName ;
      }
   }
   else if( EXPRESSION_FILTER_NUMBER == expression )
   {
      snprintf( buf, 9,"%d", root.input.filterNumber ) ;
      result = buf ;
   }
   else if( EXPRESSION_SORTINGWAY == expression )
   {
      result = root.input.sortingWay ;
   }
   else if( EXPRESSION_SORTINGFIELD == expression )
   {
      result = root.input.sortingField ;
   }
   else if( EXPRESSION_SNAPSHOTMODE_INPUTNAME == expression )
   {
      if( GLOBAL == root.input.snapshotModeChooser )
         result = STRING_NULL ;
      else if( GROUP == root.input.snapshotModeChooser )
         result = root.input.groupName ;
      else if( NODE == root.input.snapshotModeChooser )
         result = root.input.nodeName ;
      else
         result = STRING_NULL ;
   }
   else if( EXPRESSION_DISPLAYMODE == expression )
   {
      result = DISPLAYMODECHOOSER[root.input.displayModeChooser] ;
   }
   else if( EXPRESSION_SNAPSHOTMODE == expression )
   {
      result = root.input.snapshotModeChooser ;
   }
   else
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getExpression failed, wrong expression:%s\n",
                errStrBuf, expression.c_str() ) ;
      goto error ; 
   }
done :
   free( buf ) ;
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::getCurSnapshot()
{
   INT32 rc = SDB_OK ;
   INT32 snapType = -1 ;
   INT32 filterNum = 0 ;
   sdbCursor cursor ;
   BSONObj bsonobj ;
   BSONObj conditionObj ;
   BSONObj selectorObj ;
   BSONObj orderByObj ;
   string condition = "{}" ;
   string selector = "{}" ;
   string orderBy = "{" ;
   string conditionPre = NULLSTRING ;
   string conditionCon = NULLSTRING ;
   string HostName = NULLSTRING ;
   string svcname = NULLSTRING ;
   string::size_type pos ;
   fromjson( selector, selectorObj ) ;
   if( root.input.activatedPanel[0].sourceSnapShot == SDB_SNAP_CONTEXTS_TOP )
   {
      snapType = SDB_SNAP_CONTEXTS ;
   }
   else if( root.input.activatedPanel[0].sourceSnapShot == SDB_SNAP_CONTEXTS_CURRENT_TOP )
   {
      snapType = SDB_SNAP_CONTEXTS_CURRENT ;
   }
   else if( root.input.activatedPanel[0].sourceSnapShot == SDB_SNAP_SESSIONS_TOP )
   {
      snapType = SDB_SNAP_SESSIONS ;
   }
   else if( root.input.activatedPanel[0].sourceSnapShot == SDB_SNAP_SESSIONS_CURRENT_TOP )
   {
      snapType = SDB_SNAP_SESSIONS_CURRENT ;
   }
   else if( root.input.activatedPanel[0].sourceSnapShot == SDB_SNAP_COLLECTIONS_TOP )
   {
      snapType = SDB_SNAP_COLLECTIONS ;
   }
   else if( root.input.activatedPanel[0].sourceSnapShot == SDB_SNAP_COLLECTIONSPACES_TOP )
   {
      snapType = SDB_SNAP_COLLECTIONSPACES ;
   }
   else if( root.input.activatedPanel[0].sourceSnapShot == SDB_SNAP_DATABASE_TOP )
   {
      snapType = SDB_SNAP_DATABASE ;
   }
   else if( root.input.activatedPanel[0].sourceSnapShot == SDB_SNAP_SYSTEM_TOP )
   {
      snapType = SDB_SNAP_SYSTEM ;
   }
   else if( root.input.activatedPanel[0].sourceSnapShot == SDB_SNAP_CATALOG_TOP )
   {
      snapType = SDB_SNAP_CATALOG ;
   }
   else
   {
      if( root.input.activatedPanel[0].bodyPanelType != BODYTYPE_NORMAL )
      {
         goto done ;
      }

      snprintf( errStrBuf, errStrLength,"%s", errStr );
      snprintf( errStr, errStrLength,
                "%s getCurSnapshot failed,xml gave the wrong sourceSnapShot: %s\n",
                errStrBuf, root.input.activatedPanel[0].sourceSnapShot.c_str() ) ;

      goto error ;
   }
   if( NULLSTRING == root.input.sortingWay )
   {
      orderBy += "}" ;
   }
   else if( SORTINGWAY_ASC == root.input.sortingWay || SORTINGWAY_DESC == root.input.sortingWay )
   {
      if( NULLSTRING != root.input.sortingField )
         orderBy += "\"" + root.input.sortingField + "\":" + root.input.sortingWay + "}" ;
      else
         orderBy += "}" ;
   }
   else
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getCurSnapshot failed,wrong sortingWay: %s\n",
                errStrBuf, root.input.sortingWay.c_str() ) ;
      goto error ;
   }
   fromjson( orderBy, orderByObj ) ;
   if( NULLSTRING != root.input.filterCondition )
   {
      pos = root.input.filterCondition.find( ":" ) ;
      if( string::npos != pos )
      {
         conditionPre = root.input.filterCondition.substr( 0, pos ) ;
         conditionCon = root.input.filterCondition.substr( pos + 1 ) ;
      }
      else
      {
         root.input.filterCondition = NULLSTRING;
      }
   }
   if( GLOBAL == root.input.snapshotModeChooser )
   {
      if( NULLSTRING != root.input.filterCondition )
      {
         condition = "{" + conditionPre + ":" + conditionCon + "}" ;
      }
      else
         condition = "{}" ;
      fromjson(condition, conditionObj ) ;
      rc = coord.getSnapshot( cursor, snapType, conditionObj, selectorObj, orderByObj) ;
   }
   else if( GROUP == root.input.snapshotModeChooser )
   {
      condition = "{GroupName:\""  + root.input.groupName ;
      if( NULLSTRING != root.input.filterCondition )
      {
         condition += "\"," + conditionPre + ":" + conditionCon + "}" ;
      }
      else
         condition += "\"}" ;

      fromjson(condition, conditionObj ) ;
      rc = coord.getSnapshot( cursor, snapType, conditionObj, selectorObj, orderByObj) ;
   }
   else if( NODE == root.input.snapshotModeChooser )
   {
      condition = "{HostName:\"" ;
      pos = root.input.nodeName.find( ":" ) ;
      if( string::npos != pos )
      {
         HostName = root.input.nodeName.substr( 0, pos ) ;
         svcname = root.input.nodeName.substr( pos + 1 ) ;
         condition += HostName + "\",svcname:\"" + svcname + "\"" ;
      }
      else
      {
         HostName = root.input.nodeName ;
         condition += HostName + "\",svcname:\" \"" ;
      }
      if( NULLSTRING != root.input.filterCondition )
      {
         condition += "," + conditionPre + ":" + conditionCon + "}" ;
      }
      else
         condition += "}" ;
      fromjson(condition, conditionObj ) ;
      rc = coord.getSnapshot( cursor, snapType, conditionObj, selectorObj, orderByObj) ;
   }
   else
   {

      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getCurSnapshot failed,wrong snapshotModeChooser = %s\n",
                errStrBuf, root.input.snapshotModeChooser.c_str() ) ;

      goto error ;
   }
   if( rc )
   {

      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getCurSnapshot failed,can't getSnapshot, rc = %d\n", errStrBuf, rc ) ;
  
      goto error ;
   }
   
   root.input.last_absoluteMap.clear() ;
   root.input.last_absoluteMap = root.input.cur_absoluteMap ;
   root.input.cur_absoluteMap.clear() ;

   root.input.last_deltaMap.clear() ;
   root.input.last_deltaMap = root.input.cur_deltaMap ;
   root.input.cur_deltaMap.clear() ;

   root.input.last_averageMap.clear() ;
   root.input.last_averageMap = root.input.cur_averageMap ;
   root.input.cur_averageMap.clear() ;
   
   root.input.last_Snapshot.clear() ;
   root.input.last_Snapshot = root.input.cur_Snapshot ;
   root.input.cur_Snapshot.clear() ;
   filterNum = root.input.filterNumber ;
   while( !( rc = cursor.next( bsonobj ) ) )   
   {
      if( 0 < filterNum )
      {
         --filterNum ;
         continue ;
      }
      root.input.cur_Snapshot.push_back( bsonobj ) ;
   } 
   if( SDB_DMS_EOC != rc && SDB_OK != rc )   
   {      

      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s refreshDisplayContent failed, snapShotCursor.next( bsonobj ) faild, rc = %d\n", errStrBuf, rc ) ;

      goto error ;   
   }
   if( SDB_DMS_EOC == rc )
   {
      rc = SDB_OK ;
   }
   if( TRUE == root.input.isFirstGetSnapshot )
   {
      root.input.isFirstGetSnapshot = FALSE ;
      root.input.last_Snapshot.clear() ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::fixedOutputLocation( INT32 start_row, INT32 start_col,
                                  INT32 &fixed_row, INT32 &fixed_col,
                                  INT32 referRowLength,INT32 referColLength,
                                  string displayType, string autoSetType )
{
   INT32 rc = SDB_OK ;
   fixed_row = start_row ;
   fixed_col = start_col ;
   if( autoSetType == UPPER_LEFT )
   {
      fixed_row = start_row ;
      fixed_col = start_col ;
   }
   else if( autoSetType == MIDDLE_LEFT )
   {
      fixed_row = start_row + referRowLength / 2 ;
      fixed_col = start_col ;
   }
   else if( autoSetType == LOWER_LEFT )
   {
      fixed_row = start_row + referRowLength ;
      fixed_col = start_col ; 
   }
   else if( autoSetType == UPPER_MIDDLE )
   {
      fixed_row = start_row ;
      fixed_col = start_col + referColLength / 2 ;
   }
   else if( autoSetType == MIDDLE)
   {
      fixed_row = start_row + referRowLength / 2 ;
      fixed_col = start_col + referColLength / 2 ;
   }
   else if( autoSetType == LOWER_MIDDLE)
   {
      fixed_row = start_row + referRowLength ;
      fixed_col = start_col + referColLength / 2 ;
   }
   else if( autoSetType == UPPER_RIGHT)
   {
      fixed_row = start_row ;
      fixed_col = start_col + referColLength ;
   }
   else if( autoSetType == MIDDLE_RIGHT)
   {
      fixed_row = start_row + referRowLength / 2 ;
      fixed_col = start_col + referColLength ;
   }
   else if( autoSetType == LOWER_RIGHT)
   {
      fixed_row = start_row + referRowLength ;
      fixed_col = start_col + referColLength ;
   }
   else
   {

      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s fixedOutputLocation failed,wrong autoSetType:%s\n",
                errStrBuf, autoSetType.c_str() ) ;

      goto error ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::getFieldStructNameAndColour( const FieldStruct &fieldStruct,
                                          const string &displayMode,
                                          string &fieldName, Colours &fieldColour )
{
   INT32 rc = SDB_OK ;
   if( !fieldStruct.canSwitch )
   {
      fieldName = fieldStruct.absoluteName ;
      fieldColour.backGroundColor = fieldStruct.absoluteColour.backGroundColor ;
      fieldColour.foreGroundColor = fieldStruct.absoluteColour.foreGroundColor ;
   }
   else if( DELTA == displayMode )
   {
      fieldName = fieldStruct.deltaName ;
      fieldColour.backGroundColor = fieldStruct.deltaColour.backGroundColor ;
      fieldColour.foreGroundColor = fieldStruct.deltaColour.foreGroundColor ;
   }
   else if( ABSOLUTE == displayMode )
   {
      fieldName = fieldStruct.absoluteName ;
      fieldColour.backGroundColor = fieldStruct.absoluteColour.backGroundColor ;
      fieldColour.foreGroundColor = fieldStruct.absoluteColour.foreGroundColor ;
   }
   else if( AVERAGE == displayMode )
   {
      fieldName = fieldStruct.averageName ;
      fieldColour.backGroundColor = fieldStruct.averageColour.backGroundColor ;
      fieldColour.foreGroundColor = fieldStruct.averageColour.foreGroundColor ;
   }
   else
   {

      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s getFieldStructNameAndColour failed,wrong displayMode:%s\n",
                errStrBuf, displayMode.c_str() ) ;

      goto error ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::refresh_DISPLAYTYPE_DYNAMIC_HELP( DisplayContent &displayContent,
                                               string displayType, Position &actualPosition )
{
   INT32 rc = SDB_OK ;
   INT32 start_row = actualPosition.referUpperLeft_Y ;
   INT32 start_col = actualPosition.referUpperLeft_X ;
   INT32 start_col_copy = start_col ;
   KeySuite* keySuite = NULL ;
   INT32 hotKey_pos = 0 ;
   INT32 rowNumber = 0 ;
   INT32 tableColumnLength = 0 ;
   INT32 colNumber = 0 ;
   INT32 pairNumber = 0 ;
   string _str = NULLSTRING ;
   CHAR *printfstr = ( CHAR * )malloc( displayContent.dynamicHelp.cellLength * sizeof( CHAR ) ) ;
   if( !printfstr )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                   "%s MVPRINTW_TOP faild, can't malloc memory for printfstr :%d\n",
                   errStrBuf, displayContent.dynamicHelp.cellLength ) ;
      goto error ;
   }

   if( displayContent.dynamicHelp.tableRow > actualPosition.length_Y )
   {
   
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength, 
                 "%s refreshDisplayContent failed,displayContent.dynamicHelp.tableRow> actualPosition.length_Y\n",
                 errStrBuf ) ;
   
      goto error ;
   }
   rc = getActivatedKeySuite( &keySuite ) ;
   if( rc || !keySuite )
   {
      goto error ;
   }
   rc = fixedOutputLocation( actualPosition.referUpperLeft_Y, actualPosition.referUpperLeft_X,
                             start_row, start_col,
                             actualPosition.length_Y - displayContent.dynamicHelp.tableRow, 0,
                             displayType, displayContent.dynamicHelp.autoSetType) ;
   if( rc )
   {
      goto error ;
   }
   for( rowNumber = 0; rowNumber < displayContent.dynamicHelp.tableRow; ++rowNumber )
   {
      start_row += rowNumber ;
       tableColumnLength = 0 ;
      for( colNumber = 0; colNumber< displayContent.dynamicHelp.tableColumn; ++ colNumber )
      {
         if( tableColumnLength > actualPosition.length_X )
            break ;
         tableColumnLength += displayContent.dynamicHelp.cellLength ;
      }
      rc = fixedOutputLocation( start_row, actualPosition.referUpperLeft_X,
                                start_row, start_col, 
                                0,actualPosition.length_X - tableColumnLength,
                                displayType, displayContent.dynamicHelp.autoSetType) ;
      if( rc )
      {
         goto error ;
      }
      for( colNumber = 0; colNumber< displayContent.dynamicHelp.tableColumn; ++ colNumber )
      {
         if( hotKey_pos >= keySuite->hotKeyLength )
            break ;
         if( start_col + displayContent.dynamicHelp.cellLength -
             actualPosition.referUpperLeft_X  > actualPosition.length_X )
            break ;
         start_col_copy = start_col ;
         rc = SDBTOP_MEMSET( printfstr, 0, displayContent.dynamicHelp.cellLength );
         //printf prefix
         SDBTOP_MEMSET( printfstr, 0, displayContent.dynamicHelp.cellLength ) ;
         if( JUMPTYPE_FIXED == keySuite->hotKey[hotKey_pos].jumpType )
         {
            if( BUTTON_TAB == keySuite->hotKey[hotKey_pos].button )
               snprintf( printfstr, 7, "Tab -  " ) ;
            else if( BUTTON_LEFT == keySuite->hotKey[hotKey_pos].button )
               snprintf( printfstr, 7, "  < -  " ) ;
            else if( BUTTON_RIGHT == keySuite->hotKey[hotKey_pos].button )
               snprintf( printfstr, 7, "  > -  " ) ;
            else if( BUTTON_ENTER == keySuite->hotKey[hotKey_pos].button )
               snprintf( printfstr, 7, "Enter-  " ) ;
            else if( BUTTON_ESC == keySuite->hotKey[hotKey_pos].button )
               snprintf( printfstr, 7, "ESC-  " ) ;
            else if( BUTTON_F5== keySuite->hotKey[hotKey_pos].button )
               snprintf( printfstr, 7, "F5-  " ) ;
            else if( BUTTON_Q_LOWER == keySuite->hotKey[hotKey_pos].button )
               snprintf( printfstr, 7, "  %c -  ", keySuite->hotKey[hotKey_pos].button ) ;
            else if( BUTTON_H_LOWER == keySuite->hotKey[hotKey_pos].button )
               snprintf( printfstr, 7, "  %c -  ", keySuite->hotKey[hotKey_pos].button ) ;
            else 
               snprintf( printfstr, 7, "NULL-  ", keySuite->hotKey[hotKey_pos].button ) ;
         }
         else
            snprintf( printfstr, 7, "  %c -  ", keySuite->hotKey[hotKey_pos].button ) ;
         _str = printfstr ;
         pairNumber = displayContent.dynamicHelp.prefixColour.foreGroundColor +
                            displayContent.dynamicHelp.prefixColour.backGroundColor * 8 ;
         attron( COLOR_PAIR( pairNumber ) ) ;
         rc = MVPRINTW_TOP( _str, _str.length() + 1, LEFT, start_row, start_col_copy ) ;
         if( rc )
         {
            goto error ;
         }
         attroff( COLOR_PAIR( pairNumber ) ) ;
         start_col_copy += _str.length() + 1 ;
   
         //printf content
         SDBTOP_MEMSET( printfstr, 0, displayContent.dynamicHelp.cellLength ) ;
         pairNumber = displayContent.dynamicHelp.contentColour.foreGroundColor +
                      displayContent.dynamicHelp.contentColour.backGroundColor * 8 ;
         attron( COLOR_PAIR( pairNumber ) ) ;
         rc = MVPRINTW_TOP( keySuite->hotKey[hotKey_pos].jumpName,
                            keySuite->hotKey[hotKey_pos].jumpName.length(),
                            LEFT, start_row, start_col_copy );
         if( rc )
         {
            goto error ;
         }
         attroff( COLOR_PAIR( pairNumber ) ) ;
   
         start_col += displayContent.dynamicHelp.cellLength ;
         ++hotKey_pos ;
      }
      start_row -= rowNumber ;
   }

done :
   free( printfstr ) ;
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::refresh_DISPLAYTYPE_DYNAMIC_EXPRESSION( DisplayContent &displayContent,
                                                     string displayType,
                                                     Position &actualPosition )
{
   INT32 rc = SDB_OK ;
   INT32 start_row = actualPosition.referUpperLeft_Y ;
   INT32 start_col = actualPosition.referUpperLeft_X ;
   INT32 expressionLengthSum = 0 ;
   INT32 expressionLength = 0 ;
   INT32 pairNumber = 0 ;
   string result = NULLSTRING ;
   INT32 expressionNumber = 0 ;
   INT32 rowNumber = 0 ;
   rc = fixedOutputLocation( actualPosition.referUpperLeft_Y, actualPosition.referUpperLeft_X,
                             start_row, start_col,
                             actualPosition.length_Y - displayContent.dyExOutPut.rowNumber, 0,
                             displayType, displayContent.dyExOutPut.autoSetType) ;
   if( rc )
   {
      goto error ;
   }
   for( rowNumber = 0; rowNumber < displayContent.dyExOutPut.rowNumber && rowNumber < actualPosition.length_Y ; ++rowNumber )
   {
      start_row += rowNumber ;
      expressionLengthSum = 0 ;
      expressionLength = 0 ;
      pairNumber = 0 ;
      result = NULLSTRING ;
      for( expressionNumber = 0; expressionNumber < displayContent.dyExOutPut.expressionNumber; ++expressionNumber )
      {
         if( displayContent.dyExOutPut.content[expressionNumber].expressionType == STATIC_EXPRESSION )
         {
            result = displayContent.dyExOutPut.content[expressionNumber].expressionValue.text ;
         }
         else if( displayContent.dyExOutPut.content[expressionNumber].expressionType == DYNAMIC_EXPRESSION )
         {
            rc = getExpression( displayContent.dyExOutPut.content[expressionNumber].expressionValue.expression,
                                result ) ;
            if( rc )
            {
               snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
               snprintf( errStr, errStrLength,
                         "%s refreshDisplayContent failed, getExpression faild, expressionType == DYNAMIC_EXPRESSION\n",
                         errStrBuf ) ;
               goto error ;
            }
         }
         if( NULLSTRING == result )
            result = STRING_NULL ;
         expressionLength = result.length() ;
         if( expressionLengthSum + expressionLength > actualPosition.length_X )
            continue ;
         if( displayContent.dyExOutPut.content[expressionNumber].rowLocation != rowNumber + 1 )
         {
            continue ;
         }
         expressionLengthSum += expressionLength ;
         ++expressionLengthSum ; // add space
      }
      
      rc = fixedOutputLocation( start_row, actualPosition.referUpperLeft_X,
                                start_row, start_col,
                                0, actualPosition.length_X -expressionLengthSum ,
                                displayType, displayContent.dyExOutPut.autoSetType) ;
      if( rc )
      {
         goto error ;
      }
      for( expressionNumber = 0; expressionNumber < displayContent.dyExOutPut.expressionNumber; ++expressionNumber )
      {
         if( displayContent.dyExOutPut.content[expressionNumber].rowLocation != rowNumber + 1 )
         {
            continue ;
         }
         if( displayContent.dyExOutPut.content[expressionNumber].expressionType == STATIC_EXPRESSION )
         {
            result = displayContent.dyExOutPut.content[expressionNumber].expressionValue.text ;
         }
         else if( displayContent.dyExOutPut.content[expressionNumber].expressionType == DYNAMIC_EXPRESSION )
         {
            rc = getExpression( displayContent.dyExOutPut.content[expressionNumber].expressionValue.expression,
                                result ) ;
         }
         if( NULLSTRING == result )
            result = STRING_NULL ;
         expressionLength = result.length() ;
         if( start_col + expressionLength - actualPosition.referUpperLeft_X > actualPosition.length_X )
            continue ;
         pairNumber = displayContent.dyExOutPut.content[expressionNumber].colour.foreGroundColor +
                            displayContent.dyExOutPut.content[expressionNumber].colour.backGroundColor * 8 ;
         attron( COLOR_PAIR( pairNumber ) ) ;
         rc = MVPRINTW_TOP( result, expressionLength, displayContent.dyExOutPut.content[expressionNumber].alignment,
                        start_row, start_col ) ;
         if( rc )
         {
            goto error ;
         }
         attroff( COLOR_PAIR( pairNumber ) ) ;
         start_col += expressionLength ;
         ++start_col ;
      }
      start_row -= rowNumber ;
   }

done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::refresh_DISPLAYTYPE_DYNAMIC_SNAPSHOT( DisplayContent &displayContent,
                                                   string displayType, Position &actualPosition )
{
   INT32 rc = SDB_OK ;
   BSONObj bsonobj ;
   FieldStruct* Fixed = NULL ;
   FieldStruct* Mobile = NULL ;
   INT32 FixedLength = 0 ;
   INT32 MobileLength = 0 ;
   Fixed = displayContent.dySnapshotOutPut.fixedField ;
   Mobile = displayContent.dySnapshotOutPut.mobileField ;
   FixedLength = displayContent.dySnapshotOutPut.actualFixedFieldLength ;
   MobileLength = displayContent.dySnapshotOutPut.actualMobileFieldLength ;
   
   INT32 start_row = actualPosition.referUpperLeft_Y ;
   INT32 start_col = actualPosition.referUpperLeft_X ;
   string AUTOSETTYPE = NULLSTRING ;
   string STYLE = NULLSTRING ;
   INT32 ROW = 0 ;
   INT32 COL = 0 ;
   string fieldName = NULLSTRING ;
   Colours fieldColour ;
   fieldColour.backGroundColor = 6 ;
   fieldColour.foreGroundColor = 0 ;
   string displayMode = DISPLAYMODECHOOSER[root.input.displayModeChooser] ;
   INT32 tableCellLength = displayContent.dySnapshotOutPut.tableCellLength ;
   string baseField = displayContent.dySnapshotOutPut.baseField ;
   string serialNumberAlignment = LEFT ;
   INT32 end_fixed_mobile = 0 ; //use it to sign the end position of fixedFieldStruct or mobileFieldStruct 
   INT32 start_fixed_mobile = 0 ; //use it to sign the start position of fixedFieldStruct or mobileFieldStruct
   INT32 tableCol = 0 ; // when style is TABLE , use it
   INT32 start_up = 0 ; // when style is TABLE , use it
   INT32 expressionLengthSum = 0 ;
   INT32 pos_snapshot = 0 ;
   INT32 pairNumber = 0 ;
   string result =  NULLSTRING ;
   INT32 fLength = 0 ;
   INT32 mLength = 0 ;
   INT32 rowNumber = 0;
   string serialNumberStr = NULLSTRING ;
   CHAR *serialNumber = ( CHAR * )malloc( SERIALNUMBER_LENGTH * sizeof( CHAR ) ) ;
   if( !serialNumber )
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
               "%s refreshDisplayContent failed, can't malloc serialNumber == %d\n",
               errStrBuf, SERIALNUMBER_LENGTH ) ;
      goto error ; 

   }
   rc = SDBTOP_MEMSET( serialNumber, 0, SERIALNUMBER_LENGTH ) ;
   if( rc )
   {
      goto error;
   }
   if( GLOBAL == root.input.snapshotModeChooser )
   {
      AUTOSETTYPE = displayContent.dySnapshotOutPut.globalAutoSetType ;
      STYLE = displayContent.dySnapshotOutPut.globalStyle ;
      if( TABLE == STYLE )
      {
         ROW = displayContent.dySnapshotOutPut.globalRow ;
         COL = displayContent.dySnapshotOutPut.globalCol ;
      }
      else
      {
         ROW = 0 ;
         COL = 0 ;
      }
      
   }
   else if( GROUP == root.input.snapshotModeChooser )
   {
      AUTOSETTYPE = displayContent.dySnapshotOutPut.groupAutoSetType ;
      STYLE = displayContent.dySnapshotOutPut.groupStyle ;
      if( TABLE == STYLE )
      {
         ROW = displayContent.dySnapshotOutPut.groupRow ;
         COL = displayContent.dySnapshotOutPut.groupCol ;
      }
      else
      {
         ROW = 0 ;
         COL = 0 ;
      }
   }
   else if( NODE == root.input.snapshotModeChooser )
   {
      AUTOSETTYPE = displayContent.dySnapshotOutPut.nodeAutoSetType ;
      STYLE = displayContent.dySnapshotOutPut.nodeStyle ;
      if( TABLE == STYLE )
      {
         ROW = displayContent.dySnapshotOutPut.nodeRow ;
         COL = displayContent.dySnapshotOutPut.nodeCol ;
      }
      else
      {
         ROW = 0 ;
         COL = 0 ;
      }
   }
   else
   {
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
               "%s refreshDisplayContent failed, wrong snapshotModeChooser == %s\n",
               errStrBuf, root.input.snapshotModeChooser.c_str() ) ;
      goto error ;    
   }
   if( TABLE == STYLE)
   {
      rc = fixedOutputLocation( actualPosition.referUpperLeft_Y, actualPosition.referUpperLeft_X,
                                start_row, start_col, actualPosition.length_Y - ROW * 2, 0,
                                displayType, AUTOSETTYPE ) ;
      if( rc )
      {
         goto error ;
      }
      end_fixed_mobile = 0 ; //use it to sign the end position of fixedFieldStruct or mobileFieldStruct 
      start_fixed_mobile = 0 ; //use it to sign the start position of fixedFieldStruct or mobileFieldStruct
      for( tableCol = 0; tableCol <= COL; ++tableCol )
      {
         if(( tableCol + 1 ) * tableCellLength > actualPosition.length_X )
            break ;
      }
      ROW = ROW * COL / tableCol ; // adapt to the actual window
      COL = tableCol ; // adapt to the actual window
      tableCol = 0;
      for( rowNumber = 0; rowNumber < ROW; ++rowNumber )
      {
         start_row += rowNumber ;
         if( start_row - actualPosition.referUpperLeft_Y >= actualPosition.length_Y )
         {
            break ;
         }
         rc = fixedOutputLocation( start_row, actualPosition.referUpperLeft_X,
                                   start_row, start_col, 0, actualPosition.length_X -COL * tableCellLength,
                                   displayType, AUTOSETTYPE ) ;
         if( rc )
         {
            goto error ;
         }
         tableCol = 0 ;
         start_fixed_mobile = end_fixed_mobile ;
         // print the title on the screen
         while( 1 )
         {
            if( start_col + tableCellLength - actualPosition.referUpperLeft_X > actualPosition.length_X )
               break ;
            if( end_fixed_mobile >= FixedLength)
               break ;
            if( tableCol >= COL )
               break ;
            rc = getFieldStructNameAndColour( Fixed[end_fixed_mobile], displayMode, fieldName, fieldColour) ;
            if( rc )
            {
               goto error ;
            }
            pairNumber = fieldColour.foreGroundColor + fieldColour.backGroundColor * 8 ;
            attron( COLOR_PAIR( pairNumber ) ) ;
            rc = MVPRINTW_TOP( fieldName, tableCellLength, Fixed[end_fixed_mobile].alignment, start_row, start_col ) ;
            if( rc )
            {
               goto error ;
            }
            attroff( COLOR_PAIR( pairNumber ) ) ;  
            start_col += tableCellLength ;
            ++start_col ; // add separate pace
            ++end_fixed_mobile ;
            ++tableCol ;
         }
         
         while( 1 )
         {
            if( start_col + tableCellLength - actualPosition.referUpperLeft_X > actualPosition.length_X )
               break ;
            if( end_fixed_mobile - FixedLength >= MobileLength )
               break ;
            if( tableCol >= COL )
               break ;
            rc = getFieldStructNameAndColour( Mobile[end_fixed_mobile - FixedLength],
                                              displayMode, fieldName, fieldColour ) ;
            if( rc )
            {
               goto error ;
            }
            pairNumber = fieldColour.foreGroundColor + fieldColour.backGroundColor * 8 ;
            attron( COLOR_PAIR( pairNumber ) ) ;
            rc = MVPRINTW_TOP( fieldName, tableCellLength,
                               Mobile[end_fixed_mobile - FixedLength].alignment, start_row, start_col ) ;
            if( rc )
            {
               goto error ;
            }
            attroff( COLOR_PAIR( pairNumber ) ) ;  
            start_col += tableCellLength ;
            ++start_col ; // add separate pace
            ++end_fixed_mobile ;
            ++tableCol ;
         }
   
         //print the content on the screen
         start_row += 1 ; // 
         if( start_row - actualPosition.referUpperLeft_Y >= actualPosition.length_Y )
         {
            goto done ;
         }
         for( pos_snapshot = 0; pos_snapshot < root.input.cur_Snapshot.size(); ++pos_snapshot )
         {      
            start_up = start_fixed_mobile ;
            bsonobj = root.input.cur_Snapshot[pos_snapshot] ;
            rc = fixedOutputLocation( start_row, actualPosition.referUpperLeft_X,
                                      start_row, start_col,
                                      0, actualPosition.length_X -COL * tableCellLength,
                                      displayType, AUTOSETTYPE ) ;
            if( rc )
            {
               goto error ;
            }
            while( 1 )
            {
               if( start_col + tableCellLength - actualPosition.referUpperLeft_X > actualPosition.length_X )
                  break ;
               if( start_up >= FixedLength )
                  break ;
               if( start_up >= end_fixed_mobile)
                  break ;
               rc = getFieldStructNameAndColour( Fixed[start_up], displayMode, fieldName, fieldColour) ;
               if( SDB_OK != rc )
               {
                  goto error;
               }
               pairNumber = fieldColour.foreGroundColor + fieldColour.backGroundColor * 8 ;
               result = NULLSTRING ;
               rc = getResultFromBSONObj( bsonobj, Fixed[start_up].sourceField,
                                          displayMode, result, Fixed[start_up].canSwitch,
                                          baseField, Fixed[start_up].warningValue, pairNumber) ;
               attron( COLOR_PAIR( pairNumber ) ) ;
               if( SDB_OK != rc )
               {
                  goto error ;
               }
               rc = MVPRINTW_TOP( result, tableCellLength, Fixed[start_up].alignment, start_row, start_col ) ;
               if( SDB_OK != rc )
               {
                  goto error ;
               }
               attroff( COLOR_PAIR( pairNumber ) ) ;  
               start_col += tableCellLength ;
               ++start_col ; // add separate pace
               ++start_up ;
            }
            while( 1 )
            {
               if( start_col + tableCellLength - actualPosition.referUpperLeft_X > actualPosition.length_X )
                  break ;
               if( start_up >= end_fixed_mobile )
                  break ;
               rc = getFieldStructNameAndColour( Mobile[start_up - FixedLength],
                                                 displayMode, fieldName, fieldColour) ;
               if( rc )
               {
                  goto error ;
               }
               pairNumber = fieldColour.foreGroundColor + fieldColour.backGroundColor * 8 ;
               result = NULLSTRING ;
               rc = getResultFromBSONObj( bsonobj,
                                          Mobile[start_up- FixedLength].sourceField,
                                          displayMode, result,
                                          Mobile[start_up- FixedLength].canSwitch,
                                          baseField, Mobile[start_up- FixedLength].warningValue,
                                          pairNumber );
               attron( COLOR_PAIR( pairNumber ) ) ;
               if( rc )
               {
                  goto error ;
               }
               rc = MVPRINTW_TOP( result, tableCellLength,
                                  Mobile[start_up - FixedLength].alignment,
                                  start_row, start_col ) ;
               if( rc )
               {
                  goto error ;
               }
               attroff( COLOR_PAIR( pairNumber ) ) ;  
               start_col += tableCellLength;
               ++start_col ; // add separate pace
               ++start_up ;
            }
            start_row += 1 ; 
            if( start_row - actualPosition.referUpperLeft_Y >= actualPosition.length_Y )
            {
               break ;
            }
         }   
         start_row -= rowNumber ;
      }
   }// if( TABLE == STYLE)
   else // if( LIST == STYLE)
   {
      if( SDB_OK != rc )
      {
         goto error ;
      }
      expressionLengthSum = 0 ;
      for( fLength = 0; fLength < FixedLength; ++fLength )
      {
         if( expressionLengthSum + Fixed[fLength].contentLength > actualPosition.length_X )
            break ;
         expressionLengthSum += Fixed[fLength].contentLength ;
      }
      
      if( root.input.fieldPosition >= MobileLength )
      {
         root.input.fieldPosition = MobileLength - 1 ;
      }
      for( mLength = root.input.fieldPosition; mLength < MobileLength; ++mLength )
      {
         if( expressionLengthSum + Mobile[mLength].contentLength  > actualPosition.length_X )
            break ;
         expressionLengthSum += Mobile[mLength].contentLength ;
      }
      rc = fixedOutputLocation( start_row, actualPosition.referUpperLeft_X,
                                start_row, start_col,
                                0, actualPosition.length_X -expressionLengthSum,
                                displayType, AUTOSETTYPE );
      if( rc )
      {
         goto error ;
      }

      // print the serial number on the screen
      rc = SDBTOP_MEMSET( serialNumber, 0, SERIALNUMBER_LENGTH ) ;
      if( rc )
      {
         goto error;
      }
      snprintf( serialNumber, SERIALNUMBER_LENGTH, "%3d", 0 ) ;
      serialNumberStr = serialNumber ;
      rc = MVPRINTW_TOP( serialNumberStr, SERIALNUMBER_LENGTH, serialNumberAlignment, start_row, start_col ) ;
      if( rc )
      {
         goto error;
      }
      start_col += SERIALNUMBER_LENGTH ;
      ++start_col ; // add separate pace

      // print the title on the screen
      for( fLength = 0; fLength < FixedLength; ++fLength )
      {
         if( start_col + Fixed[fLength].contentLength -
             actualPosition.referUpperLeft_X > actualPosition.length_X )
            break ;
         rc = getFieldStructNameAndColour( Fixed[fLength], displayMode, fieldName, fieldColour) ;
         if( rc )
         {
            goto error ;
         }
         pairNumber = fieldColour.foreGroundColor + fieldColour.backGroundColor * 8 ;
         attron( COLOR_PAIR( pairNumber ) ) ;
         rc = MVPRINTW_TOP( fieldName, Fixed[fLength].contentLength,
                            Fixed[fLength].alignment, start_row, start_col ) ;
         if( rc )
         {
            goto error;
         }
         attroff( COLOR_PAIR( pairNumber ) ) ;  
         start_col += Fixed[fLength].contentLength ;
         ++start_col ; // add separate pace
      }
      
      if( root.input.fieldPosition >= MobileLength )
      {
         root.input.fieldPosition = MobileLength - 1 ;
      }
      for( mLength = root.input.fieldPosition; mLength < MobileLength; ++mLength )
      {
         if( start_col + Mobile[mLength].contentLength -
             actualPosition.referUpperLeft_X > actualPosition.length_X )
            break ;
         rc = getFieldStructNameAndColour( Mobile[mLength],
                                           displayMode, fieldName, fieldColour);
         if( rc )
         {
            goto error ;
         }
         pairNumber = fieldColour.foreGroundColor + fieldColour.backGroundColor * 8 ;
         attron( COLOR_PAIR( pairNumber ) ) ;
         rc = MVPRINTW_TOP( fieldName, Mobile[mLength].contentLength,
                            Mobile[mLength].alignment, start_row, start_col ) ;
         if( rc )
         {
            goto error ;
         }
         attroff( COLOR_PAIR( pairNumber ) ) ;  
         start_col += Mobile[mLength].contentLength ;
         ++start_col ; // add separate pace
      }
   
      //print the content on the screen
      start_row += 1 ; // 
      if( start_row - actualPosition.referUpperLeft_Y >= actualPosition.length_Y )
      {
         goto done ;
      }
      rc = fixedOutputLocation( start_row, actualPosition.referUpperLeft_X,
                                start_row, start_col,
                                0, actualPosition.length_X -expressionLengthSum ,
                                displayType, AUTOSETTYPE ) ;
      if( rc )
      {
         goto error ;
      }
      for( pos_snapshot = 0; pos_snapshot < root.input.cur_Snapshot.size(); ++pos_snapshot )
      {

         // print the serial number on the screen
         rc = SDBTOP_MEMSET( serialNumber, 0, SERIALNUMBER_LENGTH ) ;
         if( rc )
         {
            goto error;
         }
         snprintf( serialNumber, SERIALNUMBER_LENGTH, "%3d", pos_snapshot + 1 ) ;
         serialNumberStr = serialNumber ;
         rc = MVPRINTW_TOP( serialNumberStr, SERIALNUMBER_LENGTH, serialNumberAlignment, start_row, start_col ) ;
         if( rc )
         {
            goto error;
         }
         start_col += SERIALNUMBER_LENGTH ;
         ++start_col ; // add separate pace

         bsonobj = root.input.cur_Snapshot[pos_snapshot] ;
         for( fLength = 0; fLength < FixedLength; ++fLength )
         {
            if( start_col + Fixed[fLength].contentLength -
                actualPosition.referUpperLeft_X > actualPosition.length_X )
               break ;
            rc = getFieldStructNameAndColour( Fixed[fLength], displayMode, fieldName, fieldColour ) ;
            if( rc )
            {
               goto error ;
            }
            pairNumber = fieldColour.foreGroundColor + fieldColour.backGroundColor * 8 ;
            result = NULLSTRING ;
            rc = getResultFromBSONObj( bsonobj, Fixed[fLength].sourceField,
                                       displayMode, result,
                                       Fixed[fLength].canSwitch, baseField,
                                       Fixed[fLength].warningValue, pairNumber );
            attron( COLOR_PAIR( pairNumber ) ) ;
            if( rc )
            {
               goto error ;
            }
            rc = MVPRINTW_TOP( result, Fixed[fLength].contentLength,
                               Fixed[fLength].alignment, start_row, start_col ) ;
            if( rc )
            {
               goto error;
            }
            attroff( COLOR_PAIR( pairNumber ) ) ;  
            start_col += Fixed[fLength].contentLength ;
            ++start_col ; // add separate pace
         }
         
         if( root.input.fieldPosition >= MobileLength )
         {
            root.input.fieldPosition = MobileLength - 1 ;
         }
         for( mLength = root.input.fieldPosition; mLength < MobileLength; ++mLength )
         {
            if( start_col + Mobile[mLength].contentLength - actualPosition.referUpperLeft_X >
                actualPosition.length_X )
               break ;
            rc = getFieldStructNameAndColour( Mobile[mLength], displayMode,
                                              fieldName, fieldColour) ;
            if( rc )
            {
               goto error ;
            }
            pairNumber = fieldColour.foreGroundColor + fieldColour.backGroundColor * 8 ;
            result =  NULLSTRING ;
            rc = getResultFromBSONObj( bsonobj, Mobile[mLength].sourceField,
                                       displayMode, result, Mobile[mLength].canSwitch, baseField,
                                       Mobile[mLength].warningValue, pairNumber ) ;
            attron( COLOR_PAIR( pairNumber ) ) ;
            if( rc )
            {
               goto error ;
            }
            rc = MVPRINTW_TOP( result, Mobile[mLength].contentLength,
                               Mobile[mLength].alignment, start_row, start_col ) ;
            if( rc )
            {
               goto error ;
            }
            attroff( COLOR_PAIR( pairNumber ) ) ;  
            start_col += Mobile[mLength].contentLength ;
            ++start_col ; // add separate pace
         }
         start_row += 1 ; 
         rc = fixedOutputLocation( start_row, actualPosition.referUpperLeft_X,
                                   start_row, start_col,
                                   0, actualPosition.length_X -expressionLengthSum ,
                                   displayType, AUTOSETTYPE);
         if( rc )
         {
            goto error;
         }
         if( start_row - actualPosition.referUpperLeft_Y >= actualPosition.length_Y )
         {
            break ;
         }
      }   
   }
done :
   free( serialNumber ) ;
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;

}
   
INT32 Event::refreshDisplayContent( DisplayContent &displayContent,
                                    string displayType,
                                    Position &actualPosition )
{
   INT32 rc = SDB_OK ;
   INT32 pairNumber  = 0 ;
   if( displayType == DISPLAYTYPE_STATICTEXT_HELP_Header ||
       displayType == DISPLAYTYPE_STATICTEXT_MAIN ||
       displayType == DISPLAYTYPE_STATICTEXT_LICENSE)
   {
      pairNumber = displayContent.staticTextOutPut.colour.foreGroundColor +
                         displayContent.staticTextOutPut.colour.backGroundColor * 8 ;
      attron( COLOR_PAIR( pairNumber ) ) ;
      mvprintw( actualPosition.referUpperLeft_Y, actualPosition.referUpperLeft_X,
                displayContent.staticTextOutPut.outputText ) ;
      attroff( COLOR_PAIR( pairNumber ) ) ;
   }
   else if( displayType == DISPLAYTYPE_DYNAMIC_HELP)
   {
      rc = refresh_DISPLAYTYPE_DYNAMIC_HELP( displayContent, displayType, actualPosition ) ;
      if( rc )
      {
         goto error ;
      }

   }
   else if( displayType == DISPLAYTYPE_DYNAMIC_EXPRESSION )
   {
      rc = refresh_DISPLAYTYPE_DYNAMIC_EXPRESSION( displayContent, displayType, actualPosition ) ;
      if( rc )
      {
         goto error ;
      }
   }
   else if( displayType == DISPLAYTYPE_DYNAMIC_SNAPSHOT )
   {
      rc = refresh_DISPLAYTYPE_DYNAMIC_SNAPSHOT( displayContent, displayType, actualPosition ) ;
      if( rc )
      {
         goto error ;
      }
   }
   else if( displayType == DISPLAYTYPE_NULL )
   {
      
   }
   else
   {
      goto error ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}
   
INT32 Event::refreshNodeWindow( NodeWindow &window )
{
   INT32 rc = SDB_OK ;
   Position actualPosition ;
   INT32 row = 0;
   INT32 col = 0;
   actualPosition.length_X = 0 ;
   actualPosition.length_Y = 0 ;
   actualPosition.referUpperLeft_X = 0 ;
   actualPosition.referUpperLeft_Y = 0 ;
   rc = getActualPosition( actualPosition, window.position,
                           window.zoomMode, window.occupyMode) ;
   if( rc )
   {

      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s refreshNodeWindow failed, getActualPosition faild\n",
                errStrBuf ) ;

      goto error ;
   }
   getmaxyx( stdscr, row, col ) ;
   if( row < window.actualWindowMinRow ||
       col < window.actualWindowMinColumn )
      goto done ;
   rc = refreshDisplayContent( window.displayContent, window.displayType, actualPosition ) ;
   if( rc )
   {

      snprintf( errStrBuf, errStrLength, "%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s refreshNodeWindow failed, refreshDisplayContent faild\n",
                errStrBuf );
      goto error;
   }
   
done :
   return rc ;
error :
   rc = SDB_ERROR;
   goto done ;
}

INT32 Event::refreshHeadTail( HeadTailMap *headtail )
{
   INT32 rc = SDB_OK ;
   INT32 numOfSubWindow = 0 ;
   if( !headtail )
      goto done ;
   for( numOfSubWindow = 0; numOfSubWindow < headtail->value.numOfSubWindow; ++numOfSubWindow )
   {
      rc = refreshNodeWindow( headtail->value.subWindow[numOfSubWindow] );
      if ( rc )
      {

         snprintf( errStrBuf, errStrLength, "%s", errStr ) ;
         snprintf( errStr, errStrLength,
                   "%s refreshHeadTail failed, refreshNodeWindow faild, numOfSubWindow = %d\n",
                   errStrBuf, numOfSubWindow ) ;

         goto error ;
      }
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::refreshBody( BodyMap *body )
{
   INT32 rc = SDB_OK ;
   INT32 numOfSubWindow = 0 ;
   for( numOfSubWindow = 0; numOfSubWindow < body->value.numOfSubWindow; ++numOfSubWindow )
   {
      rc = refreshNodeWindow( body->value.subWindow[numOfSubWindow] ) ;
      if ( rc )
      {

         snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
         snprintf( errStr, errStrLength,
                   "%s refreshBody failed, refreshNodeWindow faild, numOfSubWindow = %d\n",
                   errStrBuf, numOfSubWindow ) ;

         goto error ;
      }
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}
void Event::initAllColourPairs()
{
   INT32 back = 0 ;
   INT32 fore = 0 ;
   INT32 pairNumber = 0 ;
   for( back = 0; back <= 7; ++back )
   {
      for( fore = 0; fore <= 7; ++fore )
      {
         pairNumber = fore + back * 8 ;
         init_pair( pairNumber, fore, back ) ;
      }
   }
}

INT32 Event::addFixedHotKey()
{
   INT32 rc = SDB_OK ;
   INT32 keyLength = 0 ;
   for( INT32 i = 0; i < root.keySuiteLength; ++i )
   {
      keyLength = root.keySuite[i].hotKeyLength ;
      try
      {
         root.keySuite[i].hotKey[keyLength].button = BUTTON_TAB ;
         root.keySuite[i].hotKey[keyLength].jumpType = JUMPTYPE_FIXED;
         root.keySuite[i].hotKey[keyLength].jumpName = "Evaluation Model" ;
         ++keyLength ;

         root.keySuite[i].hotKey[keyLength].button = BUTTON_LEFT;
         root.keySuite[i].hotKey[keyLength].jumpType = JUMPTYPE_FIXED;
         root.keySuite[i].hotKey[keyLength].jumpName = "Move left" ;
         ++keyLength ;

         root.keySuite[i].hotKey[keyLength].button = BUTTON_RIGHT ;
         root.keySuite[i].hotKey[keyLength].jumpType = JUMPTYPE_FIXED ;
         root.keySuite[i].hotKey[keyLength].jumpName = "Move right" ;
         ++keyLength ;

         root.keySuite[i].hotKey[keyLength].button = BUTTON_ENTER ;
         root.keySuite[i].hotKey[keyLength].jumpType = JUMPTYPE_FIXED ;
         root.keySuite[i].hotKey[keyLength].jumpName = "last view" ;
         ++keyLength ;
         
         root.keySuite[i].hotKey[keyLength].button = BUTTON_ESC;
         root.keySuite[i].hotKey[keyLength].jumpType = JUMPTYPE_FIXED ;
         root.keySuite[i].hotKey[keyLength].jumpName = "last view" ;
         ++keyLength ;

         root.keySuite[i].hotKey[keyLength].button = BUTTON_F5;
         root.keySuite[i].hotKey[keyLength].jumpType = JUMPTYPE_FIXED ;
         root.keySuite[i].hotKey[keyLength].jumpName = "refresh" ;
         ++keyLength ;

         root.keySuite[i].hotKey[keyLength].button = BUTTON_H_LOWER;
         root.keySuite[i].hotKey[keyLength].jumpType = JUMPTYPE_FIXED ;
         root.keySuite[i].hotKey[keyLength].jumpName = "Help" ;
         ++keyLength ;

         root.keySuite[i].hotKey[keyLength].button = BUTTON_Q_LOWER;
         root.keySuite[i].hotKey[keyLength].jumpType = JUMPTYPE_FIXED ;
         root.keySuite[i].hotKey[keyLength].jumpName = "Quit" ;
         ++keyLength ;

      }
      catch( std::exception &e )
      {

         snprintf( errStrBuf, errStrLength,"%s", errStr );
         snprintf( errStr, errStrLength,
                   "%s addFixedHotKey failed , e.what():%s\n",
                   errStrBuf, e.what() ) ;

         goto error ;
      }
      root.keySuite[i].hotKeyLength = keyLength ;
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}


INT32 Event::findSourceFieldByDisplayName( const string DisplayName )
{
   INT32 rc = SDB_OK ;
   INT32 numOfSubWindow = 0 ;
   FieldStruct *Fixed = NULL ;
   FieldStruct *Mobile = NULL ;
   INT32 FixedLength = 0 ;
   INT32 MobileLength = 0 ;
   root.input.sortingField = NULLSTRING ;
   for( numOfSubWindow = 0; numOfSubWindow < root.input.activatedPanel ->value.numOfSubWindow; ++numOfSubWindow )
   {
      if( DISPLAYTYPE_DYNAMIC_SNAPSHOT ==
          root.input.activatedPanel ->value.subWindow[numOfSubWindow].displayType )
      {
         DisplayContent &displayContent = root.input.activatedPanel ->value.subWindow[numOfSubWindow].displayContent ;
         FieldStruct *Fixed = displayContent.dySnapshotOutPut.fixedField ;
         FieldStruct *Mobile = displayContent.dySnapshotOutPut.mobileField ;
         FixedLength = displayContent.dySnapshotOutPut.actualFixedFieldLength ;
         MobileLength = displayContent.dySnapshotOutPut.actualMobileFieldLength ;
         while( FixedLength > 0 )
         {
            --FixedLength ;
            if( DISPLAYMODECHOOSER[root.input.displayModeChooser] == ABSOLUTE )
            {
               if( DisplayName == Fixed[FixedLength].absoluteName )
               {
                  root.input.sortingField = Fixed[FixedLength].sourceField ;
                  goto done ;
               }
            }
            else if( DISPLAYMODECHOOSER[root.input.displayModeChooser] == DELTA )
            {
               if(  Fixed[FixedLength].canSwitch )
               {
                  if( DisplayName == Fixed[FixedLength].deltaName )
                  {
                     root.input.sortingField = Fixed[FixedLength].sourceField ;
                     goto done ;
                  }
               }
               else
               {
                  if( DisplayName == Fixed[FixedLength].absoluteName )
                  {
                     root.input.sortingField = Fixed[FixedLength].sourceField ;
                     goto done ;
                  }
               }
            }
            else if( DISPLAYMODECHOOSER[root.input.displayModeChooser] == AVERAGE )
            {
               if(  Fixed[FixedLength].canSwitch )
               {
                  if( DisplayName == Fixed[FixedLength].averageName )
                  {
                     root.input.sortingField = Fixed[FixedLength].sourceField ;
                     goto done ;
                  }
               }
               else
               {
                  if( DisplayName == Fixed[FixedLength].absoluteName )
                  {
                     root.input.sortingField = Fixed[FixedLength].sourceField ;
                     goto done ;
                  }
               }
            }
         }
         while( MobileLength> 0 )
         {
            --MobileLength ;
            if( DISPLAYMODECHOOSER[root.input.displayModeChooser] == ABSOLUTE )
            {
               if( DisplayName == Mobile[MobileLength].absoluteName )
               {
                  root.input.sortingField = Mobile[MobileLength].sourceField ;
                  goto done ;
               }
            }
            else if( DISPLAYMODECHOOSER[root.input.displayModeChooser] == DELTA )
            {
               if(  Fixed[MobileLength].canSwitch )
               {
                  if( DisplayName == Mobile[MobileLength].deltaName )
                  {
                     root.input.sortingField = Mobile[MobileLength].sourceField ;
                     goto done ;
                  }
               }
               else
               {
                  if( DisplayName == Mobile[MobileLength].absoluteName )
                  {
                     root.input.sortingField = Mobile[MobileLength].sourceField ;
                     goto done ;
                  }
               }
            }
            else if( DISPLAYMODECHOOSER[root.input.displayModeChooser] == AVERAGE )
            {
               if(  Fixed[MobileLength].canSwitch )
               {
                  if( DisplayName == Mobile[MobileLength].averageName )
                  {
                     root.input.sortingField = Mobile[MobileLength].sourceField ;
                     goto done ;
                  }
               }
               else
               {
                  if( DisplayName == Mobile[MobileLength].absoluteName )
                  {
                     root.input.sortingField = Mobile[MobileLength].sourceField ;
                     goto done ;
                  }
               }
            }
         }
         goto done ;
      }
   }
done :
   return rc ;
error :
   rc = SDB_ERROR;
   goto done ;
}


INT32 Event::buttonManagement( INT64 key ,BOOLEAN isFirstStart )
{
   INT32 rc = SDB_OK ;
   fd_set fds ;
   INT32 maxfd = STDIN + 1 ;
   KeySuite* keySuite = NULL ;
   HotKey* hotKey = NULL ;
   INT32 row = 0 ;
   INT32 col = 0 ;
   CHAR inputBuf[128] ;
   INT32 filterNum = 0 ;
   string note = NULLSTRING ;
   string displayName = NULLSTRING ; // use it when sorting
   HeadTailMap *header = NULL ;
   HeadTailMap *footer = NULL ;
   BodyMap* activatedPanel = NULL ;
   const INT32 bufLength = 256 ;
   INT64 buf[bufLength] ;
   rc = SDBTOP_MEMSET( buf, 0, bufLength ) ;
   if( rc )
   {
      goto error ;
   }
   rc = getActivatedKeySuite( &keySuite ) ;
   if( rc || !keySuite )
   {
      goto error ;
   }
   if( 0 > key )
   {
      goto error ;
   }
   else
   {
      for( INT32 i = 0; i < keySuite->hotKeyLength; ++i )
      {
         if( keySuite->hotKey[i].button == key )
         {
            hotKey = &keySuite->hotKey[i] ;
            break;
         }
      }
      if( !hotKey )
      {
         if( !isFirstStart )
         {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE );
         }
         goto done ;
      }
      else
      {
         if( hotKey->jumpType == JUMPTYPE_PANEL )
         {
            rc = assignActivatedPanelByLabelName( &root.input.activatedPanel, hotKey->jumpName ) ;
            if( rc )
            {

               snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
               snprintf( errStr, errStrLength,
                         "%s assignActivatedPanelByLabelName failed\n",
                         errStrBuf ) ;
  
               goto error ;
            }
            root.input.displayModeChooser = 0 ;
            root.input.snapshotModeChooser = GLOBAL ;
            root.input.forcedToRefresh_Global = REFRESH ;
            root.input.sortingField = NULLSTRING ;
            root.input.sortingWay = NULLSTRING ;
            root.input.filterNumber = 0 ;
            root.input.filterCondition= NULLSTRING;
            root.input.isFirstGetSnapshot = TRUE ;
            root.input.isFirstGetAbsolute = TRUE ;
            root.input.isFirstGetDelta = TRUE ;
            root.input.isFirstGetAverage = TRUE ;
            goto done ;
         }
         else if( hotKey->jumpType == JUMPTYPE_FIXED )
         {
            if( hotKey->button >= 256 )
            {
               if( BUTTON_LEFT == hotKey->button )//left
               {
                  --root.input.fieldPosition ; 
                  if( root.input.fieldPosition < 0 )
                  {
                     root.input.fieldPosition = 0 ;
                  }

                  root.input.forcedToRefresh_Local = REFRESH ;
                  goto done ;
               }
               else if( BUTTON_RIGHT == hotKey->button )//right
               {
                  ++root.input.fieldPosition ; 
                  root.input.forcedToRefresh_Local = REFRESH ;
                  goto done ;
               }
               else if( BUTTON_F5 == hotKey->button )
               {
                  root.input.forcedToRefresh_Global= REFRESH ;
                  goto done ;
               }
               else
               {
                  goto done ;
               }
            }
            else if( hotKey->button == BUTTON_Q_LOWER )
            {
               rc = SDB_SDBTOP_DONE ;
               goto done ;
            }
            else if( BUTTON_ENTER == hotKey->button )
            {
               root.input.forcedToRefresh_Local= REFRESH ;
               goto done ;
            }
            else if( BUTTON_ESC == hotKey->button )
            {
               root.input.forcedToRefresh_Local= REFRESH ;
               goto done ;
            }
            else if( BUTTON_TAB == hotKey->button )
            {
               if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
               {
                  goto done;
               }
               ++root.input.displayModeChooser ;
               root.input.displayModeChooser %= DISPLAYMODENUMBER ;
               root.input.forcedToRefresh_Local = REFRESH ;
               goto done;
            }
            else if( BUTTON_H_LOWER == hotKey->button )
            {
               rc = assignActivatedPanel( &activatedPanel, BODYTYPE_HELP_DYNAMIC) ;
               rc = getActivatedHeadTailMap( activatedPanel, &header, &footer) ;
               if( rc )
               {

                  snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                  snprintf( errStr, errStrLength,
                            "%s buttonManagement faild,getActivatedHeadTailMap failed\n",
                            errStrBuf ) ;
        
                  goto error ;
               }
               clear() ;
               rc = refreshHeadTail( header ) ;
               if( rc )
               {
                  goto error ;
               }
               rc = refreshBody( activatedPanel ) ;
               if( rc )
               {
                  goto error ;
               }
               rc = refreshHeadTail( footer ) ;
               if( rc )
               {
                  goto error ;
               }
               refresh() ;
               maxfd = STDIN + 1 ;
               FD_ZERO ( &fds ) ;
               FD_SET ( STDIN, &fds ) ;
               rc = select ( maxfd, &fds, NULL, NULL, NULL ) ;
               if( rc < 0 )
               {
                  rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
                  goto done ;
               }
               else if( rc > 0 )
               {
                  SDBTOP_MEMSET( buf, 0, bufLength) ;
                  read(STDIN, buf, bufLength ) ;
                  rc = getTopKey_TOP( buf, bufLength, key) ;
                  if( rc )
                  {
                     goto error ;
                  }
                  rc = buttonManagement( key, FALSE ) ;
                  goto done ;
               }
               else
               {
                  snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
                  snprintf( errStr, errStrLength,
                            "%s buttonManagement faild,"
                            "select ( maxfd, &fds, NULL, NULL, NULL) failed\n",
                            errStrBuf ) ;
                  goto error ;
               }
            }
            else
               goto done ;
         }
         else if( JUMPTYPE_GLOBAL== hotKey->jumpType )
         {
            if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
            {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
               goto done ;
            }
            root.input.snapshotModeChooser = GLOBAL ;
            root.input.forcedToRefresh_Global = REFRESH ;
            goto done ;
         }
         else if( JUMPTYPE_GROUP == hotKey->jumpType )
         {
            if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
            {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
               goto done ;
            }
            note = "please input the group name:" ;
            getmaxyx( stdscr, row, col ) ;
            curs_set( 2 ) ;
            SDBTOP_MEMSET( inputBuf, 0, 128) ;
            
            move( row - 1, 0 ) ;
            clrtobot() ; //clear screen from the position of cursor to the end of screen
            
            nocbreak() ;
            echo() ;
            mvprintw( row - 1 , ( col - note.length() ) / 2, note.c_str() ) ;
            getnstr( inputBuf, 128 ) ;
            cbreak() ;
            noecho() ;
            root.input.groupName = inputBuf ;
            trim( root.input.groupName ) ;
            root.input.snapshotModeChooser = GROUP ;
            root.input.forcedToRefresh_Global = REFRESH ;
            curs_set( 0 ) ;
            goto done ;
         }
         else if( JUMPTYPE_NODE == hotKey->jumpType )
         {
            if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
            {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
               goto done ;
            }
            note = "please input the HostName:svcname : " ;
            getmaxyx( stdscr, row, col ) ;
            curs_set( 2 ) ;
            SDBTOP_MEMSET( inputBuf, 0, 128 ) ;
            
            move( row - 1, 0 ) ;
            clrtobot() ; //clear screen from the position of cursor to the end of screen
            
            nocbreak() ;
            echo() ;
            mvprintw( row - 1 , ( col - note.length() ) / 2, note.c_str() ) ;
            getnstr( inputBuf, 128 ) ;
            cbreak() ;
            noecho() ;
            root.input.nodeName = inputBuf ;
            trim( root.input.nodeName ) ;
            root.input.snapshotModeChooser = NODE ;
            root.input.forcedToRefresh_Global = REFRESH ;
            curs_set( 0 ) ;
            goto done ;
         }
         else if( JUMPTYPE_ASC== hotKey->jumpType )
         {
            if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
            {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
               goto done ;
            }
            note = "please input the displayName which need order by asc : " ;
            getmaxyx( stdscr, row, col ) ;
            curs_set( 2 ) ;
            SDBTOP_MEMSET( inputBuf, 0, 128) ;
            
            move( row - 1, 0 ) ;
            clrtobot() ; //clear screen from the position of cursor to the end of screen
            
            nocbreak() ;
            echo() ;
            mvprintw( row - 1 , ( col - note.length() ) / 2, note.c_str() ) ;
            getnstr( inputBuf, 128 ) ;
            cbreak() ;
            noecho() ;
            root.input.sortingWay = SORTINGWAY_ASC ;
            displayName = inputBuf ;
            trim( displayName ) ;
            findSourceFieldByDisplayName( displayName ) ;
            root.input.forcedToRefresh_Global = REFRESH ;
            curs_set( 0 ) ;
            goto done ;
         }
         else if( JUMPTYPE_DESC == hotKey->jumpType )
         {
            if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
            {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
               goto done ;
            }
            note = "please input the displayName which need order by desc : ";
            getmaxyx( stdscr, row, col ) ;
            curs_set( 2 );
            SDBTOP_MEMSET( inputBuf, 0, 128);
            
            move( row - 1, 0 );
            clrtobot(); //clear screen from the position of cursor to the end of screen
            
            nocbreak() ;
            echo() ;
            mvprintw( row - 1 , ( col - note.length() ) / 2, note.c_str() ) ;
            getnstr( inputBuf, 128 );
            cbreak() ;
            noecho() ;
            root.input.sortingWay = SORTINGWAY_DESC ;
            displayName = inputBuf ;
            trim( displayName ) ;
            findSourceFieldByDisplayName( displayName ) ;
            root.input.forcedToRefresh_Global = REFRESH ;
            curs_set( 0 ) ;
            goto done ;
         }
         else if( JUMPTYPE_FILTER_CONDITION == hotKey->jumpType )
         {
            if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
            {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
               goto done ;
            }
            note = "please input the filter condition : ";
            getmaxyx( stdscr, row, col ) ;
            curs_set( 2 ) ;
            SDBTOP_MEMSET( inputBuf, 0, 128 ) ;
            
            move( row - 1, 0 ) ;
            clrtobot() ; //clear screen from the position of cursor to the end of screen
            
            nocbreak() ;
            echo() ;
            mvprintw( row - 1 , ( col - note.length() ) / 2, note.c_str() ) ;
            getnstr( inputBuf, 128 ) ;
            cbreak() ;
            noecho() ;
            root.input.filterCondition = inputBuf ;
            trim( root.input.filterCondition ) ;
            root.input.forcedToRefresh_Global = REFRESH ;
            curs_set( 0 ) ;
            goto done ;
         }
         else if( JUMPTYPE_NO_FILTER_CONDITION == hotKey->jumpType )
         {
            if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
            {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
               goto done ;
            }
            root.input.filterCondition = NULLSTRING;
            root.input.forcedToRefresh_Global = REFRESH ;
            goto done ;
         }
         else if( JUMPTYPE_FILTER_NUMBER== hotKey->jumpType )
         {
            if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
            {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
               goto done ;
            }
            note = "please input the filter number : ";
            getmaxyx( stdscr, row, col ) ;
            curs_set( 2 ) ;
            SDBTOP_MEMSET( inputBuf, 0, 128 ) ;
            
            move( row - 1, 0 ) ;
            clrtobot() ; //clear screen from the position of cursor to the end of screen
            
            nocbreak() ;
            echo() ;
            mvprintw( row - 1 , ( col - note.length() ) / 2, note.c_str() ) ;
            getnstr( inputBuf, 128 ) ;
            cbreak() ;
            noecho() ;
            displayName = inputBuf ;
            trim( displayName ) ;
            rc = SDBTOP_strTOnum( displayName.c_str(), filterNum ) ;
            if( rc )
            {
              filterNum = 0 ;
            }
            root.input.filterNumber += filterNum;
            root.input.forcedToRefresh_Global = REFRESH ;
            curs_set( 0 ) ;
            goto done ;
         }
         else if( JUMPTYPE_NO_FILTER_NUMBER== hotKey->jumpType )
         {
            if( BODYTYPE_NORMAL != root.input.activatedPanel->bodyPanelType )
            {
               rc = buttonManagement( BUTTON_H_LOWER, FALSE ) ;
               goto done ;
            }
            if( 0 == root.input.filterNumber )
               goto done ;
            root.input.filterNumber = 0;
            root.input.forcedToRefresh_Global = REFRESH ;
            goto done ;
         }
         else
            goto done ;
      }
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}

INT32 Event::runSDBTOP( )
{
   INT32 rc = SDB_OK ;
   INT64 key = 0 ;
   fd_set fds ;
   struct timeval timeout ;
   INT32 maxfd  = STDIN + 1 ;
   const INT32 bufLength = 256 ;
   INT64 buf[bufLength] ;
   SDBTOP_MEMSET( buf, 0, bufLength ) ;
   root.input.forcedToRefresh_Global = NOTREFRESH ;
   root.input.forcedToRefresh_Local= NOTREFRESH ;
   rc = addFixedHotKey() ;
   if( rc )
   {

      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s addFixedHotKey failed\n",
                errStrBuf ) ;
      goto error ;
   }
   rc = assignActivatedPanel( &root.input.activatedPanel, BODYTYPE_MAIN) ;
   if( rc )
   {

      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s assignActivatedPanel failed\n", errStrBuf ) ;

      goto error ;
   }
   root.input.hostname = hostname ;
   root.input.serviceName = serviceName ;
   root.input.usrName = usrName ;
   root.input.password = password ;
   rc = coord.connect( root.input.hostname.c_str(), root.input.serviceName.c_str(), root.input.usrName.c_str(), root.input.password.c_str() ) ;
   if( rc )
   {

      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s can't connect to the coord: %s, %s, %s, %s, rc =%d\n",
                errStrBuf, root.input.hostname.c_str(), root.input.serviceName.c_str(), root.input.usrName.c_str(), root.input.password.c_str(), rc ) ;
      goto error ;
   }
   root.input.displayModeChooser = 0 ;
   initAllColourPairs() ;
   HeadTailMap* header ;
   HeadTailMap* footer ;
   while( 1 )
   {
      rc = getActivatedHeadTailMap( root.input.activatedPanel, &header, &footer) ;
      if( rc )
      {

         snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
         snprintf( errStr, errStrLength,
                   "%s getActivatedHeadTailMap failed\n", errStrBuf ) ;
         goto error ;
      }
      if( root.input.activatedPanel->bodyPanelType == BODYTYPE_NORMAL )
      {
         rc = getCurSnapshot() ;
      }
      if( rc )
      {
         goto error ;
      }
      clear() ;
      rc = refreshHeadTail( header ) ;
      if( rc )
      {
         goto error ;
      }
      rc = refreshBody( root.input.activatedPanel ) ;
      if( rc )
      {
         goto error ;
      }
      rc = refreshHeadTail( footer ) ;
      if( rc )
      {
         goto error ;
      }
      refresh() ;
         
      timeout.tv_sec = root.input.refreshInterval ;
      timeout.tv_usec = 0 ;
      while( 1 )
      {
         FD_ZERO ( &fds ) ;
         FD_SET ( STDIN, &fds ) ;
         rc = select ( maxfd, &fds, NULL, NULL, &timeout ) ;
         if( rc < 0 ) //when window change the size // but why can't just break? it's question
         {
            clear() ;
            rc = refreshHeadTail( header ) ;
            if( rc )
            {
               goto error ;
            }
            rc = refreshBody( root.input.activatedPanel ) ;
            if( SDB_OK != rc )
            {
               goto error ;
            }
            rc = refreshHeadTail( footer ) ;
            if( rc )
            {
               goto error ;
            }
            refresh() ;
            break ;
         }
         else if( rc > 0 )
         {
            if ( FD_ISSET ( STDIN, &fds ) ) 
            {
               SDBTOP_MEMSET( buf, 0, bufLength) ;
               read( STDIN, buf, bufLength ) ;
               rc = getTopKey_TOP( buf, bufLength, key) ;
               if( rc )
               {
                  goto error ;
               }
               rc = buttonManagement( key, TRUE ) ;
               if( rc )
               {
                  if( SDB_SDBTOP_DONE == rc )
                  {
                     goto done ;
                  }
                  goto error ;
               }
            }
         }
         else //time out
         {
            break ;
         }
         if( root.input.forcedToRefresh_Global == REFRESH )
         {
            root.input.forcedToRefresh_Global = NOTREFRESH ;
            break ;
         }
         if( root.input.forcedToRefresh_Local == REFRESH )
         {
            clear() ;
            rc = refreshHeadTail( header ) ;
            if( rc )
            {
               goto error ;
            }
            rc = refreshBody( root.input.activatedPanel ) ;
            if( rc )
            {
               goto error ;
            }
            rc = refreshHeadTail( footer ) ;
            if( rc )
            {
               goto error ;
            }
            refresh() ;
            root.input.forcedToRefresh_Local = NOTREFRESH ;
         }
         rc = SDB_OK ;
      }
   }
done :
   return rc ;
error :
   rc = SDB_ERROR ;
   goto done ;
}


void init ( po::options_description &desc )
{
   ADD_PARAM_OPTIONS_BEGIN ( desc )
      COMMANDS_OPTIONS
   ADD_PARAM_OPTIONS_END
}

void displayArg ( po::options_description &desc )
{
   std::cout << desc << std::endl ;
}

// resolve input argument
INT32 resolveArgument ( po::options_description &desc, INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   CHAR actionString[BUFFERSIZE] = {0} ;
   po::variables_map vm ;
   try
   {
      po::store ( po::parse_command_line ( argc, argv, desc ), vm ) ;
      po::notify ( vm ) ;
   }
   catch ( po::unknown_option &e )
   {
      std::cerr <<  "Unknown argument: "
                << e.get_option_name () << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch ( po::invalid_option_value &e )
   {
      std::cerr <<  "Invalid argument: "
                << e.get_option_name () << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch( po::error &e )
   {
      std::cerr << e.what () << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   if ( vm.count ( OPTION_HELP ) )
   {
      displayArg ( desc ) ;
      rc = SDB_PMD_HELP_ONLY ;
      goto done ;
   }

   if( vm.count( OPTION_CONFPATH ) )
   {
      confPath = vm[OPTION_CONFPATH].as<string>();
   }
   else
   {
      confPath = SDBTOP_DEFAULT_CONFPATH ;
   }

   if( vm.count( OPTION_HOSTNAME ) )
   {
      hostname= vm[OPTION_HOSTNAME].as<string>();
   }
   else
   {
      std::cout << "hostname must be specified\n" << std::endl ;
      displayArg ( desc ) ;
      rc = SDB_PMD_HELP_ONLY ;
      goto done ;
   }
   
   if( vm.count( OPTION_SERVICENAME) )
   {
      serviceName = vm[OPTION_SERVICENAME].as<string>();
   }
   else
   {
      std::cout << "servicename must be specified\n" << std::endl ;
      displayArg ( desc ) ;
      rc = SDB_PMD_HELP_ONLY ;
      goto done ;
   }
   
   if( vm.count( OPTION_USRNAME) )
   {
      usrName = vm[OPTION_USRNAME].as<string>();
   }
   else
   {
      usrName = NULLSTRING ;
   }
   
   if( vm.count( OPTION_PASSWORD) )
   {
      password= vm[OPTION_PASSWORD].as<string>();
   }
   else
   {
      password = NULLSTRING ;
   }
   
done :
   return rc ;
error :
   goto done ;
}


INT32 main( INT32 argc, CHAR **argv)
{
   fd_set fds ;
   INT32 rc = 0 ; 
   Event sdbtop ;
   INT32 maxfd  = STDIN + 1 ;
   po::options_description desc ( "Command options" ) ;
   init ( desc ) ;
   rc = resolveArgument ( desc, argc, argv ) ;
   if ( rc )
   {
      if ( SDB_PMD_HELP_ONLY != rc )
      {
         std::cerr<< "Error: Invalid arguments\n" << std::endl ;
         displayArg ( desc ) ;
      }
      goto done ;
   }
   errStr = ( CHAR * )malloc( errStrLength * sizeof( CHAR ) ) ;
   if( !errStr )
   {
      goto error ;
   }
   errStrBuf = ( CHAR * )malloc( errStrLength * sizeof( CHAR ) ) ;
   if( !errStrBuf )
   {
      goto error ;
   }
   sdbtop.SDBTOP_MEMSET( errStr, 0, errStrLength ) ;
   sdbtop.SDBTOP_MEMSET( errStrBuf, 0, errStrLength ) ;
   initscr() ;
   if( FALSE == has_colors() )
   { 
      snprintf( errStrBuf, errStrLength,"%s", errStr ) ;
      snprintf( errStr, errStrLength,
                "%s You terminal does not support color\n", errStrBuf ) ;
      goto error ;
   }
   start_color() ;
   cbreak() ;
   keypad( stdscr, FALSE ) ;
   noecho() ;
   curs_set( 1 ) ;
   rc = sdbtop.readConfiguration( ) ;
   if( rc )
   {
      goto error ;
   }
   
   rc = sdbtop.runSDBTOP( ) ;
   if( rc && SDB_SDBTOP_DONE != rc )
   {
      snprintf( errStrBuf, errStrLength,"%s",  errStr ) ;
      snprintf( errStr, errStrLength,  "%s can't runSDBTOP\n", errStrBuf ) ;
      goto error ;
   }
   curs_set( 1 ) ;
   endwin() ;
done :
   free( errStr ) ;
   free( errStrBuf ) ;
   return rc ;
error :
   rc = SDB_ERROR ;
   curs_set( 1 ) ;
   endwin() ;
   std::cerr << errStr << std::endl;
   goto done ;
}
