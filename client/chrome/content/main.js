Components.utils.import('resource://gre/modules/devtools/dbg-server.jsm');
if (!DebuggerServer.initialized) {
  DebuggerServer.init();
  DebuggerServer.addBrowserActors();
}
DebuggerServer.openListener(6000);

Components.utils.import('chrome://oil/content/oil.js');

/// Current opened repo. Instance of Inanity.Oil.ScriptRepo.
var currentRepo = null;

function onRepoConnect() {
	var params = {};
	window.openDialog('connectrepo.xul', '', 'chrome,modal,centerscreen,resizable', params);
	if(params.repo)
		currentRepo = params.repo;
}

window.onload = function() {
	OIL.init(document.getElementById('oil'));
};
