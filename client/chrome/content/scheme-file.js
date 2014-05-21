OIL.ids.schemesInit.push({
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
});
