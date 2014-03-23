'use strict';

OIL.ids = {
	/* Tag is characterized by:
	- name (internal): just for identification in code
	- id (4-byte): should be stable for a long time, serialized
	- display name: name for UI
	*/
	tagsInit: [{
		name: "name",
		id: "name",
		displayName: "name"
	}, {
		name: "description",
		id: "desc",
		displayName: "description"
	}, {
		name: "parent",
		id: "pare",
		displayName: "parent"
	}],

	/* Scheme is characterized by:
	- name (internal)
	- id (4-byte)
	- display name
	- fields: list of fields
		- name (internal)
		- id (4-byte)
		- displayName
		- type
	- icon: path to icon
	- tool: name of tool for opening object
	*/
	schemesInit: [{
		name: "folder",
		id: "fold",
		displayName: "folder",
		fieldsInit: [],
		icon: "chrome://global/skin/dirListing/folder.png",
		tool: "folder"
	}, {
		name: "file",
		id: "file",
		displayName: "file",
		fieldsInit: [{
			name: "originalFileName",
			id: "ornm",
			displayName: "original filename",
			type: "string"
		}]
	}],
	entities: {
		root: "0046c95e344a47569a96541a2b1f8e73"
	}
};

var initThings = function(initArray, convert, nameToIdArray, idNameToObjectArray) {
	for(var i = 0; i < initArray.length; ++i) {
		var o = initArray[i];
		if(convert)
			o.id = convert(o.id);
		nameToIdArray[o.name] = o.id;
		idNameToObjectArray[o.id] = o;
		idNameToObjectArray[o.name] = o;
	}
};

var convertSchemeId = function(s) {
	return OIL.classes.Inanity.Oil.EntitySchemeId.FromStringData(s);
};
var convertTagId = function(s) {
	return OIL.classes.Inanity.Oil.EntityTagId.FromStringData(s);
};
var convertFieldId = function(s) {
	return OIL.classes.Inanity.Oil.EntityFieldId.FromStringData(s);
};

OIL.initIds = function(schemeManager) {
	// init tags
	initThings(OIL.ids.tagsInit, convertTagId, OIL.ids.tags = {}, OIL.ids.tagDescs = {});

	// init schemes
	initThings(OIL.ids.schemesInit, convertSchemeId, OIL.ids.schemes = {}, OIL.ids.schemeDescs = {});

	// register schemes
	for(var i = 0; i < OIL.ids.schemesInit.length; ++i) {
		var schemeDesc = OIL.ids.schemesInit[i];
		var scheme = new OIL.classes.Inanity.Oil.EntityScheme(schemeDesc.id, schemeDesc.name);

		// init fields
		initThings(schemeDesc.fieldsInit, convertFieldId, schemeDesc.fields = {}, schemeDesc.fieldDescs = {});

		for(var j = 0; j < schemeDesc.fieldsInit.length; ++j) {
			var fieldDesc = schemeDesc.fieldsInit[j];
			scheme.AddField(fieldDesc.id, fieldDesc.type, fieldDesc.displayName);
		}

		// register scheme
		schemeManager.Register(scheme);
	}
};
