(function(){
   window.SdbSacManagerConf.nowRoute = [
      { path: '/Index/Index/Index',
        options: {
           templateUrl: './app/template/Index/Index/Index.html',
           resolve: resolveFun( [ './app/controller/Index/Index/Index.js' ] )
        }
      },
      { path: '/Data/Overview/Index',
        options: {
           templateUrl: './app/template/Data/Overview/Index.html',
           resolve: resolveFun( [ './app/other/function/Data/Overview/Index.js', './app/controller/Data/Overview/Index.js' ] )
        }
      },
      { path: '/Data/Operate/Index',
        options: {
           templateUrl: './app/template/Data/Operate/Index.html',
           resolve: resolveFun( [ './app/other/function/Data/Operate/Index.js', './app/controller/Data/Operate/Index.js' ] )
        }
      },
      { path: '/Data/Operate/Record',
        options: {
           templateUrl: './app/template/Data/Operate/Record.html',
           resolve: resolveFun( [ './app/other/function/Data/Operate/Record.js', './app/controller/Data/Operate/Record.js' ] )
        }
      },
      { path: '/Data/Operate/Lobs',
        options: {
           templateUrl: './app/template/Data/Operate/Lobs.html',
           resolve: resolveFun( [ './app/other/function/Data/Operate/Lob.js', './app/controller/Data/Operate/Lobs.js' ] )
        }
      },
      { path: '/Data/Database/Index',
        options: {
           templateUrl: './app/template/Data/Database/Index.html',
           resolve: resolveFun( [ './app/other/function/Data/Database/Index.js', './app/controller/Data/Database/Index.js' ] )
        }
      },
      { path: '/Monitor/HostOverview/Index',
        options: {
           templateUrl: './app/template/Monitor/Host/Preview/Index.html',
           resolve: resolveFun( [ './app/controller/Monitor/Host/Preview/Index.js' ] )
         }
      },
      { path: '/Monitor/HostPerformance/Index',
        options: {
           templateUrl: './app/template/Monitor/Host/Performance/Index.html',
           resolve: resolveFun( [ './app/controller/Monitor/Host/Performance/Index.js' ] )
         }
      },
      { path: '/Monitor/HostWarning/Index',
        options: {
           templateUrl: './app/template/Monitor/Host/Warning/Index.html',
           resolve: resolveFun( [ './app/controller/Monitor/Host/Warning/Index.js' ] )
         }
      },
      { path: '/Deploy/AddHost/Index',
        options: {
           templateUrl: './app/template/Deploy/AddHost/Index.html',
           resolve: resolveFun( [ './app/controller/Deploy/AddHost/Index.js' ] )
        }
      }
   ] ;
   window.SdbSacManagerConf.defaultRoute = { redirectTo: '/Data/Overview/Index' } ;
}());