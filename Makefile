# compress instrument-wasm
%.instrument-wasm.br: %.instrument.wasm
	brotli -o $@ $<

# instrument wasm
%.instrument.wasm: %.wasm
	wasm-split --instrument $< -o $@

# decompress wasn
%.wasm: %.wasm.br
	brotli -f -d $<


sync:
	git push $(repo) --all
	git push $(repo) --tags

.PHONY: sync
