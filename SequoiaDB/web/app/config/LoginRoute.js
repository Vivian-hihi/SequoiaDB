(function(){
   window.SdbSacManagerConf.nowRoute = [
      { path: '/',
        options: {
           templateUrl: './app/template/public/Login.html',
           resolve: resolveFun( [ './app/controller/Login.js' ] )
        }
      }
   ] ;
   window.SdbSacManagerConf.defaultRoute = { redirectTo: '/' } ;
}());