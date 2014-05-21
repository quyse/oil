OIL.ids.schemesInit.push({
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
});
