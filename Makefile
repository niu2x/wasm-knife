%.instrument-wasm.br: %.instrument.wasm
	brotli -o $@ $<

%.debug-info.json: %.wasm
	wasm-dump-debug-info -o $@ $< 

%.instrument.wasm: %.wasm
	wasm-split --instrument $< -o $@

%.wasm: %.wasm.br
	brotli -f -d $<

