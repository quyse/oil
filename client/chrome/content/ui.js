'use strict';

const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
OIL.XUL_NS = XUL_NS;

const MIME_DRAG_TOOL_TAB = "application/x-inanityoil-ui-drag-tab";

var toolTabNewId = 0;
/// Tool tab class.
function ToolTab() {
	// get tab number
	this.id = ++toolTabNewId;

	// create tab
	this.tab = document.createElementNS(XUL_NS, "tab");
	this.tab.toolTab = this;
	this.tab.id = "toolTab" + this.id;
	this.tab.setAttribute("label", "tooltab");
	this.tab.setAttribute("context", "contextMenuToolTab");

	// create tabpanel
	this.tabpanel = document.createElementNS(XUL_NS, "tabpanel");
	this.tabpanel.toolTab = this;

	// setup drag events
	var This = this;
	this.tab.addEventListener("dragstart", function(event) {
		event.dataTransfer.setData(MIME_DRAG_TOOL_TAB, This.id);
		event.dataTransfer.effectAllowed = "move";
	});
	this.tab.addEventListener("dragover", function(event) {
		// check that event of the right type
		if(!event.dataTransfer.types.contains(MIME_DRAG_TOOL_TAB))
			return;

		// prevent default cancelling
		event.preventDefault();

		// get source tab number
		var sourceTabId = event.dataTransfer.getData(MIME_DRAG_TOOL_TAB);

		This.parent.moveTab(sourceTabId, This.id);
	});
};
OIL.ToolTab = ToolTab;
ToolTab.get = function(id) {
	var tab = document.getElementById("toolTab" + id);
	return tab ? tab.toolTab : null;
};
ToolTab.prototype.close = function() {
	this.parent.removeTab(this);
};
ToolTab.prototype.setTitle = function(title) {
	this.tab.setAttribute("label", title);
};

/// Tool tabbox class.
function ToolTabbox(id) {
	this.id = id;

	// create tabbox
	this.tabbox = document.createElementNS(XUL_NS, "tabbox");
	this.tabbox.toolTabbox = this;
	this.tabbox.id = "toolTabbox" + id;
	// create tabs
	this.tabs = document.createElementNS(XUL_NS, "tabs");
	this.tabbox.appendChild(this.tabs);
	// create tabpanels
	this.tabpanels = document.createElementNS(XUL_NS, "tabpanels");
	this.tabbox.appendChild(this.tabpanels);
	this.tabpanels.flex = 1;
	this.tabpanels.setAttribute("style", "padding:0 !important");

	// tab selection history
	this.selectionHistory = [];

	// setup events
	var This = this;
	this.tabbox.addEventListener("select", function(event) {
		var selectedIndex = This.tabbox.selectedIndex;
		var toolTab = This.tabs.childNodes[selectedIndex].toolTab;
		// avoid reselecting
		if(This.selectionHistory.length
			&& This.selectionHistory[This.selectionHistory.length - 1] == toolTab)
			return;

		This.removeTabFromSelectionHistory(toolTab);
		This.selectionHistory.push(toolTab);
	});
	this.tabbox.addEventListener("dragover", function(event) {
		// check that event of the right type
		if(!event.dataTransfer.types.contains(MIME_DRAG_TOOL_TAB))
			return;

		// prevent default cancelling
		event.preventDefault();

		// get source tab number
		var sourceTabId = event.dataTransfer.getData(MIME_DRAG_TOOL_TAB);

		This.moveTab(sourceTabId, -1);
	});
};
OIL.ToolTabbox = ToolTabbox;
ToolTabbox.get = function(id) {
	var tabbox = document.getElementById("toolTabbox" + id);
	return tabbox ? tabbox.toolTabbox : null;
};
ToolTabbox.prototype.appendTab = function(toolTab) {
	this.tabs.appendChild(toolTab.tab);
	this.tabpanels.appendChild(toolTab.tabpanel);
	this.addTab(toolTab);
};
ToolTabbox.prototype.addTab = function(toolTab) {
	// check that there's no such tab already
	toolTab.parent = this;
	// select new tab
	this.selectTab(toolTab);
};
ToolTabbox.prototype.removeTab = function(toolTab) {
	toolTab.parent = null;
	this.tabs.removeChild(toolTab.tab);
	this.tabpanels.removeChild(toolTab.tabpanel);
	// remove from selection history
	this.removeTabFromSelectionHistory(toolTab);
	// select last tab
	this.selectLastTab();
};
ToolTabbox.prototype.removeTabFromSelectionHistory = function(toolTab) {
	// remove tab from selection history
	for(;;) {
		var i = this.selectionHistory.indexOf(toolTab);
		if(i < 0)
			break;
		this.selectionHistory.splice(i, 1);
	}
};
ToolTabbox.prototype.selectTab = function(toolTab) {
	this.removeTabFromSelectionHistory(toolTab);
	this.selectionHistory.push(toolTab);
	this.selectLastTab();
};
ToolTabbox.prototype.selectLastTab = function() {
	if(!this.selectionHistory.length)
		return;

	var tab = this.selectionHistory[this.selectionHistory.length - 1].tab;

	var tabs = this.tabs.childNodes;
	for(var i = 0; i < tabs.length; ++i)
		if(tabs[i] == tab) {
			this.tabbox.selectedIndex = i;
			break;
		}
};
/// Move source tab to this tabbox.
ToolTabbox.prototype.moveTab = function(sourceTabId, destTabId) {
	if(sourceTabId == destTabId)
		return false;

	// for clarify
	var destTabbox = this;

	// get source elements
	var sourceToolTab = ToolTab.get(sourceTabId);
	var sourceTab = sourceToolTab.tab;
	var sourceTabpanel = sourceToolTab.tabpanel;

	var sourceTabbox = sourceToolTab.parent;

	// perform move
	if(destTabId >= 0) {
		var destToolTab = ToolTab.get(destTabId);
		var destTab = destToolTab.tab;
		var destTabpanel = destToolTab.tabpanel;

		// on same tabboxes we have to swap them carefully
		if(sourceTabbox == destTabbox) {
			// figure out tabs' relative positions
			var sourcePosition = -1, destPosition = -1;
			var destTabs = destTabbox.tabs.childNodes;
			for(var i = 0; i < destTabs.length; ++i) {
				if(destTabs[i] == sourceTab)
					sourcePosition = i;
				if(destTabs[i] == destTab)
					destPosition = i;
			}

			// perform swapping
			if(sourcePosition < destPosition) {
				var nextTab = destTab.nextSibling;
				if(nextTab) {
					destTabbox.tabs.insertBefore(sourceTab, nextTab);
					destTabbox.tabpanels.insertBefore(sourceTabpanel, destTabpanel.nextSibling);
				}
				else {
					destTabbox.tabs.appendChild(sourceTab);
					destTabbox.tabpanels.appendChild(sourceTabpanel);
				}
			}
			else {
				destTabbox.tabs.insertBefore(sourceTab, destTab);
				destTabbox.tabpanels.insertBefore(sourceTabpanel, destTabpanel);
			}

			// selection history is not changed, we only have to reselect
			destTabbox.selectLastTab();
		}
		// else tabboxes are different
		else {
			// remove tab from source tabbox
			sourceTabbox.removeTab(sourceToolTab);
			// just insert before dest tab
			destTabbox.tabs.insertBefore(sourceTab, destTab);
			destTabbox.tabpanels.insertBefore(sourceTabpanel, destTabpanel);
			// add tab to destination (it will be selected)
			destTabbox.addTab(sourceToolTab);

			// select last tab on source
			sourceTabbox.selectLastTab();
		}
	}
	// else no dest tab, just append to dest tabbox
	// only if it's different from source
	else if(sourceTabbox != destTabbox) {
		// remove tab from source tabbox
		sourceTabbox.removeTab(sourceToolTab);
		// append tab to destination
		destTabbox.appendTab(sourceToolTab);
		// select last tab on source
		sourceTabbox.selectLastTab();
	}
	else
		return;
};
ToolTabbox.prototype.serialize = function() {
	return {
		id: this.id
	};
};
ToolTabbox.deserialize = function(o) {
	return new ToolTabbox(o.id);
};

function ToolSplitter(collapse) {
	this.splitter = document.createElementNS(XUL_NS, "splitter");
	if(collapse == "before" || collapse == "after") {
		this.splitter.appendChild(document.createElementNS(XUL_NS, "grippy"));
		this.splitter.setAttribute("collapse", collapse);
	}
};
OIL.ToolSplitter = ToolSplitter;
ToolSplitter.prototype.serialize = function() {
	return {
		collapse: this.splitter.getAttribute("collapse")
	};
};
ToolSplitter.deserialize = function(o) {
	return new ToolSplitter(o.collapse);
};

/// ToolSpace class.
/** Allowed types are "hbox" and "vbox". */
function ToolSpace(type) {
	this.box = document.createElementNS(XUL_NS, type);
	this.type = type;
	this.children = [];
};
OIL.ToolSpace = ToolSpace;
ToolSpace.prototype.add = function(tool) {
	this.children.push(tool);
};
ToolSpace.prototype.serialize = function() {
	var This = this;
	return {
		type: this.type,
		children: this.children.map(function(tool) {
			// create object to store information
			var o = {};

			// figure out what tool it is
			var control = null;
			if(tool instanceof ToolSpace) {
				o.type = tool.type;
				control = tool.box;
			}
			else if(tool instanceof ToolTabbox) {
				o.type = "tabbox";
				control = tool.tabbox;
			}
			else if(tool instanceof ToolSplitter) {
				o.type = "splitter";
			}
			else
				throw new Error("Unknown subtool");

			// store tool-specific information
			o.tool = tool.serialize();

			// store size information if needed
			if(control) {
				if(control.flex)
					o.flex = control.flex;
				switch(This.type) {
				case "hbox":
					if(control.width)
						o.width = control.width;
					break;
				case "vbox":
					if(control.height)
						o.height = control.height;
					break;
				}
			}

			return o;
		})
	};
};
ToolSpace.deserialize = function(o) {
	var space = new ToolSpace(o.type);

	for(var i = 0; i < o.children.length; ++i) {
		var child = o.children[i];

		var tool, control;
		switch(child.type) {
		case "hbox":
		case "vbox":
			tool = ToolSpace.deserialize(child.tool);
			control = tool.box;
			break;
		case "tabbox":
			tool = ToolTabbox.deserialize(child.tool);
			control = tool.tabbox;
			break;
		case "splitter":
			tool = ToolSplitter.deserialize(child.tool);
			control = tool.splitter;
			break;
		default:
			throw new Error("Unknown type of tool");
		}

		if(child.flex !== undefined)
			control.flex = child.flex;
		if(child.width !== undefined)
			control.width = child.width;
		if(child.height !== undefined)
			control.height = child.height;

		space.box.appendChild(control);
		space.children.push(tool);
	}

	return space;
};

OIL.wrongToolWindow = function(window) {
	window.location = "tool-wrong.xul";
};

OIL.getParamsFromToolWindow = function(window) {
	if(window.location.hash.length <= 0) {
		OIL.wrongToolWindow(window);
		return null;
	}

	var params = window.location.hash.substr(1).split("&");
	var res = {};
	for(var i = 0; i < params.length; ++i) {
		var param = params[i].split("=");
		if(param.length != 2)
			continue;
		res[decodeURIComponent(param[0])] = decodeURIComponent(param[1]);
	}

	// 'tab' parameter is required
	if(res.tab) {
		window.toolTab = OIL.ToolTab.get(res.tab);
		delete res.tab;
		window.toolTab.params = res;
	}
	else {
		OIL.wrongToolWindow(window);
		return null;
	}

	return res;
};
