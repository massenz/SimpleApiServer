// Copyright (c) 2017 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 10/17/17.


#pragma once

#include <microhttpd.h>
#include <functional>
#include <map>
#include <utility>

#include <glog/logging.h>

namespace api {
namespace rest {

class HttpCannotStartError : public std::exception {
};

/** API Version */
extern const char *const kApiVersionPrefix;


using QueryArgs = std::map<std::string, std::string>;

// TODO: headers should actually be a multimap<string, string>
using Headers = std::map<std::string, std::string>;

class BaseRequestResponse {
 protected:
  Headers headers_;
  std::string body_;

  BaseRequestResponse() = default;

  explicit BaseRequestResponse(std::string body) : body_(std::move(body)), headers_{} {}

 public:
  BaseRequestResponse(const BaseRequestResponse &) = delete;

  const std::string body() const { return body_; }

  void set_body(const std::string &body) { body_ = body; }

  const Headers& headers() const { return headers_; }

  void AddHeader(const std::string &header, const std::string &value) {
    headers_.insert(std::make_pair(header, value));
  }

  std::string GetHeader(const std::string &header) {
    try {
      return headers_.at(header);
    } catch (std::out_of_range& ex) {
      return "";
    }
  }
};

class Request : public BaseRequestResponse {

  QueryArgs query_args_;

 public:
  explicit Request(const std::string &body = "") :
      BaseRequestResponse{body}, query_args_{} { }

  const QueryArgs& query_args() const { return query_args_; }

  void AddQueryArg(const std::string& query, const std::string& arg) {
    query_args_[query] = arg;
  }

  std::string GetQueryArg(const std::string& query) const {
    if (query_args_.count(query) > 0) {
      return query_args_.at(query);
    }
    return "";
  }
};

class Response : public BaseRequestResponse {
  unsigned int status_code_;
  std::string reason_;

 public:
  Response(unsigned int status, const std::string &reason, const std::string &body = "") :
      BaseRequestResponse{body},
      status_code_{status},
      reason_{reason} {}

  Response(const Response &other) : BaseRequestResponse{other.body_},
                                    status_code_{other.status_code_},
                                    reason_{other.reason_}
  {
    for (auto &header : other.headers()) {
      headers_[header.first] = header.second;
    }
  }

  unsigned int status_code() const { return status_code_; }

  std::string reason() const { return reason_; }

  static Response ok() { return Response(200, "OK"); }

  static Response created() { return Response(201, "CREATED"); }

  static Response bad_request(const std::string &err_msg = "") {
    return Response(400, "BAD_REQUEST", err_msg);
  }

  static Response not_found(const std::string &err_msg = "") {
    return Response(404, "NOT_FOUND", err_msg);
  }
};

using Handler = std::function<Response(const Request &)>;

using ResourceHandlersMap = std::map<std::string, Handler>;

/**
 * Implements a minimalist REST API for a `GossipFailureDetector` server.
 * Uses GNU `libmicrohttpd` as the underlying HTTP daemon.
 *
 * <p>In the current implementation it is uniquely associated with a reference to the
 * detector server, thus <strong>it must not outlive</strong> the referenced server's
 * life.
 *
 * @see https://www.gnu.org/software/libmicrohttpd/
 */
class ApiServer {
  unsigned int port_;
  struct MHD_Daemon *httpd_;
  std::map<std::string, ResourceHandlersMap> handlers_;

  static int ConnectCallback(void *cls, struct MHD_Connection *connection,
                             const char *url,
                             const char *method,
                             const char *version,
                             const char *upload_data,
                             size_t *upload_data_size,
                             void **con_cls);

  static int HeadersQueryargsCallback(void *request, enum MHD_ValueKind kind,
                                      const char *key, const char *value);

  void AddMethodHandler(const std::string &method, const std::string &resource,
                        const Handler &handler);

  bool HasMethod(const std::string &method) const {
    return handlers_.find(method) != handlers_.end();
  }

 public:
  explicit ApiServer(unsigned int port) : port_(port) {}

  void Start() {
    LOG(INFO) << "Starting HTTP API Server on port " << std::to_string(port_);
    httpd_ = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                              port_,
                              nullptr,    // Allow all clients to connect
                              nullptr,
                              ApiServer::ConnectCallback,
                              (void *) this,       // The GFD as the extra arguments.
                              MHD_OPTION_END);  // No extra options to daemon either.

    if (httpd_ == nullptr) {
      LOG(ERROR) << "HTTPD Daemon could not be started";
      throw HttpCannotStartError();
    }
    LOG(INFO) << "API available at http://localhost:" << std::to_string(port_)
              << kApiVersionPrefix << "/*";
  }

  virtual ~ApiServer() {
    LOG(INFO) << "Stopping HTTP API Server";
    if (httpd_ != nullptr)
      MHD_stop_daemon(httpd_);
  }

  void AddGet(const std::string &resource, const Handler &handler) {
    AddMethodHandler("GET", resource, handler);
  }

  void AddPost(const std::string &resource, const Handler &handler) {
    AddMethodHandler("POST", resource, handler);
  }

  void AddPut(const std::string &resource, const Handler &handler) {
    AddMethodHandler("PUT", resource, handler);
  }

  void AddDelete(const std::string &resource, const Handler &handler) {
    AddMethodHandler("DELETE", resource, handler);
  }

  std::ostream &ListAllHandlers(std::ostream &out) const {
    out << "====\nAll handlers for server on port: " << port_ << "\n====\n";
    for (auto methodHandlers : handlers_) {
      out << "Method: " << methodHandlers.first << std::endl;
      for (auto handler : methodHandlers.second) {
        out << "\t" << handler.first << std::endl;
      }
      out << "--\n";
    }
    out << "=====\n";
    return out;
  }

  static int sendResponse(MHD_Connection *connection, const Response &response);
  static int ResourceNotFound(MHD_Connection *connection, const std::string &resource);
};

} // namespace rest
} // namespace swim
