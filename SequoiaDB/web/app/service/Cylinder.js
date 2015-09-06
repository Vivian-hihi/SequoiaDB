(function(){
   var sacApp = window.SdbSacManagerModule ;
   sacApp.service( 'Graphics', function(){
      var g = this ;
      g.list = [] ;
      g.drawCylinder = function( id, offsetX, offsetY, round, data ){
         if( typeof( offsetX ) == 'undefined' ) offsetX = 0 ;
         if( typeof( offsetY ) == 'undefined' ) offsetY = 0 ;
         if( typeof( round ) == 'undefined' ) round = 40 ;
         var width  = $( id ).width()  - offsetX ;
         var height = $( id ).height() - offsetY ;
         var rvArr = { id: id, offsetX: offsetX, offsetY: offsetY, round: round, data: data, ele: [] } ;
         //创建一个画布
         var paper = new Raphael( id, width, height ) ;
         rvArr['paper'] = paper ;
         function create( index, arg ){
            var width   = arg['width'] - 100 ;
            var height  = arg['height'] ;
            var offsetX = arg['offsetX'] ;
            var offsetY = arg['offsetY'] ;
            var round   = arg['round'] ;
            var color1  = arg['color1'] ;
            var color2  = arg['color2'] ;
            var color3  = arg['color3'] ;
            var notLast = arg['notLast'] ;
            var name    = arg['name'] ;
            var percent = arg['percent'] ;
            var rv = {} ;
            //圆柱主干
            rv['z1'] = paper.rect( offsetX, round + offsetY, width - offsetX - 1, height - round * 2 - offsetY ).
            attr( { 'stroke-width': 2, 'stroke-linecap': 'butt', 'stroke-linejoin': 'miter', 'stroke': color3, 'fill': color2, 'opacity': 1 } ) ;
            //圆柱底部
            rv['d'] = paper.ellipse( (width + offsetX)/2 - 1, height - round - 1, ( width - offsetX )/2 - 1, round ).
            attr( { 'stroke-width': 2, 'stroke-linecap': 'butt', 'stroke-linejoin': 'miter', 'stroke': color3, 'fill': color2, 'opacity': 1 } ) ;
            //圆柱主干2
            rv['z2'] = paper.rect( offsetX + 1, round + offsetY, width - offsetX - 3, height - round * 2 - offsetY - 2, 0 ).
            attr( { 'stroke-width': 0, 'fill': color2, 'opacity': 1 } ) ;
            //圆柱头
            rv['t'] = paper.ellipse( (width + offsetX)/2 - 1, round + offsetY, ( width - offsetX )/2 - 1, round ).
            attr( { 'stroke-width': 2, 'stroke-linecap': 'butt', 'stroke-linejoin': 'miter', 'stroke': notLast ? color3 : '#F5F5F5', 'fill': color1, 'opacity': 1 } ) ;
            //箭头
            var pointX = width - 2 ;
            var pointY = height / 2 + offsetY / 2 + 5 ;
            rv['j'] = paper.path( 'M' + pointX + ' ' + pointY + 'L' + ( pointX + 25 ) + ' ' + ( pointY - 6 ) ).
            attr( { 'stroke-width': 2, 'stroke-linecap': 'butt', 'stroke-linejoin': 'miter', 'stroke': color2 } ) ;
            //字体
            rv['f'] = paper.text( 0, 0, name + ' ' + ( percent * 100 ) + '%' ).
            attr( { 'fill': '#27A9E3', 'font-weight': 'bold' } ) ;
            var fontBox = rv['f'].getBBox() ;
            rv['f'].attr( { x: width + 29 + fontBox['width'] * 0.5, y: ( height / 2 + offsetY / 2 - 2 ) } ) ;
            return rv ;
         }
         //圆柱真正的高度
         var height2 = height - ( round * 2 ) - offsetY ;
         var height3 = 0 ;
         var len = data.length ;
         $.each( data, function( index, value ){
            var thisHeight = height2 * ( 1 - value['percent'] ) - height3 ;
            var tmpHeight = 0 ;
            var newCy = null ;
            var notLast = ( index + 1 < len ) ;
            if( index > 0 ) tmpHeight = 1 ;
            var cyOffsetY = ( index + 1 < len ) ? thisHeight : offsetY ;
            newCy = create( index, { width: width,
                                     height: ( height - height3 + tmpHeight ),
                                     offsetX: offsetX,
                                     offsetY: cyOffsetY,
                                     round: round,
                                     color1: value['color'],
                                     color2: value['color'],
                                     color3: '#F0F0F0',
                                     notLast: notLast,
                                     name: value['name'],
                                     percent: value['percent'] } ) ;
            rvArr['ele'].push( newCy ) ;
            height3 += height2 * value['percent'] + offsetY ;
         } ) ;
         var id = g.list.length ;
         g.list.push( rvArr ) ;
         return id ;
      }
      g.resizeCylinder = function( id ){
         var arr = g.list[id] ;
         var id = arr['id'] ;
         var offsetX = arr['offsetX'] ;
         var offsetY = arr['offsetY'] ;
         var round  = arr['round'] ;
         var data   = arr['data'] ;
         var width  = $( id ).width()  - offsetX ;
         var height = $( id ).height() - offsetY ;
         var paper  = arr['paper'] ;
         paper.setSize( width, height ) ;
         //圆柱真正的高度
         var height2 = height - ( round * 2 ) - offsetY ;
         var height3 = 0 ;
         var len = data.length ;
         function resize( index, arg, rv ){
            var width   = arg['width'] - 100 ;
            var height  = arg['height'] ;
            var offsetX = arg['offsetX'] ;
            var offsetY = arg['offsetY'] ;
            var round   = arg['round'] ;
            var color1  = arg['color1'] ;
            var color2  = arg['color2'] ;
            var color3  = arg['color3'] ;
            var notLast = arg['notLast'] ;
            var name    = arg['name'] ;
            var percent = arg['percent'] ;
            //圆柱主干
            rv['z1'].attr( { 'width': width - offsetX - 1, 'height': height - round * 2 - offsetY } ) ;
            //圆柱底部
            rv['d'].attr( { 'cx': (width + offsetX)/2 - 1, 'cy': height - round - 1, 'rx': ( width - offsetX )/2 - 1 } ) ;
            //圆柱主干2
            rv['z2'].attr( { 'width': width - offsetX - 3, 'height': height - round * 2 - offsetY - 2 } ) ;
            //圆柱头
            rv['t'].attr( { 'cx': (width + offsetX)/2 - 1, 'cy': round + offsetY, 'rx': ( width - offsetX )/2 - 1 } ) ;
            //箭头
            var pointX = width - 2 ;
            var pointY = height / 2 + offsetY / 2 + 5 ;
            rv['j'].attr( { 'path': 'M' + pointX + ' ' + pointY + 'L' + ( pointX + 25 ) + ' ' + ( pointY - 6 ) } ) ;
            //字体
            var fontBox = rv['f'].getBBox() ;
            rv['f'].attr( { x: width + 29 + fontBox['width'] * 0.5, y: ( height / 2 + offsetY / 2 - 2 ) } ) ;
         }
         $.each( data, function( index, value ){
            var thisHeight = height2 * ( 1 - value['percent'] ) - height3 ;
            var tmpHeight = 0 ;
            var notLast = ( index + 1 < len ) ;
            if( index > 0 ) tmpHeight = 1 ;
            var cyOffsetY = ( index + 1 < len ) ? thisHeight : offsetY ;
            resize( index, { width: width,
                             height: ( height - height3 + tmpHeight ),
                             offsetX: offsetX,
                             offsetY: cyOffsetY,
                             round: round,
                             color1: value['color'],
                             color2: value['color'],
                             color3: '#F0F0F0',
                             notLast: notLast,
                             name: value['name'],
                             percent: value['percent'] }, arr['ele'][index] ) ;
            height3 += height2 * value['percent'] + offsetY ;
         } ) ;
      }
   } ) ;
}());