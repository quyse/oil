'use strict';

Components.utils.import('chrome://oil/content/oil.js');

function getTree() {
	return document.getElementById("tree");
}

/// Class of item in the tree.
function Item(entity) {
	this.destroyed = false;
	this.entity = entity;
	this.entityId = entity.GetId();
	this.entityCallback = entity.AddCallback(OIL.weakCallback(this, 'onChange'));

	this.parent = null;
	this.view = null;
	this.opened = false;
};
Item.prototype.init = function(view) {
	this.view = view;
	this.onChange('scheme', null, this.entity.GetScheme());
};
Item.prototype.destroy = function() {
	this.destroyed = true;

	this.entity = null;
	this.entityId = null;
	this.entityCallback = null;
	this.parent = null;
	this.view = null;

	if(Array.isArray(this.children)) {
		for(var i = 0; i < this.children; ++i)
			this.children[i].destroy();
		this.children = undefined;
	}
};
/// Enable update of UI.
Item.addRowsEnabled = true;
Item.prototype.onChange = function(type, key, value) {
	switch(type) {
	case 'scheme':
		// reinitialize
		this.open(false);
		this.scheme = value;
		// this.children could be:
		//   an array (initialized list of folder's children)
		//   null (non-initialized list of folder's children)
		//   undefined (this is not folder)
		this.children = (this.scheme && this.scheme.GetId() == OIL.uuids.schemes.folder) ? null : undefined;
		this.onChange('tag', OIL.uuids.tags.name, this.entity.ReadTag(OIL.uuids.tags.name));
		break;
	case 'tag':
		if(key == OIL.uuids.tags.name) {
			this.name = value ? OIL.f2s(value) : "<unnamed>";
			var row = this.getRow();
			if(row >= 0)
				this.view.treebox.invalidateRow(row);
		}
		break;
	case 'data':
		// we only interested in data if children is an array
		if(Array.isArray(this.children)) {
			var childEntityId = OIL.f2eid(key);
			// if new child added
			if(value) {
				// check that there is no identical child
				for(var i = 0; i < this.children.length; ++i)
					if(this.children[i].entityId == childEntityId)
						break;
				if(i < this.children.length)
					break;
				// add new child
				var newItem = new Item(OIL.entityManager.GetEntity(childEntityId));
				this.addChild(newItem);
				newItem.init(this.view);
				// update ui
				var newItemRow = newItem.getRow();
				if(newItemRow >= 0)
					this.view.treebox.rowCountChanged(newItemRow, 1);
			}
			// else child removed
			else {
				// find and remove
				for(var i = 0; i < this.children.length; ++i) {
					var item = this.children[i];
					if(item.entityId == childEntityId) {
						// remove item from tree
						var row = item.getRow();
						if(row >= 0)
							this.view.treebox.rowCountChanged(row, -(1 + item.getFullCount()));
						// remote item from parent (this)
						this.removeChild(item);
						// destroy item
						item.destroy();
						// if parent count became 0, close it
						if(this.children.length == 0)
							this.open(false);
						break;
					}
				}
			}
			// invalidate parent row in any case
			this.view.treebox.invalidateRow(this.getRow());
		}
		break;
	}
};
Item.prototype.getIndexInParent = function() {
	return this.parent.children.indexOf(this);
};
Item.prototype.getRow = function() {
	var row = -1;
	for(var item = this; item.parent; item = item.parent) {
		if(!item.parent.isOpen())
			return -1;
		row += 1 + item.getIndexInParent();
	}
	return row;
};
Item.prototype.isContainer = function() {
	return this.children !== undefined;
};
/// Ensure list of children is initialized if this is folder.
Item.prototype.ensureChildren = function() {
	if(this.children === null) {
		this.children = [];
		this.entityCallback.EnumerateData();
	}
};
/// Add new child to container.
/** Could be called only if isContainer() == true. */
Item.prototype.addChild = function(child) {
	child.parent = this;
	this.children.push(child);
};
/// Remove child from container.
/** Could be called only if isContainer() == true. */
Item.prototype.removeChild = function(child) {
	child.parent = null;
	var index = this.children.indexOf(child);
	if(index >= 0) {
		this.children.splice(index, 1);
	}
};
/// Signal that item can uninitialize children's list.
Item.prototype.dontCareChildren = function() {
	if(Array.isArray(this.children)) {
		for(var i = 0; i < this.children.length; ++i)
			this.children[i].destroy();
		this.children = null;
	}
	this.opened = false;
};
/// Is item open.
Item.prototype.isOpen = function() {
	return this.opened;
};
Item.prototype.open = function(open) {
	if(this.opened == open)
		return;
	if(open) {
		this.opened = true;
		if(Array.isArray(this.children)) {
			// all children items are closed, so no need to call getFullCount
			var count = this.children.length;
			// add items to tree
			if(count > 0)
				this.view.treebox.rowCountChanged(this.getRow() + 1, count);
		}
		else {
			// just get items
			this.ensureChildren();
		}
	}
	else {
		// remove items from tree
		var fullCount = this.getFullCount();
		if(fullCount > 0)
			this.view.treebox.rowCountChanged(this.getRow() + 1, -fullCount);

		this.opened = false;

		// inform that children can uninitialize their children
		if(Array.isArray(this.children))
			for(var i = 0; i < this.children.length; ++i)
				this.children[i].dontCareChildren();
	}
};
/// Gets number of all visible children.
Item.prototype.getFullCount = function() {
	if(!this.opened)
		return 0;
	var fullCount = 0;
	if(Array.isArray(this.children))
		for(var i = 0; i < this.children.length; ++i)
			fullCount += 1 + this.children[i].getFullCount();
	return fullCount;
};
Item.prototype.getName = function() {
	return this.name;
};
Item.prototype.getScheme = function() {
	return this.scheme;
};

/// View class for tree. Implements nsITreeView.
function View(rootItem) {
	/// Root item. Contains level 0 of items.
	/// Doesn't draw itself.
	this.rootItem = rootItem;

	this.selection = null;
};
Object.defineProperty(View.prototype, "rowCount", {
	enumerable: true,
	configurable: false,
	get: function() {
		return this.rootItem.getFullCount();
	}
});
View.prototype.getItem = function(row) {
	var container = this.rootItem;
	// loop for containers
	for(;;) {
		// loop for items
		container.ensureChildren();
		var itemsCount = container.children.length;
		for(var itemIndex = 0; itemIndex < itemsCount; ++itemIndex) {
			var item = container.children[itemIndex];
			if(row == 0)
				return item;
			--row;
			var itemFullCount = item.getFullCount();
			if(row < itemFullCount) {
				container = item;
				break;
			}
			row -= itemFullCount;
		}
		if(itemIndex >= itemsCount)
			throw Error("Wrong tree view index");
	}
};
//*** nsITreeView methods.
View.prototype.getCellText = function(row, column) {
	var item = this.getItem(row);
	switch(column.id) {
	case "treecolName":
		return item.getName();
	case "treecolType":
		return item.getScheme().GetName();
	default:
		return "???";
	}
};
View.prototype.setTree = function(treebox) {
	this.treebox = treebox;
};
View.prototype.isSeparator = function(row) {
	// we don't use separators
	return false;
};
View.prototype.isSorted = function() {
	return false;
};
View.prototype.getImageSrc = function(row, col) {
	if(col.id != "treecolName")
		return null;

	var item = this.getItem(row);
	var schemeName = OIL.uuids.schemes[item.getScheme().GetId()];
	return OIL.uuids.schemeDescs[schemeName].icon;
};
View.prototype.getRowProperties = function(row, props) {
};
View.prototype.getCellProperties = function(row, col, props) {
};
View.prototype.getColumnProperties = function(colid, col, props) {
};
View.prototype.getLevel = function(row) {
	var item = this.getItem(row);
	for(var level = -2; item; ++level)
		item = item.parent;
	return level;
};
View.prototype.hasNextSibling = function(row, afterIndex) {
	var item = this.getItem(row);
	var parent = item.parent;
	parent.ensureChildren();
	var itemFullCount = item.getFullCount();
	var itemParentIndex = item.getIndexInParent();
	return
		// there is next sibling
		itemParentIndex + 1 < parent.children.length &&
		// and its position after
		row + 1 + itemFullCount > afterIndex;
};
View.prototype.getParentIndex = function(row) {
	var parent = this.getItem(row).parent;
	return parent ? parent.getRow() : -1;
};
View.prototype.isContainer = function(row) {
	return this.getItem(row).isContainer();
};
View.prototype.isContainerEmpty = function(row) {
	var item = this.getItem(row);
	item.ensureChildren();
	return item.children.length == 0;
};
View.prototype.isContainerOpen = function(row) {
	return this.getItem(row).opened;
};
View.prototype.toggleOpenState = function(row) {
	var item = this.getItem(row);
	item.open(!item.opened);
	this.treebox.invalidateRow(row);
};

var view;

function getSelectedItems() {
	var selection = view.selection;
	var rangeCount = selection.getRangeCount();
	var start = {}, end = {};
	var selectedItems = [];
	for(var i = 0; i < rangeCount; ++i) {
		selection.getRangeAt(i, start, end);
		for(var j = start.value; j <= end.value; ++j)
			selectedItems.push(view.getItem(j));
	}

	return selectedItems;
}

function onCreateFolder() {
	var selection = view.selection;
	var selectedItem;
	if(selection.count == 1) {
		var start = {}, end = {};
		selection.getRangeAt(0, start, end);
		selectedItem = view.getItem(start.value);
	}
	else
		selectedItem = view.rootItem;

	var action = OIL.createAction("create folder");
	var entity = OIL.entityManager.CreateEntity(action, OIL.uuids.schemes.folder);
	entity.WriteTag(action, OIL.uuids.tags.name, OIL.s2f("New Folder"));
	selectedItem.entity.WriteData(action, OIL.eid2f(entity.GetId()), OIL.fileTrue());
	OIL.finishAction(action);
}

function onDelete() {
	var selectedItems = getSelectedItems();
	if(selectedItems.length <= 0)
		return;

	// construct action description
	var actionDescription;
	if(selectedItems.length == 1)
		actionDescription = "delete " + JSON.stringify(selectedItems[0].getName());
	else
		actionDescription = "delete " + selectedItems.length + " items";
	// if all items from the same folder, add it to description
	var parentItem = selectedItems[0].parent;
	for(var i = 1; i < selectedItems.length; ++i)
		if(selectedItems[i].parent != parentItem)
			break;
	if(i >= selectedItems.length)
		actionDescription += " from folder " + JSON.stringify(parentItem.getName());

	var action = OIL.createAction(actionDescription);

	for(var i = 0; i < selectedItems.length; ++i) {
		var item = selectedItems[i];
		var itemParent = item.parent;
		if(itemParent)
			itemParent.entity.WriteData(action, OIL.eid2f(item.entityId), null);
	}

	OIL.finishAction(action);
}

function onProperties() {
	var selection = view.selection;
	if(selection.count != 1)
		return;
	var start = {}, end = {};
	selection.getRangeAt(0, start, end);
	var selectedItem = view.getItem(start.value);

	OIL.createTool("entity", selectedItem.entityId);
}

function onContextMenuShowing() {
	var selection = view.selection;
	document.getElementById("contextMenuDelete").hidden = selection.count == 0;
	document.getElementById("contextMenuProperties").hidden = selection.count != 1;
}

var rootItem;

window.addEventListener('load', function() {
	var rootEntity = OIL.getEntityFromToolWindow(window);
	if(!rootEntity)
		return;

	rootItem = new Item(rootEntity);

	document.getElementById("labelEmpty").remove();

	view = new View(rootItem);
	getTree().view = view;
	rootItem.init(view);
	rootItem.open(true);
});

window.addEventListener('unload', function() {
	if(rootItem)
		rootItem.destroy();
});
