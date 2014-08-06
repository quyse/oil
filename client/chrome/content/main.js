'use strict';

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
		OIL.initIds(OIL.entityManager.GetSchemeManager());

		OIL.repo.SetUndoRedoChangedCallback(onUndoRedoChanged);

		{
			let onChangedTarget;
			let onSyncedTarget;
			let progressCallback;
			let cancelCallback;
			window.openDialog('progress.xul', '', 'chrome,modal,centerscreen,close=no', {
				setProgressCallback: function(callback) {
					progressCallback = callback
				},
				setCancelCallback: function(callback) {
					cancelCallback = callback
				},
				onStart: function() {
					onChangedTarget = OIL.syncProgress.onChanged.addTarget(function(pushDone, pushTotal, pullDone, pullTotal) {
						progressCallback(pushDone + pullDone, pushTotal + pullTotal);
					});
					onSyncedTarget = OIL.syncProgress.onSynced.addTarget(cancelCallback);
					OIL.syncRepo();
				},
				onUnload: function() {
					OIL.syncProgress.onChanged.removeTarget(onChangedTarget);
					OIL.syncProgress.onSynced.removeTarget(onSyncedTarget);
				},
				title: "syncing...",
				description: "syncing to server version of the repo, please wait...",
				cancelButtonText: "don't wait"
			});
		}

		// show root folder
		var rootFolderToolTab = createTool("folder", {
			entity: OIL.ids.entities.root
		});
		// TEST
		// show default viewer
		var defaultToolTab = createTool("default", {});
		rootFolderToolTab.addDependentToolTab(defaultToolTab);
		// show properties
		var entityToolTab = createTool("entity", {});
		rootFolderToolTab.addDependentToolTab(entityToolTab);

		// init upgrade
		initClientUpgrade();
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
		entity: OIL.ids.entities.root
	});
}

function onMaintenanceCreateEntity() {
	try {
		// prompt for entity id
		var entityIdWrap = { value: "" };
		if(!OIL.getPromptService().prompt(window, "create entity", "enter new entity id:", entityIdWrap, null, { value: false }))
			return;
		var entityId = entityIdWrap.value;
		entityId = OIL.ids.entities[entityId] || entityId;

		// check that there is no such entity
		var entity = OIL.entityManager.GetEntity(entityId);
		if(entity.GetScheme())
			throw "entity already exists";

		// prompt for entity scheme id
		var entitySchemeIdWrap = { value: "" };
		if(!OIL.getPromptService().prompt(window, "create entity", "enter new entity scheme id:", entitySchemeIdWrap, null, { value: false }))
			return;
		var entitySchemeId = entitySchemeIdWrap.value;
		entitySchemeId = OIL.ids.schemes[entitySchemeId] || entitySchemeId;

		// create (hack way)
		var action = OIL.createAction("create entity (maintenance)");
		action.AddChange(OIL.eid2f(entityId), OIL.esid2f(entitySchemeId));
		OIL.finishAction(action);
	} catch(e) {
		OIL.getPromptService().alert(window, "can't create entity", e);
	}
}

function onMaintenanceOpenEntity() {
	try {
		// prompt for entity id
		var entityIdWrap = { value: "" };
		if(!OIL.getPromptService().prompt(window, "open entity", "enter existing entity id:", entityIdWrap, null, { value: false }))
			return;
		var entityId = entityIdWrap.value;
		entityId = OIL.ids.entities[entityId] || entityId;

		// check that it converts to id
		OIL.eid2f(entityId);

		// open
		createTool("entity", {
			entity: entityId
		});

	} catch(e) {
		OIL.getPromptService().alert(window, "can't open entity", e);
	}
}

function onMaintenanceOpenTool() {
	try {
		// prompt for str
		var strWrap = { value: "" };
		if(!OIL.getPromptService().prompt(window, "open tool", "<page>:{params}", strWrap, null, { value: false }))
			return;

		var str = strWrap.value;

		// find page
		var pageLength = str.indexOf(":");
		if(pageLength < 0)
			throw "can't get page from str";

		var page = str.substr(0, pageLength);
		var params = JSON.parse(str.substr(pageLength + 1));

		createTool(page, params);

	} catch(e) {
		OIL.getPromptService().alert(window, "can't open tool", e);
	}
}

function onMaintenanceVacuum() {
	try {
		OIL.repo.GetClientRepo().Vacuum();
		OIL.getPromptService().alert(window, "defragment local cache", "completed successfully");
	} catch(e) {
		OIL.getPromptService().alert(window, "defragment local cache error", e);
	}
}

function onMaintenanceIntegrityCheck() {
	try {
		OIL.getPromptService().alert(window, "local cache integrity check", OIL.repo.GetClientRepo().IntegrityCheck());
	} catch(e) {
		OIL.getPromptService().alert(window, "local cache integrity check", e);
	}
}

// get preferences service
OIL.prefs = Components.classes["@mozilla.org/preferences-service;1"]
	.getService(Components.interfaces.nsIPrefService)
	.getBranch("oil.");

function createTool(page, params) {
	// get tabbox from prefs
	var tabbox = null;
	try {
		tabbox = OIL.ToolTabbox.get(OIL.prefs.getCharPref("tool-" + page + ".place"));
	} catch(e) {}
	if(!tabbox)
		tabbox = OIL.ToolTabbox.get("main");

	return createToolInTabbox(tabbox, page, params);
}
OIL.createTool = createTool;

function createToolInTabbox(tabbox, page, params) {
	// create tool tab
	var toolTab = new OIL.ToolTab();
	tabbox.appendTab(toolTab);

	// navigate tool tab
	toolTab.page = page;
	toolTab.params = params;
	toolTab.navigate();

	return toolTab;
};

/// open tool dialog
/** params = {
	page: "page",
	title: "dialog title",
	acceptButtonLabel: "accept button label",
	toolParams: { tool params }
} */
function openToolDialog(params) {
	var result = {
		ok: false
	};
	window.openDialog('tooldialog.xul', '', 'chrome,modal,centerscreen,resizable', params, result);
	return result.ok;
}
OIL.openToolDialog = openToolDialog;

var toolTabContextTarget = null;
function onToolTabContextShowing(event) {
	toolTabContextTarget = event.target.triggerNode;

	// hide "make independent" command if tab is already independent
	document.getElementById("contextMenuToolTabMakeIndependent").hidden = toolTabContextTarget.toolTab.independent;
}
function onToolTabContextHiding(event) {
	toolTabContextTarget = null;
}
function onToolTabContextDuplicate(event) {
	createToolInTabbox(toolTabContextTarget.toolTab.parent, toolTabContextTarget.toolTab.page, toolTabContextTarget.toolTab.params);
}
function onToolTabContextMakeIndependent(event) {
	toolTabContextTarget.toolTab.independent = true;
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

// get current version of app
var currentVersion = OIL.currentVersion = Components.classes["@mozilla.org/xre/app-info;1"]
	.getService(Components.interfaces.nsIXULAppInfo).version

function initClientUpgrade() {
	var scheme, version, url, upgradeNeeded = false;

	// compare versions (strings like 1.12.435)
	function versionLess(a, b) {
		a = a.split('.');
		b = b.split('.');
		for(var i = 0; i < a.length && i < b.length; ++i) {
			// convert parts to numbers
			var aa = parseInt(a[i], 10);
			var bb = parseInt(b[i], 10);
			// if not a number, compare as strings
			if(Number.isNaN(aa) || Number.isNaN(bb)) {
				aa = a[i];
				bb = b[i];
			}
			if(aa < bb)
				return true;
			if(aa > bb)
				return false;
		}

		return a.length < b.length;
	}

	var check = function() {
		upgradeNeeded = scheme && version && url && versionLess(currentVersion, version);

		var label = document.getElementById("labelClientUpgrade");
		label.value = upgradeNeeded ? "available client upgrade to version " + version : "";
		label.hidden = !upgradeNeeded;
	};

	onClientUpgrade = function() {
		if(!upgradeNeeded)
			return;

		// confirm upgrade
		if(!OIL.getPromptService().confirm(window, "client upgrade",
			"Server announced that preferred client version is " + version + ".\n" +
			"Current version is " + currentVersion + ".\n" +
			"Download client upgrade from " + url + "?"))
			return;

		// launch download
		var ioService = Components.classes["@mozilla.org/network/io-service;1"]
			.getService(Components.interfaces.nsIIOService);

		var uri = ioService.newURI(url, null, null);

		var externalProtocolService = Components.classes["@mozilla.org/uriloader/external-protocol-service;1"]
			.getService(Components.interfaces.nsIExternalProtocolService);

		externalProtocolService.loadURI(uri, window);
	};

	// get client version entity
	var entity = OIL.entityManager.GetEntity(OIL.ids.entities.client_version);
	var callback = function(type, key, value) {
		switch(type) {
		case "scheme":
			scheme = value;
			break;
		case "field":
			switch(key) {
			case OIL.ids.schemeDescs.client_version.fields.version:
				version = value;
				break;
			case OIL.ids.schemeDescs.client_version.fields.url:
				url = value;
				break;
			default:
				return;
			}
			break;
		default:
			return;
		}
		check();
	};
	clientVersionEntityCallback = entity.AddCallback(callback);
	callback("scheme", null, entity.GetScheme());
	clientVersionEntityCallback.EnumerateFields();
}
var clientVersionEntityCallback;
var onClientUpgrade;

function onHelpAbout() {
	window.openDialog('about.xul', '', 'chrome,modal,centerscreen');
}

// enable debug server if needed
try {
	if(OIL.prefs.getBoolPref("debug")) {
		Components.utils.import('resource://gre/modules/devtools/dbg-server.jsm');
		if (!DebuggerServer.initialized) {
			DebuggerServer.init();
			DebuggerServer.addBrowserActors();
		}
		DebuggerServer.openListener(6000);
		OIL.log("debug server enabled");
	}
} catch(e) {}
