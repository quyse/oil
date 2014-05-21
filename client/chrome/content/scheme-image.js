OIL.ids.schemesInit.push({
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

				OIL.setTimeout(function() {
					if(version != currentVersion)
						return;

					entity.SetInterfaceResult(OIL.ids.interfaces.image, dataResult ? OIL.core.GetEngine().LoadRawTexture(dataResult).GenerateMips(0) : null);
				}, 0);
			}

			entityCallback = entity.AddCallback(function(type, key, value) {
				switch(type) {
				case "field":
					switch(key) {
					case OIL.ids.schemeDescs.image.fields.data:
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
					case OIL.ids.schemeDescs.image.fields.transform:
						if(transformInterfaceCallback)
							transformInterfaceCallback.__reclaim();
						if(value) {
							var entity = OIL.entityManager.GetEntity(value);
							transformInterfaceCallback = entity.GetInterface(OIL.ids.interfaces.imageTransform).AddCallback(function(result) {
								transformResult = result;
								recalculate();
							});
						}
						else {
							transformInterfaceCallback = null;
							recalculate();
						}
						break;
					}
					break;
				}
			});

			entityCallback.EnumerateFields();

			return function() {
				// cancel pending recalculation
				++currentVersion;
				// free resources
				entity = null;
				entityCallback.__reclaim();
				if(dataInterfaceCallback)
					dataInterfaceCallback.__reclaim();
				if(transformInterfaceCallback)
					transformInterfaceCallback.__reclaim();
			};
		}
	}],
	tool: "image"
});
