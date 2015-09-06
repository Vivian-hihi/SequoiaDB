(function(){
   window.SdbSacManagerConf.recordEchart = {
       tooltip: {
           trigger: 'axis',
           formatter: '{a0}: {c0}'
       },
       legend: {
           borderWidth: 1,
           padding: [ 5, 10, 5, 10 ],
           borderColor: '#727272',
           data: [ 'Record' ],
           textStyle: {
              color: 'auto'
           },
           y: 'bottom'
       },
       color: [ 'rgba(39,169,227,1)' ],
       calculable: true,
       xAxis : [
           {
               type : 'category',
               boundaryGap : false,
               data : [ '', '', '', '', '', '', '' ]
           }
       ],
       yAxis : [
           {
               type : 'value'
           }
       ],
       grid: {
          x: 40,
          y: 20,
          x2: 20,
          y2: 60
       },
       series : [
           {
               name: 'Record',
               type: 'line',
               data: [ 0, 150, 200, 270, 400, 440, 600 ]
           }
       ]
   } ;
}());