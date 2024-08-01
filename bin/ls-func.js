#!/usr/bin/env node
import getopt from 'posix-getopt';
import {WasmLoader} from "../src/loader.js"
import {WasmHelper} from "../src/wasm_helper.js"

var parser, option;
parser = new getopt.BasicParser('', process.argv);

while ((option = parser.getopt()) !== undefined) {
  switch (option.option) {}
}

let optind = parser.optind();
if (optind < process.argv.length) {
  let input = process.argv[optind];
  let wasmModule = await WasmLoader.loadFromFile(input)
  let wasmHelper = new WasmHelper(wasmModule);
  let allFunctionNames = wasmHelper.getAllFunctionNames();

  console.log("allFunctionNames", allFunctionNames)
}