#!/usr/bin/env node

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

  -c,                        config file, one function name per line
  -f,                        function name[s], separated by ','
  -h,                        show help
  -n,                        dry run
  -o,                        output file
  -t,                        output WebAssembly Text

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
  parser = new getopt.BasicParser('o:f:c:tnh', process.argv);
  let config = {
    text: false,
    dryRun: false,
  }

  while ((option = parser.getopt()) !== undefined) {
    switch (option.option) {
      case 'o': {
        config.output = option.optarg;
        break;
      }

      case 'f': {
        config.name = option.optarg;
        break;
      }

      case 'c': {
        config.config = option.optarg;
        break;
      }

      case 'n': {
        config.dryRun = true;
        break;
      }

      case 't': {
        config.text = true;
        break;
      }
      case 'h': {
        usage();
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

  if (!(config.text || config.output)) {
    console.error("binary wasm can't print to console.");
    usage();
  }

  if (!(config.config || config.name)) {
    console.error("tell me what to trim");
    usage();
  }
}

async function getFuncNamesToTrim(config) {
  if (config.config) {
    let lines = await fs.readFile(config.config, {
      encoding: "utf-8"
    })
    lines = getValidList(lines.split('\n'))
    return lines;

  } else if (config.name) {
    return getValidList(config.name.split(','));
  }

  return []
}

async function main() {
  let [config, optind] = parseCmdArguments()

  checkCmdArguments(config, optind)

  let input = process.argv[optind];
  let wasmModule = await WasmLoader.loadFromFile(input)
  let wasmHelper = new WasmHelper(wasmModule);

  let funcNames = await getFuncNamesToTrim(config)

  for (let n of funcNames) {
    wasmHelper.trimFunctionByName(n);
  }

  if (!wasmModule.validate())
    throw new Error("validation error");

  let wasm;
  if (config.text) {
    wasm = wasmModule.emitText();
  } else {
    wasm = wasmModule.emitBinary();
  }

  if (!config.dryRun) {
    if (config.output) {
      await fs.writeFile(config.output, wasm)
    } else {
      if (config.text) {
        console.log(wasm)
      }
    }
  }
}

main();