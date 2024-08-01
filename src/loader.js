import binaryen from "binaryen"
import {
	promises as fs
} from "fs"

export class WasmLoader {

	static async loadBinary(path) {
		let buffer = await fs.readFile(path)
		return binaryen.readBinary(buffer)
	}

	static async loadText(path) {
		let buffer = await fs.readFile(path)
		return binaryen.parseText(buffer.toString())
	}

	static async loadFromFile(path) {
		if (path.endsWith(".wasm")) {
			return this.loadBinary(path)
		} else if (path.endsWith(".wat")) {
			return this.loadText(path)
		} 
		throw new Error(`cannot load from ${path}`);
	}
};