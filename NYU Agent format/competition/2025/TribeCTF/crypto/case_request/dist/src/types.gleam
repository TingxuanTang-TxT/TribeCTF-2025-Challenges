import gleam/dict.{type Dict}
import gleam/option.{type Option}

pub type View {
  Login
  CaseRequest(username: String)
  Case(id: String, token: BitArray)
  CaseStatus(verify_msg: String)
}

pub type Model {
  Model(
    view: View,
    cases: Dict(String, String),
    username: Option(String),
    key: BitArray,
  )
}

pub type Msg {
  UserChangedName(Dict(String, String))
  UserAddedRequest(description: String, pay: Int)
  Nothing
  UserClickedCase(id: String)
  UserClickedBackFromCase
  UserCheckedCaseStatus
  UserCheckedToken(token: String)
}
