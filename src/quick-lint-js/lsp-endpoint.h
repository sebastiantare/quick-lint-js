// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#ifndef QUICK_LINT_JS_LSP_ENDPOINT_H
#define QUICK_LINT_JS_LSP_ENDPOINT_H

#if defined(__EMSCRIPTEN__)
// No LSP on the web.
#else

#include <cstddef>
#include <quick-lint-js/assert.h>
#include <quick-lint-js/byte-buffer.h>
#include <quick-lint-js/char8.h>
#include <quick-lint-js/have.h>
#include <quick-lint-js/json.h>
#include <quick-lint-js/lsp-message-parser.h>
#include <quick-lint-js/simdjson.h>
#include <quick-lint-js/unreachable.h>
#include <quick-lint-js/warning.h>
#include <simdjson.h>
#include <tuple>
#include <utility>
#include <vector>

namespace quick_lint_js {
class lsp_endpoint_remote {
 public:
  virtual ~lsp_endpoint_remote();

  virtual void send_message(byte_buffer&& message) = 0;
};

class lsp_endpoint_handler {
 public:
  virtual ~lsp_endpoint_handler();

  virtual void handle_request(::simdjson::ondemand::object& request,
                              std::string_view method, string8_view id_json,
                              byte_buffer& reply) = 0;
  virtual void handle_notification(::simdjson::ondemand::object& request,
                                   std::string_view method) = 0;
  virtual void take_pending_notification_jsons(
      void (*write_notification_json)(byte_buffer&&, lsp_endpoint_remote*),
      lsp_endpoint_remote* remote) = 0;

  void flush_pending_notifications(lsp_endpoint_remote& remote) {
    this->take_pending_notification_jsons(
        [](byte_buffer&& notification_json, lsp_endpoint_remote* r) {
          if (notification_json.empty()) {
            // TODO(strager): Fix our tests so they don't make empty
            // byte_buffer-s.
            return;
          }
          r->send_message(std::move(notification_json));
        },
        &remote);
  }
};

// An lsp_endpoint parses Language Server Protocol messages, dispatches them to
// lsp_endpoint_handler, and sends responses to lsp_endpoint_remote.
//
// lsp_endpoint implements JSON-RPC.
class lsp_endpoint : private lsp_message_parser<lsp_endpoint> {
 private:
  using message_parser = lsp_message_parser<lsp_endpoint>;

 public:
  explicit lsp_endpoint(lsp_endpoint_handler* handler,
                        lsp_endpoint_remote* remote);

  using message_parser::append;

  void message_parsed(string8_view message);

 private:
  void handle_message(::simdjson::ondemand::object& request,
                      byte_buffer& response_json,
                      bool add_comma_before_response);

  void write_json_parse_error_response(byte_buffer& response_json);

  static void write_invalid_request_error_response(byte_buffer& response_json);

  lsp_endpoint_remote* remote_;
  lsp_endpoint_handler* handler_;
  ::simdjson::ondemand::parser json_parser_;

  friend message_parser;
};
}

#endif

#endif

// quick-lint-js finds bugs in JavaScript programs.
// Copyright (C) 2020  Matthew "strager" Glazar
//
// This file is part of quick-lint-js.
//
// quick-lint-js is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// quick-lint-js is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with quick-lint-js.  If not, see <https://www.gnu.org/licenses/>.
