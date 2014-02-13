'use strict';

Components.utils.import('chrome://oil/content/oil.js');

//*** syncing

var maxRequestTryings = 3;

var repoWatching = false;
var repoWatchTryings = maxRequestTryings;
function watchRepo() {
	if(repoWatching)
		return;

	repoWatching = true;

	setTimeout(function() {
		OIL.repo.Watch(function(ok, message) {
			repoWatching = false;

			if(ok) {
				repoWatchTryings = maxRequestTryings;
				if(message)
					syncRepo();
				else
					watchRepo();
			}
			else {
				if(--repoWatchTryings <= 0)
					alert(message);
				else
					watchRepo();
			}
		});
	}, 0);
}
OIL.watchRepo = watchRepo;

var syncProgress = {
	pushDone: 0,
	pushTotal: 0,
	pullDone: 0,
	pullTotal: 0,
	onChanged: new Event()
};
OIL.syncProgress = syncProgress;

var repoSyncing = false;
var repoSyncTryings = maxRequestTryings;
function syncRepo() {
	if(repoSyncing)
		return;

	repoSyncing = true;

	setTimeout(function() {
		OIL.repo.Sync(function(ok, message) {
			repoSyncing = false;

			if(ok) {
				// do stats
				OIL.syncProgress.pushTotal = OIL.syncProgress.pushDone + OIL.repo.GetPushLag();
				OIL.syncProgress.pushDone += OIL.repo.GetPushedKeysCount();
				OIL.syncProgress.pullTotal = OIL.syncProgress.pullDone + OIL.repo.GetPullLag();
				OIL.syncProgress.pullDone += OIL.repo.GetPulledKeysCount();
				syncProgress.onChanged.fire(
					OIL.syncProgress.pushDone, OIL.syncProgress.pushTotal,
					OIL.syncProgress.pullDone, OIL.syncProgress.pullTotal);

				OIL.repo.ProcessEvents();
				repoSyncTryings = maxRequestTryings;
				if(message)
					syncRepo();
				else {
					OIL.syncProgress.pushDone = 0;
					OIL.syncProgress.pushTotal = 0;
					OIL.syncProgress.pullDone = 0;
					OIL.syncProgress.pullTotal = 0;
					watchRepo();
				}
			}
			else {
				if(--repoSyncTryings <= 0)
					alert(message);
				else
					syncRepo();
			}
		});
	}, 0);
}
OIL.syncRepo = syncRepo;

//*** actions

function createAction(description) {
	return new OIL.classes.Inanity.Oil.Action(description);
};
OIL.createAction = createAction;

function finishAction(action) {
	OIL.repo.MakeAction(action);
	OIL.repo.ProcessEvents();
	syncRepo();
}
OIL.finishAction = finishAction;

// fast create/add changes/finish function
function makeAction(description, changes) {
	var action = createAction(description);
	for(var i = 0; i < changes.length; ++i)
		action.AddChange(changes[i][0], changes[i][1]);
	finishAction(action);
}
OIL.makeAction = makeAction;

function undo() {
	OIL.repo.Undo();
	syncRepo();
}
OIL.undo = undo;

function redo() {
	OIL.repo.Redo();
	syncRepo();
}
OIL.redo = redo;

//*** console

var consoleService = Components.classes['@mozilla.org/consoleservice;1']
		.getService(Components.interfaces.nsIConsoleService);
function log(str) {
	consoleService.logStringMessage(str);
}
OIL.log = log;

//*** things

OIL.f2s = function(file) {
	return OIL.classes.Inanity.Strings.File2String(file);
};
OIL.s2f = function(string) {
	return OIL.classes.Inanity.Strings.String2File(string);
};

OIL.f2eid = function(file) {
	return OIL.classes.Inanity.Oil.EntityId.FromFile(file);
};
OIL.eid2f = function(eid) {
	return OIL.classes.Inanity.Oil.EntityId.StaticToFile(eid);
};

OIL.getEntityFromToolWindow = function(window) {
	var a = /^\#(.+)$/.exec(window.location.hash);
	if(!a) {
		window.location = "tool-wrong.xul";
		return null;
	}
	return OIL.entityManager.GetEntity(a[1]);
};
