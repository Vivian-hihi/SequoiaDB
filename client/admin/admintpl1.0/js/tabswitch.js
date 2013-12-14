    $(document).ready(function(){
    var j=0;
    $("#tabMenus li:first").addClass("current");
    $("#tabCons div:first").show();
    $("#tabMenus li").each(function(i){
    $(this).click(function(){
    $(this).addClass("current").siblings().removeClass();
    $("#tabCons > div").hide();
    $("#tabCons div:eq(" + i + ")").show();
    })
    })
    })