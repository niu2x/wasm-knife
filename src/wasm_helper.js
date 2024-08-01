import binaryen from "binaryen"

export class WasmHelper {

	constructor(wasmModule) {
		this.module = wasmModule;
	}

	getAllFunctionNames() {
		let namesList = []
		let num = this.module.getNumFunctions()
		for(let i = 0; i < num; i ++) {
			let funcRef = this.module.getFunctionByIndex(i);
			let name = binaryen.Function.getName(funcRef)
			namesList.push(name)
		}
		return namesList;
	} 

}

