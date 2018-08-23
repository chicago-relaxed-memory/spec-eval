let file = read("rust_dse_bg.wasm", "binary");
WebAssembly.instantiate(file, {}).then(wasm => {
  var a = 0;
  for(var i = 0; i < 100000; i++) {
    a += wasm.instance.exports.anotherRecognizableFunctionName();
  }
  print(a);
}).catch(e => print(e));
print("Hello");
