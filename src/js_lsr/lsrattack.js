const SECRET0 = 0;
const SECRET1 = 1;
var x = 0;
var y = 0;
var z = 0;
var obj = {};
obj.x = 0;
obj.y = 0;
obj.z = 0;
var sab = new ArrayBuffer(16);
var arr = new Int32Array(sab);

function lsrattackA0() {
  x = 1;
  if(false) x = SECRET0;
  if(y > 0) { z = 0x12345; } else { z = 0x23456; }
  return x + y + z
}

function lsrattackA1() {
  x = 1;
  if(false) x = SECRET1;
  if(y > 0) { z = 0x12345; } else { z = 0x23456; }
  return x + y + z
}

function lsrattackB0() {
  obj.x = 1;
  if(false) obj.x = SECRET0;
  if(obj.y > 0) { obj.z = 0x12345; } else { obj.z = 0x23456; }
  return obj.x + obj.y + obj.z
}

function lsrattackB1() {
  obj.x = 1;
  if(false) obj.x = SECRET1;
  if(obj.y > 0) { obj.z = 0x12345; } else { obj.z = 0x23456; }
  return obj.x + obj.y + obj.z
}

function lsrattackC0() {
  arr[1] = 1;
  if(false) arr[1] = SECRET0;
  if(arr[2] > 0) { arr[3] = 0x12345; } else { arr[3] = 0x23456; }
  return arr[1] + arr[2] + arr[3]
}

function lsrattackC1() {
  arr[1] = 1;
  if(false) arr[1] = SECRET1;
  if(arr[2] > 0) { arr[3] = 0x12345; } else { arr[3] = 0x23456; }
  return arr[1] + arr[2] + arr[3]
}

function callVeryRecognizableFunctions() {
  var a = 0;
  for(i = 0; i < 1000000; i++) {
    a += lsrattackA0();
    y = x;
    obj.y = obj.x;
    arr[2] = arr[1];

    a += lsrattackA1();
    y = x;
    obj.y = obj.x;
    arr[2] = arr[1];

    a += lsrattackB0();
    y = x;
    obj.y = obj.x;
    arr[2] = arr[1];

    a += lsrattackB1();
    y = x;
    obj.y = obj.x;
    arr[2] = arr[1];

    a += lsrattackC0();
    y = x;
    obj.y = obj.x;
    arr[2] = arr[1];

    a += lsrattackC1();
    y = x;
    obj.y = obj.x;
    arr[2] = arr[1];
  }
  print(a)
}

callVeryRecognizableFunctions()
