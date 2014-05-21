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

	/* Interface is characterized by:
	- name (internal)
	- id (4-byte)
	- display name
	*/
	interfacesInit: [
	/// data interface
	/** result is ptr<File> */
	{
		name: "data",
		id: "data",
		displayName: "data"
	},
	/// image interface
	/** result is ptr<RawImageData> */
	{
		name: "image",
		id: "img ",
		displayName: "image"
	},
	/// image transform interface
	/** result is {
		width: <required width or null>,
		height: <required height or null>,
		mips: <required mips or null>
	} */
	{
		name: "imageTransform",
		id: "imgt",
		displayName: "image transform"
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
	- interfacesInit: [{
			name (internal name of interface)
			callback: function(entity) {
				return function() {
					// free all resources
				};
			}
		}]

	How interface callback works.

	Interface callback is a function with one argument - entity.
	Interface callback creates all it needs to keep track of entity's
	interface view, and returns a cancel callback - callback to
	free all resources which was allocated for tracking.
	For the first time interface callback should call
	Entity::SetInterfaceResult with the current result, regardless of
	did entity changed or not. After that interface callback should
	call Entity::SetInterfaceResult each time data was changed.

	*/
	schemesInit: [{
		name: "client_version",
		id: "cver",
		displayName: "client version",
		fieldsInit: [{
			name: "version",
			id: "vrsn",
			displayName: "version",
			type: "string"
		}, {
			name: "url",
			id: "url ",
			displayName: "url",
			type: "string"
		}]
	}, {
		name: "folder",
		id: "fold",
		displayName: "folder",
		fieldsInit: [],
		interfacesInit: [],
		icon: "chrome://global/skin/dirListing/folder.png",
		tool: "folder"
	}],

	// entities
	entitiesInit: [{
		// root folder
		name: "root",
		id: "0046c95e344a47569a96541a2b1f8e73"
	}, {
		// client version object
		name: "client_version",
		id: "9b57146613cf4e889aa233ac81075294"
	}]
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
var convertInterfaceId = function(s) {
	return OIL.classes.Inanity.Oil.EntityInterfaceId.FromStringData(s);
};
var convertFieldId = function(s) {
	return OIL.classes.Inanity.Oil.EntityFieldId.FromStringData(s);
};

var getFieldType = function(schemeManager, desc) {
	var type = null;

	if(typeof desc == 'string')
		type = schemeManager.GetStandardFieldType(desc);
	else if(typeof desc == 'object') {
		switch(desc.type) {
		case 'reference':
			type = new OIL.classes.Inanity.Oil.ReferenceEntityFieldType();
			// replace interface names with ids
			if(!desc.interfaces)
				desc.interfaces = [];
			for(var i = 0; i < desc.interfaces.length; ++i)
				desc.interfaces[i] = OIL.ids.interfaces[desc.interfaces[i]];
			break;
		}
	}

	if(!type)
		throw 'invalid field type';

	return type;
};

OIL.initIds = function(schemeManager) {
	// init tags
	initThings(OIL.ids.tagsInit, convertTagId, OIL.ids.tags = {}, OIL.ids.tagDescs = {});

	// init interfaces
	initThings(OIL.ids.interfacesInit, convertInterfaceId, OIL.ids.interfaces = {}, OIL.ids.interfaceDescs = {});

	// init schemes
	initThings(OIL.ids.schemesInit, convertSchemeId, OIL.ids.schemes = {}, OIL.ids.schemeDescs = {});

	// init entities
	initThings(OIL.ids.entitiesInit, null, OIL.ids.entities = {}, OIL.ids.entityDescs = {});

	// register schemes
	for(var i = 0; i < OIL.ids.schemesInit.length; ++i) {
		var schemeDesc = OIL.ids.schemesInit[i];
		var scheme = new OIL.classes.Inanity.Oil.EntityScheme(schemeDesc.id, schemeDesc.displayName);

		// init fields
		initThings(schemeDesc.fieldsInit, convertFieldId, schemeDesc.fields = {}, schemeDesc.fieldDescs = {});

		// add fields
		for(var j = 0; j < schemeDesc.fieldsInit.length; ++j) {
			var fieldDesc = schemeDesc.fieldsInit[j];
			scheme.AddField(fieldDesc.id, getFieldType(schemeManager, fieldDesc.type), fieldDesc.displayName);
		}

		// register interfaces
		var interfaces = schemeDesc.interfacesInit || [];
		for(var j = 0; j < interfaces.length; ++j) {
			scheme.AddInterface(OIL.ids.interfaces[interfaces[j].name], interfaces[j].callback);
		}

		// register scheme
		schemeManager.RegisterScheme(scheme);
	}
};
