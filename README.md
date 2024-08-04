# WASM Knife

## wasm-trim-func

    Usage: wasm-trim-func [OPTION]... [FILE]
    trim function from wasm
      -c,                        config file, one function name per line, start with '- '
                                 eg:
                                   - main
                                   - funcNameA
                                   - funcNameB
                                   - all_those_will_be_trim
      -g,                        output wasm with debug info
      -f,                        function name[s], separated by ','
      -h,                        show help
      -n,                        dry run
      -o,                        output file
      -t,                        output WebAssembly Text

Example:
    wasm-trim-func -f main -t wasm/w2.wasm


