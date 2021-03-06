// Copyright (c) 2016 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 10/8/16.


#include <chrono>
#include <iostream>
#include <thread>

#include <glog/logging.h>

#include <atomic>
#include <api/rest/ApiServer.hpp>

#include "version.h"

#include "distlib/utils/ParseArgs.hpp"
#include "distlib/utils/utils.hpp"

namespace {

/**
 * Prints out usage instructions for this application.
 */
void usage() {
  std::cout << "Usage: server_demo --port=PORT [--debug] [--version] [--help]\n\n"
            << "\t--debug    verbose output (LOG_v = 2)\n"
            << "\t--help     prints this message and exits\n"
            << "\t--version  prints the version string and exits\n\n"
            << "\tPORT       an int specifying the port the server will listen on\n\n";
}

std::atomic<bool> stopped(false);

} // namespace


int main(int argc, const char **argv) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = true;

  ::utils::ParseArgs parser(argv, argc);
  if (parser.Enabled("debug")) {
    FLAGS_v = 2;
  }

  if (parser.has("help")) {
    usage();
    return EXIT_SUCCESS;
  }

  ::utils::PrintVersion("Server Demo", RELEASE_STR);
  if (parser.has("version")) {
    return EXIT_SUCCESS;
  }

  unsigned int port = parser.getUInt("port", 6060);
  if (port > 65535) {
    usage();
    LOG(ERROR) << "Port number must be a positive integer, less than 65,535. "
               << "Found: " << port;
    return EXIT_FAILURE;
  }

  api::rest::ApiServer server(port);
  server.AddGet("demo", [](const api::rest::Request& req) {
    auto query = req.GetQueryArg("q");

    auto resp = api::rest::Response::ok();
    resp.AddHeader("Content-Type", "text/plain");

    if (!query.empty()) {
      resp.set_body("Your query: " + query + "\nThe answer is: 42");
    } else {
      resp.set_body("Use the `q` query argument to ask me anything");
    }
    return resp;
  });
  server.AddGet("stop", [=](const api::rest::Request& req) {
    ::stopped.store(true);
    return api::rest::Response::ok("Stopping server", true);
  });

  server.Start();

  while (!stopped) {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }

  LOG(INFO) << "done";
  return EXIT_SUCCESS;
}
