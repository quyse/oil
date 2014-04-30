'use strict';

Components.utils.import('chrome://oil/content/oil.js');

function onUrlClick() {
	var ioService = Components.classes["@mozilla.org/network/io-service;1"]
		.getService(Components.interfaces.nsIIOService);

	OIL.log(document.getElementById("labelUrl").value);
	var uri = ioService.newURI(document.getElementById("labelUrl").value, null, null);

	var externalProtocolService = Components.classes["@mozilla.org/uriloader/external-protocol-service;1"]
		.getService(Components.interfaces.nsIExternalProtocolService);

	externalProtocolService.loadURI(uri, window);
}

window.addEventListener("load", function() {
	document.getElementById("labelVersion").value = OIL.currentVersion;
});
