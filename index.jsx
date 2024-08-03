import React from "react";
import {
	createRoot
} from "react-dom/client";


let root = createRoot(document.getElementById("main"))
root.render(React.createElement(App, null))

function App() {
	let [code, setCode] = React.useState(0)

	let HEAP32
	let HEAPU8


	fetch("http://127.0.0.1:5173/wasm/2/build/w2.wasm")

		.then(x => x.arrayBuffer())
		.then(buf => WebAssembly.compile(buf))
		.then((m) => {
			// globalThis.externalMemory = new WebAssembly.Memory({initial: 128});

			return WebAssembly.instantiate(m, {
				env: {
					emscripten_memcpy_big: (dest, src, num) => {
						HEAPU8.copyWithin(dest, src, src + num);
					},
					setTempRet0: (...args) => {
						console.log("setTempRet0", args)
					}
				},
				wasi_snapshot_preview1: {
					fd_write: (fd, iov, iovcnt, pnum) => {
						var num = 0;
						for (var i = 0; i < iovcnt; i++) {
							var ptr = HEAP32[((iov) >> 2)];
							var len = HEAP32[(((iov) + (4)) >> 2)];
							iov += 8;
							for (var j = 0; j < len; j++) {
								// SYSCALLS.printChar(fd, HEAPU8[ptr + j]);
								console.log(String.fromCharCode(HEAPU8[ptr + j]))
							}
							num += len;
						}
						HEAP32[((pnum) >> 2)] = num;
						return 0;
					}
				}
			})
		}).then((wasmModule) => {
			console.log(wasmModule.exports)
			HEAP32 = new Int32Array(wasmModule.exports.memory.buffer);
			HEAPU8 = new Uint8Array(wasmModule.exports.memory.buffer);
			// let code = wasmModule.exports.main()
			// console.log(wasmModule)
			// setCode(code)
			globalThis.wasmModule = wasmModule;
		})

	return React.createElement("p", null, code)
}