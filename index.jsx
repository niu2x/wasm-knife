import React from "react";
import {createRoot} from "react-dom/client";


let root = createRoot(document.getElementById("main"))
root.render(<App/>)

function App() {
	let [code, setCode] = React.useState(0)

	fetch("http://127.0.0.1:5173/wasm/1/1.wasm")

		.then(x => x.arrayBuffer())
		.then(buf => WebAssembly.compile(buf))
		.then((m) => {
			globalThis.externalMemory = new WebAssembly.Memory({initial: 128});
			return WebAssembly.instantiate(m, {
				env: {
					memory: globalThis.externalMemory
				}
			})
		}).then((wasmModule)=>{
			let code = wasmModule.exports.hello()
			console.log(wasmModule.exports)
			console.log(wasmModule)
			setCode(code)
			globalThis.wasmModule = wasmModule;
		})

	return <p>{code}</p>
}

