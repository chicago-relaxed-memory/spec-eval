#![feature(use_extern_macros, wasm_import_module)]
#![allow(non_upper_case_globals)]

extern crate wasm_bindgen;
use wasm_bindgen::prelude::*;

static SECRET0 : i32 = 0;
static SECRET1 : i32 = 1;
static ALWAYS_FALSE : bool = false;

static mut x : i32 = 0;
static mut y : i32 = 0;
static mut z : i32 = 0;

#[wasm_bindgen]
pub fn lsrattack0() -> i32 {
    unsafe {
        x = 1;
        if ALWAYS_FALSE { x = SECRET0; }
        if y > 0 { z = 0x12345; } else { z = 0x23456; }
        x + y + z
    }
}

#[wasm_bindgen]
pub fn lsrattack1() -> i32 {
    unsafe {
        x = 1;
        if ALWAYS_FALSE { x = SECRET1; }
        if y > 0 { z = 0x12345; } else { z = 0x23456; }
        x + y + z
    }
}

#[wasm_bindgen]
pub fn lsrattack_310_0() -> i32 {
    unsafe {
        if y == 0 { x = 1; }
        else if ALWAYS_FALSE { x = SECRET0; }
        else { x = 1; z = 1; }
        z
    }
}

#[wasm_bindgen]
pub fn lsrattack_310_1() -> i32 {
    unsafe {
        if y == 0 { x = 1; }
        else if ALWAYS_FALSE { x = SECRET1; }
        else { x = 1; z = 1; }
        z
    }
}

#[wasm_bindgen]
pub fn ifhoist() {
    unsafe {
        if x > 0 { y = 0x54321; } else { y = 0x54321; }
    }
}
