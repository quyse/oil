'use strict';

Components.utils.import('chrome://oil/content/oil.js');

const MIME_DRAG_ENTITY = "application/x-inanityoil-entity";

var entityId = null;
var entity = null;
var entityCallback = null;
var schemeId = null;

/* Control classes. */

function StringControl(parent, writeCallback) {
	this.textbox = document.createElementNS(OIL.XUL_NS, "textbox");
	parent.appendChild(this.textbox);

	this.writeCallback = writeCallback;

	this.baseValue = "";

	var This = this;

	this.textbox.addEventListener("keydown", function(event) {
		switch(event.keyCode) {
		case KeyEvent.DOM_VK_RETURN:
			This.write();
			break;
		case KeyEvent.DOM_VK_ESCAPE:
			This.textbox.value = This.baseValue;
			break;
		}
	});

	this.textbox.addEventListener("blur", function(event) {
		This.write();
	});
}
StringControl.prototype.write = function() {
	var newValue = this.textbox.value;
	if(newValue != this.baseValue)
		this.writeCallback(newValue);
};
StringControl.prototype.setValue = function(newValue) {
	this.baseValue = newValue;
	this.textbox.value = newValue;
};
StringControl.prototype.setFileValue = function(newFileValue) {
	this.setValue(OIL.f2s(newFileValue));
};

function NumberControl(parent, writeCallback) {
	this.textbox = document.createElementNS(OIL.XUL_NS, "textbox");
	parent.appendChild(this.textbox);

	this.writeCallback = writeCallback;

	this.baseValue = 0;

	var This = this;

	this.textbox.addEventListener("keydown", function(event) {
		switch(event.keyCode) {
		case KeyEvent.DOM_VK_RETURN:
			This.write();
			break;
		case KeyEvent.DOM_VK_ESCAPE:
			This.textbox.value = This.baseValue;
			break;
		}
	});

	this.textbox.addEventListener("blur", function(event) {
		This.write();
	});
}
NumberControl.prototype.write = function() {
	var newValueStr = this.textbox.value;
	if(newValueStr == "")
		return;
	var newValue = parseFloat(newValueStr);
	if(Number.isNaN(newValue)) {
		alert("not a number");
		return;
	}
	if(newValue != this.baseValue) {
		this.writeCallback(newValue);
	}
};
NumberControl.prototype.setValue = function(newValue) {
	this.baseValue = newValue;
	this.textbox.value = newValue + "";
};

function BoolControl(parent, writeCallback) {
	this.checkbox = document.createElementNS(OIL.XUL_NS, "checkbox");
	parent.appendChild(this.checkbox);

	var This = this;

	this.checkbox.addEventListener("command", function(event) {
		writeCallback(This.checkbox.checked);
	});
}
BoolControl.prototype.setValue = function(newValue) {
	this.checkbox.checked = newValue;
};

function ReferenceControl(parent, writeCallback, interfaces) {
	// hack: set center alignment for row
	parent.setAttribute("align", "center");
	// create hbox
	this.hbox = document.createElementNS(OIL.XUL_NS, "hbox");
	parent.appendChild(this.hbox);
	this.hbox.setAttribute("align", "center");
	// create image
	this.image = document.createElementNS(OIL.XUL_NS, "image");
	this.hbox.appendChild(this.image);
	// create label
	this.label = document.createElementNS(OIL.XUL_NS, "label");
	this.hbox.appendChild(this.label);
	this.label.flex = 1;
	this.label.setAttribute("class", "text-link");
	// create browse button
	this.buttonBrowse = document.createElementNS(OIL.XUL_NS, "button");
	this.hbox.appendChild(this.buttonBrowse);
	this.buttonBrowse.label = "...";

	this.entityId = null;
	this.entityName = null;

	var This = this;

	this.entityCallback = null;

	var check = function(dataTransfer) {
		if(dataTransfer.mozItemCount != 1)
			return false;
		var entityId = dataTransfer.mozGetDataAt(MIME_DRAG_ENTITY, 0);
		if(!entityId)
			return false;

		// get scheme
		var entity = OIL.entityManager.GetEntity(entityId);
		var entityScheme = entity.GetScheme();
		if(!entityScheme && interfaces.length)
			return false;

		// check interfaces
		for(var i = 0; i < interfaces.length; ++i)
			if(!entityScheme.HasInterface(interfaces[i]))
				return false;

		return entityId;
	};

	this.hbox.addEventListener("dragenter", function(event) {
		var entityId = check(event.dataTransfer);
		if(!entityId)
			return;

		event.preventDefault();
	});
	this.hbox.addEventListener("dragover", function(event) {
		var entityId = check(event.dataTransfer);
		if(!entityId)
			return;

		event.preventDefault();
	});
	this.hbox.addEventListener("drop", function(event) {
		var entityId = check(event.dataTransfer);
		if(!entityId)
			return;

		event.preventDefault();

		// set value
		writeCallback(entityId);
	});

	this.label.addEventListener("click", function(event) {
		if(This.entityId)
			OIL.createTool("entity", {
				entity: This.entityId
			});
	});

	window.addEventListener("unload", function(event) {
		if(This.entityCallback)
			This.entityCallback.__reclaim();
	});
};
ReferenceControl.prototype.setValue = function(newValue) {
	if(this.entityCallback)
		this.entityCallback.__reclaim();

	var This = this;

	var entity = newValue ? OIL.entityManager.GetEntity(newValue) : null;
	this.entityId = newValue;
	this.entityCallback = entity ? entity.AddCallback(function(type, key, value) {
		switch(type) {
		case "scheme":
			This.image.src = value ? (OIL.ids.schemeDescs[value.GetId()] || {}).icon : "";
			break;
		case "tag":
			switch(key) {
			case OIL.ids.tags.name:
				This.entityName = OIL.f2s(value) || "<unnamed>";
				This.label.value = This.entityName;
				break;
			}
			break;
		}
	}) : null;
	if(this.entityCallback) {
		this.entityCallback.EnumerateScheme();
		this.entityCallback.EnumerateTag(OIL.ids.tags.name);
	} else {
		this.entityName = "";
		this.image.src = "";
		this.label.value = "";
	}
};

var tagControls = {};
var fieldControls = {};

function onChange(type, key, value) {
	switch(type) {
	case "scheme":
		window.location.reload();
		break;
	case "tag":
		{
			let control = tagControls[key];
			if(control)
				control.setFileValue(value);

			if(key == OIL.ids.tags.name)
				window.toolTab.setTitle("entity: " + (OIL.f2s(value) || "<unnamed>"));
		}
		break;
	case "field":
		{
			let control = fieldControls[key];
			if(control)
				control.setValue(value);
		}
		break;
	case "data":
		// nothing
		break;
	}
}

function getMainGridRows() {
	return document.getElementById("gridMain").getElementsByTagNameNS(OIL.XUL_NS, "rows")[0];
}

/// Create row
function createRow(rows) {
	var row = document.createElementNS(OIL.XUL_NS, "row");
	rows.appendChild(row);
	row.align = "baseline";
	return row;
}

/// Create label.
function createLabel(parent, value) {
	var label = document.createElementNS(OIL.XUL_NS, "label");
	parent.appendChild(label);
	label.setAttribute("value", value);
}

function addField(scheme, fieldId) {
	var row = createRow(getMainGridRows());

	var schemeName = scheme.GetName();
	var fieldName = scheme.GetFieldName(fieldId);

	var label = document.createElementNS(OIL.XUL_NS, "label");
	row.appendChild(label);
	label.value = fieldName;

	var writeField = function(value) {
		var action = OIL.createAction("change " + fieldName + " of " + schemeName + " to " + JSON.stringify(value));
		entity.WriteField(action, fieldId, value);
		OIL.finishAction(action);
	};

	var control = null;

	switch(scheme.GetFieldType(fieldId).GetName()) {
	case "float":
	case "integer":
		control = new NumberControl(row, writeField);
		break;
	case "string":
		control = new StringControl(row, writeField);
		break;
	case "vec3":
		break;
	case "vec4":
		break;
	case "color3":
		break;
	case "color4":
		break;
	case "reference":
		control = new ReferenceControl(row, writeField, OIL.ids.schemeDescs[scheme.GetId()].fieldDescs[fieldId].type.interfaces);
		break;
	}

	if(control)
		fieldControls[fieldId] = control;
}

window.addEventListener('load', function() {
	if(!OIL.initToolWindow(window) || !window.toolTab.params.entity)
		return;
	entity = OIL.entityManager.GetEntity(window.toolTab.params.entity);
	if(!entity)
		return;

	window.toolTab.setTitle("entity");

	init();
});

function init() {
	entityId = entity.GetId();
	entityCallback = entity.AddCallback(onChange);

	document.getElementById("textboxEntityId").value = entityId;

	var scheme = entity.GetScheme();
	if(!scheme) {
		document.getElementById("boxMain").remove();
		return;
	}

	document.getElementById("labelEmpty").remove();

	schemeId = scheme.GetId();
	var schemeName = scheme.GetName();

	{
		let imageScheme = document.getElementById("imageScheme");
		imageScheme.src = OIL.ids.schemeDescs[schemeId].icon || "";

		let labelScheme = document.getElementById("labelScheme");
		labelScheme.value = scheme.GetName();
		labelScheme.setAttribute("tooltiptext", "Scheme ID: " + schemeId);
	}

	// add tags
	let addTag = function(tag) {
		var tagId = OIL.ids.tags[tag];

		var tagDesc = OIL.ids.tagDescs[tag];
		var tagName = tagDesc.name;

		var row = createRow(getMainGridRows());

		createLabel(row, tagName);

		var writeTag = function(value) {
			var action = OIL.createAction("change " + tagName + " of " + schemeName + " to " + JSON.stringify(value));
			entity.WriteTag(action, tagId, OIL.s2f(value));
			OIL.finishAction(action);
		};

		tagControls[tagId] = new StringControl(row, writeTag);

		// force set of first value
		onChange("tag", tagId, entity.ReadTag(tagId));
	};
	addTag("name");
	addTag("description");

	// add fields from scheme
	var schemeDesc = OIL.ids.schemeDescs[scheme.GetId()];
	var fieldsCount = scheme.GetFieldsCount();
	for(var i = 0; i < fieldsCount; ++i)
		addField(scheme, scheme.GetFieldId(i));

	// update values of fields
	entityCallback.EnumerateFields();
}

window.addEventListener('unload', function() {
	entity = null;
	if(entityCallback)
		entityCallback.__reclaim();
});
