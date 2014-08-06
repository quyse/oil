'use strict';

Components.utils.import('chrome://oil/content/oil.js');

window.addEventListener('load', function() {
	if(!OIL.initToolWindow(window) || !window.toolTab.params.entity)
		return;

	window.toolTab.setTitle("default");

	var entity = OIL.entityManager.GetEntity(window.toolTab.params.entity);
	var scheme = entity.GetScheme();
	if(!scheme)
		return;

	var page = OIL.ids.schemeDescs[scheme.GetId()].tool || "wrong";
	var url = window.toolTab.composeUrlForPage(page);

	document.getElementById("iframe").setAttribute("src", url);

	document.getElementById("labelNote").remove();
});
