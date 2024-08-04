#!/usr/bin/env node

import {promises as fs} from "fs";
import getopt from 'posix-getopt';

let usage_text = `\
Usage: wasm-merge-profile [OPTION]... [FILE]
merge command lines that is startWith "- "

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

  let dict = {

  }

  let fileCount = 0;

  for(let i = optind; i < process.argv.length; i ++) {
  	let lines = await fs.readFile(process.argv[i], {encoding: "utf-8"})
  	lines = lines.split('\n')
  	lines = getValidList(lines)
  	lines = lines.filter(x => x.startsWith('- '))
  	lines = getValidList(lines)
  	lines = lines.sort()

  	for(let l of lines) {
  		dict[l] = dict[l] || 0;
  		dict[l] += 1;
  	}
  	fileCount += 1;
  }

  let common = []

  for(let k in dict) {
  	let count = dict[k]
  	if(count == fileCount) {
  		common.push(k)
  	}
  }

  if(config.output) {
  	fs.writeFile(config.output, common.join('\n'))
  }
  else {
	for(let k of common) console.log(k)  	
  }
}

main();