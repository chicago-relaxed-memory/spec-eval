let file = read("rust_lsr_bg.wasm", "binary");
WebAssembly.instantiate(file, {}).then(wasm => {
  var a = 0;
  for(var i = 0; i < 100000; i++) {
    a += wasm.instance.exports.lsrattack0();
    a += wasm.instance.exports.lsrattack1();
    a += wasm.instance.exports.lsrattack_310_0();
    a += wasm.instance.exports.lsrattack_310_1();
    wasm.instance.exports.ifhoist();
  }
  print(a);
}).catch(e => print(e));
print("Hello");
