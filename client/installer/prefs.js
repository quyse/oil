pref("toolkit.defaultChromeURI", "chrome://oil/content/main.xul");
pref("toolkit.singletonWindowType", "main");

pref("network.protocol-handler.warn-external.http", false);
pref("network.protocol-handler.warn-external.https", false);

// application prefs
pref("oil.debug", false);
pref("oil.tool-entity.place", "right");
pref("oil.tool-folder.place", "left");
pref("oil.layout", "{\"type\":\"hbox\",\"children\":[{\"type\":\"tabbox\",\"tool\":{\"id\":\"left\"},\"width\":200},{\"type\":\"splitter\",\"tool\":{\"collapse\":\"before\"}},{\"type\":\"tabbox\",\"tool\":{\"id\":\"main\"},\"flex\":1},{\"type\":\"splitter\",\"tool\":{\"collapse\":\"after\"}},{\"type\":\"tabbox\",\"tool\":{\"id\":\"right\"},\"width\":200}]}");
