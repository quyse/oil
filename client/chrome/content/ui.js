const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
OIL.XUL_NS = XUL_NS;

const MIME_DRAG_TOOL_TAB = "application/x-inanityoil-ui-drag-tab";

function getToolTab(tabNumber) {
	return document.getElementById("tab" + tabNumber);
}

function getToolTabpanel(tabNumber) {
	return document.getElementById("tabpanel" + tabNumber);
}

var toolTabsCount = 0;
function createToolTab(tabbox, title) {
	// get tab number
	var tabNumber = ++toolTabsCount;

	// create tab
	var tab = document.createElementNS(XUL_NS, "tab");
	tab.id = "tab" + tabNumber;
	tab.setAttribute("label", title);
	tabbox.tabs.appendChild(tab);

	// create tabpanel
	var tabpanel = document.createElementNS(XUL_NS, "tabpanel");
	tabpanel.id = "tabpanel" + tabNumber;
	tabbox.tabpanels.appendChild(tabpanel);

	// setup drag events
	tab.addEventListener("dragstart", function(event) {
		event.dataTransfer.setData(MIME_DRAG_TOOL_TAB, tabNumber);
		event.dataTransfer.effectAllowed = "move";
	});
	tab.addEventListener("dragover", function(event) {
		// check that event of the right type
		if(!event.dataTransfer.types.contains(MIME_DRAG_TOOL_TAB))
			return;

		// prevent default cancelling
		event.preventDefault();

		// get source tab number
		var sourceTabNumber = event.dataTransfer.getData(MIME_DRAG_TOOL_TAB);

		moveToolTab(this.parentNode.parentNode, sourceTabNumber, tabNumber);
	});
	tab.addEventListener("dblclick", function(event) {
		var tab = getToolTab(tabNumber);
		var tabbox = tab.parentNode.parentNode;
		tab.remove();
		getToolTabpanel(tabNumber).remove();
		tabbox.selectedIndex = 0;
	});

	// select new tab
	tabbox.selectedIndex = tabbox.tabs.itemCount - 1;

	return tabNumber;
}

function createToolTabbox() {
	// create
	var tabbox = document.createElementNS(XUL_NS, "tabbox");
	tabbox.appendChild(document.createElementNS(XUL_NS, "tabs"));
	tabbox.appendChild(document.createElementNS(XUL_NS, "tabpanels"));

	var tabpanels = tabbox.getElementsByTagNameNS(XUL_NS, "tabpanels")[0];
	tabpanels.flex = 1;
	tabpanels.setAttribute("style", "padding:0 !important");

	// setup drag events
	tabbox.addEventListener("dragover", function(event) {
		// check that event of the right type
		if(!event.dataTransfer.types.contains(MIME_DRAG_TOOL_TAB))
			return;

		// prevent default cancelling
		event.preventDefault();

		// get source tab number
		var sourceTabNumber = event.dataTransfer.getData(MIME_DRAG_TOOL_TAB);

		moveToolTab(this, sourceTabNumber, -1);
	});

	return tabbox;
}

function moveToolTab(destTabbox, sourceTabNumber, destTabNumber) {
	if(sourceTabNumber == destTabNumber)
		return false;

	// get source elements
	var sourceTab = getToolTab(sourceTabNumber);
	var sourceTabpanel = getToolTabpanel(sourceTabNumber);

	var sourceTabbox = sourceTab.parentNode.parentNode;

	// perform move
	if(destTabNumber >= 0) {
		var destTab = getToolTab(destTabNumber);
		var destTabpanel = getToolTabpanel(destTabNumber);

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
		}
		else
		{
			// just insert before dest tab
			destTabbox.tabs.insertBefore(sourceTab, destTab);
			destTabbox.tabpanels.insertBefore(sourceTabpanel, destTabpanel);
		}
	}
	else if(sourceTabbox != destTabbox) {
		destTabbox.tabs.appendChild(sourceTab);
		destTabbox.tabpanels.appendChild(sourceTabpanel);
	}
	else
		return;

	// select source tab on new place
	var destTabs = destTabbox.tabs.childNodes;
	for(var i = 0; i < destTabs.length; ++i)
		if(destTabs[i] == sourceTab) {
			destTabbox.selectedIndex = i;
			break;
		}

	// reset selection on source tabbox
	if(sourceTabbox != destTabbox)
		sourceTabbox.selectedIndex = 0;
}
