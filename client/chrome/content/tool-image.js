'use strict';

Components.utils.import('chrome://oil/content/oil.js');

var entity, entityCallback, entityImageInterfaceCallback;
var imageResult;

function onChange(type, key, value) {
	switch(type) {
	case "tag":
		switch(key) {
		case OIL.ids.tags.name:
			window.toolTab.setTitle("image: " + OIL.f2s(value));
			break;
		}
		break;
	}
}

function onImageChange(result) {
	viewRenderer.SetTexture(result);
	var mipsCount = result.GetImageMips();
	document.getElementById("scaleMipLod").max = (mipsCount - 1) * 10;
	document.getElementById("scaleMipBias").max = (mipsCount - 1) * 10;
}

function onMipModeChange(mipMode) {
	document.getElementById("buttonMipAuto").checked = mipMode == 0;
	document.getElementById("buttonMipLod").checked = mipMode == 1;
	document.getElementById("buttonMipBias").checked = mipMode == 2;

	viewRenderer.SetMipMode(mipMode);
}

function onMipLodChange() {
	var mipLod = document.getElementById("scaleMipLod").value * 0.1;
	document.getElementById("textboxMipLod").value = mipLod;
	onMipModeChange(1);
	viewRenderer.SetMipLod(mipLod);
}

function onMipBiasChange() {
	var mipBias = document.getElementById("scaleMipBias").value * 0.1;
	document.getElementById("textboxMipBias").value = mipBias;
	onMipModeChange(2);
	viewRenderer.SetMipBias(mipBias);
}

function onFilterChange(filterType, filterValue) {
	viewRenderer.SetFilter(filterType, filterValue);
}

function onScaleChange() {
	var scale = document.getElementById("scaleScale").value;
	document.getElementById("textboxScale").value = scale + "%";
	viewRenderer.SetScale(scale * 0.01);
}

function onTileChange() {
	viewRenderer.SetTile(document.getElementById("checkboxTile").checked);
}

var offset = [0, 0];
var scaleExp = 0;
var scale = 1;
function setScaleExp(newScaleExp, centerX, centerY) {
	if(centerX === undefined || centerY === undefined) {
		var container = document.getElementById("container");
		if(centerX === undefined)
			centerX = container.clientWidth / 2;
		if(centerY === undefined)
			centerY = container.clientHeight / 2;
	}

	if(newScaleExp < -3)
		newScaleExp = -3;
	else if(newScaleExp > 2)
		newScaleExp = 2;
	var scaleExpDelta = newScaleExp - scaleExp;
	scaleExp = newScaleExp;

	scale = Math.exp(scaleExp);
	viewRenderer.SetScale(scale);

	var scaleDelta = Math.exp(scaleExpDelta);
	offset[0] = centerX - (centerX - offset[0]) * scaleDelta;
	offset[1] = centerY - (centerY - offset[1]) * scaleDelta;
	viewRenderer.SetOffset(offset);

	document.getElementById("textboxScale").value = Math.floor(scale * 100) + "%";
};

var plugin;
var viewRenderer;

window.addEventListener("load", function() {

	if(!OIL.initToolWindow(window) || !window.toolTab.params.entity)
		return;

	window.toolTab.setTitle("image");

	entity = OIL.entityManager.GetEntity(window.toolTab.params.entity);
	var entityScheme = entity.GetScheme();
	if(!entityScheme)
		return;

	entityCallback = entity.AddCallback(onChange);
	entityCallback.EnumerateTag(OIL.ids.tags.name);

	var entityImageInterface = entity.GetInterface(OIL.ids.interfaces.image);
	entityImageInterfaceCallback = entityImageInterface.AddCallback(onImageChange);

	document.getElementById("labelNote").remove();

	plugin = document.getElementById("plugin");
	// workaround the wrapper problem
	var pluginId = plugin.GetId();
	plugin.__reclaim();
	plugin = OIL.classes.Inanity.Oil.ViewScriptObject.GetById(pluginId);

	// set view renderer
	viewRenderer = new OIL.classes.Inanity.Oil.TextureViewRenderer(OIL.core.GetEngine());
	plugin.SetViewRenderer(viewRenderer);

	// set initial settings
	document.getElementById("buttonFilterMinLinear").setAttribute("checked", true); onFilterChange(0, 1);
	document.getElementById("buttonFilterMipLinear").setAttribute("checked", true); onFilterChange(1, 1);
	document.getElementById("buttonFilterMagLinear").setAttribute("checked", true); onFilterChange(2, 1);
	onMipModeChange(0);
	onTileChange();

	// register move control
	var lastMoveX, lastMoveY;
	var container = document.getElementById("container");
	container.addEventListener("mousedown", function(event) {
		this.setCapture(true);
	}, true);
	container.addEventListener("mousemove", function(event) {
		if(event.buttons & 1) {
			if(lastMoveX !== undefined && lastMoveY !== undefined) {
				offset[0] += (event.clientX - lastMoveX);
				offset[1] += (event.clientY - lastMoveY);
				viewRenderer.SetOffset(offset);
			}
			lastMoveX = event.clientX;
			lastMoveY = event.clientY;
		} else {
			lastMoveX = undefined;
			lastMoveY = undefined;
		}
	}, true);
	// register scale control
	container.addEventListener("wheel", function(event) {
		setScaleExp(scaleExp - event.deltaY * 0.05, event.clientX, event.clientY);

		event.preventDefault();
	}, true);

	// run an update loop
	var updatePlugin = function() {
		plugin.Invalidate();
		setTimeout(updatePlugin, 10);
	};
	setTimeout(updatePlugin, 0);
});
