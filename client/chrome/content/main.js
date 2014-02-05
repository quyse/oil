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

		OIL.watchRepo();
		OIL.repo.SetUndoRedoChangedCallback(onUndoRedoChanged);
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

/// Panel on which new tabs should be created.
var mainTabbox;

function getToolUrl(page, param) {
	return "chrome://oil/content/tool-" + page + ".xul" + (param ? "#" + param : "");
}
OIL.getToolUrl = getToolUrl;

function createTool(title, page, param) {
	var tabNumber = createToolTab(mainTabbox, title);
	var tabpanel = getToolTabpanel(tabNumber);
	var iframe = document.createElementNS(XUL_NS, "iframe");
	iframe.setAttribute("src", getToolUrl(page, param));
	iframe.flex = 1;
	tabpanel.appendChild(iframe);
};
OIL.createTool = createTool;

window.onload = function() {
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

		if(i == 1) {
			tabbox.flex = 1;
			mainTabbox = tabbox;
		}
	}
};
