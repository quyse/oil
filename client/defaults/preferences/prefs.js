pref("toolkit.defaultChromeURI", "chrome://oil/content/main.xul");
pref("toolkit.singletonWindowType", "main");

pref("network.protocol-handler.warn-external.http", false);
pref("network.protocol-handler.warn-external.https", false);

/* debugging prefs, disable these before you deploy your application! */
pref("browser.dom.window.dump.enabled", true);
pref("javascript.options.showInConsole", true);
pref("javascript.options.strict", true);
pref("nglayout.debug.disable_xul_cache", true);
pref("nglayout.debug.disable_xul_fastload", true);
pref("devtools.debugger.remote-enabled", true);

// application prefs
pref("oil.debug", true);
pref("oil.tool-entity.place", "right");
pref("oil.tool-folder.place", "left");
pref("oil.layout", "{\"type\":\"hbox\",\"children\":[{\"type\":\"tabbox\",\"tool\":{\"id\":\"left\"},\"width\":200},{\"type\":\"splitter\",\"tool\":{\"collapse\":\"before\"}},{\"type\":\"tabbox\",\"tool\":{\"id\":\"main\"},\"flex\":1},{\"type\":\"splitter\",\"tool\":{\"collapse\":\"after\"}},{\"type\":\"tabbox\",\"tool\":{\"id\":\"right\"},\"width\":200}]}");
