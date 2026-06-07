case_request
============================================================

Flag
------------------------------------------------------------

The flag is in `wasm/src/flag.txt`, and is

```
tribectf{Baby, you can drive my car.  Yes, I'm gonna be a (Gleam programming) star.}
```

Overview
------------------------------------------------------------
The teams get a bundle for a website. The goal is to forge a
token which says their parking appeal has been approved.

The token is a JSON string and a CRC32, both encrypted with ChaCha20.
```gleam
pub fn create_token(username, key_handle) {
let payload =
  json.object([
    #("approved", json.bool(False)),
    #("username", json.string(username)),
  ])
  |> json.to_string()
  |> bit_array.from_string()
let crc = crc32(payload)
let nonce = random_bytes(12)
let enc = chacha20_encrypt(<<crc:bits, payload:bits>>, nonce, key_handle)
<<nonce:bits, enc:bits>>
}
```

ChaCha20 just xors the keystream with the plaintext, and CRC32 is an affine
transform -- `crc32(a ^ b) = crc32(a) ^ crc32(b) ^ crc32(zeros)`.

Find the diff between '"approved":false,' and '"approved": true,' at the
appropriate spot (by padding with zeros), then find the change in crc by taking
crc32(diff) ^ crc32(zeros). Xor both the diff and the crc into the ciphertext,
then submit to get the flag. 

NOTE: you can’t just call the `get_flag` function, you first need to decrypt a
correct token. If you don’t, it will return a slightly different error message
from the normal one and not give you the flag.


Setting Up the Challenge
------------------------------------------------------------
To build the challenge, run:

```
./build.py
cp DESCRIPTION.md dist/
cp -R dist case_request
zip -r case_request.zip case_request.zip
```

Solvving the Challenge
------------------------------------------------------------
To solve the challenge, run:

```
./solve_crc32.py
```

When prompted, enter the token; it will output the new token.
