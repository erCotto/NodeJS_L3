
function creer_page() {
	console.log("creer_page")

	jQuery.get('template.html', function(data) {
	
		var pos = data.indexOf("<!-- contenu -->")
		console.log("pos = "+pos)
		
		jQuery("#template").html(data.substring(0,pos) + jQuery("#content").html() + data.substring(pos))
		
		
		
		
	});
}

