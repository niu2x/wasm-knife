# WASM Knife

## wasm-trim-func
`wasm-trim-func` is used to trim func from wasm.

### Usage
```
wasm-trim-func [OPTION]... [FILE]
```

### Options
- -c, config file, one function name per line, start with '- '
- -g, output wasm with debug info
- -f, function name[s], separated by ','
- -h, show help
- -n, dry run
- -o, output file
- -t, output WebAssembly Text

### Example
```
wasm-trim-func -f main -t wasm/w2.wasm
```


