'use strict';

Components.utils.import('chrome://oil/content/oil.js');

//*** Timers

// We use Mozilla timers because ordinary setTimeout gets blocked
// by modal dialogs. https://bugzilla.mozilla.org/show_bug.cgi?id=52209
var timerClass = Components.classes["@mozilla.org/timer;1"];
// Keep references to pending timers in order to assure firing.
var timers = [];
var timerNumber = 0;
OIL.setTimeout = function(callback, delay) {
	var timer = timerClass.createInstance(Components.interfaces.nsITimer);
	var timerId = timerNumber++;
	timers[timerId] = timer;
	timer.initWithCallback({
		notify: function(timer) {
			delete timers[timerId];
			callback();
		}
	}, delay, Components.interfaces.nsITimer.TYPE_ONE_SHOT);
};

//*** Event

var Event = OIL.Event = function() {
	this.targetNumber = 0;
	this.targets = [];
};
Event.prototype.addTarget = function(target) {
	this.targets[this.targetNumber] = target;
	return this.targetNumber++;
};
Event.prototype.removeTarget = function(targetNumber) {
	delete this.targets[targetNumber];
};
Event.prototype.fire = function() {
	for(var i in this.targets)
		this.targets[i].apply(undefined, arguments);
};

//*** weak callbacks

var weakCallbackObjects = new WeakMap();
/// Returns function which keeps a weak reference to an object argument.
OIL.weakCallback = function(object, method) {
	var key = {};
	weakCallbackObjects.set(key, object);
	return function() {
		var object = weakCallbackObjects.get(key);
		if(object === undefined)
			return;
		return object[method].apply(object, arguments);
	};
};

//*** quit

OIL.quit = function() {
	Components.classes['@mozilla.org/toolkit/app-startup;1']
		.getService(Components.interfaces.nsIAppStartup)
		.quit(Components.interfaces.nsIAppStartup.eAttemptQuit);
};

//*** syncing

var maxRequestTryings = 3;

var repoWatching = false;
var repoWatchTryings = maxRequestTryings;
function watchRepo() {
	if(repoWatching)
		return;

	repoWatching = true;

	OIL.setTimeout(function() {
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
	onChanged: new Event(),
	onSynced: new Event(),
	onUnsynced: new Event()
};
OIL.syncProgress = syncProgress;

var repoSyncing = false;
var repoSyncTryings = maxRequestTryings;
function syncRepo() {
	if(repoSyncing)
		return;

	repoSyncing = true;

	OIL.syncProgress.onUnsynced.fire();

	OIL.setTimeout(function() {
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
					OIL.syncProgress.onSynced.fire();
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
