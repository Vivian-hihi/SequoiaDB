
// MarkdownEditorView.cpp : CMarkdownEditorView 类的实现
//

#include "stdafx.h"

#include "Util.h"
#include <string>
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MarkdownEditor.h"
#endif

#include "MarkdownEditorDoc.h"
#include "MarkdownEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMarkdownEditorView

IMPLEMENT_DYNCREATE(CMarkdownEditorView, CHtmlView)

BEGIN_MESSAGE_MAP(CMarkdownEditorView, CHtmlView)

END_MESSAGE_MAP()

// CMarkdownEditorView 构造/析构

CMarkdownEditorView::CMarkdownEditorView()
{
	// TODO: 在此处添加构造代码
	_bFirstNavigate = true;
   _type = 1 ;
	initCSS();
}

CMarkdownEditorView::~CMarkdownEditorView()
{
}

BOOL CMarkdownEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CHtmlView::PreCreateWindow(cs);
}

void CMarkdownEditorView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();

}


// CMarkdownEditorView 诊断

#ifdef _DEBUG
void CMarkdownEditorView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CMarkdownEditorView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}

CMarkdownEditorDoc* CMarkdownEditorView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMarkdownEditorDoc)));
	return (CMarkdownEditorDoc*)m_pDocument;
}
#endif //_DEBUG


// CMarkdownEditorView 消息处理程序

void CMarkdownEditorView::NavigateHTML(const string& strHtml)
{
	IDispatch* pDoc = GetHtmlDocument();
	if(NULL == pDoc)
		return;
	// 取得文档中的IPersistStreamInit对象
    CComPtr<IHTMLDocument2> pHtmlDoc;
	HRESULT hr = pDoc ->QueryInterface(IID_IHTMLDocument2, (void**)&pHtmlDoc);
    if (FAILED(hr))
        return;

	BSTR bstr = _com_util::ConvertStringToBSTR(strHtml.c_str());
	// Creates a new one-dimensional array
	SAFEARRAY *psaStrings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	if (psaStrings == NULL) {
		pHtmlDoc->close();
		return;
	}
	VARIANT *param;
	hr = SafeArrayAccessData(psaStrings, (LPVOID*)&param);
	param->vt = VT_BSTR;
	param->bstrVal = bstr;
	hr = SafeArrayUnaccessData(psaStrings);
	hr = pHtmlDoc->write(psaStrings);
	// SafeArrayDestroy calls SysFreeString for each BSTR
	if (psaStrings != NULL) {
		SafeArrayDestroy(psaStrings);
		pHtmlDoc->close();
	}
}

CComPtr<IHTMLTextContainer> getContainer(IDispatch* pDisp){
	if(NULL == pDisp)
		return NULL;
		CComPtr<IHTMLDocument2> pDocument2 = NULL; 
            if (S_OK == pDisp->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&pDocument2)) 
            { 
                CComPtr<IHTMLElement> pElement = NULL; 
                if (S_OK == pDocument2->get_body(&pElement)) 
                { 
                    CComPtr<IHTMLTextContainer> pTextContainer = NULL; 
                    if (S_OK == pElement->QueryInterface(IID_IHTMLTextContainer, (LPVOID*)&pTextContainer)) 
                    { 
						return pTextContainer;
                    } 
                }                 
           } 
		return NULL;
}
float getScrollTop(IDispatch* pDisp)
{
    long scrollTop;
    long height;
    /*
	CComPtr<IHTMLTextContainer> pTextContainer = getContainer(pDisp);
    if (pTextContainer &&  S_OK == pTextContainer->get_scrollTop(&scrollTop) ) 
    {
		long height;
		pTextContainer->get_scrollHeight(&height);
		return ((float)scrollTop)/height ;
    } */
    
    CComPtr<IHTMLDocument2> pDocument2 = NULL; 
    if( S_OK == pDisp->QueryInterface( IID_IHTMLDocument2, (LPVOID*)&pDocument2 ) )
    {
       CComQIPtr<IHTMLDocument3> pDoc3 = pDocument2 ;
       CComPtr<IHTMLElement> pElement = NULL; 
       pDoc3->get_documentElement( &pElement ) ;
       CComQIPtr<IHTMLElement2> pElement2 = pElement ;
       pElement2->get_scrollTop( &scrollTop ) ;
       pElement2->get_scrollHeight(&height);
       return ((float)scrollTop)/height ;
    }
	return 0.0;
}
void setScrollTop(IDispatch* pDisp, float scrollPercent)
{
   /*
	CComPtr<IHTMLTextContainer> pTextContainer = getContainer(pDisp);
    if (pTextContainer)
    {
		long top,height;
		pTextContainer->get_scrollTop(&top);
		pTextContainer->get_scrollHeight(&height);
		pTextContainer->put_scrollTop((long)(scrollPercent * height));
    } */
   long height;
    CComPtr<IHTMLDocument2> pDocument2 = NULL; 
    if( S_OK == pDisp->QueryInterface( IID_IHTMLDocument2, (LPVOID*)&pDocument2 ) )
    {
       CComQIPtr<IHTMLDocument3> pDoc3 = pDocument2 ;
       CComPtr<IHTMLElement> pElement = NULL; 
       pDoc3->get_documentElement( &pElement ) ;
       CComQIPtr<IHTMLElement2> pElement2 = pElement ;
       pElement2->get_scrollHeight(&height);
       pElement2->put_scrollTop( (long)(scrollPercent * height) ) ;
    }
}
void CMarkdownEditorView::OnUpdate(CView* pSender, LPARAM /*lHint*/lParam, CObject* /*pHint*/)
{
	if(_bFirstNavigate){
		_bFirstNavigate = false;
		Navigate2(_T("about:blank"),NULL,NULL);
		//return;
	}
	if(!(lParam & LPARAM_Update))
		return;
	float scrollTop = 0;
	IDispatch* pDisp =GetHtmlDocument();
	
	if(pSender != NULL){
		scrollTop = getScrollTop(pDisp);
	}
	const string& str = GetDocument()->getText();	

	UpdateMd(str);
	if(lParam & LPARAM_MoveEnd){
		scrollTop = 1.0;
	}
	if(pSender != NULL){
		setScrollTop(pDisp,scrollTop);
	}



	// TODO: 在此添加专用代码和/或调用基类
}

void CMarkdownEditorView::setHtmlScroll( float scrollTop )
{
   IDispatch* pDisp = GetHtmlDocument() ;
	setScrollTop(pDisp,scrollTop);
}

void CMarkdownEditorView::initCSS(){
	string strUserCss = Util::GetExePath() + "user.css";
	if(PathFileExists(strUserCss.c_str())){
		_strCSS = Util::ReadStringFile(strUserCss.c_str());
	}else{
		Util::LoadStringRes(IDR_CSS,"CSS",_strCSS); 
	}
}

string&  replaceImgSrc(string& str, string path)
{
	if (path.size() == 0)
		return str;
	string old_value = "<img src=\"";
	string new_value = "<img src=\"" + path;
	for (string::size_type pos(0); pos != string::npos; pos += old_value.length())   {
		if ((pos = str.find(old_value, pos)) != string::npos){
			const char* start = str.c_str() + pos + old_value.length();
			if (strnicmp(start, "http://", 7) != 0 && strnicmp(start, "https://", 8) != 0)
				str.replace(pos, old_value.length(), new_value);
		}
		else   
			break;
	}
	return   str;
}

const string WWW_CSS = "\
<style type=\"text/css\">\r\n\
.code{ font-size:12px; background-color:#E7E5DC; width:auto; padding-top:1px solid #eee; text-align:left; font-family:\"Consolas\" , \"Courier New\" , Courier, mono, serif; margin:10px 0 0 0; }\r\n\
.code > ol{ font-size:12px; list-style:decimal; margin:0px 0px 1px 45px !important; background-color:#fff; padding:0px; color:#5C5C5C; width:auto; }\r\n\
.code > ol > li{ font-size:12px; background-color:#FFF; list-style:decimal-leading-zero; list-style-position:outside !important; border-left:3px solid #6CE26C; padding:2px 3px 2px 10px !important; margin:0 !important; line-height:150%; word-wrap:break-word; }\r\n\
.code > ol > li:nth-child(2n+1){ background-color:#EEE; }\r\n\
.code .alt { background-color:#EEE; color:inherit; }\r\n\
.code .alt > span{ font-weight:bold; }\r\n\
..tab-wrap{margin:40px 10px}.tab-wrap>ul{margin:0;padding:0;font-size:0}.tab-wrap>.tab-menu>li{font-size:16px;position:relative;background-color:#f2f2f2;border-top:1px solid #505050;border-bottom:1px solid #505050;border-right:1px solid #505050;display:inline-block;padding:20px 40px;cursor:pointer;z-index:0;font-weight:bold;color:#434343;top:1px}.tab-wrap>.tab-menu>li:first-of-type{border-left:1px solid #505050;border-radius:5px 0 0 0}.tab-wrap>.tab-menu>li:last-of-type{border-radius:0 5px 0 0}.tab-wrap>.tab-menu>li:hover{color:#464646}.tab-wrap>.tab-menu>li.active{background-color:#FFF;opacity:1;border-top:4px solid #333;padding:18px 40px 20px 40px;border-bottom:0;top:2px;color:#009}.tab-wrap>.tab-cont>div{border:1px solid #505050;box-sizing:border-box;width:100%;padding:25px;min-height:200px;display:none;border-radius:0 5px 5px 5px;box-shadow:rgba(0,0,0,0.2) 4px 4px 5px 0}.tab-wrap>.tab-cont>.active{display:block}\r\n\
.tab-wrap>.tab-menu>.first-tab {border-left: 1px solid #505050;border-radius: 5px 0 0 0}\r\n\
</style>\r\n\
\r\n\
<script style=\"text/javascript\">\r\n\
   function getElementsByClassName(node, className)\r\n\
   {\r\n\
        var results = [],\r\n\
        elems = node.getElementsByTagName(\"*\");\r\n\
		for (var i = 0, len = elems.length; i < len; i++) {\r\n\
			if (elems[i].className.indexOf(className) != -1) {\r\n\
				results[results.length] = elems[i];\r\n\
			}\r\n\
		}\r\n\
		return results;\r\n\
	}\r\n\
   function getChildByTagName( node, tag )\r\n\
   {\r\n\
	 var results = [];\r\n\
	 var child = node.childNodes;\r\n\
	 for (var i = 0; i < child.length; ++i)\r\n\
	 {\r\n\
	 	if (typeof(child[i].tagName) == 'string' &&\r\n\
			child[i].tagName.toLowerCase() == tag.toLowerCase())\r\n\
		{\r\n\
			results.push(child[i]);\r\n\
		}\r\n\
	 }\r\n\
	 return results;\r\n\
   }\r\n\
\r\n\
	window.onload = function() {\r\n\
		function initTabEvent(tabBox)\r\n\
		{\r\n\
			var contBox = getElementsByClassName(tabBox, 'tab-cont')[0];\r\n\
			var ulBox = getChildByTagName( tabBox, 'ul' )[0] ;\r\n\
			var liBoxes = getChildByTagName( ulBox, 'li' ) ;\r\n\
			var divBoxes = getChildByTagName( contBox, 'div' ) ;\r\n\
\r\n\
			function onTabTitleClick()\r\n\
			{\r\n\
				for (var n = 0; n < liBoxes.length; n++)\r\n\
				{\r\n\
					liBoxes[n].className = n == 0 ? 'first-tab' : '';\r\n\
					divBoxes[n].className = '';\r\n\
				}\r\n\
\r\n\
				this.className = this.index == 0 ? 'active first-tab' : 'active';\r\n\
				divBoxes[this.index].className = 'active';\r\n\
			}\r\n\
\r\n\
			for (var i = 0; i < liBoxes.length; i++)\r\n\
			{\r\n\
				liBoxes[i].index = i;\r\n\
				liBoxes[i].onclick = onTabTitleClick;\r\n\
			}\r\n\
		}\r\n\
\r\n\
		var tabBoxes = getElementsByClassName(document, 'tab-wrap');\r\n\
		for (var i = 0; i < tabBoxes.length; ++i)\r\n\
		{\r\n\
			initTabEvent(tabBoxes[i]); \r\n\
		}\r\n\
		\r\n\
      function htmlEncode( str )\r\n\
      {\r\n\
          str = str + '' ;\r\n\
          if( str.length == 0 ) return '' ;\r\n\
          var s = str.replace( /&/g, \"&amp;\" ) ;\r\n\
          s = s.replace( /</g, \"&lt;\" ) ;\r\n\
          s = s.replace( />/g, \"&gt;\" ) ;\r\n\
          s = s.replace( / /g, \"&nbsp;\" ) ;\r\n\
          s = s.replace( /'/g, \"&#39;\" ) ;\r\n\
          s = s.replace( /\\\"/g, \"&quot;\" ) ;\r\n\
          return s ;\r\n\
      }\r\n\
      function htmlDecode( str )\r\n\
      {\r\n\
      	var s = '' ;\r\n\
      	str = str + '' ;\r\n\
      	if( str.length ==0 ) return '' ;\r\n\
      	s = str.replace( /&amp;/g, \"&\" ) ;\r\n\
      	s = s.replace( /&lt;/g, \"<\" ) ;\r\n\
      	s = s.replace( /&gt;/g, \">\" ) ;\r\n\
      	s = s.replace( /&nbsp;/g, \" \" ) ;\r\n\
      	s = s.replace( /&#39;/g, \"\'\" ) ;\r\n\
      	s = s.replace( /&quot;/g, \"\\\"\" ) ;\r\n\
      	s = s.replace( /<br>/g, \"\\n\" ) ;\r\n\
      	return s ;\r\n\
      }\r\n\
      var codeList = document.getElementsByTagName( 'pre' ) ;\r\n\
      var len = codeList.length ;\r\n\
\r\n\
      for( var i = 0; i < len; ++i )\r\n\
      {\r\n\
         if( codeList[i] && codeList[i].parentNode )\r\n\
         {\r\n\
            codeList[i].style.display = 'none' ;\r\n\
            var str = htmlDecode( codeList[i].innerHTML ) ;\r\n\
            var strArr = str.split( String.fromCharCode( 10 ) ) ;\r\n\
            var len2 = strArr.length ;\r\n\
            var div = document.createElement( 'div' ) ;\r\n\
            var ol = document.createElement( 'ol' ) ;\r\n\
            div.className = 'code' ;\r\n\
            for( var k = 0; k < len2; ++k )\r\n\
            {\r\n\
               var li = document.createElement( 'li' ) ;\r\n\
               li.innerHTML = htmlEncode( strArr[k] ) ;\r\n\
               if( k % 2 === 0 )\r\n\
               {\r\n\
                  li.className = 'alt' ;\r\n\
               }\r\n\
               ol.appendChild( li ) ;\r\n\
            }\r\n\
            div.appendChild( ol ) ;\r\n\
            codeList[i].parentNode.insertBefore( div, codeList[i] ) ;\r\n\
         }\r\n\
      }\r\n\
   }\r\n\
</script>" ;
//<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">
//<meta http-equiv=\"X-UA-Compatible\"content=\"IE=9; IE=8; IE=7; IE=EDGE\">
const string HTML_TMPL = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"><html><head><meta http-equiv=\"X-UA-Compatible\"content=\"IE=9; IE=8; IE=7; IE=EDGE\"><style type=\"text/css\">{{0}}</style></head><body>{{1}}{{2}}</body></html>";

string CMarkdownEditorView::GetMdHtml(const string& str){
   string rootPath = Util::ReplaceAllStr(Util::GetExePath(), "\\", "/") + "../" ;
	string strHtml = HTML_TMPL;
	Util::ReplaceAllStr(strHtml,"{{0}}", _strCSS);

   if( _type == 1 )
   {
      Util::ReplaceAllStr(strHtml,"{{2}}", "");
   }
   else if( _type == 2 )
   {
      Util::ReplaceAllStr(strHtml,"{{2}}", WWW_CSS);
   }

	string md = Util::Text2Md( str, rootPath, _type );
	//md = replaceImgSrc(md, GetDocument()->getFilePath());
	Util::ReplaceAllStr(strHtml, "{{1}}", md);
	return strHtml;
}

void CMarkdownEditorView::UpdateMd(const string& strMd)
{
	string strHtml = GetMdHtml(strMd);
	NavigateHTML(strHtml);
}


void CMarkdownEditorView::setType( int type )
{
   _type = type ;
}