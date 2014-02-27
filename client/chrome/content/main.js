'use strict';

Components.utils.import('resource://gre/modules/devtools/dbg-server.jsm');
if (!DebuggerServer.initialized) {
  DebuggerServer.init();
  DebuggerServer.addBrowserActors();
}
DebuggerServer.openListener(6000);

Components.utils.import('chrome://oil/content/oil.js');

// set profile path
OIL.profilePath = Components.classes["@mozilla.org/file/directory_service;1"]
	.getService(Components.interfaces.nsIProperties)
	.get("ProfD", Components.interfaces.nsIFile);

function onRepoConnect() {
	var params = {};
	window.openDialog('connectrepo.xul', '', 'chrome,modal,centerscreen,resizable', params);
	if(params.repo) {
		OIL.repo = params.repo;
		OIL.entityManager = OIL.repo.GetEntityManager();
		OIL.registerEntitySchemes(OIL.entityManager.GetSchemeManager());

		OIL.syncRepo();
		OIL.repo.SetUndoRedoChangedCallback(onUndoRedoChanged);

		window.openDialog('syncprogress.xul', '', 'chrome,modal,centerscreen,close=no');

		// show root folder
		createTool("folder", {
			entity: OIL.uuids.entities.root
		});
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

	update(undoAction, "menuUndo", "commandUndo", "undo");
	update(redoAction, "menuRedo", "commandRedo", "redo");
}

function onShowRoot() {
	createTool("folder", {
		entity: OIL.uuids.entities.root
	});
}

function onMaintenanceRootCreate() {
	// check that there is no root
	var entity = OIL.entityManager.GetEntity(OIL.uuids.entities.root);
	if(entity.GetScheme())
		return;

	// create (hack way)
	var action = OIL.createAction("create root");
	action.AddChange(OIL.eid2f(OIL.uuids.entities.root), OIL.eid2f(OIL.uuids.schemes.folder));
	OIL.finishAction(action);
}

// get preferences service
OIL.prefs = Components.classes["@mozilla.org/preferences-service;1"]
	.getService(Components.interfaces.nsIPrefService)
	.getBranch("oil.");

function createTool(page, params) {
	// get tabbox from prefs
	var tabbox = OIL.ToolTabbox.get(OIL.prefs.getCharPref("tool-" + page + ".place"));
	if(!tabbox)
		tabbox = OIL.ToolTabbox.get("main");

	createToolInTabbox(tabbox, page, params);
}
OIL.createTool = createTool;

function createToolInTabbox(tabbox, page, params) {
	// create tool tab
	var toolTab = new OIL.ToolTab();
	tabbox.appendTab(toolTab);

	// compose url
	var url = "chrome://oil/content/tool-" + page + ".xul#tab=" + toolTab.id;
	for(var i in params)
		url += "&" + i + "=" + params[i];

	// init iframe
	var tabpanel = toolTab.tabpanel;
	var iframe = document.createElementNS(XUL_NS, "iframe");
	iframe.setAttribute("src", url);
	iframe.flex = 1;
	tabpanel.appendChild(iframe);

	// set duplicate command
	toolTab.duplicateTool = function() {
		createToolInTabbox(this.parent, page, toolTab.params);
	};
};

var toolTabContextTarget = null;
function onToolTabContextShowing(event) {
	toolTabContextTarget = event.target.triggerNode;
	document.getElementById("contextMenuToolTabDuplicate").hidden = toolTabContextTarget.toolTab.duplicateTool === undefined;
}
function onToolTabContextHiding(event) {
	toolTabContextTarget = null;
}
function onToolTabContextDuplicate(event) {
	toolTabContextTarget.toolTab.duplicateTool();
}
function onToolTabContextClose(event) {
	toolTabContextTarget.toolTab.close();
}

var toolspace;

window.addEventListener('load', function() {
	OIL.init(document.getElementById('oil'));
	OIL.core.SetProfilePath(OIL.profilePath.path);
	OIL.core.Init();
	onUndoRedoChanged(null, null);

	// create toolspace
	var layout = OIL.prefs.getCharPref("layout");
	toolspace = OIL.ToolSpace.deserialize(JSON.parse(layout));
	toolspace.box.flex = 1;
	var placeholder = document.getElementById("toolspace");
	placeholder.parentNode.replaceChild(toolspace.box, placeholder);

	// register sync status feedback
	var labelSyncStatus = document.getElementById("labelSyncStatus");
	OIL.syncProgress.onSynced.addTarget(function() {
		labelSyncStatus.value = "synced";
	});
	OIL.syncProgress.onUnsynced.addTarget(function() {
		labelSyncStatus.value = "syncing...";
	});
});

window.addEventListener('unload', function() {
	OIL.prefs.setCharPref("layout", JSON.stringify(toolspace.serialize()));
});
