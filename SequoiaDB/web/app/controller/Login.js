(function(){
   var sacApp = window.SdbSacManagerModule ;
   //全局模板
   sacApp.controllerProvider.register( 'Login.Ctrl', function ( $scope, $rootScope, $window, $compile, SdbFunction, SdbRest ) {
      //默认语言
      if( SdbFunction.LocalData( 'SdbLanguage' ) == null )
      {
         SdbFunction.LocalData( 'SdbLanguage', 'zh-CN' ) ;
      }
      //获取语言
      $scope.Language = SdbFunction.LocalData( 'SdbLanguage' ) ;
      //语言控制
      $rootScope.autoLanguage = function( text ){
         return _IndexPublic.languageCtrl( $scope, text ) ;
      }
      $scope.isLoading = false ;
      //用户名
      $scope.username = $scope.autoLanguage( '用户名' ) ;
      //密码
      $scope.password = '' ;
      //语言列表
      $scope.LanguageList = [ { 'key': 'en', 'value': 'English' }, { 'key': 'zh-CN', 'value': '中文' } ] ;
      //执行结果
      $scope.result = '' ;
      //用户名和密码的动画事件
      setTimeout( function(){
         $( '.animateTip' ).show() ;
         $scope.username = '' ;
         $scope.$apply() ;
         $( '.animateTip' ).animate( { 'right': '180px' }, 'normal', 'swing', function(){
            $scope.username = 'admin' ;
            $scope.$apply() ;
         } ) ;
      }, 400 ) ;
      //登陆
      $scope.login = function(){
         $scope.isLoading = true ;
         $scope.result = '' ;
	      SdbRest.Login( $scope.username, $scope.password, function( json, textStatus, jqXHR ){
		      var id = jqXHR.getResponseHeader( 'SdbSessionID' ) ;
		      SdbFunction.LocalData( 'SdbSessionID', id ) ;
		      SdbFunction.LocalData( 'SdbUser', $scope.username ) ;
		      window.location.href = '/deployment/index.html' ;

            //var sql = 'SELECT T1.FullName, T1.IsMainCL, T1.MainCLName, T1.ShardingType, T1.Details.TotalRecord, T1.Details.Indexes AS TotalIndexes FROM (SELECT * FROM $SNAPSHOT_CL split BY Details) AS T1' ;
            //获取集合列表
            //SdbRest.Exec( sql, function( data ){}, function( errorInfo ){}, function(){} ) ;


	      }, function( errorInfo ){
            $scope.result = errorInfo['detail'] ;
	      }, function( XMLHttpRequest, textStatus, errorThrown ){
            $scope.result = $scope.autoLanguage( '网络错误' ) ;
         }, function(){
            $scope.isLoading = false ;
            $scope.$apply() ;
         } ) ;
      }
      var mask = $( '<div class="mask-screen unalpha"></div>' ).on( 'click', function(){
         $( '#languageMenu' ).hide() ;
         mask.detach() ;
      } ) ;
      $scope.showLanguageMenu = function(){     
         $( '#languageMenu' ).show() ;
         mask.appendTo( document.body ) ;
      }
      $scope.chooseLanguage = function( listIndex ){
         var newLanguage = $scope.LanguageList[listIndex]['key'] ;
         $scope.Language = newLanguage ;
         SdbFunction.LocalData( 'SdbLanguage', newLanguage ) ;
         $( '#languageMenu' ).hide() ;
         mask.detach() ;
      }
      //回车事件
      angular.element( $window ).bind( 'keydown', function( event ){
         var e = event || $window.event;
         if( !e.ctrlKey && e.keyCode == 13 )
         {
            $scope.login() ;
         }
      } ) ;
   } ) ;
}());