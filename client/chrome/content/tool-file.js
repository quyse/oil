'use strict';

Components.utils.import('chrome://oil/content/oil.js');

var entity, entityCallback, entityDataInterfaceCallback;
var originalFileName, mimeType;
var dataResult;

function onChange(type, key, value) {
	switch(type) {
	case "scheme":
		if(value.GetId() != OIL.ids.schemes.file)
			window.location.reload();
		break;
	case "tag":
		switch(key) {
		case OIL.ids.tags.name:
			window.toolTab.setTitle("file: " + OIL.f2s(value));
			break;
		}
		break;
	case "field":
		switch(key) {
		case OIL.ids.schemeDescs.file.fields.originalFileName:
			if(originalFileName != value) {
				originalFileName = value;
				rebuild();
			}
			break;
		case OIL.ids.schemeDescs.file.fields.mimeType:
			if(mimeType != value) {
				mimeType = value;
				rebuild();
			}
			break;
		}
		break;
	}
}

function onDataChange(result) {
	dataResult = result;
	rebuild();
}

function detectMimeType() {
	if(mimeType)
		return mimeType;

	if(originalFileName) {
		var extension = /\.([^.]+)$/.exec(originalFileName);
		if(!extension)
			return null;
		extension = extension[1];
		try {
			var mimeService = Components.classes["@mozilla.org/mime;1"]
				.getService(Components.interfaces.nsIMIMEService);
			return mimeService.getTypeFromExtension(extension);
		} catch(e) {
			OIL.log(e);
		}
	}

	return null;
}

function rebuild() {
	var mime = detectMimeType() || "";
	if(!dataResult)
		mime = "";

	var container = document.getElementById("container");
	// clear container
	container.textContent = "";

	// if image
	if(mime.startsWith("image/")) {
		var dataUrl = "data:" + mime + ";base64," + OIL.f2s(OIL.classes.Inanity.Data.Base64OutputStream.EncodeFile(dataResult));

		// create grid
		var grid = document.createElementNS(OIL.XUL_NS, "grid");
		container.appendChild(grid);
		grid.flex = 1;
		grid.setAttribute("style", "overflow:auto");
		var columns = document.createElementNS(OIL.XUL_NS, "columns");
		grid.appendChild(columns);
		var column = document.createElementNS(OIL.XUL_NS, "column");
		columns.appendChild(column);
		var rows = document.createElementNS(OIL.XUL_NS, "rows");
		grid.appendChild(rows);
		var row = document.createElementNS(OIL.XUL_NS, "row");
		rows.appendChild(row);

		var image = document.createElementNS(OIL.XUL_NS, "image");
		row.appendChild(image);
		image.src = dataUrl;
	}
	// if text
	else if(mime.startsWith("text/")) {
		var textbox = document.createElementNS(OIL.XUL_NS, "textbox");
		container.appendChild(textbox);
		textbox.setAttribute("multiline", true);
		textbox.flex = 1;
		textbox.readOnly = true;
		textbox.setAttribute("value", OIL.f2s(dataResult));
		textbox.setAttribute("style", "font-family:monospace");
	}
}

window.addEventListener('load', function() {
	var params = OIL.getParamsFromToolWindow(window);
	if(!params || !params.entity)
		return;

	window.toolTab.setTitle("file");

	entity = OIL.entityManager.GetEntity(params.entity);
	if(entity.GetScheme().GetId() != OIL.ids.schemes.file) {
		document.getElementById("labelNote").value = "scheme is not file";
		return;
	}

	entityCallback = entity.AddCallback(onChange);
	entityCallback.EnumerateScheme();
	onChange("tag", OIL.ids.tags.name, entity.ReadTag(OIL.ids.tags.name));
	entityCallback.EnumerateFields();

	var entityDataInterface = entity.GetInterface(OIL.ids.interfaces.data);
	entityDataInterfaceCallback = entityDataInterface.AddCallback(onDataChange);
	entityDataInterfaceCallback.Fire();

	document.getElementById("labelNote").remove();
});

window.addEventListener('unload', function() {
	entity = null;
	entityCallback = null;
	entityDataInterfaceCallback = null;
});
