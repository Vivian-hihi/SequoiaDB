(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Deploy.Index.Ctrl', function( $scope, InheritSize ){
      InheritSize.append( $( '#DeployIndex' ) ) ;
      InheritSize.append( $( '#DeployIndexIframe' ) ) ;
   } ) ;
}());