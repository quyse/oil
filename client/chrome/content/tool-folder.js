'use strict';

Components.utils.import('chrome://oil/content/oil.js');

const MIME_DRAG_FOLDER_ENTRIES = "application/x-inanityoil-folder-entry";
// maximum length of hierarchy to process
const MAX_HIERARCHY_LENGTH = 64;

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
	this.name = null;
	this.parentId = null;
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
		for(var i = 0; i < this.children.length; ++i)
			this.children[i].destroy();
		this.children = undefined;
	}
};
/// Enable update of UI.
Item.addRowsEnabled = true;
Item.prototype.onChange = function(type, key, value) {
	if(this.destroyed)
		return;
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
		this.onChange('tag', OIL.uuids.tags.parent, this.entity.ReadTag(OIL.uuids.tags.parent));
		break;
	case 'tag':
		switch(key) {
		case OIL.uuids.tags.name:
			this.name = value ? OIL.f2s(value) : null;
			break;
		case OIL.uuids.tags.parent:
			this.parentId = OIL.f2eid(value);
			break;
		}
		var row = this.getRow();
		if(row >= 0)
			this.view.treebox.invalidateRow(row);
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
			var row = this.getRow();
			if(row >= 0)
				this.view.treebox.invalidateRow(row);
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
Item.prototype.getVisibleName = function() {
	return (!this.parent || this.parentId == this.parent.entityId ? "" : "→ ") + (this.name || "<unnamed>");
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
		return item.getVisibleName();
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
View.prototype.canDrop = function(row, orientation, dataTransfer) {
	if(!dataTransfer.types.contains(MIME_DRAG_FOLDER_ENTRIES))
		return false;

	if(orientation != 0)
		return false;

	return true;
};
View.prototype.drop = function(row, orientation, dataTransfer) {
	var entries = JSON.parse(dataTransfer.getData(MIME_DRAG_FOLDER_ENTRIES));
	if(entries.length <= 0)
		return;

	var actionDescription;
	var operation = dataTransfer.dropEffect;
	switch(operation) {
	case "move":
		actionDescription = "move ";
		break;
	case "link":
		actionDescription = "link ";
		break;
	default:
		return;
	}
	if(entries.length == 1) {
		let entryName = OIL.entityManager.GetEntity(entries[0].itemId).ReadTag(OIL.uuids.tags.name);
		actionDescription += entryName ? JSON.stringify(OIL.f2s(entryName)) : "<unnamed>";
	}
	else
		actionDescription += entries.length + " items";

	var destItem = this.getItem(row);
	var destEntity = destItem.entity;
	var destIdFile = OIL.eid2f(destEntity.GetId());

	actionDescription += " to " + JSON.stringify(destItem.getVisibleName());

	var action = OIL.createAction(actionDescription);
	for(var i = 0; i < entries.length; ++i) {
		var sourceFolderId = entries[i].folderId;
		var sourceFolderEntity = OIL.entityManager.GetEntity(sourceFolderId);
		var sourceFileEntity = OIL.entityManager.GetEntity(entries[i].itemId);

		var entryIdFile = OIL.eid2f(entries[i].itemId);

		// if we moving
		if(operation == "move") {
			// if we moving solid link, change the tag
			if(OIL.f2eid(sourceFileEntity.ReadTag(OIL.uuids.tags.parent)) == sourceFolderId)
				sourceFileEntity.WriteTag(action, OIL.uuids.tags.parent, destIdFile);

			// remove entry from source folder
			sourceFolderEntity.WriteData(action, entryIdFile, null);
		}

		// place entry into dest folder
		destEntity.WriteData(action, entryIdFile, OIL.fileTrue());
	}
	OIL.finishAction(action);
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

function onCommandOpen() {
	var selectedItems = getSelectedItems();
	for(var i = 0; i < selectedItems.length; ++i) {
		var item = selectedItems[i];
		// TODO: open with corresponding tool
	}
}

function onCommandOpenFolder() {
	var selectedItems = getSelectedItems();
	for(var i = 0; i < selectedItems.length; ++i) {
		var item = selectedItems[i];
		if(item.entity.GetScheme().GetId() == OIL.uuids.schemes.folder)
			OIL.createTool("folder", item.entityId);
	}
}

function onCommandCreateFolder() {
	var selectedItems = getSelectedItems();
	var selectedItem = selectedItems.length == 1 ? selectedItems[0] : view.rootItem;

	var action = OIL.createAction("create folder");
	var entity = OIL.entityManager.CreateEntity(action, OIL.uuids.schemes.folder);
	entity.WriteTag(action, OIL.uuids.tags.name, OIL.s2f("New Folder"));
	entity.WriteTag(action, OIL.uuids.tags.parent, OIL.eid2f(selectedItem.entityId));
	selectedItem.entity.WriteData(action, OIL.eid2f(entity.GetId()), OIL.fileTrue());
	OIL.finishAction(action);
}

function onCommandDelete() {
	var selectedItems = getSelectedItems();
	if(selectedItems.length <= 0)
		return;

	// construct action description
	var actionDescription;
	if(selectedItems.length == 1)
		actionDescription = "delete " + JSON.stringify(selectedItems[0].getVisibleName());
	else
		actionDescription = "delete " + selectedItems.length + " items";
	// if all items from the same folder, add it to description
	var parentItem = selectedItems[0].parent;
	for(var i = 1; i < selectedItems.length; ++i)
		if(selectedItems[i].parent != parentItem)
			break;
	if(i >= selectedItems.length)
		actionDescription += " from folder " + JSON.stringify(parentItem.getVisibleName());

	var action = OIL.createAction(actionDescription);

	for(var i = 0; i < selectedItems.length; ++i) {
		var item = selectedItems[i];
		var itemParent = item.parent;
		if(item.parentId == itemParent.entityId)
			item.entity.WriteTag(action, OIL.uuids.tags.parent, null);
		itemParent.entity.WriteData(action, OIL.eid2f(item.entityId), null);
	}

	OIL.finishAction(action);
}

function onCommandProperties() {
	var selectedItems = getSelectedItems();

	for(var i = 0; i < selectedItems.length; ++i)
		OIL.createTool("entity", selectedItems[i].entityId);
}

function onCommandPlace() {
	var selectedItems = getSelectedItems();

	var action = OIL.createAction("place");

	for(var i = 0; i < selectedItems.length; ++i) {
		var item = selectedItems[i];

		// check that this is link
		if(item.parentId == item.parent.entityId)
			continue;

		// do place
		item.entity.WriteTag(action, OIL.uuids.tags.parent, OIL.eid2f(item.parent.entityId));
	}

	OIL.finishAction(action);
}

function onCommandShowRealPlace() {
	var selectedItems = getSelectedItems();
	if(selectedItems.length != 1)
		return;

	var pathIds = [];
	var targetId = selectedItems[0].entityId;
	var id = targetId;
	var i = 0;
	for(;;) {
		if(i++ >= MAX_HIERARCHY_LENGTH) {
			alert("Can't find real place of selected entity.");
			return;
		}
		if(id == rootItem.entityId)
			break;
		pathIds.push(id);

		var entity = OIL.entityManager.GetEntity(id);
		var parentId = OIL.f2eid(entity.ReadTag(OIL.uuids.tags.parent));
		if(!parentId) {
			alert("Selected entity doesn't have real place.");
			return;
		}

		id = parentId;
	}

	var item = rootItem;
	for(i = pathIds.length - 1; i >= 0; --i) {
		if(!item.isContainer())
			throw new Error("Not a folder in path");
		item.open(true);
		var itemChildren = item.children;
		for(var j = 0; j < itemChildren.length; ++j)
			if(itemChildren[j].entityId == pathIds[i]) {
				item = itemChildren[j];
				break;
			}
		if(j >= itemChildren.length)
			throw new Error("Can't follow path");
	}

	var row = item.getRow();
	if(row < 0)
		throw new Error("Can't show target item");

	item.view.treebox.ensureRowIsVisible(row);
	item.view.selection.select(row);
}

function onContextMenuShowing() {
	var selectedItems = getSelectedItems();

	var hasFolder = false;
	var hasLink = false;
	for(var i = 0; i < selectedItems.length; ++i) {
		var item = selectedItems[i];
		if(item.entity.GetScheme().GetId() == OIL.uuids.schemes.folder)
			hasFolder = true;
		if(item.parentId != item.parent.entityId)
			hasLink = true;
	}

	document.getElementById("contextMenuOpen").hidden = selectedItems.length <= 0;
	document.getElementById("contextMenuOpenFolder").hidden = !hasFolder;
	document.getElementById("contextMenuDelete").hidden = selectedItems.length <= 0;
	document.getElementById("contextMenuCreate").hidden = !hasFolder && selectedItems.length > 0;
	document.getElementById("contextMenuPlace").hidden = !hasLink;
	document.getElementById("contextMenuShowRealPlace").hidden = !hasLink || selectedItems.length != 1;
	document.getElementById("contextMenuProperties").hidden = selectedItems.length <= 0;
}

function onTreeDragStart(event) {
	var selectedItems = getSelectedItems();
	if(selectedItems.length <= 0)
		return;
	event.dataTransfer.setData(MIME_DRAG_FOLDER_ENTRIES, JSON.stringify(selectedItems.map(function(item) {
		return {
			itemId: item.entityId,
			folderId: item.parent.entityId
		};
	})));
	event.dataTransfer.effectAllowed = "linkMove";
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
