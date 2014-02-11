OIL.uuids = {
	tags: {
		name: "0cc78838-4bc7-4da6-bb4b-c4bfcf0363a1",
		description: "c6f9edf5-ae60-4a38-8408-5f8af4ec7ef3",
		tags: "dc25f310-0e15-4913-a546-d6eee3eb87d0"
	},
	tagDescs: {
		name: {
			name: "name"
		},
		description: {
			name: "description"
		},
		tags: {
			name: "tags"
		}
	},
	schemes: {
		folder: "fa327807-86bb-4d8e-9ea4-fef2e943bd47"
	},
	schemeDescs: {
		folder: {
			name: "folder",
			fields: [],
			icon: "chrome://global/skin/dirListing/folder.png"
		}
	},
	rootFolder: "23f43821-92e3-4782-a460-d6944952e02b"
};

OIL.registerEntitySchemes = function(schemeManager) {
	var schemeDescs = OIL.uuids.schemeDescs;

	for(var schemeName in schemeDescs) {
		var schemeDesc = schemeDescs[schemeName];
		var scheme = new OIL.classes.Inanity.Oil.EntityScheme(OIL.uuids.schemes[schemeName], schemeDesc.name);

		// TODO: add fields

		// register scheme
		schemeManager.Register(scheme);
	}
};