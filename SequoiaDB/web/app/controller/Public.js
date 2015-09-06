(function(){
   var sacApp = window.SdbSacManagerModule ;
   //全局模板
   sacApp.controller( 'Index.Ctrl', function( $scope, $window, $rootScope, Tip, FormModal, SdbFunction, Loading ){
      //校验登录信息
      if( SdbFunction.getLocalData( 'SdbUser' ) === null || SdbFunction.getLocalData( 'SdbSessionID' ) === null )
		{
			window.location.href = './login.html#/Login' ;
         return;
      }
      //设置默认语言
      if( SdbFunction.getLocalData( 'SdbLanguage' ) == null )
      {
         SdbFunction.setLocalData( 'SdbLanguage', 'zh-CN' ) ;
      }
      //预加载模板
      $scope.Templates = {} ;
      $scope.Templates.Top = './app/template/public/Top.html' ;
      $scope.Templates.Left = './app/template/public/Left.html' ;
      $scope.Templates.Bottom = './app/template/public/Bottom.html' ;
      //获取语言
      $scope.Language = SdbFunction.getLocalData( 'SdbLanguage' ) ;
      //-------- 全局组件 ---------
      $scope.Components = {} ;
      $scope.Components.Confirm = {} ;
      $scope.Components.Modal = {} ;
      //初始化提示标签
      Tip.create() ;
      Tip.auto() ;
      //-------- 全局函数 ---------
      //格式化
      $rootScope.sprintf = sprintf ;
      //语言控制
      $rootScope.autoLanguage = function( text ){
         return _IndexPublic.languageCtrl( $scope, text ) ;
      }
      //选择集群
      $rootScope.selectCluster = function( selectEvent ){
         _IndexPublic.createSelectClusterModel( $scope, selectEvent ) ;
      }
      //选择业务
      $rootScope.selcetModule = function( selectEvent ){
         _IndexPublic.createSelectModuleModel( $scope, selectEvent ) ;
      }
   } ) ;

   //顶部
   sacApp.controller( 'Index.Top.Ctrl', function( $scope, SdbRest ){
      $scope.Top = {} ;
      //给用户菜单创建一个遮罩
      var mask = $( '<div class="mask-screen unalpha"></div>' ).on( 'click', function(){
         $( '#userMenu' ).hide() ;
         mask.detach() ;
      } ) ;
      $( '#userMenu li' ).on( 'click', function(){
         $( '#userMenu' ).hide() ;
         mask.detach() ;
      } ) ;
      $scope.Top.showUserMenu = function(){
         $( '#userMenu' ).show() ;
         mask.appendTo( document.body ) ;
      }
   } ) ;
   //左边
   sacApp.controller( 'Index.Left.Ctrl', function( $scope, $rootScope, $location ){
      $scope.Left = {} ;
      var navMenu;
      function getMenu( nav, moduleName )
      {
         var rv = null ;
         $.each( nav, function( index, value ){
            if( value['module'] == moduleName )
            {
               rv = value['list'] ;
               return false ;
            }
         } ) ;
         return rv ;
      }
      //获取路由
      var navMenu = [
         {
            'text': $scope.autoLanguage( '部署' ),
            'module': 'Deploy',
            'icon': 'fa-share-alt',
            'action': '/deployment/index.html'
         },
         {
            'text': $scope.autoLanguage( '数据' ),
            'module': 'Data',
            'icon': 'fa-database',
            'list': [
               {
                  'title': $scope.autoLanguage( '数据库' ),
                  'list': [
                     {
                        'text': $scope.autoLanguage( '预览' ),
                        'action': 'Overview'
                     },
                     {
                        'text': $scope.autoLanguage( '数据操作' ),
                        'action': 'Operate'
                     },
                     {
                        'text': $scope.autoLanguage( 'Lob操作' ),
                        'action': 'Lob'
                     },
                     {
                        'text': $scope.autoLanguage( '元数据操作' ),
                        'action': 'Metadata'
                     }
                  ]
               }
            ]
         }
      ] ;
      $rootScope.$on( '$locationChangeStart', function( event, newUrl, oldUrl ){
         printfDebug( '切换路由 ' + newUrl + ' form ' + oldUrl ) ;
         var route = $location.url().split( '/' ) ;
         $scope.Left.navMenu = navMenu ;
         $scope.Left.selectModule = route[1] ;
         $scope.Left.selectAction = route[2] ;
      } ) ;
      var route = $location.url().split( '/' ) ;
      $scope.Left.navMenu = navMenu ;
      $scope.Left.selectModule = route[1] ;
      $scope.Left.selectAction = route[2] ;
      $scope.Left.selectMenu = getMenu( navMenu, $scope.Left.selectModule ) ;
      $scope.Left.isHideMenu = false ;
   } ) ;
   //底部
   sacApp.controller( 'Index.Bottom.Ctrl', function( $scope, SdbRest ){
      $scope.Bottom = {} ;
      //获取系统时间
      _IndexBottom.getSystemTime( $scope ) ;
      //获取系统状态
      _IndexBottom.checkPing( $scope, SdbRest ) ;
   } ) ;

   //左边导航的折叠指令
   sacApp.directive( 'navFold', function( $window, InheritSize ){
      var isShow = true ;
      var leftButton, rightButton ;
      var left = $( '#PublicLeft' ) ;
      var leftSub = $( '#PublicNav2 > .second' ) ;
      var centre = $( '#PublicCentre' ) ;
      function toogleMenu()
      {
         if( isShow == false )
         {
            left.attr( 'data-width', '260' ) ;
            centre.attr( 'data-offsetx', '-260' ) ;
            centre.attr( 'data-mLeft', '260' ) ;
            leftButton.css( 'visibility', 'hidden' ) ;
            leftSub.show() ;
            centre.animate( { marginLeft: 260 }, 'normal' ) ;
            centre.outerWidth( centre.width() - 180 ) ;
            InheritSize.renderAll( centre ) ;
            $( document.body ).css( 'overflow', 'hidden' ) ;
            left.animate( { width: 260 }, 'normal', function(){
               $( document.body ).css( 'overflow', 'auto' ) ;
            } ) ;
         }
         else
         {
            left.attr( 'data-width', '80' ) ;
            centre.attr( 'data-offsetx', '-80' ) ;
            centre.attr( 'data-mLeft', '80' ) ;
            centre.animate( { marginLeft: 80 }, 'normal' ) ;
            centre.outerWidth( centre.width() + 180 ) ;
            InheritSize.renderAll( centre ) ;
            $( document.body ).css( 'overflow', 'hidden' ) ;
            left.animate( { width: 80 }, 'normal', function(){
               leftSub.hide() ;
               leftButton.css( 'visibility', 'visible' ) ;
               $( document.body ).css( 'overflow', 'auto' ) ;
            } ) ;
         }
         isShow = !isShow ;
      }
      return {
         link: function( scope, element ){
            var visibility = $( element ).css( 'visibility' ) ;
            if( visibility == 'hidden' )
            {
               leftButton = $( '> i', element ) ;
               leftButton.bind( 'click', function(){
                  toogleMenu() ;
               } ) ;
            }
            else
            {
               rightButton = $( '> i', element ) ;
               rightButton.bind( 'click', function(){
                  toogleMenu() ;
               } ) ;
            }
         }
      }
   } ) ;
}());