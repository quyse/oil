OIL.ids.schemesInit.push({
	name: "modelFromScene",
	id: "mdls",
	displayName: "model from scene",
	fieldsInit: [{
		name: "scene",
		id: "scen",
		displayName: "scene",
		type: {
			type: "reference",
			interfaces: ["scene"]
		}
	}, {
		name: "modelName",
		id: "nm  ",
		displayName: "model name",
		type: "string"
	}],
	interfacesInit: [{
		name: "model",
		callback: function(entity) {
		}
	}],
	tool: "model"
});
