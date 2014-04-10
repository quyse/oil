'use strict';

Components.utils.import('chrome://oil/content/oil.js');

function enableConnectButton(enable) {
	document.getElementById("connectrepo").setAttribute("buttondisableaccept", !enable);
}

function onChangeSomething(types, currentType) {
	// adjust visibility of all controls
	for(var type in types) {
		var hidden = type != currentType;
		var typeControls = types[type].controls;
		for(var i in typeControls)
			document.getElementById(typeControls[i]).hidden = hidden;
	}

	// perform necessary update
	if(types[currentType].update)
		types[currentType].update();

	// focus element if needed
	if(types[currentType].focus)
		document.getElementById(types[currentType].focus).focus();
}

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

	onChangeSomething(types, remoteType);
}

function onChangeCacheType() {
	var cacheType = document.getElementById("menulistCacheType").value;

	var types = {
		file: {
			controls: ['rowCacheFile'],
			focus: 'textboxCacheFile'
		},
		temp: {
			controls: ['rowCacheTemp']
		},
		memory: {
			controls: ['rowCacheMemory']
		}
	};

	onChangeSomething(types, cacheType);
}

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
	var file = OIL.profilePath.clone();
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

/* List of recent repos.
Format: {
	type: "url" or "file"
	remoteUrl: "url",
	remoteFile: "path",
	login: "login",
	localCache: "local cache path"
}
*/
var recentRepos = null;

function loadRecentRepos() {
	try {
		recentRepos = JSON.parse(OIL.prefs.getCharPref("recentRepos"));
	} catch(e) {}

	if(!recentRepos)
		recentRepos = [];

	var listbox = document.getElementById("listboxRecentRepos");

	for(var i = 0; i < recentRepos.length; ++i) {
		var repo = recentRepos[i];
		var repoType = repo.type;
		var repoRemote, repoLogin, repoLocalCache;
		switch(repoType) {
		case "url":
			repoRemote = repo.remoteUrl;
			repoLogin = repo.login;
			break;
		case "path":
			repoRemote = repo.remoteFile;
			break;
		default:
			continue;
		}
		repoLocalCache = repo.localCache;
		if(!repoLocalCache)
			continue;

		var listItem = document.createElement("listitem");
		listItem.value = i;
		var listCellRemote = document.createElement("listcell");
		listCellRemote.setAttribute("label", repoRemote);
		var listCellLogin = document.createElement("listcell");
		listCellLogin.setAttribute("label", repoLogin || "");
		var listCellLocalCache = document.createElement("listcell");
		listCellLocalCache.setAttribute("label", repoLocalCache);

		listItem.appendChild(listCellRemote);
		listItem.appendChild(listCellLogin);
		listItem.appendChild(listCellLocalCache);

		listbox.appendChild(listItem);
	}

	if(listbox.itemCount > 0)
		listbox.selectedIndex = 0;
}

function saveRecentRepos() {
	OIL.prefs.setCharPref("recentRepos", JSON.stringify(recentRepos));
}

function addRecentRepo(repo) {
	// remove duplicates of the repo
	var newLength = 0;
	for(var i = 0; i < recentRepos.length; ++i)
		if(recentRepos[i].type == repo.type) {
			var keep = false;
			switch(repo.type) {
			case "url":
				keep = repo.remoteUrl != recentRepos[i].remoteUrl;
				break;
			case "file":
				keep = repo.remoteFile != recentRepos[i].remoteFile;
				break;
			}
			if(keep)
				recentRepos[newLength++] = recentRepos[i];
		}

	if(newLength != recentRepos.length)
		recentRepos.splice(newLength, recentRepos.length - newLength);

	recentRepos.splice(0, 0, repo);
}

function editRecentRepo() {
	var selectedIndex = document.getElementById("listboxRecentRepos").selectedIndex;
	if(selectedIndex < 0)
		return false;

	// fill details from repo
	var repo = recentRepos[selectedIndex];
	switch(repo.type) {
	case "url":
		document.getElementById("textboxRemoteUrl").value = repo.remoteUrl;
		document.getElementById("textboxLogin").value = repo.login;
		break;
	case "file":
		document.getElementById("textboxRemoteFile").value = repo.remoteFile;
		break;
	default:
		return false;
	}

	// set remote type
	document.getElementById("menulistRemoteType").value = repo.type;
	onChangeRemoteType();

	// set local cache
	document.getElementById("textboxCacheFile").value = repo.localCache;
	document.getElementById("menulistCacheType").value = "file";
	onChangeCacheType();

	return true;
}

function onEditRecentRepo() {
	if(editRecentRepo())
		switchToNewRepoTab();
}

function onRemoveRecentRepo() {
	var listbox = document.getElementById("listboxRecentRepos");
	var selectedIndex = listbox.selectedIndex;
	if(selectedIndex < 0)
		return;

	listbox.removeItemAt(selectedIndex);
	recentRepos.splice(selectedIndex, 1);
	saveRecentRepos();
}

function switchToNewRepoTab() {
	document.getElementById("tabbox").selectedIndex = 0;
}
function switchToRecentReposTab() {
	document.getElementById("tabbox").selectedIndex = 1;
}

function onContextMenuRecentRepoShowing() {
	var hasSelectedItem = document.getElementById("listboxRecentRepos").selectedIndex >= 0;
	document.getElementById("contextMenuEdit").hidden = !hasSelectedItem;
	document.getElementById("contextMenuRemove").hidden = !hasSelectedItem;
}

var alreadyConnected = false;

function onConnect() {
	// if called from acceptDialog()
	if(alreadyConnected)
		return true;

	try {

		// if current tab is recent repos, get info from that
		if(document.getElementById("tabbox").selectedIndex == 1)
			if(!editRecentRepo()) {
				alert("no recent repo selected");
				return false;
			}

		enableConnectButton(false);

		var recentRepoToAdd = null;

		// create remote repo
		var remoteRepo;
		switch(document.getElementById("menulistRemoteType").value) {
		case "url":
			{
				let remoteUrl = document.getElementById("textboxRemoteUrl").value;
				let login = document.getElementById("textboxLogin").value;
				let password = document.getElementById("textboxPassword").value;
				remoteRepo = OIL.core.CreateUrlRemoteRepo(remoteUrl);
				recentRepoToAdd = {
					type: "url",
					remoteUrl: remoteUrl,
					login: login
				};
			}
			break;
		case "file":
			{
				let remoteFile = document.getElementById("textboxRemoteFile").value;
				remoteRepo = OIL.core.CreateLocalRemoteRepo(remoteFile);
				recentRepoToAdd = {
					type: "file",
					remoteFile: remoteFile
				};
			}
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
			{
				let localCache = document.getElementById("textboxCacheFile").value;
				clientRepo = OIL.core.CreateLocalClientRepo(localCache);
				recentRepoToAdd.localCache = localCache;
			}
			break;
		case "temp":
			clientRepo = OIL.core.CreateTempClientRepo();
			recentRepoToAdd = null;
			break;
		case "memory":
			clientRepo = OIL.core.CreateMemoryClientRepo();
			recentRepoToAdd = null;
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

				// add recent repo
				if(recentRepoToAdd) {
					addRecentRepo(recentRepoToAdd);
					saveRecentRepos();
				}
			} else {
				alert(message);
				enableConnectButton(true);
			}
		});

		return false;

	} catch(e) {
		enableConnectButton(true);
		alert(e);
		return false;
	}
};

window.onload = function() {
	onChangeRemoteType();
	onChangeCacheType();
	loadRecentRepos();
	if(recentRepos.length)
		switchToRecentReposTab();
};
