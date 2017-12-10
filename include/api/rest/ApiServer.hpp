// Copyright (c) 2017 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 10/17/17.


#pragma once

#include <microhttpd.h>
#include <functional>
#include <map>
#include <utility>


namespace api {
namespace rest {

class HttpCannotStartError : public std::exception {

};

/** API Version */
extern const char *const kApiVersionPrefix;

class BaseRequestResponse {
protected:
  std::vector<std::pair<std::string, std::string>> headers;
  std::string body_;

  BaseRequestResponse(const BaseRequestResponse&) = delete;
  BaseRequestResponse() = default;
  explicit BaseRequestResponse(std::string body) : body_(std::move(body)) { }

public:
  void AddHeader(const std::string& header, const std::string& value) {
    headers.push_back(std::make_pair(header, value));
  }

  const std::string body() const { return body_; }
};

class Request : public BaseRequestResponse {
public:
  explicit Request(const std::string& request = "") : BaseRequestResponse(request) { }

  const std::string GetHeader(const std::string &header) {
    for(auto pair : headers) {
      if (pair.first == header) {
        return pair.second;
      }
    }
    return "";
  }
};

class Response : public BaseRequestResponse {
  unsigned int status_code_;
  std::string reason_;

public:
  Response(unsigned int status, std::string reason) : BaseRequestResponse{""},
    status_code_{status}, reason_{std::move(reason)} { }

  Response(const Response& other) : BaseRequestResponse{other.body_},
                                    status_code_{other.status_code_},
                                    reason_{other.reason_} { }

  unsigned int status_code() const { return status_code_; }
  std::string reason() const { return reason_; }
  std::string body() const { return body_; }

  static Response ok() { return Response(200, "OK"); }
  static Response created() { return Response(201, "CREATED"); }
  static Response not_found() { return Response(404, "NOT_FOUND"); }
};

using Handler = std::function<Response(const Request&)>;

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

  void AddMethodHandler(const std::string& method, const std::string& resource,
                        const Handler& handler);

  bool HasMethod(const std::string& method) const {
    return handlers_.find(method) != handlers_.end();
  }

public:
  explicit ApiServer(unsigned int port) : port_(port) {
    handlers_["GET"] = ResourceHandlersMap{};
  }

  void Start() {
    LOG(INFO) << "Starting HTTP API Server on port " << std::to_string(port_);
    httpd_ = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
                              port_,
                              nullptr,    // Allow all clients to connect
                              nullptr,
                              ApiServer::ConnectCallback,
                              (void *)this,       // The GFD as the extra arguments.
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

  std::ostream& ListAllHandlers(std::ostream& out) const {
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
};


} // namespace rest
} // namespace swim
