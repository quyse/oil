'use strict';

Components.utils.import('chrome://oil/content/oil.js');

var onChangedTarget;
var onSyncedTarget;

window.addEventListener('load', function() {
	var progress = document.getElementById("progress");
	onChangedTarget = OIL.syncProgress.onChanged.addTarget(function(pushDone, pushTotal, pullDone, pullTotal) {
		progress.max = pushTotal + pullTotal;
		progress.value = pushDone + pullDone;
	});

	var dialog = document.getElementById("syncprogress");
	onSyncedTarget = OIL.syncProgress.onSynced.addTarget(function() {
		dialog.acceptDialog();
	});

	var callback = window.arguments[0];
	if(callback)
		callback();
});

window.addEventListener('unload', function() {
	OIL.syncProgress.onChanged.removeTarget(onChangedTarget);
	OIL.syncProgress.onSynced.removeTarget(onSyncedTarget);
});
