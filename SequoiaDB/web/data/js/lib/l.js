//https://github.com/malko/l.js
;(function(window, undefined){
/*
* script for js/css parallel loading with dependancies management
* @author Jonathan Gotti < jgotti at jgotti dot net >
* @licence dual licence mit / gpl
* @since 2012-04-12
* @todo add prefetching using text/cache for js files
* @changelog
*            - 2014-06-26 - bugfix in css loaded check when hashbang is used
*            - 2014-05-25 - fallback support rewrite + null id bug correction + minification work
*            - 2014-05-21 - add cdn fallback support with hashbang url
*            - 2014-05-22 - add support for relative paths for stylesheets in checkLoaded
*            - 2014-05-21 - add support for relative paths for scripts in checkLoaded
*            - 2013-01-25 - add parrallel loading inside single load call
*            - 2012-06-29 - some minifier optimisations
*            - 2012-04-20 - now sharp part of url will be used as tag id
*                         - add options for checking already loaded scripts at load time
*            - 2012-04-19 - add addAliases method
* @note coding style is implied by the target usage of this script not my habbits
*/
	/** gEval credits goes to my javascript idol John Resig, this is a simplified jQuery.globalEval */
	var gEval = function(js){ ( window.execScript || function(js){ window[ "eval" ].call(window,js);} )(js); }
		, isA =  function(a,b){ return a instanceof (b || Array);}
		//-- some minifier optimisation
		, D = document
		, getElementsByTagName = 'getElementsByTagName'
		, length = 'length'
		, readyState = 'readyState'
		, onreadystatechange = 'onreadystatechange'
		//-- get the current script tag for further evaluation of it's eventual content
		, scripts = D[getElementsByTagName]("script")
		, scriptTag = scripts[scripts[length]-1]
		, script  = scriptTag.innerHTML.replace(/^\s+|\s+$/g,'')
	;
	//avoid multiple inclusion to override current loader but allow tag content evaluation
	if( ! window.ljs ){
		var checkLoaded = scriptTag.src.match(/checkLoaded/)?1:0
			//-- keep trace of header as we will make multiple access to it
			,header  = D[getElementsByTagName]("head")[0] || D.documentElement
			, urlParse = function(url){
				var parts={}; // u => url, i => id, f = fallback
				parts.u = url.replace(/#(=)?([^#]*)?/g,function(m,a,b){ parts[a?'f':'i'] = b; return '';});
				return parts;
			}
			,appendElmt = function(type,attrs,cb){
				var e = D.createElement(type), i;
				if( cb ){ //-- this is not intended to be used for link
					if(e[readyState]){
						e[onreadystatechange] = function(){
							if (e[readyState] === "loaded" || e[readyState] === "complete"){
								e[onreadystatechange] = null;
								cb();
							}
						};
					}else{
						e.onload = cb;
					}
				}
				for( i in attrs ){ attrs[i] && (e[i]=attrs[i]); }
				header.appendChild(e);
				// return e; // unused at this time so drop it
			}
			,load = function(url,cb){
				if( this.aliases && this.aliases[url] ){
					var args = this.aliases[url].slice(0);
					isA(args) || (args=[args]);
					cb && args.push(cb);
					return this.load.apply(this,args);
				}
				if( isA(url) ){ // parallelized request
					for( var l=url[length]; l--;){
						this.load(url[l]);
					}
					cb && url.push(cb); // relaunch the dependancie queue
					return this.load.apply(this,url);
				}
				if( url.match(/\.css\b/) ){
					return this.loadcss(url,cb);
				}
				return this.loadjs(url,cb);
			}
			,loaded = {}  // will handle already loaded urls
			,loader  = {
				aliases:{}
				,loadjs: function(url,cb){
					var parts = urlParse(url);
					url = parts.u;
					if( loaded[url] === true ){ // already loaded exec cb if any
						cb && cb();
						return this;
					}else if( loaded[url]!== undefined ){ // already asked for loading we append callback if any else return
						if( cb ){
							loaded[url] = (function(ocb,cb){ return function(){ ocb && ocb(); cb && cb(); }; })(loaded[url],cb);
						}
						return this;
					}
					// first time we ask this script
					loaded[url] = (function(cb){ return function(){loaded[url]=true; cb && cb();};})(cb);
					cb = function(){ loaded[url](); };
					appendElmt('script',{type:'text/javascript',src:url,id:parts.i,onerror:function(error){
						if( parts.f ){
							var c = error.currentTarget;
							c.parentNode.removeChild(c);
							appendElmt('script',{type:'text/javascript',src:parts.f,id:parts.i},cb);
						}
					}},cb);
					return this;
				}
				,loadcss: function(url,cb){
					var parts = urlParse(url);
					url = parts.u;
					loaded[url] || appendElmt('link',{type:'text/css',rel:'stylesheet',href:url,id:parts.i});
					loaded[url] = true;
					cb && cb();
					return this;
				}
				,load: function(){
					var argv=arguments,argc = argv[length];
					if( argc === 1 && isA(argv[0],Function) ){
						argv[0]();
						return this;
					}
					load.call(this,argv[0], argc <= 1 ? undefined : function(){ loader.load.apply(loader,[].slice.call(argv,1));} );
					return this;
				}
				,load2: function( pathNum, arr ){
					var g = this ;
					var path = '' ;
					if( pathNum == 0 )
					{
						path = './' ;
					}
					else
					{
						for( var i = 0; i < pathNum; ++i ){
							path += '../' ;
						}
					}
					var key = 0 ;
					function loadAgain( filePath ){
						g.load( filePath, function(){
							++key < arr.length && loadAgain( path + arr[key] ) ;
						} ) ;
					}
					loadAgain( path + arr[key] ) ;
				}
				,addAliases:function(aliases){
					for(var i in aliases ){
						this.aliases[i]= isA(aliases[i]) ? aliases[i].slice(0) : aliases[i];
					}
					return this;
				}
			}
		;
		if( checkLoaded ){
			var i,l,links,url;
			for(i=0,l=scripts[length];i<l;i++){
				(url = scripts[i].getAttribute('src')) && (loaded[url.replace(/#.*$/,'')] = true);
			}
			links = D[getElementsByTagName]('link');
			for(i=0,l=links[length];i<l;i++){
				(links[i].rel==='stylesheet' || links[i].type==='text/css') && (loaded[links[i].getAttribute('href').replace(/#.*$/,'')]=true);
			}
		}
		//export ljs
		window.ljs = loader;
		// eval inside tag code if any
	}
	script && gEval(script);
})(window);

var __levelNum = 0 ;

(function(){
	var htmlJsPath = '' ;
	var num = 0 ;
	(function(){
		var rootPath = 'js/public' ;
		var fileName = 'index' ;
		var pathName = window.document.location.pathname ;
		if( pathName == '/' )
		{
			rootPath += '/' + fileName + '.js' ;
		}
		else
		{
			var pathArr = pathName.split( '/' ) ;
			var pathNum = pathArr.length ;
            fileName = pathArr[pathNum-1] ;
            fileName = fileName.split( '.' ) ;
            fileName = fileName[0] ;
			if( pathNum > 3 )
			{
				for( var i = 3; i < pathNum - 1; ++i )
				{
					rootPath += '/' + pathArr[i] ;
				}
				rootPath += '/' + fileName + '.js' ;
				num = pathNum - 3 ;
			}
			else
			{
				rootPath += '/' + fileName + '.js' ;
			}
		}
		htmlJsPath = rootPath ;
	})() ;
	var files = [
		'./css/skins/Aqua/css/ligerui-all.css',
		'./css/extend.css',
		'./css/style.css',
		'./js/lib/jquery/jquery-1.11.1.min.js',
		'./js/lib/jquery.cookie.js',
		'./js/lib/json2.js',
		'./js/lib/ligerUI/js/ligerui.all.js',
		'./js/lib/ligerUI/js/ligerui.extend.js',
		htmlJsPath
	] ;
	__levelNum = num ;
	ljs.load2( num, files ) ;
})() ;

function F( path )
{
	var tmpPath = '' ;
	for( var i = 0; i < __levelNum; ++i )
	{
		tmpPath += '../' ;
	}
	path = tmpPath + 'html/Frame/' + path ;
	return path ;
}

function M( path )
{
	var tmpPath = '' ;
	for( var i = 0; i < __levelNum; ++i )
	{
		tmpPath += '../' ;
	}
	path = tmpPath + 'html/Module/' + path ;
	return path ;
}

function O( path )
{
	var tmpPath = '' ;
	for( var i = 0; i < __levelNum; ++i )
	{
		tmpPath += '../' ;
	}
	path = tmpPath + path ;
	return path ;
}

function I( path )
{
	var tmpPath = '' ;
	for( var i = 0; i < __levelNum; ++i )
	{
		tmpPath += '../' ;
	}
	path = tmpPath + 'images/' + path ;
	return path ;
}


function setUrlParam()
{
	var param = '' ;
	var argLen = arguments.length ;
	if( argLen == 1 )
	{
		var isFirst = true ;
		var jsonObj = arguments[0] ;
		for( var key in jsonObj )
		{
			!isFirst && ( param += '&' ) ;
			param += encodeURI( key ) + '=' + encodeURI( jsonObj[key] ) ;
			isFirst = false ;
		}
	}
	else if( argLen > 1 )
	{
		param += encodeURI( arguments[0] ) + '=' + encodeURI( arguments[1] ) ;
	}
	return param ;
}

function getUrlParam( key )
{
	var reg = new RegExp("(^|&)" + key + "=([^&]*)(&|$)");
	var r = window.location.search.substr(1).match(reg);
	if (r != null) return decodeURI(r[2]); return null;
}

function parseJsons( str )
{
	var json_array = [] ;
	var i = 0, len = str.length ;
	var char, level, isEsc, isString, start, end, subStr, json ;
	while( i < len )
	{
		while( i < len ){	char = str.charAt( i ) ;	if( char === '{' ){	break ;	}	++i ;	}
		level = 0, isEsc = false, isString = false, start = i ;
		while( i < len )
		{
			char = str.charAt( i ) ;
			if( isEsc ){	isEsc = false ;	}
			else
			{
				if( ( char === '{' || char === '[' ) && isString === false ){	++level ;	}
				else if( ( char === '}' || char === ']' ) && isString === false )
				{
					--level ;
					if( level === 0 )
					{
						++i ;
						end = i ;
						subStr = str.substring( start, end ) ;
						//try{	json = eval( '(' + subStr + ')' ) ;	json_array.push( json ) ;	}catch(e){}
						json = JSON.parse( subStr ) ;
						json_array.push( json ) ;
						break ;
					}
				}
				else if( char === '"' ){	isString = !isString ;	}
				else if( char === '\\' ){	isEsc = true ;	}
			}
			++i ;
		}
	}
	return json_array ;
}