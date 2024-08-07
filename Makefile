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

build-wasm-release:
	cmake -S. -Bbuild-wasm-release -DCMAKE_BUILD_TYPE=Release  -DCMAKE_TOOLCHAIN_FILE=~/project/emsdk/./upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
	cmake --build build-wasm-release
	cp build-wasm-release/trim-func.js build-wasm-release/trim-func.cjs
	cp build-wasm-release/trim-func.js src/trim-func.cjs
	cp build-wasm-release/trim-func.wasm src/trim-func.wasm


build-release:
	cmake -S. -Bbuild-release -DCMAKE_BUILD_TYPE=Release
	cmake --build build-release -j 7

.PHONY: sync build-wasm-release build-release
