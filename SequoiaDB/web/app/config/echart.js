(function(){
   window.SdbSacManagerConf.recordEchart = {
       tooltip: {
           trigger: 'axis',
           formatter: '{a0}: {c0}'
       },
       animation: false,
       addDataAnimation: false,
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
               data : [ '0', '5', '10', '15', '20', '25', '30' ]
           }
       ],
       yAxis : [
           {
               type : 'value'
           }
       ],
       grid: {
          x: 55,
          y: 20,
          x2: 20,
          y2: 60
       },
       series : [
           {
               name: 'Record',
               type: 'line',
               data: [ 0, 0, 0, 0, 0, 0, 0 ]
           }
       ]
   } ;
}());