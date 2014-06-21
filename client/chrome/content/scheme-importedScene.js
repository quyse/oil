OIL.ids.schemesInit.push({
	name: "importedScene",
	id: "iscn",
	displayName: "imported scene",
	fieldsInit: [{
		name: "data",
		id: "data",
		displayName: "data",
		type: {
			type: "reference",
			interfaces: ["data"]
		}
	}],
	interfacesInit: [{
		name: "importedScene",
		callback: function(entity) {
			var dataInterfaceCallback;
			var dataResult;
			var currentVersion = 0;

			function recalculate() {
				var version = ++currentVersion;

				OIL.setTimeout(function() {
					if(version != currentVersion)
						return;

					entity.SetInterfaceResult(OIL.ids.interfaces.importedScene, dataResult ? OIL.core.GetEngine().ImportScene(dataResult) : null);
				}, 0);
			}

			var entityCallback = entity.AddCallback(function(type, key, value) {
				switch(type) {
				case "field":
					switch(key) {
					case OIL.ids.schemeDescs.importedScene.fields.data:
						if(dataInterfaceCallback)
							dataInterfaceCallback.__reclaim();
						if(value) {
							var entity = OIL.entityManager.GetEntity(value);
							dataInterfaceCallback = entity.GetInterface(OIL.ids.interfaces.data).AddCallback(function(result) {
								dataResult = result;
								recalculate();
							});
						}
						else {
							dataInterfaceCallback = null;
							recalculate();
						}
						break;
					}
				}
			});
		}
	}]
});
