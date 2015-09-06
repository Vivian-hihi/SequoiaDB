(function(){
   window.SdbSacManagerConf.nowRoute = [
      { path: '/Data/Overview/Index',
        options: {
           templateUrl: './app/template/Data/Overview/Index.html',
           resolve: resolveFun( [ './app/controller/Data/Overview/Index.js' ] )
        }
      },
      { path: '/Data/Operate/Index',
        options: {
           templateUrl: './app/template/Data/Operate/Index.html',
           resolve: resolveFun( [ './app/controller/Data/Operate/Index.js' ] )
        }
      },
      { path: '/Data/Operate/Record',
        options: {
           templateUrl: './app/template/Data/Operate/Record.html',
           resolve: resolveFun( [ './app/controller/Data/Operate/Record.js' ] )
        }
      },
      { path: '/Data/Lob/Index',
        options: {
           templateUrl: './app/template/Data/Lob/Index.html',
           resolve: resolveFun( [ './app/controller/Data/Lob/Index.js' ] )
        }
      },
      { path: '/Data/Lob/Lobs',
        options: {
           templateUrl: './app/template/Data/Lob/Lobs.html',
           resolve: resolveFun( [ './app/controller/Data/Lob/Lobs.js' ] )
        }
      },
      { path: '/Data/Metadata/Index',
        options: {
           templateUrl: './app/template/Data/Metadata/Index.html',
           resolve: resolveFun( [ './app/controller/Data/Metadata/Index.js' ] )
        }
      },
      { path: '/Deploy/Index',
        options: {
           templateUrl: './app/template/Deploy/Index.html',
           resolve: resolveFun( [ './app/controller/Deploy/Index.js' ] )
        }
      },
      { path: '/Login',
        options: {
           templateUrl: './app/template/public/Login.html',
           resolve: resolveFun( [ './app/controller/Login.js' ] )
        }
      }
   ] ;
   window.SdbSacManagerConf.defaultRoute = { redirectTo: '/Data/Overview/Index' } ;
}());