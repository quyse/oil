exports.configureCompiler = function(objectFile, compiler) {
	// object files: <conf>/object
	var a = /^([^\/]+)\/([^\/]+)$/.exec(objectFile);
	compiler.configuration = a[1];
	// replace dots with slashes, and add extension
	// if ends with .c - this is .c, otherwise .cpp
	var source = a[2];
	var b = /^(.*)\.c$/.exec(source);
	if(b) {
		// .c
		source = b[1].replace(/\./g, '/') + '.c';
		compiler.cppMode = false;
	}
	else
		// .cpp
		source = source.replace(/\./g, '/') + '.cpp';
	compiler.setSourceFile(source);
};

var libraries = {
};

var executables = {
	npoil: {
		objects: ['npoil', 'MainPluginInstance', 'ViewPluginInstance', 'ScriptObject', 'meta'],
		staticLibraries: [
			'../inanity//libinanity-base',
			'../inanity//libinanity-graphics',
			'../inanity//libinanity-dx11',
			'../inanity//libinanity-gl',
			'../inanity//libinanity-shaders',
			'../inanity//libinanity-platform',
			'../inanity//libinanity-npapi',
			'../inanity//libinanity-np',
			'../inanity//libinanity-input',
			'../inanity/deps/glew//libglew'
		],
		dynamicLibraries: ['user32.lib', 'gdi32.lib', 'opengl32.lib'],
		defFile: 'windows/npoil.def',
		resFiles: ['windows/npoil.res']
	},
	'oilserver': {
		objects: ['oilserver', 'Repo', 'ServerRepo'],
		staticLibraries: [
			'../inanity//libinanity-base',
			'../inanity//libinanity-sqlite',
			'../inanity/deps/sqlite//libsqlite',
			'../inanity//libinanity-fcgi',
			'../inanity/deps/fcgi//libfcgi'
			],
		'dynamicLibraries-win32': ['ws2_32.lib']
	},
	'repo-sync-test': {
		objects: ['repo-sync-test', 'Repo', 'ServerRepo', 'ClientRepo'],
		staticLibraries: [
			'../inanity//libinanity-base',
			'../inanity//libinanity-sqlite',
			'../inanity/deps/sqlite//libsqlite'
		]
	}
};

var platformed = function(object, field, platform) {
	return (object[field] || []).concat(object[field + '-' + platform] || []);
};

exports.configureComposer = function(libraryFile, composer) {
	// library files: <conf>/library
	var a = /^(([^\/]+)\/)([^\/]+)$/.exec(libraryFile);
	var confDir = a[1];
	composer.configuration = a[2];
	var library = libraries[a[3]];
	var objects = platformed(library, 'objects', composer.platform);
	for ( var i = 0; i < objects.length; ++i)
		composer.addObjectFile(confDir + objects[i]);
};

exports.configureLinker = function(executableFile, linker) {
	// executable files: <conf>/executable
	var a = /^(([^\/]+)\/)([^\/]+)$/.exec(executableFile);
	var confDir = a[1];
	linker.configuration = a[2];
	var executable = executables[a[3]];
	for ( var i = 0; i < executable.objects.length; ++i)
		linker.addObjectFile(confDir + executable.objects[i]);
	for ( var i = 0; i < executable.staticLibraries.length; ++i) {
		var staticLibrary = executable.staticLibraries[i];
		var confPos = staticLibrary.indexOf('//');
		if(confPos >= 0)
			staticLibrary = staticLibrary.replace('//', '/' + confDir);
		else
			staticLibrary = confDir + staticLibrary;
		linker.addStaticLibrary(staticLibrary);
	}
	var dynamicLibraries = platformed(executable, 'dynamicLibraries', linker.platform);
	for ( var i = 0; i < dynamicLibraries.length; ++i)
		linker.addDynamicLibrary(dynamicLibraries[i]);
	linker.defFile = executable.defFile;
	if(executable.resFiles)
		for(var i = 0; i < executable.resFiles.length; ++i)
			linker.resFiles.push(executable.resFiles[i]);
};
