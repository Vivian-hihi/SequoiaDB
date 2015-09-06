function fly(){
    $(".cloud2").animate({left:50},40000,'swing',function(){
        $('.cloud2').css( 'left',2500 );
        fly() ;
    } ) ;
}
$(document).ready(function(){
    fly();

});

var _speed = 900 ;

function fly1( times ){
	$(".cloud1").animate( { left: -1250 }, times, 'linear', function(){
		var left = $( window ).width() + 850 ;
		var distance = left + 1250 ;
		$('.cloud1').css( 'left', left );
		fly1( distance / _speed * 60000 ) ;
	} ) ;
}
$(document).ready(function(){
	var times = 2150 / _speed ;
	fly1( times * 60000 );
});

//让登陆框自适应居中
$(window).ready(function(){
	$('.landed').css({
		position:'absolute',
		left: ($(window).width() - $('.landed').outerWidth())/2,
		top: ($(window).height() - $('.landed').outerHeight())/2 + $(document).scrollTop()
	});
});
$(window).ready();

$(window).resize(function(){
	$('.landed').css({
		position:'absolute',
		left: ($(window).width() - $('.landed').outerWidth())/2,
		top: ($(window).height() - $('.landed').outerHeight())/2 + $(document).scrollTop()
	});
});
$(window).resize();