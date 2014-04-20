'use strict';

Components.utils.import('chrome://oil/content/oil.js');

/* window.arguments[0]: {
	setProgressCallback = function(callback) {},
	setCancelCallback = function(callback) {},
	onStart = function() {},
	onUnload = function() {},
	title = "Title",
	description = "Description",
	cancelButtonText = "Cancel"
}
*/

var onUnloadCallback;

window.addEventListener('load', function() {

	var config = window.arguments[0];

	// set progress callback
	var progress = document.getElementById("progress");
	config.setProgressCallback(function(value, total) {
		progress.max = total;
		progress.value = value;
	});

	// set cancel callback
	var dialog = document.getElementById("dialog");
	if(config.setCancelCallback)
		config.setCancelCallback(function() {
			dialog.getButton("cancel").disabled = false;
			dialog.cancelDialog();
		});

	// remember unload callback
	onUnloadCallback = config.onUnload;

	// set attributes
	if(config.cancelButtonText !== null) {
		dialog.getButton("cancel").label = config.cancelButtonText || "cancel";
	} else {
		var cancelButton = dialog.getButton("cancel");
		cancelButton.hidden = true;
		cancelButton.disabled = true;
	}

	if(config.description)
		document.getElementById("description").textContent = config.description;

	if(config.title)
		dialog.setAttribute("title", config.title);

	// start
	if(config.onStart)
		config.onStart();
});

window.addEventListener('unload', function() {
	if(onUnloadCallback)
		onUnloadCallback();
});
