import binaryen from "binaryen"

export class WasmHelper {

	constructor(wasmModule) {
		this.module = wasmModule;
	}

	getAllFunctionNames() {
		let namesList = []
		let num = this.module.getNumFunctions()
		for (let i = 0; i < num; i++) {
			let funcRef = this.module.getFunctionByIndex(i);
			let name = binaryen.Function.getName(funcRef)
			namesList.push(name)
		}
		return namesList;
	}

	getAllExports() {
		let exportsList = []
		let num = this.module.getNumExports()
		for (let i = 0; i < num; i++) {
			let ref = this.module.getExportByIndex(i);
			let name = binaryen.getExportInfo(ref)
			exportsList.push(name)
		}
		return exportsList;
	}

	getAllExportFunctions() {
		return this.getAllExports().filter(x => x.kind == binaryen.ExternalFunction)
	}

	getAllExportTables() {
		return this.getAllExports().filter(x => x.kind == binaryen.ExternalTable)
	}

	getAllExportMemorys() {
		return this.getAllExports().filter(x => x.kind == binaryen.ExternalMemory)
	}

	getAllExportGlobals() {
		return this.getAllExports().filter(x => x.kind == binaryen.ExternalGlobal)
	}

	getAllExportTags() {
		return this.getAllExports().filter(x => x.kind == binaryen.ExternalTag)
	}

}