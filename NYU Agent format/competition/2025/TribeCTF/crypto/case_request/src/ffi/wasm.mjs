import { BitArray } from "../gleam.mjs";

const memory = new WebAssembly.Memory({ initial: 100, maximum: 100 })
const import_object = {
    env: {
        fill_random(base, length) {
            let buffer = new Uint8Array(memory.buffer, base, length);
            window.crypto.getRandomValues(buffer);
        },
        memory: memory
    }
}

const wasm = await WebAssembly.compileStreaming(fetch("wasm.wasm")).then((module) => WebAssembly.instantiate(module, import_object));

function alloc(n) {
    let ptr = wasm.exports.alloc(n);
    let array = new Uint8Array(memory.buffer, ptr, n);
    return array;
}

function copy_bitarr_to_wasm(arr) {
    let buffer = alloc(arr.byteSize);
    buffer.set(arr.rawBuffer);
    return buffer;
}

function dealloc(ptr) {
    wasm.exports.dealloc(ptr);
}

function i32_to_bytes(val) {
    let buffer = new Uint8Array(4);
    for (let i = 0; i < 4; i++) {
        buffer[i] = (val >> (i * 8)) & 0xff;
    }
    return buffer;
}

function bytes_to_i32(val) {
    return new DataView(val.buffer.slice(val.byteOffset, val.byteLength + val.byteOffset)).getInt32(0, true);
}

function crc32(data) {
    let buffer = copy_bitarr_to_wasm(data);
    let result = wasm.exports.crc32(buffer.byteOffset, buffer.length);
    dealloc(buffer.byteOffset);

    return new BitArray(i32_to_bytes(result));
}

function get_key_handle() {
    let result = wasm.exports.get_key_handle();
    return new BitArray(i32_to_bytes(result));
}

function chacha20_encrypt(data, nonce, key) {
    let data_copy = copy_bitarr_to_wasm(data);
    let nonce_copy = copy_bitarr_to_wasm(nonce);
    let key_i32 = bytes_to_i32(key.rawBuffer);
    wasm.exports.chacha20_encrypt(data_copy.byteOffset, data_copy.length, nonce_copy.byteOffset, key_i32);

    let result = new Uint8Array(data_copy.length);
    result.set(data_copy);
    dealloc(data_copy);

    return new BitArray(result);
}

function chacha20_decrypt(data, nonce, key) {
    return chacha20_encrypt(data, nonce, key);
}

function get_flag() {
    let flag = wasm.exports.get_flag();
    let i = 0;
    let potential_flag = new Uint8Array(memory.buffer, flag, 100);
    while (potential_flag[flag + i] != 0) {
        i += 1;
    }
    let flag_buffer = new Uint8Array(memory.buffer, flag, i);
    return new TextDecoder("ascii").decode(flag_buffer);
}

export { crc32, get_key_handle, chacha20_encrypt, chacha20_decrypt, get_flag };
