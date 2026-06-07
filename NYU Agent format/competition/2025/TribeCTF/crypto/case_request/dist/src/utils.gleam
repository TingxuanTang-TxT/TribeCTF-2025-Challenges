import gleam/bool
import gleam/int
import gleam/list
import gleam/option
import gleam/string
import lustre/effect.{type Effect}
import types.{type Msg}

pub fn filter(opt, func) {
  opt
  |> option.then(fn(value) {
    case func(value) {
      True -> option.Some(value)
      False -> option.None
    }
  })
}

pub fn filter_empty(opt) {
  filter(opt, fn(a) { a |> string.is_empty |> bool.negate })
}

pub fn no_effect(input: a) -> #(a, Effect(Msg)) {
  #(input, effect.none())
}

pub fn first_upper(s) {
  let assert Ok(#(first, rest)) = string.pop_grapheme(s)
  string.uppercase(first) <> rest
}

pub fn bit_array_to_hex(arr) -> String {
  do_bit_array_to_hex(arr, []) |> list.reverse() |> string.join("")
}

fn do_bit_array_to_hex(arr, parts) {
  case arr {
    <<first:int, rest:bits>> -> {
      let as_hex = first |> int.to_base16 |> string.pad_start(2, "0")
      do_bit_array_to_hex(rest, [as_hex, ..parts])
    }
    <<>> -> parts
    _ -> panic as "bit array is not byte-aligned"
  }
}

pub fn bit_array_from_hex(str) -> BitArray {
  do_bit_array_from_hex(str |> string.to_graphemes, <<>>)
}

fn do_bit_array_from_hex(str, arr) {
  case str {
    [] -> arr
    [first, second, ..rest] -> {
      let assert Ok(hex) = int.base_parse(first <> second, 16)
      do_bit_array_from_hex(rest, <<arr:bits, hex:int-size(8)>>)
    }
    _ -> panic as "string is not 2-byte aligned"
  }
}
