Components.utils.import('chrome://oil/content/oil.js');

function checkUrl() {
	// disable button
	var button = document.getElementById("buttonCheckUrl");
	button.disabled = true;

	// get an url
	var url = document.getElementById("menulistUrl").value;

	// check it
	OIL.core.CheckRepo(url, function(result, message) {
		document.getElementById("labelCheckUrlMessage").textContent = JSON.stringify({
			result: result,
			message: message
		});
		document.getElementById("panelCheckUrl").openPopup(
			document.getElementById("menulistUrl"),
			"after_start");
		button.disabled = false;
	});
}
