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
	// create client repo
	var clientRepo = OIL.core.CreateMemoryClientRepo();
	// create script repo
	var scriptRepo = OIL.core.CreateScriptRepo(clientRepo, remoteRepo);

	// initialize it
	scriptRepo.Init(function(result, message) {
		alert(JSON.stringify({
			result: result,
			message: message
		}));
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

var alreadyConnected = false;

function onConnect() {
	// if called from acceptDialog()
	if(alreadyConnected)
		return true;

	try {

		window.buttondisableaccept = true;

		// create remote repo
		var remoteRepo;
		switch(document.getElementById("menulistRemoteType").value) {
		case "url":
			remoteRepo = OIL.core.CreateUrlRemoteRepo(document.getElementById("textboxRemoteUrl").value);
			break;
		case "file":
			remoteRepo = OIL.core.CreateLocalRemoteRepo(document.getElementById("textboxRemoteFile").value);
			break;
		case "temp":
			remoteRepo = OIL.core.CreateTempRemoteRepo();
			break;
		case "memory":
			remoteRepo = OIL.core.CreateMemoryRemoteRepo();
			break;
		default:
			throw "wrong type of remote repo";
		}

		// create client repo
		var clientRepo;
		switch(document.getElementById("menulistCacheType").value) {
		case "file":
			clientRepo = OIL.core.CreateLocalClientRepo(document.getElementById("textboxCacheFile").value);
			break;
		case "temp":
			clientRepo = OIL.core.CreateTempClientRepo();
			break;
		case "memory":
			clientRepo = OIL.core.CreateMemoryClientRepo();
			break;
		default:
			throw "wrong type of client repo";
		}

		// create script repo
		var scriptRepo = OIL.core.CreateScriptRepo(clientRepo, remoteRepo);

		// init it
		scriptRepo.Init(function(ok, message) {
			if(ok) {
				window.arguments[0].repo = scriptRepo;
				alreadyConnected = true;
				var dialog = document.getElementById("connectrepo");
				dialog.acceptDialog();
			} else {
				alert(message);
				window.buttondisableaccept = false;
			}
		});

		return false;

	} catch(e) {
		window.buttondisableaccept = false;
		alert(e);
		return false;
	}
};

window.onload = function() {
	onChangeRemoteType();
	onChangeCacheType();
};
