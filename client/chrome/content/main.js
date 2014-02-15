'use strict';

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
		OIL.entityManager = OIL.repo.GetEntityManager();
		OIL.registerEntitySchemes(OIL.entityManager.GetSchemeManager());

		OIL.syncRepo();
		OIL.repo.SetUndoRedoChangedCallback(onUndoRedoChanged);

		window.openDialog('syncprogress.xul', '', 'chrome,modal,centerscreen');

		// TEST
		createTool("Project", "folder", OIL.uuids.rootFolder);
	}
}

function onUndoRedoChanged(undoAction, redoAction) {
	var update = function(action, menuId, commandId, label) {
		var menuitem = document.getElementById(menuId);
		var command = document.getElementById(commandId);
		if(action) {
			menuitem.label = label + " " + action.GetDescription();
			command.setAttribute("disabled", "false");
		} else {
			menuitem.label = label;
			command.setAttribute("disabled", "true");
		}
	};

	update(undoAction, "menuUndo", "commandUndo", "Undo");
	update(redoAction, "menuRedo", "commandRedo", "Redo");
}

function createRootFolder() {
	// check that there is no root folder
	var entity = OIL.entityManager.GetEntity(OIL.uuids.rootFolder);
	if(entity.GetScheme())
		return;

	// create (hack way)
	var action = OIL.createAction("create root folder");
	action.AddChange(OIL.eid2f(OIL.uuids.rootFolder), OIL.eid2f(OIL.uuids.schemes.folder));
	OIL.finishAction(action);
}

// get preferences service
OIL.prefs = Components.classes["@mozilla.org/preferences-service;1"]
	.getService(Components.interfaces.nsIPrefService)
	.getBranch("oil.");

var tabboxes = {};

function getToolUrl(page, param) {
	return "chrome://oil/content/tool-" + page + ".xul" + (param ? "#" + param : "");
}
OIL.getToolUrl = getToolUrl;

function createTool(title, page, param) {
	// get tabbox from prefs
	var tabbox = tabboxes[OIL.prefs.getCharPref("tool-" + page + ".place")] || tabboxes.main;
	// create tool tab
	var tabNumber = createToolTab(tabbox, title);
	// init iframe
	var tabpanel = getToolTabpanel(tabNumber);
	var iframe = document.createElementNS(XUL_NS, "iframe");
	iframe.setAttribute("src", getToolUrl(page, param));
	iframe.flex = 1;
	tabpanel.appendChild(iframe);
};
OIL.createTool = createTool;

window.addEventListener('load', function() {
	OIL.init(document.getElementById('oil'));
	onUndoRedoChanged(null, null);

	// create panels
	var toolspace = document.getElementById("toolspace");

	for(var i = 0; i < 3; ++i) {
		if(i == 1 || i == 2) {
			var splitter = document.createElementNS(XUL_NS, "splitter");
			splitter.setAttribute("collapse", i == 1 ? "before" : "after");
			splitter.appendChild(document.createElementNS(XUL_NS, "grippy"));
			toolspace.appendChild(splitter);
		}

		var tabbox = createToolTabbox();

		toolspace.appendChild(tabbox);

		if(i == 0) {
			tabboxes["left"] = tabbox;
		}
		if(i == 1) {
			tabbox.flex = 1;
			tabboxes["main"] = tabbox;
		}
		if(i == 2) {
			tabboxes["right"] = tabbox;
		}
	}

	// register sync status feedback
	var labelSyncStatus = document.getElementById("labelSyncStatus");
	OIL.syncProgress.onSynced.addTarget(function() {
		labelSyncStatus.value = "Synced";
	});
	OIL.syncProgress.onUnsynced.addTarget(function() {
		labelSyncStatus.value = "Syncing...";
	});
});
