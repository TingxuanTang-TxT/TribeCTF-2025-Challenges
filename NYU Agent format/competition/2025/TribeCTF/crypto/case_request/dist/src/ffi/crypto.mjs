import { BitArray } from "../gleam.mjs";

function random_bytes(n) {
    let bytes = new Uint8Array(n);
    return new BitArray(window.crypto.getRandomValues(bytes));
}

function get_uuid() {
    return window.crypto.randomUUID();
}

export { random_bytes, get_uuid };