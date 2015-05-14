//自定义导航插件
(function ($){
	$.fn.ligerSdbMenuBar = function(options){
		return $.ligerui.run.call( this, "ligerSdbMenuBar", arguments ) ;
	} ;
	$.fn.ligerGetSdbMenuBarManager = function(){
		return $.ligerui.run.call( this, "ligerGetSdbMenuBarManager", arguments ) ;
	} ;

	$.ligerDefaults.SdbMenuBar = {} ;

	$.ligerMethos.SdbMenuBar = {} ;

	$.ligerui.controls.SdbMenuBar = function(element, options){
		$.ligerui.controls.SdbMenuBar.base.constructor.call( this, element, options ) ;
	};
	$.ligerui.controls.SdbMenuBar.ligerExtend( $.ligerui.core.UIComponent, {
		__getType: function(){
			return 'SdbMenuBar' ;
		},
		__idPrev: function(){
			return 'SdbMenuBar' ;
		},
		_extendMethods: function(){
			return $.ligerMethos.SdbMenuBar ;
		},
		_render: function(){
			var g = this ;
			var p = this.options ;
			g.SdbMenuBar = $( this.element ) ;
			if( !g.SdbMenuBar.hasClass( 'ext-nav' ) )
			{
				g.SdbMenuBar.addClass( 'ext-nav' ) ;
			}
			var colum = $( '<div class="ext-colum"></div>' ) ;
			var menu = $( '<div class="ext-menu"></div>' ) ;
			var ul = $( '<ul></ul>' ) ;
			g.Menu = menu ;
			g.SdbMenuBar.append( colum ) ;
			g.SdbMenuBar.append( menu ) ;
			if( p && p.items )
			{
				colum.append( ul ) ;
				$( p.items ).each( function( i, item ){
					var tmpName = g.SdbMenuBar.attr( 'id' ) ;
					if( typeof( tmpName ) == 'undefined' )
					{
						tmpName = 'SdbMenuBar_' ;
					}
					else
					{
						tmpName += '_' ;
					}
					g.addItem( tmpName + i, ul, item ) ;
				} ) ;
			}
			g.set( p ) ;
		},
		show: function( button, menu ){
			var sdbWidth = $( window ).width() ;
			var menuLeft = $( button ).offset().left ;
			var menuWidth = $( menu ).outerWidth() ;
			if( menuLeft + menuWidth > sdbWidth )
			{
				menuLeft = sdbWidth - menuWidth ;
			}
			$( menu ).css( 'margin-left', menuLeft ) ;
			$( menu ).show();
		},
		hide: function( menu ){
			$( menu ).hide() ;
		},
		addItem: function( key, ul, item ){
			var g = this ;
			var p = this.options ;
			var ditem = $( '<li></li>' ) ;
			var ditema = $( '<a></a>' ) ;
			ditem.append( ditema ) ;
			ul.append( ditem ) ;
			ditema.attr( 'sdbmenubarid', key ) ;
			item.text && $( ditema ).html( item.text ) ;
			item.click && ditem.click( function(){ item.click( item ) ; } ) ;
			if( item.menu )
			{
				item.menu._id = key ;
				var menu = g._addMenu( item.menu ) ;
				ditema.click( function(){
					g.show( this, menu ) ;
				} ) ;
				$(document).bind( 'click', function ( jEvent ){
					 var domObj = jEvent.target ;
					 var menubarid = $( domObj ).attr( 'sdbmenubarid' ) ;
					 if( !p || typeof( menubarid ) == 'undefined' || menubarid != key )
					 {
						 g.hide( menu ) ;
					 }
				} ) ;
			}
		},
		_addMenuItem: function( ul, item ){
			var newLi = $( '<li></li>' ) ;
			var newA = $( '<a></a>' ) ;
			newLi.append( newA ) ;
			item.text && $( newA ).html( item.text ) ;
			item.click && newA.click( function (){
				 item.click( item ) ;
			} ) ;
            item.click && newLi.css( 'cursor', 'pointer' ) ;
			ul.append( newLi ) ;
		},
		_addMenu: function( p ){
			var g = this ;
			var ul = $( '<ul></ul>' ) ;
			g.Menu.append( ul ) ;
			p.width && $( ul ).css( 'width', p.width ) ;
			p.items && $( p.items ).each( function ( i, item ){
				 g._addMenuItem( ul, item ) ;
			});
			return ul ;
		}
	} ) ;

})(jQuery);

//自定义Iframe
(function ($){
	$.fn.ligerSdbIframe = function(options){
		return $.ligerui.run.call( this, "ligerSdbIframe", arguments ) ;
	} ;
	$.fn.ligerGetSdbIframeManager = function(){
		return $.ligerui.run.call( this, "ligerGetSdbIframeManager", arguments ) ;
	} ;

	$.ligerDefaults.SdbIframe = {} ;

	$.ligerMethos.SdbIframe = {} ;

	$.ligerui.controls.SdbIframe = function(element, options){
		$.ligerui.controls.SdbIframe.base.constructor.call( this, element, options ) ;
	};
	$.ligerui.controls.SdbIframe.ligerExtend( $.ligerui.core.UIComponent, {
		__getType: function(){
			return 'SdbIframe' ;
		},
		__idPrev: function(){
			return 'SdbIframe' ;
		},
		_extendMethods: function(){
			return $.ligerMethos.SdbIframe ;
		},
		_render: function(){
			//对象
			var g = this ;
			//参数
			var p = this.options ;
			g.SdbIframe = $( this.element ) ;
			if( p.url )
			{
				g.jiframe = $( '<iframe frameborder="0" style="width:100%"></iframe>' ) ;
				var framename = p.frameName ? p.frameName : 'ligerpanel' + new Date().getTime() ;
				g.jiframe.attr( 'name', framename ) ;
				g.jiframe.attr( 'id', framename ) ;
				g.SdbIframe.append( g.jiframe ) ;
				setTimeout( function(){
					if( g.SdbIframe.find( '.l-panel-loading:first' ).length == 0 )
					{
						g.SdbIframe.append( '<div class="l-panel-loading" style="display:block;"></div>' ) ;
					}
					var iframeloading = $( '.l-panel-loading:first', g.SdbIframe ) ;
					g.jiframe.attr( 'src', p.url ).bind( 'load.panel', function(){
						iframeloading.hide() ;
						g.trigger( 'loaded' ) ;
						p.callback && p.callback() ;
					} ) ;
					g.frame = window.frames[ g.jiframe.attr( 'name' ) ] ;
				}, 0 ) ; 
			}
			g.set( p ) ;
		},
        setUrl: function( url )
        {
            var g = this, p = this.options ;
			if( url )
			{
                p.url = url ;
				var content = g.SdbIframe.find( ".l-panel-content:first" ) ;
				var iframeloading = $( ".l-panel-loading:first", content ) ;
				iframeloading.show();
				g.jiframe.attr("src", p.url).bind('load.panel', function (){
					iframeloading.hide();
					g.trigger('loaded');
				} ) ;
			}
        },
		reload: function()
		{
			var g = this, p = this.options ;
			if( p.url )
			{
				var content = g.SdbIframe.find( ".l-panel-content:first" ) ;
				var iframeloading = $( ".l-panel-loading:first", content ) ;
				iframeloading.show();
				g.jiframe.attr("src", p.url).bind('load.panel', function (){
					iframeloading.hide();
					g.trigger('loaded');
				} ) ;
			}
		},
		getIframeObj: function()
		{
			var g = this ;
			return g.frame ;
		}
	} ) ;

})(jQuery);

//自定义普通列表
(function ($){
	$.fn.ligerSdbNorList = function(options){
		return $.ligerui.run.call( this, "ligerSdbNorList", arguments ) ;
	} ;
	$.fn.ligerGetSdbNorListManager = function(){
		return $.ligerui.run.call( this, "ligerGetSdbNorListManager", arguments ) ;
	} ;

	$.ligerDefaults.SdbNorList = {} ;

	$.ligerMethos.SdbNorList = {} ;

	$.ligerui.controls.SdbNorList = function(element, options){
		$.ligerui.controls.SdbNorList.base.constructor.call( this, element, options ) ;
	};
	$.ligerui.controls.SdbNorList.ligerExtend( $.ligerui.core.UIComponent, {
		__getType: function(){
			return 'SdbNorList' ;
		},
		__idPrev: function(){
			return 'SdbNorList' ;
		},
		_extendMethods: function(){
			return $.ligerMethos.SdbNorList;
		},
		_render: function(){
			//对象
			var g = this ;
			//参数
			var p = this.options ;
			//根对象
			g.SdbNorList = $( this.element ) ;
			g.ul = $( '<ul class="ext-list"></ul>' ) ;
			if( p.items )
			{
				$( p.items ).each( function( i, item ){
					var li = $( '<li></li>' ) ;
					var a = $( '<a></a>' ) ;
					item.text && a.html( item.text ) ;
					item.img && a.css( 'background', 'url(' + item.img + ') no-repeat 8px 9px transparent' ) ;
					item.img && a.css( 'padding-left', 35 ) ;
					item.click && a.click( function(){ item.click( item ) ; } ) ;
					li.append( a ) ;
					g.ul.append( li ) ;
				} ) ;
			}
			g.SdbNorList.append( g.ul ) ;
			g.set( p ) ;
		}
	} ) ;
})(jQuery);

//扩展panel插件的reload方法
$.extend( $.ligerui.controls.Panel.prototype, {
	reload: function()
	{
		var g = this, p = this.options ;
		if( p.url )
		{
			var content = g.panel.find( ".l-panel-content:first" ) ;
			var iframeloading = $( ".l-panel-loading:first", content ) ;
			iframeloading.show();
			g.jiframe.attr("src", p.url).bind('load.panel', function (){
				iframeloading.hide();
				g.trigger('loaded');
			} ) ;
		}
	}
} ) ;

//扩展grid插件的resize方法
$.extend( $.ligerui.controls.Grid.prototype, {
	resize: function( height )
	{
		var g = this, p = this.options ;
		var columnsWidth = [] ;
		var columnsLen = p.columns.length ;
		var width = $( this.element ).width() - columnsLen ;
		if( typeof( height) != 'undefined' )
		{
			g.setHeight( height ) ;
			if( g.hasBodyScroll() )
			{
				g.setHeight( height );
			}
			else
			{
				var inner = g.gridbody.find(".l-grid-body-inner:first");
				var newHeight = inner.height() + g.gridheader.height() + 33 ;
				g.setHeight( newHeight ) ;
			}
		}
		if( g.hasBodyScroll() )
		{
			width -= 22 ;
		}
        if( p.rownumbers == true )
        {
            width -= g.gridview1.width() ;
        }
		$.each( p.columns, function( index, column ){
			var curWidth = 0 ;
			if( typeof( column['width'] ) == 'string' && column['width'].indexOf( '%' ) > 0 && index + 1 == columnsLen )
			{
				var sumWidth = 0 ;
				$.each( columnsWidth, function( index2, columnWidth ){
					sumWidth += columnWidth ;
				} ) ;
				if( sumWidth < width )
				{
					curWidth = width - sumWidth ;
				}
			}
			else if( typeof( column['width'] ) == 'string' && column['width'].indexOf( '%' ) > 0 )
			{
				curWidth = parseInt( column['width'] ) * 0.01 * width ;
			}
			else if( typeof( column['width'] ) == 'string' || typeof( column['width'] ) == 'number' )
			{
				curWidth = parseInt( column['width'], 10 ) ;
			}
			columnsWidth.push( curWidth ) ;
			g.setColumnWidth( column['name'], curWidth ) ;
		} ) ;
	},
	hasBodyScroll: function ()
	{
		var g = this;
		var inner = g.gridbody.find(".l-grid-body-inner:first");
		if (!inner.length) return false;
		//20为横向滚动条的宽度
		return g.gridbody.height() < inner.height() ;
	},
} ) ;
	