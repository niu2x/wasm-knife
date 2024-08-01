fetch("http://127.0.0.1:5173/public/1.wasm")
	.then(x => x.arrayBuffer())
	.then(buf => WebAssembly.compile(buf))
	.then((m) => {
		return WebAssembly.instantiate(m, {})
	}).then((wasmModule)=>{
		console.log(wasmModule)
		wasmModule.exports.hello()
	})