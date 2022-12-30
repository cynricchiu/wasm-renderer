var Module = typeof Module != 'undefined' ? Module : {};
var moduleOverrides = Object.assign({}, Module);
var arguments_ = [];
var thisProgram = './this.program';
var quit_ = (status, toThrow) => {
	throw toThrow;
};
var ENVIRONMENT_IS_WEB = typeof window == 'object';
var ENVIRONMENT_IS_WORKER = typeof importScripts == 'function';
var ENVIRONMENT_IS_NODE =
	typeof process == 'object' && typeof process.versions == 'object' && typeof process.versions.node == 'string';
var scriptDirectory = '';
function locateFile(path) {
	if (Module['locateFile']) {
		return Module['locateFile'](path, scriptDirectory);
	}
	return scriptDirectory + path;
}
var read_, readAsync, readBinary, setWindowTitle;
function logExceptionOnExit(e) {
	if (e instanceof ExitStatus) return;
	let toLog = e;
	err('exiting due to exception: ' + toLog);
}
if (ENVIRONMENT_IS_NODE) {
	var fs = require('fs');
	var nodePath = require('path');
	if (ENVIRONMENT_IS_WORKER) {
		scriptDirectory = nodePath.dirname(scriptDirectory) + '/';
	} else {
		scriptDirectory = __dirname + '/';
	}
	read_ = (filename, binary) => {
		filename = isFileURI(filename) ? new URL(filename) : nodePath.normalize(filename);
		return fs.readFileSync(filename, binary ? undefined : 'utf8');
	};
	readBinary = filename => {
		var ret = read_(filename, true);
		if (!ret.buffer) {
			ret = new Uint8Array(ret);
		}
		return ret;
	};
	readAsync = (filename, onload, onerror) => {
		filename = isFileURI(filename) ? new URL(filename) : nodePath.normalize(filename);
		fs.readFile(filename, function (err, data) {
			if (err) onerror(err);
			else onload(data.buffer);
		});
	};
	if (process['argv'].length > 1) {
		thisProgram = process['argv'][1].replace(/\\/g, '/');
	}
	arguments_ = process['argv'].slice(2);
	if (typeof module != 'undefined') {
		module['exports'] = Module;
	}
	process['on']('uncaughtException', function (ex) {
		if (!(ex instanceof ExitStatus)) {
			throw ex;
		}
	});
	process['on']('unhandledRejection', function (reason) {
		throw reason;
	});
	quit_ = (status, toThrow) => {
		if (keepRuntimeAlive()) {
			process['exitCode'] = status;
			throw toThrow;
		}
		logExceptionOnExit(toThrow);
		process['exit'](status);
	};
	Module['inspect'] = function () {
		return '[Emscripten Module object]';
	};
} else if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
	if (ENVIRONMENT_IS_WORKER) {
		scriptDirectory = self.location.href;
	} else if (typeof document != 'undefined' && document.currentScript) {
		scriptDirectory = document.currentScript.src;
	}
	if (scriptDirectory.indexOf('blob:') !== 0) {
		scriptDirectory = scriptDirectory.substr(0, scriptDirectory.replace(/[?#].*/, '').lastIndexOf('/') + 1);
	} else {
		scriptDirectory = '';
	}
	{
		read_ = url => {
			var xhr = new XMLHttpRequest();
			xhr.open('GET', url, false);
			xhr.send(null);
			return xhr.responseText;
		};
		if (ENVIRONMENT_IS_WORKER) {
			readBinary = url => {
				var xhr = new XMLHttpRequest();
				xhr.open('GET', url, false);
				xhr.responseType = 'arraybuffer';
				xhr.send(null);
				return new Uint8Array(xhr.response);
			};
		}
		readAsync = (url, onload, onerror) => {
			var xhr = new XMLHttpRequest();
			xhr.open('GET', url, true);
			xhr.responseType = 'arraybuffer';
			xhr.onload = () => {
				if (xhr.status == 200 || (xhr.status == 0 && xhr.response)) {
					onload(xhr.response);
					return;
				}
				onerror();
			};
			xhr.onerror = onerror;
			xhr.send(null);
		};
	}
	setWindowTitle = title => (document.title = title);
} else {
}
var out = Module['print'] || console.log.bind(console);
var err = Module['printErr'] || console.warn.bind(console);
Object.assign(Module, moduleOverrides);
moduleOverrides = null;
if (Module['arguments']) arguments_ = Module['arguments'];
if (Module['thisProgram']) thisProgram = Module['thisProgram'];
if (Module['quit']) quit_ = Module['quit'];
var wasmBinary;
if (Module['wasmBinary']) wasmBinary = Module['wasmBinary'];
var noExitRuntime = Module['noExitRuntime'] || true;
if (typeof WebAssembly != 'object') {
	abort('no native wasm support detected');
}
var wasmMemory;
var ABORT = false;
var EXITSTATUS;
var buffer, HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;
function updateGlobalBufferAndViews(buf) {
	buffer = buf;
	Module['HEAP8'] = HEAP8 = new Int8Array(buf);
	Module['HEAP16'] = HEAP16 = new Int16Array(buf);
	Module['HEAP32'] = HEAP32 = new Int32Array(buf);
	Module['HEAPU8'] = HEAPU8 = new Uint8Array(buf);
	Module['HEAPU16'] = HEAPU16 = new Uint16Array(buf);
	Module['HEAPU32'] = HEAPU32 = new Uint32Array(buf);
	Module['HEAPF32'] = HEAPF32 = new Float32Array(buf);
	Module['HEAPF64'] = HEAPF64 = new Float64Array(buf);
}
var INITIAL_MEMORY = Module['INITIAL_MEMORY'] || 16777216;
var wasmTable;
var __ATPRERUN__ = [];
var __ATINIT__ = [];
var __ATPOSTRUN__ = [];
var runtimeInitialized = false;
function keepRuntimeAlive() {
	return noExitRuntime;
}
function preRun() {
	if (Module['preRun']) {
		if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
		while (Module['preRun'].length) {
			addOnPreRun(Module['preRun'].shift());
		}
	}
	callRuntimeCallbacks(__ATPRERUN__);
}
function initRuntime() {
	runtimeInitialized = true;
	callRuntimeCallbacks(__ATINIT__);
}
function postRun() {
	if (Module['postRun']) {
		if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
		while (Module['postRun'].length) {
			addOnPostRun(Module['postRun'].shift());
		}
	}
	callRuntimeCallbacks(__ATPOSTRUN__);
}
function addOnPreRun(cb) {
	__ATPRERUN__.unshift(cb);
}
function addOnInit(cb) {
	__ATINIT__.unshift(cb);
}
function addOnPostRun(cb) {
	__ATPOSTRUN__.unshift(cb);
}
var runDependencies = 0;
var runDependencyWatcher = null;
var dependenciesFulfilled = null;
function addRunDependency(id) {
	runDependencies++;
	if (Module['monitorRunDependencies']) {
		Module['monitorRunDependencies'](runDependencies);
	}
}
function removeRunDependency(id) {
	runDependencies--;
	if (Module['monitorRunDependencies']) {
		Module['monitorRunDependencies'](runDependencies);
	}
	if (runDependencies == 0) {
		if (runDependencyWatcher !== null) {
			clearInterval(runDependencyWatcher);
			runDependencyWatcher = null;
		}
		if (dependenciesFulfilled) {
			var callback = dependenciesFulfilled;
			dependenciesFulfilled = null;
			callback();
		}
	}
}
function abort(what) {
	if (Module['onAbort']) {
		Module['onAbort'](what);
	}
	what = 'Aborted(' + what + ')';
	err(what);
	ABORT = true;
	EXITSTATUS = 1;
	what += '. Build with -sASSERTIONS for more info.';
	var e = new WebAssembly.RuntimeError(what);
	throw e;
}
var dataURIPrefix = 'data:application/octet-stream;base64,';
function isDataURI(filename) {
	return filename.startsWith(dataURIPrefix);
}
function isFileURI(filename) {
	return filename.startsWith('file://');
}
var wasmBinaryFile;
wasmBinaryFile = 'main.wasm';
if (!isDataURI(wasmBinaryFile)) {
	wasmBinaryFile = locateFile(wasmBinaryFile);
}
function getBinary(file) {
	try {
		if (file == wasmBinaryFile && wasmBinary) {
			return new Uint8Array(wasmBinary);
		}
		if (readBinary) {
			return readBinary(file);
		}
		throw 'both async and sync fetching of the wasm failed';
	} catch (err) {
		abort(err);
	}
}
function getBinaryPromise() {
	if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER)) {
		if (typeof fetch == 'function' && !isFileURI(wasmBinaryFile)) {
			return fetch(wasmBinaryFile, { credentials: 'same-origin' })
				.then(function (response) {
					if (!response['ok']) {
						throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
					}
					return response['arrayBuffer']();
				})
				.catch(function () {
					return getBinary(wasmBinaryFile);
				});
		} else {
			if (readAsync) {
				return new Promise(function (resolve, reject) {
					readAsync(
						wasmBinaryFile,
						function (response) {
							resolve(new Uint8Array(response));
						},
						reject
					);
				});
			}
		}
	}
	return Promise.resolve().then(function () {
		return getBinary(wasmBinaryFile);
	});
}
function createWasm() {
	var info = { a: asmLibraryArg };
	function receiveInstance(instance, module) {
		var exports = instance.exports;
		Module['asm'] = exports;
		wasmMemory = Module['asm']['b'];
		updateGlobalBufferAndViews(wasmMemory.buffer);
		wasmTable = Module['asm']['k'];
		addOnInit(Module['asm']['c']);
		removeRunDependency('wasm-instantiate');
	}
	addRunDependency('wasm-instantiate');
	function receiveInstantiationResult(result) {
		receiveInstance(result['instance']);
	}
	function instantiateArrayBuffer(receiver) {
		return getBinaryPromise()
			.then(function (binary) {
				return WebAssembly.instantiate(binary, info);
			})
			.then(function (instance) {
				return instance;
			})
			.then(receiver, function (reason) {
				err('failed to asynchronously prepare wasm: ' + reason);
				abort(reason);
			});
	}
	function instantiateAsync() {
		if (
			!wasmBinary &&
			typeof WebAssembly.instantiateStreaming == 'function' &&
			!isDataURI(wasmBinaryFile) &&
			!isFileURI(wasmBinaryFile) &&
			!ENVIRONMENT_IS_NODE &&
			typeof fetch == 'function'
		) {
			return fetch(wasmBinaryFile, { credentials: 'same-origin' }).then(function (response) {
				var result = WebAssembly.instantiateStreaming(response, info);
				return result.then(receiveInstantiationResult, function (reason) {
					err('wasm streaming compile failed: ' + reason);
					err('falling back to ArrayBuffer instantiation');
					return instantiateArrayBuffer(receiveInstantiationResult);
				});
			});
		} else {
			return instantiateArrayBuffer(receiveInstantiationResult);
		}
	}
	if (Module['instantiateWasm']) {
		try {
			var exports = Module['instantiateWasm'](info, receiveInstance);
			return exports;
		} catch (e) {
			err('Module.instantiateWasm callback failed with error: ' + e);
			return false;
		}
	}
	instantiateAsync();
	return {};
}
function ExitStatus(status) {
	this.name = 'ExitStatus';
	this.message = 'Program terminated with exit(' + status + ')';
	this.status = status;
}
function callRuntimeCallbacks(callbacks) {
	while (callbacks.length > 0) {
		callbacks.shift()(Module);
	}
}
function abortOnCannotGrowMemory(requestedSize) {
	abort('OOM');
}
function _emscripten_resize_heap(requestedSize) {
	var oldSize = HEAPU8.length;
	requestedSize = requestedSize >>> 0;
	abortOnCannotGrowMemory(requestedSize);
}
function uleb128Encode(n, target) {
	if (n < 128) {
		target.push(n);
	} else {
		target.push(n % 128 | 128, n >> 7);
	}
}
function sigToWasmTypes(sig) {
	var typeNames = { i: 'i32', j: 'i32', f: 'f32', d: 'f64', p: 'i32' };
	var type = { parameters: [], results: sig[0] == 'v' ? [] : [typeNames[sig[0]]] };
	for (var i = 1; i < sig.length; ++i) {
		type.parameters.push(typeNames[sig[i]]);
		if (sig[i] === 'j') {
			type.parameters.push('i32');
		}
	}
	return type;
}
function generateFuncType(sig, target) {
	var sigRet = sig.slice(0, 1);
	var sigParam = sig.slice(1);
	var typeCodes = { i: 127, p: 127, j: 126, f: 125, d: 124 };
	target.push(96);
	uleb128Encode(sigParam.length, target);
	for (var i = 0; i < sigParam.length; ++i) {
		target.push(typeCodes[sigParam[i]]);
	}
	if (sigRet == 'v') {
		target.push(0);
	} else {
		target.push(1, typeCodes[sigRet]);
	}
}
function convertJsFunctionToWasm(func, sig) {
	if (typeof WebAssembly.Function == 'function') {
		return new WebAssembly.Function(sigToWasmTypes(sig), func);
	}
	var typeSectionBody = [1];
	generateFuncType(sig, typeSectionBody);
	var bytes = [0, 97, 115, 109, 1, 0, 0, 0, 1];
	uleb128Encode(typeSectionBody.length, bytes);
	bytes.push.apply(bytes, typeSectionBody);
	bytes.push(2, 7, 1, 1, 101, 1, 102, 0, 0, 7, 5, 1, 1, 102, 0, 0);
	var module = new WebAssembly.Module(new Uint8Array(bytes));
	var instance = new WebAssembly.Instance(module, { e: { f: func } });
	var wrappedFunc = instance.exports['f'];
	return wrappedFunc;
}
var wasmTableMirror = [];
function getWasmTableEntry(funcPtr) {
	var func = wasmTableMirror[funcPtr];
	if (!func) {
		if (funcPtr >= wasmTableMirror.length) wasmTableMirror.length = funcPtr + 1;
		wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr);
	}
	return func;
}
function updateTableMap(offset, count) {
	if (functionsInTableMap) {
		for (var i = offset; i < offset + count; i++) {
			var item = getWasmTableEntry(i);
			if (item) {
				functionsInTableMap.set(item, i);
			}
		}
	}
}
var functionsInTableMap = undefined;
var freeTableIndexes = [];
function getEmptyTableSlot() {
	if (freeTableIndexes.length) {
		return freeTableIndexes.pop();
	}
	try {
		wasmTable.grow(1);
	} catch (err) {
		if (!(err instanceof RangeError)) {
			throw err;
		}
		throw 'Unable to grow wasm table. Set ALLOW_TABLE_GROWTH.';
	}
	return wasmTable.length - 1;
}
function setWasmTableEntry(idx, func) {
	wasmTable.set(idx, func);
	wasmTableMirror[idx] = wasmTable.get(idx);
}
function addFunction(func, sig) {
	if (!functionsInTableMap) {
		functionsInTableMap = new WeakMap();
		updateTableMap(0, wasmTable.length);
	}
	if (functionsInTableMap.has(func)) {
		return functionsInTableMap.get(func);
	}
	var ret = getEmptyTableSlot();
	try {
		setWasmTableEntry(ret, func);
	} catch (err) {
		if (!(err instanceof TypeError)) {
			throw err;
		}
		var wrapped = convertJsFunctionToWasm(func, sig);
		setWasmTableEntry(ret, wrapped);
	}
	functionsInTableMap.set(func, ret);
	return ret;
}
var asmLibraryArg = { a: _emscripten_resize_heap };
var asm = createWasm();
var ___wasm_call_ctors = (Module['___wasm_call_ctors'] = function () {
	return (___wasm_call_ctors = Module['___wasm_call_ctors'] = Module['asm']['c']).apply(null, arguments);
});
var _init = (Module['_init'] = function () {
	return (_init = Module['_init'] = Module['asm']['d']).apply(null, arguments);
});
var _createColorBuffer = (Module['_createColorBuffer'] = function () {
	return (_createColorBuffer = Module['_createColorBuffer'] = Module['asm']['e']).apply(null, arguments);
});
var _clearRect = (Module['_clearRect'] = function () {
	return (_clearRect = Module['_clearRect'] = Module['asm']['f']).apply(null, arguments);
});
var _drawCircle = (Module['_drawCircle'] = function () {
	return (_drawCircle = Module['_drawCircle'] = Module['asm']['g']).apply(null, arguments);
});
var _getString = (Module['_getString'] = function () {
	return (_getString = Module['_getString'] = Module['asm']['h']).apply(null, arguments);
});
var _getArray = (Module['_getArray'] = function () {
	return (_getArray = Module['_getArray'] = Module['asm']['i']).apply(null, arguments);
});
var _test = (Module['_test'] = function () {
	return (_test = Module['_test'] = Module['asm']['j']).apply(null, arguments);
});
Module['addFunction'] = addFunction;
var calledRun;
dependenciesFulfilled = function runCaller() {
	if (!calledRun) run();
	if (!calledRun) dependenciesFulfilled = runCaller;
};
function run(args) {
	args = args || arguments_;
	if (runDependencies > 0) {
		return;
	}
	preRun();
	if (runDependencies > 0) {
		return;
	}
	function doRun() {
		if (calledRun) return;
		calledRun = true;
		Module['calledRun'] = true;
		if (ABORT) return;
		initRuntime();
		if (Module['onRuntimeInitialized']) Module['onRuntimeInitialized']();
		postRun();
	}
	if (Module['setStatus']) {
		Module['setStatus']('Running...');
		setTimeout(function () {
			setTimeout(function () {
				Module['setStatus']('');
			}, 1);
			doRun();
		}, 1);
	} else {
		doRun();
	}
}
if (Module['preInit']) {
	if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
	while (Module['preInit'].length > 0) {
		Module['preInit'].pop()();
	}
}
run();
