'use strict';

Components.utils.import('chrome://oil/content/oil.js');

window.setDialogTitle = function(title) {
	document.getElementById("tooldialog").setAttribute("title", title);
};

window.setToolTabTitle = function(title) {
	document.getElementById("header").setAttribute("title", title);
};

window.setAcceptOk = function(ok) {
	document.getElementById("tooldialog").getButton("accept").disabled = !ok;
};

function onAccept() {
	window.arguments[1].ok = true;
	return true;
}

var toolDialog;

window.addEventListener("load", function(event) {
	var params = window.arguments[0];

	toolDialog = new OIL.ToolDialog(window, params.toolParams);

	window.toolDialog = toolDialog;
	document.getElementById("iframe").setAttribute("src", toolDialog.getUrl(params.page));
	toolDialog.setTitle(params.title);
	document.getElementById("tooldialog").setAttribute("buttonlabelaccept", params.acceptButtonLabel || "accept");
});

window.addEventListener("unload", function(event) {
	toolDialog.destroy();
});
