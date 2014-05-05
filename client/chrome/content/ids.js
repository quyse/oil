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
	}, {
		name: "file",
		id: "file",
		displayName: "file",
		fieldsInit: [{
			name: "originalFileName",
			id: "ornm",
			displayName: "original filename",
			type: "string"
		}, {
			name: "mimeType",
			id: "mime",
			displayName: "MIME type",
			type: "string"
		}],
		interfacesInit: [{
			name: "data",
			callback: function(entity) {
				// number of current version, in order to cancel pending recalculation
				var currentVersion = 0;

				// recalculate interface result
				function recalculate(version) {
					const blockSize = 0x10000;
					const timeout = 0;

					var sourceStream = null, resultStream = null;

					function cleanup() {
						if(sourceStream)
							sourceStream.__reclaim();
						if(resultStream)
							resultStream.__reclaim();
					}

					// one step in recalculation
					function step() {
						try {
							if(version != currentVersion) {
								cleanup();
								return;
							}

							if(!sourceStream)
								sourceStream = new OIL.classes.Inanity.Oil.FileEntitySchemeInputStream(entity);
							if(!resultStream)
								resultStream = new OIL.classes.Inanity.MemoryStream();

							// perform one step
							var block = sourceStream.Read(blockSize);
							// if there is some data
							if(block.GetSize() > 0) {
								// write
								resultStream.Write(block);
								// schedule next step
								OIL.setTimeout(step, timeout);
							} else {
								// data ends, finalize work
								setResult(resultStream.ToFile());
								cleanup();
							}

						} catch(e) {
							// error
							OIL.log(e);
							setResult(null);
							cleanup();
						}
					};
					OIL.setTimeout(step, timeout);
				}

				function setResult(result) {
					entity.SetInterfaceResult(OIL.ids.interfaces.data, result);
				}

				var entityCallback = entity.AddCallback(function(type, key, value) {
					if(type == "data")
						recalculate(++currentVersion);
				});

				// run first time
				OIL.setTimeout(function() {
					recalculate(0);
				}, 0);

				return function() {
					// cancel pending recalculation
					++currentVersion;
					// free resources
					entity = null;
					entityCallback.__reclaim();
				};
			}
		}],
		tool: "file"
	},
	// object to import image from raw image data
	{
		name: "image",
		id: "img ",
		displayName: "image",
		fieldsInit: [{
			name: "data",
			id: "data",
			displayName: "data",
			type: {
				type: "reference",
				interfaces: ["data"]
			}
		}, {
			name: "transform",
			id: "tran",
			displayName: "transform",
			type: {
				type: "reference",
				interfaces: ["imageTransform"]
			}
		}],
		interfacesInit: [{
			name: "image",
			callback: function(entity) {
				var entityCallback;
				var dataInterfaceCallback;
				var dataResult;
				var transformInterfaceCallback;
				var transformResult;
				var currentVersion = 0;

				function recalculate() {
					var version = ++currentVersion;

					setTimeout(function() {
						if(version != currentVersion)
							return;

						entity.SetInterfaceResult(OIL.ids.interfaces.image, dataResult ? OIL.core.LoadImage(dataResult) : null);
					}, 0);
				}

				entityCallback = entity.AddCallback(function(type, key, value) {
					switch(type) {
					case "field":
						switch(key) {
						case OIL.ids.schemeDescs.image.fields.data:
							if(dataInterfaceCallback)
								dataInterfaceCallback.__reclaim();
							if(value)
								dataInterfaceCallback = value.GetInterface(OIL.ids.interfaces.data).AddCallback(function(result) {
									dataResult = result;
									recalculate();
								});
							else {
								dataInterfaceCallback = null;
								recalculate();
							}
							break;
						case OIL.ids.schemeDescs.image.fields.transform:
							if(transformInterfaceCallback)
								transformInterfaceCallback.__reclaim();
							if(value)
								transformInterfaceCallback = value.GetInterface(OIL.ids.interfaces.imageTransform).AddCallback(function(result) {
									transformResult = result;
									recalculate();
								});
							else {
								transformInterfaceCallback = null;
								recalculate();
							}
							break;
						}
						break;
					}
				});

				return function() {
					entity = null;
					entityCallback.__reclaim();
					if(dataInterfaceCallback)
						dataInterfaceCallback.__reclaim();
					if(transformInterfaceCallback)
						transformInterfaceCallback.__reclaim();
				};
			}
		}]
	},
	// image transform
	{
		name: "imageTransform",
		id: "imgt",
		displayName: "image transform",
		fieldsInit: [
		// resize top mip to that width if > 0, else don't change
		{
			name: "width",
			id: "sz_x",
			displayName: "width",
			type: "integer"
		},
		// resize top mip to that height if > 0, else don't change
		{
			name: "height",
			id: "sz_y",
			displayName: "height",
			type: "integer"
		},
		// recalculate mips if > 0, else don't change
		{
			name: "mips",
			id: "mips",
			displayName: "mips",
			type: "integer"
		}],
		interfacesInit: [{
			name: "imageTransform",
			callback: function(entity) {
				var fields = {};

				var entityCallback;
				var currentVersion = 0;

				function recalculate(version) {
					OIL.setTimeout(function() {
						if(currentVersion != version)
							return;

						entity.SetInterfaceResult(OIL.ids.interfaces.imageTransform, {
							width: fields.width,
							height: fields.height,
							mips: fields.mips
						});
					}, 0);
				};

				entityCallback = entity.AddCallback(function(type, key, value) {
					switch(type) {
					case "field":
						key = OIL.ids.schemeDescs.imageTransform.fieldDescs[key];
						if(key) {
							fields[key.name] = value;
							recalculate(++currentVersion);
						}
						break;
					}
				});

				return function() {
					// cancel pending recalculation
					++currentVersion;
					// free resources
					entity = null;
					entityCallback.__reclaim();
				};
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
