DBG_JS=~/code/mozjs-60.0.2pre2/js/src/build_DBG.OBJ/dist/bin/js
OPT_JS=~/code/mozjs-60.0.2pre2/js/src/build_OPT.OBJ/dist/bin/js
D8=~/code/v8/v8/out.gn/x64.optdebug/d8
CARGO=cargo

SMFLAGS= --ion-offthread-compile=off --ion-sink=on --ion-instruction-reordering=on -D
D8FLAGS= --single-threaded --wasm-opt --wasm-tier-mask-for-testing 65535 --print-opt-code --print-wasm-code --code-comments --no-debug-code --trace-turbo

SMDUMPFILE=smdump.txt
D8DUMPFILE=d8dump.txt

$(SMDUMPFILE) : index.js rust_lsr.js rust_lsr_bg.wasm
	$(DBG_JS) $(SMFLAGS) $< > $@

$(D8DUMPFILE) : index.js rust_lsr.js rust_lsr_bg.wasm
	$(D8) $(D8FLAGS) $< > $@

rust_lsr.js rust_lsr_bg.wasm : src/lib.rs Cargo.toml Cargo.lock
	$(CARGO) +nightly build --target=wasm32-unknown-unknown
	wasm-bindgen target/wasm32-unknown-unknown/debug/rust_lsr.wasm --out-dir .

.PHONY: wat
wat : rust_lsr_bg.wat

rust_lsr_bg.wat : rust_lsr_bg.wasm
	wasm2wat $< -o $@

.PHONY: clean
clean :
	-rm $(SMDUMPFILE) $(D8DUMPFILE)
	-rm *.wat *.wasm
	-rm rust_lsr.js
	$(CARGO) +nightly clean
