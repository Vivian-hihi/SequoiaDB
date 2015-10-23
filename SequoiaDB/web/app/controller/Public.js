(function(){
   var sacApp = window.SdbSacManagerModule ;
   //全局模板
   sacApp.controller( 'Index.Ctrl', function( $scope, $window, $rootScope, $location, Tip, SdbFunction, Loading, SdbRest ){
      //校验登录信息
      if( SdbFunction.LocalData( 'SdbUser' ) === null || SdbFunction.LocalData( 'SdbSessionID' ) === null )
		{
			window.location.href = './login.html#/Login' ;
         return;
      }
      //设置默认语言
      if( SdbFunction.LocalData( 'SdbLanguage' ) == null )
      {
         SdbFunction.LocalData( 'SdbLanguage', 'zh-CN' ) ;
      }
      //预加载模板
      $scope.Templates = {} ;
      $scope.Templates.Top = './app/template/public/Top.html' ;
      $scope.Templates.Left = './app/template/public/Left.html' ;
      $scope.Templates.Bottom = './app/template/public/Bottom.html' ;
      //获取语言
      $scope.Language = SdbFunction.LocalData( 'SdbLanguage' ) ;
      //初始化提示标签
      Tip.create() ;
      Tip.auto() ;
      //-------- 全局变量 ---------
      $rootScope.Url = { Module: '', Action: '', Method: '' } ;
      //-------- 全局组件 ---------
      $rootScope.Components = {} ;
      $rootScope.Components.Confirm = {} ;
      $rootScope.Components.Modal = {} ;
      //-------- 全局函数 ---------
      //格式化
      $rootScope.sprintf = sprintf ;
      //判断数组
      $rootScope.isArray = isArray ;
      //语言控制
      $rootScope.autoLanguage = function( text ){
         return _IndexPublic.languageCtrl( $scope, text ) ;
      }
      //选择集群
      $rootScope.selectCluster = function( main ){
         _IndexPublic.createSelectClusterModel( $scope, SdbRest, SdbFunction, main ) ;
      }
      //选择业务
      $rootScope.selcetModule = function( clusterName, main ){
         _IndexPublic.createSelectModuleModel( $scope, $location, SdbRest, SdbFunction, clusterName, main ) ;
      }
   } ) ;

   //顶部
   sacApp.controller( 'Index.Top.Ctrl', function( $scope, $location, SdbFunction, SdbRest ){
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
      //修改密码弹窗
      $scope.showChangePasswd = function(){
         _IndexTop.createPasswdModel( $scope, SdbRest ) ;
      }
      //登出
      $scope.logout = function(){
         _IndexTop.logout( $location, SdbFunction ) ;
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
            'module': 'Deploy2',
            'icon': 'fa-share-alt',
            'action': '/deployment/index.html'
         },
         {
            'text': $scope.autoLanguage( '主页' ),
            'module': 'Index',
            'icon': 'fa-home',
            'list': [
               {
                  'title': $scope.autoLanguage( '集群管理' ),
                  'list': [
                     {
                        'text': $scope.autoLanguage( '集群预览' ),
                        'action': 'Index'
                     }
                  ]
               }
            ]
         },
         {
            'text': $scope.autoLanguage( '部署' ),
            'module': 'Deploy',
            'icon': 'fa-share-alt',
            'list': [
               {
                  'title': $scope.autoLanguage( '主机' ),
                  'list': [
                     {
                        'text': $scope.autoLanguage( '添加主机' ),
                        'action': 'AddHost'
                     },
                     {
                        'text': $scope.autoLanguage( '发现主机' ),
                        'action': 'FindHost'
                     }
                  ]
               },
               {
                  'title': $scope.autoLanguage( '业务' ),
                  'list': [
                     {
                        'text': $scope.autoLanguage( '安装业务' ),
                        'action': 'InstallModule'
                     },
                     {
                        'text': $scope.autoLanguage( '发现业务' ),
                        'action': 'FindModule'
                     },
                     {
                        'text': $scope.autoLanguage( '测试业务' ),
                        'action': 'TestModule'
                     }
                  ]
               }
            ]
         },
         {
            'text': $scope.autoLanguage( '监控' ),
            'module': 'Monitor',
            'icon': 'fa-flash',
            'list': [
               {
                  'title': $scope.autoLanguage( '主机管理' ),
                  'list': [
                     {
                        'text': $scope.autoLanguage( '预览' ),
                        'action': 'HostOverview'
                     },
                     {
                        'text': $scope.autoLanguage( '性能' ),
                        'action': 'HostPerformance'
                     },
                     {
                        'text': $scope.autoLanguage( '告警' ),
                        'action': 'HostWarning'
                     }
                  ]
               },
               {
                  'title': $scope.autoLanguage( '业务管理' ),
                  'list': [
                     {
                        'text': $scope.autoLanguage( '预览' ),
                        'action': 'ModuleOverview'
                     },
                     {
                        'text': $scope.autoLanguage( '性能监控' ),
                        'action': 'ModulePerformance'
                     },
                     {
                        'text': $scope.autoLanguage( '数据分析' ),
                        'action': 'ModuleAnalysis'
                     },
                     {
                        'text': $scope.autoLanguage( '告警' ),
                        'action': 'ModuleWarning'
                     },
                     {
                        'text': $scope.autoLanguage( '配置' ),
                        'action': 'ModuleConfig'
                     },
                     {
                        'text': $scope.autoLanguage( '策略' ),
                        'action': 'ModuleStrategy'
                     }
                  ]
               }
            ]
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
                        'text': $scope.autoLanguage( '数据库操作' ),
                        'action': 'Database'
                     },
                     {
                        'text': $scope.autoLanguage( '数据操作' ),
                        'action': 'Operate'
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
         $rootScope.Url.Module = route[1] ;
         $rootScope.Url.Action = route[2] ;
         $rootScope.Url.Method = route[3] ;
      } ) ;
      $scope.selectLeftModule = function( moduleName ){
         $scope.Left.selectModule = moduleName ;
         $scope.Left.selectMenu = getMenu( navMenu, $scope.Left.selectModule ) ;
      }
      var route = $location.url().split( '/' ) ;
      $scope.Left.navMenu = navMenu ;
      $scope.Left.selectModule = route[1] ;
      $scope.Left.selectAction = route[2] ;
      $scope.Left.selectMenu = getMenu( navMenu, $scope.Left.selectModule ) ;
      $scope.Left.isHideMenu = false ;
      $rootScope.Url.Module = route[1] ;
      $rootScope.Url.Action = route[2] ;
      $rootScope.Url.Method = route[3] ;
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