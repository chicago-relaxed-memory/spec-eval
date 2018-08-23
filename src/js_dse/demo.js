function aVeryRecognizableFunctionName() {
  var x = 13;
  x = 15;
  x = 17;
  var y = {};
  y.foo = 21;
  y.foo = 23;
  y.foo = 25;
  var sab = new ArrayBuffer(16);
  var arr = new Int32Array(sab);
  arr[1] = 5;
  arr[1] = 7;
  arr[1] = 9;
  return arr[1] + x + y.foo
}

function callVeryRecognizableFunctions() {
  var a = 0;
  for(i = 0; i < 100000; i++) {
    a = aVeryRecognizableFunctionName();
  }
  print(a)
}

callVeryRecognizableFunctions()
