import gleam/dict
import gleam/int
import gleam/list
import gleam/option
import gleam/result
import gleam/string
import lustre
import lustre/attribute
import lustre/effect.{type Effect}
import lustre/element.{type Element}
import lustre/element/html
import lustre/event
import token
import types.{
  type Model, type Msg, Case, CaseRequest, Login, Model, Nothing,
  UserAddedRequest, UserChangedName, UserCheckedCaseStatus,
  UserClickedBackFromCase, UserClickedCase,
}
import utils.{bit_array_to_hex, filter_empty, first_upper, no_effect}

@external(javascript, "./ffi/crypto.mjs", "get_uuid")
fn get_uuid() -> String

@external(javascript, "./ffi/wasm.mjs", "get_key_handle")
fn get_key_handle() -> BitArray

@external(javascript, "./ffi/wasm.mjs", "get_flag")
fn get_flag() -> String

fn init(_args) -> #(Model, Effect(msg)) {
  #(Model(Login, dict.new(), option.None, get_key_handle()), effect.none())
}

fn update(model: Model, msg: Msg) -> #(Model, Effect(Msg)) {
  case msg {
    UserChangedName(values) -> {
      let username =
        values
        |> dict.get("username")
        |> option.from_result
        |> filter_empty

      case username {
        option.Some(username) ->
          Model(
            ..model,
            view: CaseRequest(username),
            username: option.Some(username),
          )
          |> no_effect()
        _ -> model |> no_effect()
      }
    }
    Nothing -> model |> no_effect()

    UserAddedRequest(desc, pay) -> {
      let id = get_uuid()
      let cases = model.cases
      let assert CaseRequest(username:) = model.view
      let request =
        "User "
        <> username
        <> " would like the W&M to review the following case: "
        <> desc
        <> ". They want to $"
        <> int.to_string(pay)
        <> " refunded to their student account."
      let cases = dict.insert(cases, id, request)
      let tok = token.create_token(username, model.key)
      Model(..model, cases:, view: Case(id:, token: tok)) |> no_effect()
    }
    UserClickedBackFromCase -> {
      Model(
        ..model,
        view: CaseRequest(model.username |> option.unwrap("No username")),
      )
      |> no_effect()
    }
    UserClickedCase(id:) -> {
      let token = <<>>
      Model(..model, view: Case(id:, token:))
      |> no_effect()
    }
    UserCheckedCaseStatus -> {
      Model(..model, view: types.CaseStatus("")) |> no_effect()
    }
    types.UserCheckedToken(token:) -> {
      let token = token |> utils.bit_array_from_hex
      let message = case token.verify_token(model.key, token) {
        Ok(True) -> get_flag()
        Ok(False) -> "Your appeal was not granted"
        Error(e) -> "ERROR: " <> string.inspect(e)
      }
      Model(..model, view: types.CaseStatus(message)) |> no_effect()
    }
  }
}

fn form(names: List(String), on_submit) {
  html.form(
    [event.on_submit(on_submit), attribute.class("centered-v")],
    names
      |> list.map(fn(name) {
        html.span([], [
          html.label([attribute.for(name)], [
            html.text({ name <> ":" } |> first_upper()),
          ]),
          html.input([attribute.name(name), attribute.type_("text")]),
          html.br([]),
        ])
      })
      |> list.append([html.button([], [html.text("Submit")])]),
  )
}

fn mack() {
  html.img([attribute.class("mackmage"), attribute.src("/priv/static/car.png")])
}

fn login() {
  html.div([attribute.class("centered-v")], [
    h1(
      "Welcome to the W&M Parking Ticket Appeals Office",
    ),
    mack(),
    p("Please enter your name to sign in:"),
    form(["username"], fn(names) { UserChangedName(dict.from_list(names)) }),
  ])
}

fn h1(text) {
  html.h1([attribute.styles([#("text-align", "center"), #("margin", "1rem")])], [
    html.text(text),
  ])
}

fn p(text) {
  html.p([], [html.text(text)])
}

fn view(model: Model) -> Element(Msg) {
  case model.view {
    Login -> login()
    Case(id, token:) -> {
      let this_case = dict.get(model.cases, id) |> result.unwrap("None")
      let other_cases =
        dict.keys(model.cases)
        |> list.filter(fn(other_id) { other_id != id })
        |> list.map(fn(elem) {
          [
            html.br([]),
            html.a([event.on_click(UserClickedCase(elem))], [html.text(elem)]),
          ]
        })
        |> list.flatten()
      html.div([attribute.class("centered-v")], [
        p("Case ID: " <> id),
        html.br([]),
        p(this_case),
        p("Your token is " <> token |> bit_array_to_hex),
        p("Please copy your token before leaving this page"),
        html.div([attribute.class("centered-h")], [
          html.button([event.on_click(UserClickedBackFromCase)], [
            html.text("Back to appeals request"),
          ]),
          html.button([event.on_click(UserCheckedCaseStatus)], [
            html.text("Check your case's status"),
          ]),
        ]),
        html.br([]),
        ..other_cases
      ])
    }
    CaseRequest(username:) ->
      html.div([attribute.class("centered-v")], [
        p(
          "Welcome, "
          <> username
          <> "! Please enter the description ID on your ticket and its amount: ",
        ),
        form(["description", "amount"], fn(names) {
          let d = dict.from_list(names)
          let description =
            d |> dict.get("description") |> option.from_result |> filter_empty()
          let amount =
            d
            |> dict.get("amount")
            |> option.from_result
            |> option.then(fn(i) { int.parse(i) |> option.from_result })
          case description, amount {
            option.Some(description), option.Some(pay) ->
              UserAddedRequest(description:, pay:)
            _, _ -> Nothing
          }
        }),
      ])
    types.CaseStatus(verify_msg) -> {
      case verify_msg {
        "" ->
          html.div([], [
            p("Enter your token:"),
            form(["token"], fn(pairs) {
              let assert [#(_, token)] = pairs
              types.UserCheckedToken(token:)
            }),
          ])
        _ -> {
          p(verify_msg)
        }
      }
    }
  }
}

pub fn main() {
  let app = lustre.application(init, update, view)
  let assert Ok(_) = lustre.start(app, "#app", Nil)

  Nil
}
