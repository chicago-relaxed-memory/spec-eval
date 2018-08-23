var x = 1;
var y = 1;

function maybehoist() {
  if(x) {
    y = 0x54321;
  } else {
    y = 0x54321;
  }
}

function callMaybeHoist() {
  var a = 0;
  for(i = 0; i < 1000000; i++) {
    maybehoist();
    a += y;
    if(x == 1) x = 0;
    else x = 1;
    y = 1;
  }
  print(a)
}

callMaybeHoist()
