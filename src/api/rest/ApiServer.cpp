// Copyright (c) 2017 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 10/17/17.


#include <glog/logging.h>
#include <iostream>
#include "api/rest/ApiServer.hpp"

namespace api {
namespace rest {

// Mark: CONSTANTS
const char *const kApiVersionPrefix = "/api/v1";
const char *const kApplicationJson = "application/json";
const char *const kApplicationProtobuf = "application/x-protobuf";

// Mark: ERROR CONSTANTS
const char *const kNoApiUrl = "Unknown API endpoint; should start with /api/v1/";
const char *const kIllegalRequest = "Cannot parse JSON into valid PB";
const char *const kInvalidResource = "Not a valid resource";
const char *const kMethodNotAllowed = "Method Not Allowed";

struct post_info {
  Response* response;
};

int ApiServer::ConnectCallback(void *cls,
                               struct MHD_Connection *connection,
                               const char *url,
                               const char *method,
                               const char *version,
                               const char *upload_data,
                               size_t *upload_data_size,
                               void **con_cls) {
  ApiServer *server = static_cast<ApiServer *>(cls);
  std::string path{url};

  VLOG(2) << method << " " << path;
  if (!server->HasMethod(method)) {
    // TODO: move this out to MethodNotAllowed() method.
    auto response = MHD_create_response_from_buffer(strlen(kMethodNotAllowed),
                                                    (void *) kMethodNotAllowed,
                                                    MHD_RESPMEM_PERSISTENT);
    LOG(ERROR) << "415: Not an allowed method: " << method;
    int ret = MHD_queue_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, response);
    MHD_destroy_response(response);
    return ret;
  }

  if (path.find(kApiVersionPrefix) != 0) {
    auto response = MHD_create_response_from_buffer(strlen(kNoApiUrl),
                                                    (void *) kNoApiUrl,
                                                    MHD_RESPMEM_PERSISTENT);
    LOG(ERROR) << "Not a valid API request: " << path;
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
  }

  std::string resource = path.substr(path.rfind('/') + 1);
  VLOG(2) << "Resource: " << resource;

  if (strcmp(method, "GET") == 0) {
    auto handlers_map = server->handlers_["GET"];
    if (handlers_map.find(resource) != handlers_map.end()) {

      // TODO: Populate Request with whatever headers, etc. necessary
      auto response = handlers_map[resource](Request{});
      return sendResponse(connection, response);
    }
  } else if (strcmp(method, "POST") == 0) {
    auto handlers_map = server->handlers_["POST"];
    if (handlers_map.find(resource) != handlers_map.end()) {
      VLOG(2) << "Handler found for " << resource;

      if (*con_cls == nullptr) {
        VLOG(2) << "Pre-processing for POST";
        *con_cls = new post_info{};
        return MHD_YES;
      }

      auto con_info = static_cast<post_info *>(*con_cls);
      if (*upload_data_size != 0) {
        std::string body{upload_data, *upload_data_size};
        VLOG(2) << "Received " << *upload_data_size << " bytes";
        con_info->response = new Response(handlers_map[resource](Request{body}));
        *upload_data_size = 0;
        return MHD_YES;
      }


      Response* response = con_info->response;
      auto res_bytes = response->body().c_str();

      VLOG(2) << "POST processing completed; returning ("
              << response->status_code() << "): "
              << res_bytes;
      auto mhd_response = MHD_create_response_from_buffer(strlen(res_bytes),
          (void *) res_bytes, MHD_RESPMEM_MUST_COPY);
      int ret = MHD_queue_response(connection, response->status_code(), mhd_response);
      MHD_destroy_response(mhd_response);
      return ret;
    }
  }

  // TODO: move this out to NotFound() method.
  auto response = MHD_create_response_from_buffer(strlen(kInvalidResource),
                                                  (void *) kInvalidResource,
                                                  MHD_RESPMEM_PERSISTENT);
  LOG(ERROR) << "404: No handler registered for: " << resource;
  int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response(response);
  return ret;
}

int ApiServer::sendResponse(MHD_Connection *connection, const Response &response) {
  auto response_body = response.body().c_str();
  auto res = MHD_create_response_from_buffer(strlen(response_body),
                                                 (void *) response_body,
                                                 MHD_RESPMEM_MUST_COPY);
  MHD_add_response_header(res, MHD_HTTP_HEADER_CONTENT_TYPE, kApplicationJson);
  int ret = MHD_queue_response(connection, response.status_code(), res);
  MHD_destroy_response(res);
  return ret;
}


void ApiServer::AddMethodHandler(const std::string &method,
                                 const std::string &resource,
                                 const Handler &handler) {

  LOG(INFO) << "Registering " << method << " handler for: " << resource;
  handlers_[method][resource] = handler;
}

} // namespace rest
} // namespace api
