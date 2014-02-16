'use strict';

Components.utils.import('chrome://oil/content/oil.js');

var entityId = null;
var entity = null;
var entityCallback = null;
var schemeId = null;

/// Functions to update tags.
var tagsUpdate = {};
/// Functions to update fields.
var fieldsUpdate = [];

function onChange(type, key, value) {
	switch(type) {
	case "scheme":
		window.location.reload();
		break;
	case "tag":
		{
			let update = tagsUpdate[key];
			if(update)
				update(value);
		}
		break;
	case "field":
		{
			let update = fieldsUpdate[key];
			if(update)
				update(value);
		}
		break;
	case "data":
		// TODO
		break;
	}
}

var textboxCount = 0;

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

/// Create textbox.
/** Returns function(value) for setting value. */
function createTextbox(parent, set, reset) {
	var textbox = document.createElementNS(OIL.XUL_NS, "textbox");
	parent.appendChild(textbox);

	textbox.addEventListener("keydown", function(event) {
		switch(event.keyCode) {
		case KeyEvent.DOM_VK_RETURN:
			set(textbox.value);
			break;
		case KeyEvent.DOM_VK_ESCAPE:
			textbox.value = reset() || "";
			break;
		}
	});

	textbox.addEventListener("blur", function(event) {
		set(textbox.value);
	});

	return function(value) {
		textbox.value = value;
	};
}

function addFieldControl(scheme, fieldIndex) {
	var row = createRow(getMainGridRows());

	var schemeName = scheme.GetName();
	var fieldName = scheme.GetFieldName(fieldIndex);

	var label = document.createElementNS(OIL.XUL_NS, "label");
	row.appendChild(label);
	label.value = fieldName;

	var lastValue;
	var writeField = function(value) {
		if(lastValue !== undefined && lastValue == value)
			return;

		var action = OIL.createAction("change " + fieldName + " of " + schemeName);
		entity.WriteField(action, fieldIndex, value);
		OIL.finishAction(action);

		lastValue = value;
	};

	var checkValue;

	var setField = function(value) {
		if(checkValue(value))
			writeField(value);
	};

	var resetField = function() {
		return lastValue;
	};

	switch(scheme.GetFieldType(fieldIndex)) {

	case "float":

		checkValue = function(value) {
			// check that this is float
			var number = parseFloat(value);
			if(Number.isNaN(number)) {
				alert("Not a number");
				return false;
			}
			return true;
		};

		fieldsUpdate[fieldIndex] = createTextbox(row, setField, resetField);

		break;

	case "integer":

		checkValue = function(value) {
			// check that this is integer
			if(!/^[0-9]+$/.test(value)) {
				alert("Not an integer");
				return false;
			}
			return true;
		};

		fieldsUpdate[fieldIndex] = createTextbox(row, setField, resetField);

		break;

	case "string":

		checkValue = function(value) {
			return true;
		};

		fieldsUpdate[fieldIndex] = createTextbox(row, setField, resetField);

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
		break;
	}
}

window.addEventListener('load', function() {
	entity = OIL.getEntityFromToolWindow(window);

	if(!entity)
		return;

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
		let textbox = document.getElementById("textboxScheme");
		textbox.value = scheme.GetName();
		textbox.setAttribute("tooltiptext", "Scheme ID: " + schemeId);
	}

	// add tags
	let addTag = function(tag) {
		var tagId = OIL.uuids.tags[tag];

		var tagDesc = OIL.uuids.tagDescs[tag];
		var tagName = tagDesc.name;

		var lastValue;

		var row = createRow(getMainGridRows());

		createLabel(row, tagName);

		var updateTextbox = createTextbox(row, function(value) {
			if(lastValue !== undefined && lastValue == value)
				return;

			var action = OIL.createAction("change " + tagName + " of " + schemeName + " to " + JSON.stringify(value));
			entity.WriteTag(action, tagId, OIL.s2f(value));
			OIL.finishAction(action);

			lastValue = value;
		}, function() {
			return lastValue;
		});

		// store update function
		tagsUpdate[tagId] = function(value) {
			value = value ? OIL.f2s(value) : "";
			updateTextbox(value);
			lastValue = value;
		};

		// force set of first value
		onChange("tag", tagId, entity.ReadTag(tagId));
	};
	addTag("name");
	addTag("description");

	// add fields from scheme
	var fieldsCount = scheme.GetFieldsCount();
	for(var i = 0; i < fieldsCount; ++i)
		addFieldControl(scheme, i);

	// update values of fields
	entityCallback.EnumerateFields();
	// TODO: update values of data
	entityCallback.EnumerateData();
}

window.addEventListener('unload', function() {
	entity = null;
	entityCallback = null;
});
