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
  if(y == 0) { x = 1; }
  else if (false) { x = SECRET0; }
  else { x = 1; z = 1; }
  return z;
}

function lsrattackA1() {
  if(y == 0) { x = 1; }
  else if (false) { x = SECRET1; }
  else { x = 1; z = 1; }
  return z;
}

function lsrattackB0() {
  if(obj.y == 0) { obj.x = 1; }
  else if (false) { obj.x = SECRET0; }
  else { obj.x = 1; obj.z = 1; }
  return obj.z;
}

function lsrattackB1() {
  if(obj.y == 0) { obj.x = 1; }
  else if (false) { obj.x = SECRET1; }
  else { obj.x = 1; obj.z = 1; }
  return obj.z;
}

function lsrattackC0() {
  if(arr[2] == 0) { arr[1] = 1; }
  else if (false) { arr[1] = SECRET0; }
  else { arr[1] = 1; arr[3] = 1; }
  return arr[3];
}

function lsrattackC1() {
  if(arr[2] == 0) { arr[1] = 1; }
  else if (false) { arr[1] = SECRET1; }
  else { arr[1] = 1; arr[3] = 1; }
  return arr[3];
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
