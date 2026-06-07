import gleam/bit_array
import gleam/dynamic/decode
import gleam/json
import gleam/result

pub type TokenError {
  IncorrectSize
  NonUtf8
  Crc32Mismatch
  JsonError(json.DecodeError)
}

@external(javascript, "./ffi/wasm.mjs", "chacha20_encrypt")
fn chacha20_encrypt(
  data: BitArray,
  nonce: BitArray,
  key_handle: BitArray,
) -> BitArray

@external(javascript, "./ffi/wasm.mjs", "chacha20_decrypt")
fn chacha20_decrypt(
  data: BitArray,
  nonce: BitArray,
  key_handle: BitArray,
) -> BitArray

@external(javascript, "./ffi/wasm.mjs", "crc32")
fn crc32(data: BitArray) -> BitArray

@external(javascript, "./ffi/crypto.mjs", "random_bytes")
fn random_bytes(n: Int) -> BitArray

pub fn decrypt_token(
  key: BitArray,
  token: BitArray,
) -> Result(String, TokenError) {
  use #(nonce, enc) <- result.try(case token {
    <<nonce:bytes-size(12), enc:bits>> -> Ok(#(nonce, enc))
    _ -> Error(IncorrectSize)
  })
  use #(crc, dec) <- result.try(case chacha20_decrypt(enc, nonce, key) {
    <<crc:bytes-size(4), dec:bits>> -> {
      Ok(#(crc, dec))
    }
    _ -> Error(IncorrectSize)
  })
  use dec <- result.try(case crc32(dec) == crc {
    False -> {
      Error(Crc32Mismatch)
    }
    True -> Ok(dec)
  })
  use as_string <- result.map(
    dec |> bit_array.to_string() |> result.map_error(fn(_) { NonUtf8 }),
  )
  as_string
}

pub fn verify_token(key: BitArray, token: BitArray) -> Result(Bool, TokenError) {
  let decoder = {
    use approved <- decode.field("approved", decode.bool)
    use _username <- decode.field("username", decode.string)
    decode.success(approved)
  }

  use dec <- result.try(decrypt_token(key, token))
  use approved <- result.map(
    json.parse(dec, decoder)
    |> result.map_error(fn(err) { JsonError(err) }),
  )
  approved
}

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
