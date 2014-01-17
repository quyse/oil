Components.utils.import('resource://gre/modules/devtools/dbg-server.jsm');
if (!DebuggerServer.initialized) {
  DebuggerServer.init();
  DebuggerServer.addBrowserActors();
}
DebuggerServer.openListener(6000);

Components.utils.import('chrome://oil/content/oil.js');

function onRepoConnect() {
	var params = {};
	window.openDialog('connectrepo.xul', '', 'chrome,modal,centerscreen,resizable', params);
	if(params.repo) {
		OIL.repo = params.repo;
		watchRepo();
	}
}

function onAssign() {
	var key = document.getElementById("textboxAssignKey").value;
	var value = document.getElementById("textboxAssignValue").value;
	OIL.repo.Change(key, value);
}

//*** syncing

var repoWatching = false;
function watchRepo() {
	if(repoWatching)
		return;

	repoWatching = true;
	OIL.repo.Watch(function(ok, message) {
		repoWatching = false;

		if(ok) {
			if(message)
				syncRepo();
			else
				watchRepo();
		}
		else
			alert(message);
	});
}

var repoSyncing = false;
function syncRepo() {
	if(repoSyncing)
		return;
	repoSyncing = true;
	OIL.repo.Sync(function(ok, message) {
		repoSyncing = false;

		if(ok) {
			if(message)
				syncRepo();
			else
				watchRepo();
		}
		else
			alert(message);
	});
}

window.onload = function() {
	OIL.init(document.getElementById('oil'));
};
