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
	interfacesInit: [{
		name: "data",
		id: "data",
		displayName: "data"
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
	}, {
		name: "file",
		id: "file",
		displayName: "file",
		fieldsInit: [{
			name: "originalFileName",
			id: "ornm",
			displayName: "original filename",
			type: "string"
		}],
		interfacesInit: [{
			name: "data",
			callback: function(entity) {
				function getData() {
					var file = null;
					try {
						var stream = new OIL.classes.Inanity.Oil.FileEntitySchemeInputStream(entity);
						file = stream.Read(stream.GetSize());
					} catch(e) {
						OIL.log(e);
					}
					return file;
				}

				function setResult() {
					entity.SetInterfaceResult(OIL.ids.interfaces.data, getData());
				}

				var entityCallback = entity.AddCallback(function(type, key, value) {
					if(type == "data")
						setResult();
				});

				setResult();

				return function() {
					entity = null;
					entityCallback = null;
				};
			}
		}]
	}, {
		name: "texture",
		id: "tex ",
		displayName: "texture",
		fieldsInit: [{
			name: "data",
			id: "data",
			displayName: "data",
			type: {
				type: "reference",
				interfaces: ["data"]
			}
		}]
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
			{
				type = new OIL.classes.Inanity.Oil.ReferenceEntityFieldType();

				let interfaces = desc.interfaces || [];
				for(var i = 0; i < interfaces.length; ++i)
					type.AddInterface(OIL.ids.interfaces[interfaces[i]]);
			}
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
