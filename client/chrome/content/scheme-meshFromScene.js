OIL.ids.schemesInit.push({
	name: "meshFromScene",
	id: "mshs",
	displayName: "mesh from scene",
	fieldsInit: [{
		name: "scene",
		id: "scene",
		displayName: "scene",
		type: {
			type: "reference",
			interfaces: ["importedScene"]
		}
	}, {
		name: "meshName",
		id: "nm  ",
		displayName: "mesh name",
		type: "string"
	}],
	interfacesInit: [{
		name: "mesh",
		callback: function(entity) {

		}
	}],
	tool: "mesh"
});
