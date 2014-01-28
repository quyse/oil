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
				OIL.repo.ProcessEvents();
				repoSyncTryings = maxRequestTryings;
				if(message)
					syncRepo();
				else
					watchRepo();
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

function makeAction(description, changes) {
	var action = new OIL.classes.Inanity.Oil.Action(description);
	for(var i = 0; i < changes.length; ++i)
		action.AddChange(changes[i][0], changes[i][1]);
	OIL.repo.MakeAction(action);
	OIL.repo.ProcessEvents();
	syncRepo();
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
