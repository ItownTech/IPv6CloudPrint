$(function () {
	document.getElementById("js_bd").style.height = (document.body.clientHeight - 144) +"px";
	$(window).resize(function () {
		$("#js_bd").height($(window).height() - 42);
	});
	var bangdingshoujihtml;
	
	var pagebangdingshouji = {};
	layer.bangdingshouji = function(optionbangdingshouji){
		layer.closeAll();
		options = optionbangdingshouji || {};
		$.layer({
			type: 1,
			title: '���ֻ�',
			offset: [($(window).height() - 242)/2+'px', ''],
			border : [5, 0.5, '#666'],
			area: ['695px','242px'],
			shadeClose: true,
			page: pagebangdingshouji,
			closeBtn: [0, true]
		});
	};
		
	$('#Btnbangdingshouji').on('click', function(){
		//����Ѿ����������ֱ�Ӷ�ȡ����ڵ�
		if(bangdingshoujihtml){
			pagebangdingshouji.html = bangdingshoujihtml;
		} else {
			pagebangdingshouji.url = APP+'/User/bangdingshouji'
			pagebangdingshouji.ok = function(databangdingshouji){
				bangdingshoujihtml = databangdingshouji; //�����¼�ڵ�
			}
		}
		layer.bangdingshouji();    
	});
	
	$('.pay_this_order').click(function(){
		var print_order_id = $(this).parent().attr('print_order_id');
		var pay_order_box = {};
		pay_order_box.url = APP+'/User/pay_order/print_order_id/'+print_order_id;
		layer.pay_order_box = function(optionlogin){
			options = optionlogin || {};
			$.layer({
				type: 1,
				title: "֧������#"+print_order_id,
				offset: [($(window).height() - 201)/2+'px', ''],
				border : [5, 0.5, '#666'],
				area: ['420px','201px'],
				shadeClose: false,
				page: pay_order_box,
				closeBtn: [0, true]
			});
		};
		layer.pay_order_box(); 
	});
 });