#!/usr/bin/env node

import getopt from 'posix-getopt';
import {
  WasmLoader
} from "../src/loader.js"
import {
  WasmHelper
} from "../src/wasm_helper.js"

var parser, option;
parser = new getopt.BasicParser('fet', process.argv);
let config = {}

while ((option = parser.getopt()) !== undefined) {
  switch (option.option) {
    case 'f': {
      config.func = true;
      break;
    }

    case 'e': {
      config.export = true;
      break;
    }

    case 't': {
      config.table = true;
      break;
    }
  }
}

let optind = parser.optind();
if (optind < process.argv.length) {

  let input = process.argv[optind];
  let wasmModule = await WasmLoader.loadFromFile(input)
  let wasmHelper = new WasmHelper(wasmModule);

  if (config.export) {
    if (config.func) {
      console.log("Functions:")
      let allExportNames = wasmHelper.getAllExportFunctions();
      for (let n of allExportNames) {
        console.log(n.name)
      }
    }
    if (config.table) {
      console.log("Tables:")
      let allExportNames = wasmHelper.getAllExportTables();
      for (let n of allExportNames) {
        console.log(n.name)
      }
    }
  } else {
    if (config.func) {
      let allFunctionNames = wasmHelper.getAllFunctionNames();
      for (let n of allFunctionNames) {
        console.log(n)
      }
    }

    if (config.func) {
      let allFunctionNames = wasmHelper.getAllFunctionNames();
      for (let n of allFunctionNames) {
        console.log(n)
      }
    }
  }
}