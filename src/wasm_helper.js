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

	trimFunctionByName(funcName) {
		let ref = this.module.getFunction(funcName);
		if (ref > 0) {
			let info = binaryen.getFunctionInfo(ref)
			this.module.removeFunction(funcName)

			if (info.results != binaryen.none) {
				this.module.addFunction(funcName, info.params, info.results, [],
					this.createReturn(info.results)
				);
			} else {
				this.module.addFunction(funcName, info.params, info.results, [],
					this.module.return()
				);
			}

		} else {
			console.warn(`no such func ${funcName}`);
		}
	}

	createReturn(type) {
		return this.module.return(
			this.createDefaultConst(type)
		)
	}

	createDefaultConst(type) {
		switch (type) {
			case binaryen.i32: {
				return this.module.i32.const(0);
			}
			case binaryen.f32: {
				return this.module.f32.const(0);
			}
			case binaryen.i64: {
				return this.module.i64.const(0);
			}
			case binaryen.f64: {
				return this.module.f64.const(0);
			}
		}

		throw new Error(`createDefaultConst is unsupported for ${type}`);
	}

}