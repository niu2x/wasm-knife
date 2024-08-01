import React from "react";
import {createRoot} from "react-dom/client";


let root = createRoot(document.getElementById("main"))
root.render(<App/>)

function App() {
	let [code, setCode] = React.useState(0)

	fetch("http://127.0.0.1:5173/tests/1.wasm")
		.then(x => x.arrayBuffer())
		.then(buf => WebAssembly.compile(buf))
		.then((m) => {
			return WebAssembly.instantiate(m, {})
		}).then((wasmModule)=>{
			let code = wasmModule.exports.hello()
			setCode(code)
		})

	return <p>{code}</p>
}

