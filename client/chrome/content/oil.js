var EXPORTED_SYMBOLS = ['OIL'];

var OIL = {
	init: function(pluginObject) {
		OIL.core = pluginObject;
		OIL.classes = pluginObject.GetRootNamespace();
	}
};
