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
const char *const kTextHtml = "text/html";
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

  // TODO: Populate Request with whatever headers, etc. necessary
  Request request;

  // Parsing the request URI query arguments & headers.
  // This method (according to the documentation) can take a bitmask (and the enums are built to work correctly
  // that way); however, the compiler complains because it sees an int and cannot convert to an enum.
  // TODO: Need to figure out a way to coalesce the following two calls into one.
  MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND,
                             &ApiServer::HeadersQueryargsCallback, &request);

  MHD_get_connection_values (connection, MHD_HEADER_KIND,
                             &ApiServer::HeadersQueryargsCallback, &request);

  if (strcmp(method, "GET") == 0) {
    auto& handlers_map = server->handlers_["GET"];
    if (handlers_map.find(resource) != handlers_map.end()) {
      auto response = handlers_map[resource](request);
      return sendResponse(connection, response);
    }
  } else if (strcmp(method, "POST") == 0) {
    auto& handlers_map = server->handlers_["POST"];
    if (handlers_map.find(resource) != handlers_map.end()) {
      if (*con_cls == nullptr) {
        *con_cls = new post_info{};
        return MHD_YES;
      }

      auto con_info = static_cast<post_info *>(*con_cls);
      if (*upload_data_size != 0) {

        VLOG(2) << "Received " << *upload_data_size << " bytes";
        request.set_body(std::string{upload_data, *upload_data_size});

        con_info->response = new Response(handlers_map[resource](request));
        *upload_data_size = 0;
        return MHD_YES;
      }
      return sendResponse(connection, *con_info->response);
    }
  }
  return ResourceNotFound(connection, resource);
}

int ApiServer::ResourceNotFound(MHD_Connection *connection, const std::string &resource) {
  auto response = MHD_create_response_from_buffer(strlen(kInvalidResource),
                                                  (void *) kInvalidResource,
                                                  MHD_RESPMEM_PERSISTENT);
  LOG(ERROR) << "404: No handler registered for: " << resource;
  int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response(response);
  return ret;
}

int ApiServer::sendResponse(MHD_Connection *connection, const Response &response) {
  auto response_body = response.body();
  auto res = MHD_create_response_from_buffer(response_body.size(),
                                             (void *) response_body.c_str(),
                                             MHD_RESPMEM_MUST_COPY);

  for(auto& header : response.headers()) {
    MHD_add_response_header(res, header.first.c_str(), header.second.c_str());
  }

  // The default Content-Type will be application/json, unless otherwise set in the handler.
  // See BaseRequestResponse constructor.
  if (response.headers().count(MHD_HTTP_HEADER_CONTENT_TYPE) == 0) {
    MHD_add_response_header(res, MHD_HTTP_HEADER_CONTENT_TYPE, kApplicationJson);
  }

  auto ret = MHD_queue_response(connection, response.status_code(), res);
  MHD_destroy_response(res);
  return ret;
}


void ApiServer::AddMethodHandler(const std::string &method,
                                 const std::string &resource,
                                 const Handler &handler) {

  LOG(INFO) << "Registering " << method << " handler for: " << resource;
  handlers_[method][resource] = handler;
}

int ApiServer::HeadersQueryargsCallback(void *req, enum MHD_ValueKind kind, const char *key, const char *value) {

  auto request = static_cast<Request *>(req);
  switch (kind) {
    case MHD_GET_ARGUMENT_KIND:
      VLOG(2) << "[URI Query Arg] " << key << " = " << value;
      request->AddQueryArg(key, value);
      break;
    case MHD_HEADER_KIND:
      VLOG(2) << "[Header] " << key << ": " << value;
      request->AddHeader(key, value);
      break;
    default:
      LOG(ERROR) << "Unexpected kind: " << kind << " cannot process (" << key << ", " << value << ")";
      return MHD_NO;
  }
  return MHD_YES;
}

} // namespace rest
} // namespace api
