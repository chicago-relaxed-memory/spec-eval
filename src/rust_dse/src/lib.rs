#![feature(use_extern_macros)]

extern crate wasm_bindgen;
use wasm_bindgen::prelude::*;

static SECRET : i32 = 1;
static ALWAYS_FALSE : bool = false;

#[wasm_bindgen]
pub fn aVeryRecognizableFunctionName() -> i32 {
    let mut x = 123456;
    if ALWAYS_FALSE {
        if SECRET > 0 { x = 654321; }
    } else {
        x = 654321;
    }
    return x;
}

#[wasm_bindgen]
pub fn anotherRecognizableFunctionName() -> i32 {
    let mut x = 23456;
    x = 65432;
    return x;
}
