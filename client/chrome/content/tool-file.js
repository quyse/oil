'use strict';

Components.utils.import('chrome://oil/content/oil.js');

var entity, entityCallback, entityDataInterfaceCallback;
var originalFileName, mimeType;
var dataResult;

function onChange(type, key, value) {
	switch(type) {
	case "scheme":
		if(!value || value.GetId() != OIL.ids.schemes.file)
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
	// if there is explicit MIME type, use it
	if(mimeType)
		return mimeType;

	// else try to detect MIME by extension
	if(originalFileName) {
		var extension = /\.([^.]+)$/.exec(originalFileName);
		if(extension) {
			extension = extension[1].toLowerCase();

			// try to use Mozilla MIME service
			try {
				var mimeService = Components.classes["@mozilla.org/mime;1"]
					.getService(Components.interfaces.nsIMIMEService);
				return mimeService.getTypeFromExtension(extension);
			} catch(e) {
				OIL.log(e);
			}

			// detect for self
			if(extension == "tga")
				return "image/x-tga";
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
		var imageData = dataResult;

		// TGA is not supported by Mozilla, convert to bmp
		if(mime == "image/x-tga" || mime == "image/x-targa") {
			try {
				let memoryStream = new OIL.classes.Inanity.MemoryStream();
				OIL.classes.Inanity.Graphics.BmpImage.Save((new OIL.classes.Inanity.Graphics.TgaImageLoader()).Load(imageData), memoryStream);
				imageData = memoryStream.ToFile();
				mime = "image/bmp";
			} catch(e) {
				OIL.log(e);
				imageData = null;
			}
		}

		var dataUrl = imageData ? ("data:" + mime + ";base64," + OIL.f2s(OIL.classes.Inanity.Data.Base64OutputStream.EncodeFile(imageData))) : "";

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
		var lastMoveX, lastMoveY;
		grid.addEventListener("mousedown", function(event) {
			this.setCapture();
		});
		grid.addEventListener("mousemove", function(event) {
			if(event.buttons & 1) {
				if(lastMoveX !== undefined && lastMoveY !== undefined) {
					grid.scrollLeft = grid.scrollLeft - (event.clientX - lastMoveX);
					grid.scrollTop = grid.scrollTop - (event.clientY - lastMoveY);
				}
				lastMoveX = event.clientX;
				lastMoveY = event.clientY;
			} else {
				lastMoveX = undefined;
				lastMoveY = undefined;
			}
		});
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
	if(!OIL.initToolWindow(window) || !window.toolTab.params.entity)
		return;

	window.toolTab.setTitle("file");

	entity = OIL.entityManager.GetEntity(window.toolTab.params.entity);
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

	document.getElementById("labelNote").remove();
});

window.addEventListener('unload', function() {
	entity = null;
	if(entityCallback)
		entityCallback.__reclaim();
	if(entityCallback)
		entityDataInterfaceCallback.__reclaim();
});
