'use strict';

Components.utils.import('chrome://oil/content/oil.js');

var plugin;

window.addEventListener('load', function() {
	plugin = document.getElementById("plugin");

	var updatePlugin = function() {
		plugin.Invalidate();
		setTimeout(updatePlugin, 100);
	};
	setTimeout(updatePlugin, 0);
});
