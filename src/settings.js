Pebble.addEventListener("showConfiguration",
function(){
	Pebble.openURL('https://dl.dropboxusercontent.com/u/17829339/codiqa_jquery_game_time.html');		
});

Pebble.addEventListener("webviewclosed",
function(e){
	try{
		var options = JSON.parse(decodeURIComponent(e.response));
		var game = options.game;
		var color = options.color;
		console.log("Game: " + game + ", Color: " + color);
		Pebble.sendAppMessage({"GAME" : game+"", "COLOR" : color+""});
	}
	catch(err){
		console.error(err);
	}
});