Components.utils.import('chrome://oil/content/oil.js');

function onChangeRemoteType() {
	var remoteType = document.getElementById("menulistRemoteType").value;

	var types = {
		url: {
			controls: ['rowRemoteUrl', 'rowLogin', 'rowPassword'],
			update: onChangeRemoteUrl,
			focus: 'textboxRemoteUrl'
		},
		file: {
			controls: ['rowRemoteFile'],
			update: onChangeRemoteFile,
			focus: 'textboxRemoteFile'
		},
		temp: {
			controls: ['rowRemoteTemp']
		},
		memory: {
			controls: ['rowRemoteMemory']
		}
	};

	// adjust visibility of all remote-type related controls
	for(var type in types) {
		var hidden = type != remoteType;
		var typeControls = types[type].controls;
		for(var i in typeControls)
			document.getElementById(typeControls[i]).hidden = hidden;
	}

	// perform necessary update
	if(types[remoteType].update)
		types[remoteType].update();

	// focus element if needed
	if(types[remoteType].focus)
		document.getElementById(types[remoteType].focus).focus();
}

function onChangeCacheType() {
	var cacheType = document.getElementById("menulistCacheType").value;

	var controls = {
		file: ['rowCacheFile'],
		temp: ['rowCacheTemp'],
		memory: ['rowCacheMemory']
	};

	for(var type in controls) {
		var hidden = type != cacheType;
		var typeControls = controls[type];
		for(var i in typeControls)
			document.getElementById(typeControls[i]).hidden = hidden;
	}
}

var defaultDocumentsPath =
	Components.classes["@mozilla.org/file/directory_service;1"]
	.getService(Components.interfaces.nsIProperties)
	.get("ProfD", Components.interfaces.nsIFile);
var lastCacheFileName = "";

function suggestCacheFile(cacheFileName) {
	// if cache filename is not changed since last time, change it
	var textboxCacheFile = document.getElementById("textboxCacheFile");
	if(textboxCacheFile.value == lastCacheFileName) {
		lastCacheFileName = cacheFileName;
		textboxCacheFile.value = cacheFileName;
	}
}

function onChangeRemoteUrl() {
	var url = document.getElementById("textboxRemoteUrl").value;
	var file = defaultDocumentsPath.clone();
	file.append(encodeURIComponent(url));
	suggestCacheFile(file.path);
}

function onChangeRemoteFile() {
	var file = document.getElementById("textboxRemoteFile").value;
	suggestCacheFile(file + '.cache.oil');
}

function checkRemoteUrl() {
	// disable button
	var button = document.getElementById("buttonCheckRemoteUrl");
	button.disabled = true;

	// get an url
	var url = document.getElementById("textboxRemoteUrl").value;

	// create remote repo
	var remoteRepo = OIL.core.CreateUrlRemoteRepo(url);

	// check it
	OIL.core.CheckRemoteRepoManifest(remoteRepo, function(result, message) {
		document.getElementById("labelCheckRemoteUrlMessage").textContent = JSON.stringify({
			result: result,
			message: message
		});
		document.getElementById("panelCheckRemoteUrl").openPopup(
			document.getElementById("textboxRemoteUrl"),
			"after_start");
		button.disabled = false;
	});
}

function browseFile(settings, callback) {
	var nsIFilePicker = Components.interfaces.nsIFilePicker;
	var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
	fp.init(window, settings.title, nsIFilePicker.modeSave);
	fp.defaultExtension = "oil";
	fp.appendFilter(settings.filterName, "*.oil");
	fp.open({
		done: function(result) {
			switch(result) {
			case nsIFilePicker.returnOK:
			case nsIFilePicker.returnReplace:
				callback(fp.file.path);
				break;
			}
		}
	});
};

function onBrowseRemoteFile() {
	browseFile({
		title: "Select Location of Oil Server Repo",
		filterName: "Inanity Oil Server Repo"
	}, function(fileName) {
		document.getElementById("textboxRemoteFile").value = fileName;
	});
}

function onBrowseCacheFile() {
	browseFile({
		title: "Select Location of Oil Cache File",
		filterName: "Inanity Oil Client Repo"
	}, function(fileName) {
		document.getElementById("textboxCacheFile").value = fileName;
	});
}

window.onload = function() {
	onChangeRemoteType();
	onChangeCacheType();
};
