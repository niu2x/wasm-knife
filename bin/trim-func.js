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

var parser, option;
parser = new getopt.BasicParser('o:f:c:tn', process.argv);
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
  }
}

let optind = parser.optind();
if (optind < process.argv.length) {
  let input = process.argv[optind];
  let wasmModule = await WasmLoader.loadFromFile(input)
  let wasmHelper = new WasmHelper(wasmModule);

  if (config.config) {
    let lines = await fs.readFile(config.config, {
      encoding: "utf-8"
    })
    lines = lines.split('\n').map(x => x.trim()).filter(x => x.length > 0)
    for (let line of lines) {
      wasmHelper.trimFunctionByName(line);
    }
  } else if (config.name) {
    wasmHelper.trimFunctionByName(config.name);
  } else {
    console.error("tell me what to trim");
    usage();
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
      } else {
        console.error("binary wasm can't print to console.");
        usage();
      }
    }
  }
}

function usage() {
  process.exit();
}