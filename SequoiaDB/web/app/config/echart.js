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

   window.SdbSacManagerConf.pieEchart = {
    tooltip : {
        trigger: 'item',
        formatter: "{a} <br/>{b} : {c} ({d}%)"
    },
    legend: {
        orient : 'vertical',
        x : 'left',
        data:['直接访问','邮件营销','联盟广告','视频广告','搜索引擎']
    },
    toolbox: {
        show : false,
        feature : {
            mark : {show: false},
            dataView : {show: false, readOnly: false},
            magicType : {
                show: false, 
                type: ['pie', 'funnel'],
                option: {
                    funnel: {
                        x: '25%',
                        width: '50%',
                        funnelAlign: 'center',
                        max: 1548
                    }
                }
            },
            restore : {show: false},
            saveAsImage : {show: false}
        }
    },
    calculable : true,
    series : [
        {
            name:'访问来源',
            type:'pie',
            radius : ['50%', '70%'],
            itemStyle : {
                normal : {
                    label : {
                        show : false
                    },
                    labelLine : {
                        show : false
                    }
                },
                emphasis : {
                    label : {
                        show : true,
                        position : 'center',
                        textStyle : {
                            fontSize : '30',
                            fontWeight : 'bold'
                        }
                    }
                }
            },
            data:[
                {value:335, name:'直接访问'},
                {value:310, name:'邮件营销'},
                {value:234, name:'联盟广告'},
                {value:135, name:'视频广告'},
                {value:1548, name:'搜索引擎'}
            ]
        }
    ]
};
                    
}());