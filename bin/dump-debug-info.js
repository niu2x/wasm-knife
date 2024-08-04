#!/usr/bin/env node

import binaryen from "binaryen"
import getopt from 'posix-getopt';
import {
  promises as fs
} from "fs";

import {
  WasmLoader
} from "../src/loader.js"

import {
  WasmHelper
} from "../src/wasm_helper.js"

let usage_text = `\
Usage: wasm-trim-func [OPTION]... [FILE]
trim function from wasm

  -o,                        output file

`

function usage() {
  console.log(usage_text)
  process.exit();
}

let getValidList = (x) => {
  return x.map(e => e.trim()).filter(e => e.length > 0)
}


let parseCmdArguments = () => {

  var parser, option;
  parser = new getopt.BasicParser('o:', process.argv);
  let config = {}

  while ((option = parser.getopt()) !== undefined) {
    switch (option.option) {
      case 'o': {
        config.output = option.optarg;
        break;
      }
    }
  }
  return [config, parser.optind()];
}

function checkCmdArguments(config, optind) {
  if (optind >= process.argv.length) {
    usage();
  }
}


async function main() {
  let [config, optind] = parseCmdArguments()

  checkCmdArguments(config, optind)

  let input = process.argv[optind];
  let wasmModule = await WasmLoader.loadFromFile(input)
  let wasmHelper = new WasmHelper(wasmModule);
  let funcNames = wasmHelper.getAllFunctionNames();

  let info = {
    funcNames: funcNames
  }

  info = JSON.stringify(info, null, 2)

  if (config.output) {
    await fs.writeFile(config.output, info)
  } else {
    console.log(info)
  }

}

main();