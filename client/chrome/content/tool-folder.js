'use strict';

Components.utils.import('chrome://oil/content/oil.js');

const MIME_DRAG_ENTITY = "application/x-inanityoil-entity";
const MIME_DRAG_FOLDER_ENTRY = "application/x-inanityoil-folder-entry";
// maximum length of hierarchy to process
const MAX_HIERARCHY_LENGTH = 64;

/// mode of tool
/** could be "normal" or "select.
Select mode means no context menu, no editing actions (renaming, drag-n-drop). */
var toolMode = null;
/// read-only mode of tool
var toolReadOnly = false;
/// single-selection mode
var toolSingleSelection = false;
/// filter callback
var toolFilterCallback = null;

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
		this.children = (this.scheme && this.scheme.GetId() == OIL.ids.schemes.folder) ? null : undefined;
		this.onChange('tag', OIL.ids.tags.name, this.entity.ReadTag(OIL.ids.tags.name));
		this.onChange('tag', OIL.ids.tags.parent, this.entity.ReadTag(OIL.ids.tags.parent));
		break;
	case 'tag':
		switch(key) {
		case OIL.ids.tags.name:
			this.name = value ? OIL.f2s(value) : null;
			// if this is root, update title
			if(!this.parent)
				window.toolTab.setTitle("folder: " + this.getVisibleName());
			break;
		case OIL.ids.tags.parent:
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
				// check with filter callback
				if(toolFilterCallback && !toolFilterCallback(childEntityId))
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
		// make account of all children in parent before this item
		var indexInParent = item.getIndexInParent();
		for(var i = 0; i < indexInParent; ++i)
			row += 1 + item.parent.children[i].getFullCount();
		// plus the parent itself
		++row;
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
Item.prototype.getStringifiedName = function() {
	return this.name ? JSON.stringify(this.name) : "<unnamed>";
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
	return OIL.ids.schemeDescs[item.getScheme().GetId()].icon;
};
View.prototype.getRowProperties = function(row, props) {
	return '';
};
View.prototype.getCellProperties = function(row, col, props) {
	return '';
};
View.prototype.getColumnProperties = function(colid, col, props) {
	return '';
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
	// return if there is next sibling and its position after
	return itemParentIndex + 1 < parent.children.length && row + 1 + itemFullCount > afterIndex;
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
View.prototype.isEditable = function(row, col) {
	return col.id == "treecolName";
};
View.prototype.setCellText = function(row, col, value) {
	if(col.id != "treecolName")
		return;

	var item = this.getItem(row);

	var action = OIL.createAction("rename " + item.getStringifiedName() + " to " + JSON.stringify(value));
	item.entity.WriteTag(action, OIL.ids.tags.name, OIL.s2f(value));
	OIL.finishAction(action);
};
View.prototype.canDrop = function(row, orientation, dataTransfer) {
	return checkDrop(row, orientation, dataTransfer);
};
View.prototype.drop = function(row, orientation, dataTransfer) {
	var checkOutput = {};
	if(!checkDrop(row, orientation, dataTransfer, checkOutput)) {
		if(checkOutput.message)
			OIL.getPromptService().prompt(window, "drop error", checkOutput.message);
		return;
	}

	var entries = checkOutput.entries;
	var entities = checkOutput.entities;
	var files = checkOutput.files;

	var destItem = this.getItem(row);
	var destEntity = destItem.entity;

	// if there are some entries
	if(entries.length) {
		var actionDescription;
		var operation = dataTransfer.dropEffect;
		switch(operation) {
		case "move":
			actionDescription = "move ";
			break;
		case "link":
			actionDescription = "link ";
			break;
		}

		if(entries.length == 1) {
			let entryName = OIL.entityManager.GetEntity(entries[0].itemId).ReadTag(OIL.ids.tags.name);
			actionDescription += entryName ? JSON.stringify(OIL.f2s(entryName)) : "<unnamed>";
		}
		else
			actionDescription += entries.length + " items";

		var destIdFile = OIL.eid2f(destEntity.GetId());

		actionDescription += " to " + destItem.getStringifiedName();

		var action = OIL.createAction(actionDescription);

		// perform operation on entries
		for(var i = 0; i < entries.length; ++i) {
			var sourceFolderId = entries[i].folderId;
			var sourceFolderEntity = OIL.entityManager.GetEntity(sourceFolderId);
			var sourceFileEntity = OIL.entityManager.GetEntity(entries[i].itemId);

			var entryIdFile = OIL.eid2f(entries[i].itemId);

			// if we moving
			if(operation == "move") {
				// if we moving solid link, change the tag
				if(OIL.f2eid(sourceFileEntity.ReadTag(OIL.ids.tags.parent)) == sourceFolderId)
					sourceFileEntity.WriteTag(action, OIL.ids.tags.parent, destIdFile);

				// remove entry from source folder
				sourceFolderEntity.WriteData(action, entryIdFile, null);
			}

			// place entry into dest folder
			destEntity.WriteData(action, entryIdFile, OIL.fileTrue());
		}
		OIL.finishAction(action);
	}

	// if there are some entities
	if(entities.length > 0) {
		var actionDescription = "link ";
		var operation = dataTransfer.dropEffect;

		if(entities.length == 1) {
			let entityName = OIL.entityManager.GetEntity(entities[0]).ReadTag(OIL.ids.tags.name);
			actionDescription += entityName ? JSON.stringify(OIL.f2s(entityName)) : "<unnamed>";
		}
		else
			actionDescription += entities.length + " items";

		var destIdFile = OIL.eid2f(destEntity.GetId());

		actionDescription += " to " + destItem.getStringifiedName();

		var action = OIL.createAction(actionDescription);

		// perform operation on entities
		for(var i = 0; i < entities.length; ++i) {
			// place entity into dest folder
			destEntity.WriteData(action, OIL.eid2f(entities[i]), OIL.fileTrue());
		}
		OIL.finishAction(action);
	}

	// perform operation on files
	if(files.length > 0)
		uploadFiles(files, destEntity);
};

function checkDrop(row, orientation, dataTransfer, output) {
	if(orientation != 0)
		return false;

	if(toolReadOnly)
		return false;

	// get items to drop
	var entries = [], entities = [], files = [];
	var itemsCount = dataTransfer.mozItemCount;
	for(var i = 0; i < itemsCount; ++i) {
		// if it's folder entry
		var entry = dataTransfer.mozGetDataAt(MIME_DRAG_FOLDER_ENTRY, i);
		if(entry) {
			entries.push(JSON.parse(entry));
			continue;
		}

		// else if it's entity
		var entityId = dataTransfer.mozGetDataAt(MIME_DRAG_ENTITY, i);
		if(entityId) {
			entities.push(entityId);
			continue;
		}

		// else if it's file
		var file = dataTransfer.mozGetDataAt("application/x-moz-file", i);
		if(file) {
			file = file.QueryInterface(Components.interfaces.nsIFile);
			if(file)
				files.push(file);
			else
				return false;
			continue;
		}
	}

	// check operation
	var operation = dataTransfer.dropEffect;
	switch(operation) {
	case "move":
		break;
	case "link":
		// cannot link files
		if(files.length > 0)
			return false;
		break;
	default:
		return false;
	}

	// get target item
	var targetItem = view.getItem(row);

	// if operation is move, check for cycles for entries
	if(operation == "move") {
		for(var i = 0; i < entries.length; ++i)
			if(isItemInto(targetItem.entityId, entries[i].itemId)) {
				if(output)
					output.message = "operation would create a cycle of real paths which is forbidden";
				return false;
			}
	}

	if(output) {
		output.entries = entries;
		output.entities = entities;
		output.files = files;
	}

	return true;
}

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

/// Checks if item is into another item (recursively) by real places.
/** Could return true, false or undefined. */
function isItemInto(targetId, containerId) {
	var i = 0;
	var id = targetId;
	for(;;) {
		if(i++ >= MAX_HIERARCHY_LENGTH)
			return undefined;

		if(id == containerId)
			return true;

		var entity = OIL.entityManager.GetEntity(id);
		var parentId = OIL.f2eid(entity.ReadTag(OIL.ids.tags.parent));
		if(!parentId)
			return false;

		id = parentId;
	}
}

function onCommandOpen() {
	if(toolMode != "normal")
		return;

	var selectedItems = getSelectedItems();
	for(var i = 0; i < selectedItems.length; ++i)
		window.toolTab.addDependentToolTab(OIL.createTool("default", {
			entity: selectedItems[i].entityId
		}));
}

function onCommandRename() {
	if(toolReadOnly)
		return;

	var selectedItems = getSelectedItems();
	if(selectedItems.length != 1)
		return;
	var tree = getTree();
	tree.startEditing(selectedItems[0].getRow(), tree.columns.getNamedColumn("treecolName"));
}

function startRenameNewItem(parentItem, newEntityId) {
	parentItem.open(true);
	for(var i = 0; i < parentItem.children.length; ++i) {
		var item = parentItem.children[i];
		if(item.entityId == newEntityId) {
			var row = item.getRow();
			view.selection.select(row);
			view.treebox.ensureRowIsVisible(row);
			onCommandRename();
			break;
		}
	}
}

function onCommandCreateFolder() {
	if(toolReadOnly)
		return;

	var selectedItems = getSelectedItems();
	var selectedItem = selectedItems.length == 1 ? selectedItems[0] : view.rootItem;

	var action = OIL.createAction("create folder");
	var entity = OIL.entityManager.CreateEntity(action, OIL.ids.schemes.folder);
	entity.WriteTag(action, OIL.ids.tags.name, OIL.s2f("new folder"));
	entity.WriteTag(action, OIL.ids.tags.parent, OIL.eid2f(selectedItem.entityId));
	selectedItem.entity.WriteData(action, OIL.eid2f(entity.GetId()), OIL.fileTrue());
	OIL.finishAction(action);

	startRenameNewItem(selectedItem, entity.GetId());
}

function onCommandCreateImage() {
	if(toolReadOnly)
		return;

	var selectedItems = getSelectedItems();
	var selectedItem = selectedItems.length == 1 ? selectedItems[0] : view.rootItem;

	var action = OIL.createAction("create image");
	var entity = OIL.entityManager.CreateEntity(action, OIL.ids.schemes.image);
	entity.WriteTag(action, OIL.ids.tags.name, OIL.s2f("new image"));
	entity.WriteTag(action, OIL.ids.tags.parent, OIL.eid2f(selectedItem.entityId));
	selectedItem.entity.WriteData(action, OIL.eid2f(entity.GetId()), OIL.fileTrue());
	OIL.finishAction(action);

	startRenameNewItem(selectedItem, entity.GetId());
}

function onCommandCreateImageTransform() {
	if(toolReadOnly)
		return;

	var selectedItems = getSelectedItems();
	var selectedItem = selectedItems.length == 1 ? selectedItems[0] : view.rootItem;

	var action = OIL.createAction("create image transform");
	var entity = OIL.entityManager.CreateEntity(action, OIL.ids.schemes.imageTransform);
	entity.WriteTag(action, OIL.ids.tags.name, OIL.s2f("new image transform"));
	entity.WriteTag(action, OIL.ids.tags.parent, OIL.eid2f(selectedItem.entityId));
	selectedItem.entity.WriteData(action, OIL.eid2f(entity.GetId()), OIL.fileTrue());
	OIL.finishAction(action);

	startRenameNewItem(selectedItem, entity.GetId());
}

function onCommandUploadFile() {
	if(toolReadOnly)
		return;

	var selectedItems = getSelectedItems();
	var selectedItem = selectedItems.length == 1 ? selectedItems[0] : view.rootItem;

	var folderId = selectedItem.entityId;

	var nsIFilePicker = Components.interfaces.nsIFilePicker;
	var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
	fp.init(window, "select files to upload", nsIFilePicker.modeOpenMultiple);
	fp.open({
		done: function(result) {
			try {
				if(result != nsIFilePicker.returnOK)
					return;

				// get list of files with total size
				var files = [];
				var pickedFiles = fp.files;
				while(pickedFiles.hasMoreElements())
					files.push(pickedFiles.getNext().QueryInterface(Components.interfaces.nsIFile));

				uploadFiles(files, OIL.entityManager.GetEntity(folderId));
			}
			catch(e) {
				alert(e);
			}
		}
	});
}

/// upload files into folder
/** \param files [nsIFile] */
function uploadFiles(sourceFiles, folderEntity) {
	var entries = [];
	var totalSize = 0;
	function addFiles(sourceFiles, entries) {
		for(var i = 0; i < sourceFiles.length; ++i) {
			var file = sourceFiles[i];
			// if this is normal file
			if(file.isFile()) {
				var fileSize = file.fileSize;
				entries.push({
					type: "file",
					path: file.path,
					name: file.leafName,
					size: fileSize
				});
				totalSize += fileSize;
			}
			// else if it's a directory
			else if(file.isDirectory()) {
				var subFiles = [];
				var directoryEntries = file.directoryEntries;
				while(directoryEntries.hasMoreElements())
					subFiles.push(directoryEntries.getNext().QueryInterface(Components.interfaces.nsIFile));
				var subPaths = [];
				addFiles(subFiles, subPaths);
				entries.push({
					type: "directory",
					name: file.leafName,
					entries: subPaths
				});
			}
		}
	}

	// add source files
	addFiles(sourceFiles, entries);

	if(entries.length <= 0)
		return;

	// create action
	var actionDescription;
	if(entries.length == 1)
		actionDescription = "upload file " + JSON.stringify(entries[0].name);
	else
		actionDescription = "upload " + entries.length + " files";
	var action = OIL.createAction(actionDescription);

	// create directories, and get flatten list of files
	var files = [];
	var processEntries = function(parentEntity, entries) {
		for(var i = 0; i < entries.length; ++i) {
			var entry = entries[i];
			switch(entry.type) {
			case "file":
				// remember parent entity
				entry.parentEntity = parentEntity;
				// add entry to list of files
				files.push(entry);
				break;
			case "directory":
				// create directory
				var entity = OIL.entityManager.CreateEntity(action, OIL.ids.schemes.folder);
				entity.WriteTag(action, OIL.ids.tags.name, OIL.s2f(entry.name));
				parentEntity.WriteData(action, OIL.eid2f(entity.GetId()), OIL.fileTrue());
				entity.WriteTag(action, OIL.ids.tags.parent, OIL.eid2f(parentEntity.GetId()));
				// process sub-entries
				processEntries(entity, entry.entries);
				break;
			}
		}
	};
	processEntries(folderEntity, entries);

	// upload files
	const blockSize = 1024 * 1024;
	var fileIndex = 0, fileEntity = null, sourceStream = null, entityStream = null, fileSize, fileWrittenSize, totalWrittenSize = 0;
	// do one step of uploading
	var step = function() {
		try {
			// if we're starting new file
			if(!fileEntity) {
				// if there is no more files
				if(fileIndex >= files.length) {
					// finish
					OIL.finishAction(action);
					cancelCallback();
					return;
				}

				var file = files[fileIndex++];

				// open file stream
				sourceStream = OIL.core.GetNativeFileSystem().LoadStream(file.path);
				// remember file size
				fileSize = file.size;
				// create file entity, perform starting changes
				fileEntity = OIL.entityManager.CreateEntity(action, OIL.ids.schemes.file);
				fileEntity.WriteTag(action, OIL.ids.tags.name, OIL.s2f(file.name));
				fileEntity.WriteField(action, OIL.ids.schemeDescs.file.fields.originalFileName, file.name);
				file.parentEntity.WriteData(action, OIL.eid2f(fileEntity.GetId()), OIL.fileTrue());
				fileEntity.WriteTag(action, OIL.ids.tags.parent, OIL.eid2f(file.parentEntity.GetId()));

				// create entity stream (without action)
				entityStream = new OIL.classes.Inanity.Oil.FileEntitySchemeOutputStream(null, fileEntity, blockSize);

				// reset written file size
				fileWrittenSize = 0;
			}

			// write a block
			var block = sourceStream.Read(blockSize);
			// if there is some data
			var sizeOfBlock = block.GetSize();
			if(sizeOfBlock) {
				// write it
				entityStream.Write(block);
				// make some progress
				fileWrittenSize += sizeOfBlock;
				totalWrittenSize += sizeOfBlock;
				// correct total size if needed
				if(fileWrittenSize > fileSize) {
					totalSize = totalSize - fileSize + fileWrittenSize;
					fileSize = fileWrittenSize;
				}
				// report progress
				progressCallback(totalWrittenSize, totalSize);
			} else {
				// else there is no more data
				// finish with this file
				entityStream.End();

				// reset pointers
				fileEntity = null;
				sourceStream.__reclaim();
				sourceStream = null;
				entityStream.__reclaim();
			}
			block.__reclaim();

			// next step
			OIL.setTimeout(step, 0);
		} catch(e) {
			cancelCallback();
			alert(e);
		}
	};

	// show progress window
	var progressCallback, cancelCallback;
	window.openDialog('progress.xul', '', 'chrome,modal,centerscreen,close=no', {
		setProgressCallback: function(callback) {
			progressCallback = callback
		},
		setCancelCallback: function(callback) {
			cancelCallback = callback
		},
		onStart: function() {
			OIL.setTimeout(step, 0);
		},
		title: "uploading...",
		description: "preparing file(s) to upload, please wait...",
		cancelButtonText: null
	});
}

function onCommandDelete() {
	if(toolReadOnly)
		return;

	var selectedItems = getSelectedItems();
	if(selectedItems.length <= 0)
		return;

	// construct action description
	var actionDescription;
	if(selectedItems.length == 1)
		actionDescription = "delete " + selectedItems[0].getStringifiedName();
	else
		actionDescription = "delete " + selectedItems.length + " items";
	// if all items from the same folder, add it to description
	var parentItem = selectedItems[0].parent;
	for(var i = 1; i < selectedItems.length; ++i)
		if(selectedItems[i].parent != parentItem)
			break;
	if(i >= selectedItems.length)
		actionDescription += " from folder " + parentItem.getStringifiedName();

	var action = OIL.createAction(actionDescription);

	for(var i = 0; i < selectedItems.length; ++i) {
		var item = selectedItems[i];
		var itemParent = item.parent;
		if(item.parentId == itemParent.entityId)
			item.entity.WriteTag(action, OIL.ids.tags.parent, null);
		itemParent.entity.WriteData(action, OIL.eid2f(item.entityId), null);
	}

	OIL.finishAction(action);
}

function onCommandProperties() {
	if(toolMode != "normal")
		return;

	var selectedItems = getSelectedItems();

	for(var i = 0; i < selectedItems.length; ++i)
		window.toolTab.addDependentToolTab(OIL.createTool("entity", {
			entity: selectedItems[i].entityId
		}));
}

function onCommandPlace() {
	if(toolReadOnly)
		return;

	var selectedItems = getSelectedItems();

	var action = OIL.createAction("place");

	for(var i = 0; i < selectedItems.length; ++i) {
		var item = selectedItems[i];

		// check that this is link
		if(item.parentId == item.parent.entityId)
			continue;

		// do place
		item.entity.WriteTag(action, OIL.ids.tags.parent, OIL.eid2f(item.parent.entityId));
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
		var parentId = OIL.f2eid(entity.ReadTag(OIL.ids.tags.parent));
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

function onCommandDownloadFile() {
	var selectedItems = getSelectedItems();
	if(selectedItems.length != 1)
		return;

	var nsIFilePicker = Components.interfaces.nsIFilePicker;
	var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
	fp.init(window, "Select Destination", nsIFilePicker.modeSave);
	fp.appendFilters(nsIFilePicker.filterAll);
	fp.defaultString = selectedItems[0].entity.ReadField(OIL.ids.schemeDescs.file.fields.originalFileName) || "";
	fp.open({
		done: function(result) {
			try {
				if(result != nsIFilePicker.returnOK)
					return;

				var entity = selectedItems[0].entity;
				var scheme = entity.GetScheme();
				if(!scheme)
					return;
				if(scheme.GetId() != OIL.ids.schemes.file)
					return;

				var fileStream = OIL.core.GetNativeFileSystem().SaveStream(fp.file.path);
				var entityStream = new OIL.classes.Inanity.Oil.FileEntitySchemeInputStream(entity);
				fileStream.ReadAllFromStream(entityStream);
				entityStream.__reclaim();
				fileStream.__reclaim();
			}
			catch(e) {
				alert(e);
			}
		}
	});
}

function onContextMenuShowing() {
	var selectedItems = getSelectedItems();

	var hasFolder = false;
	var hasFile = false;
	var hasLink = false;
	for(var i = 0; i < selectedItems.length; ++i) {
		var item = selectedItems[i];
		if(item.entity.GetScheme().GetId() == OIL.ids.schemes.folder)
			hasFolder = true;
		if(item.entity.GetScheme().GetId() == OIL.ids.schemes.file)
			hasFile = true;
		if(item.parentId != item.parent.entityId)
			hasLink = true;
	}

	var oneFolder = hasFolder && selectedItems.length == 1 || selectedItems.length == 0;
	var oneFile = hasFile && selectedItems.length == 1;

	document.getElementById("contextMenuOpen").hidden = toolMode != "normal" || selectedItems.length <= 0;
	document.getElementById("contextMenuRename").hidden = toolReadOnly || selectedItems.length != 1;
	document.getElementById("contextMenuDelete").hidden = toolReadOnly || selectedItems.length <= 0;
	document.getElementById("contextMenuCreate").hidden = toolReadOnly || !oneFolder;
	document.getElementById("contextMenuUploadFile").hidden = toolReadOnly || !oneFolder;
	document.getElementById("contextMenuPlace").hidden = toolReadOnly || !hasLink;
	document.getElementById("contextMenuShowRealPlace").hidden = selectedItems.length != 1;
	document.getElementById("contextMenuDownloadFile").hidden = !oneFile;
	document.getElementById("contextMenuProperties").hidden = toolMode != "normal" || selectedItems.length <= 0;
}

function onTreeDragStart(event) {
	if(toolMode != "normal" || toolReadOnly)
		return;

	var selectedItems = getSelectedItems();
	if(selectedItems.length <= 0)
		return;

	for(var i = 0; i < selectedItems.length; ++i) {
		event.dataTransfer.mozSetDataAt(MIME_DRAG_FOLDER_ENTRY, JSON.stringify({
			itemId: selectedItems[i].entityId,
			folderId: selectedItems[i].parent.entityId
		}), i);
		event.dataTransfer.mozSetDataAt(MIME_DRAG_ENTITY, selectedItems[i].entityId, i);
	}

	event.dataTransfer.effectAllowed = "linkMove";
}

var lastSelectedEntityId = null;
function onTreeSelect(event) {
	var selectedItems = getSelectedItems();

	// update dependent tabs with selected entity
	if(toolMode == "normal" && selectedItems.length == 1) {
		var selectedEntityId = selectedItems[0].entityId;
		if(lastSelectedEntityId != selectedEntityId) {
			lastSelectedEntityId = selectedEntityId;
			window.toolTab.updateDependentToolTabs(function(params) {
				params.entity = selectedEntityId;
			});
		}
	}

	// update select callback
	var selectCallback = window.toolTab.params.selectCallback;
	if(selectCallback) {
		var ok = selectCallback(selectedItems.map(function(v) {
			return v.entityId;
		}));
		if(window.toolDialog)
			window.toolDialog.setAcceptOk(ok);
	}
}

var rootItem;

window.addEventListener('load', function() {
	if(!OIL.initToolWindow(window) || !window.toolTab.params.entity)
		return;

	window.toolTab.setTitle("folder");

	rootItem = new Item(OIL.entityManager.GetEntity(window.toolTab.params.entity));

	document.getElementById("labelEmpty").remove();

	toolMode = window.toolTab.params.mode || "normal";
	toolReadOnly = !!window.toolTab.params.readOnly;
	toolSingleSelection = !!window.toolTab.params.singleSelection;
	toolFilterCallback = window.toolTab.params.filterCallback;

	if(toolReadOnly) {
		getTree().setAttribute("editable", false);
	}

	if(toolSingleSelection) {
		getTree().setAttribute("seltype", "single");
	}

	view = new View(rootItem);
	getTree().view = view;
	rootItem.init(view);
	rootItem.open(true);
});

window.addEventListener('unload', function() {
	if(rootItem)
		rootItem.destroy();
});
