// Copyright (c) 2017 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 7/23/17.


#include <memory>
#include <thread>

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <SimpleHttpRequest.hpp>

#include "api/rest/ApiServer.hpp"

#include "tests.h"

using namespace api::rest;
using namespace std::chrono;


class ApiServerTest : public ::testing::Test {
protected:

  std::shared_ptr<ApiServer> server_;
  request::SimpleHttpRequest client_;

  void SetUp() override {
    server_ = std::make_shared<api::rest::ApiServer>(7999);
    client_.timeout = 150;
    client_.setHeader("Accept", "application/json");

    server_->AddGet("test", [] (const Request& request) -> Response {
      LOG(INFO) << "GET Handler for 'test' invoked";
      return Response::ok();
    });

    server_->AddPost("test", [] (const Request& request) -> Response {
      LOG(INFO) << "POST Handler for 'test' invoked with: "
                << request.body();
      return Response::created();
    });

    server_->AddGet("body", [] (const Request& request) -> Response {
      LOG(INFO) << "GET Handler for 'test' invoked";
      auto response = Response::ok();
      response.set_body("Response Body");
      return response;
    });

    server_->Start();
  }

  void TearDown() override {
    // teardown goes here
  }

  const std::shared_ptr<ApiServer> server() const { return server_; }
};


TEST_F(ApiServerTest, noApiEndpointReturnsNotFound) {
  try {
    client_.get("http://localhost:7999/foobar")
        .on("error", [](request::Error &&err) -> void {
          FAIL() << "Could not connect to API Server: "
                 << err.message;
        }).on("response", [](request::Response &&res) -> void {
          EXPECT_EQ(404, res.statusCode);
          EXPECT_NE(std::string::npos, res.str().find("Unknown API endpoint")) << "Found "
                    "instead: " << res.str();
        }).end();
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}


TEST_F(ApiServerTest, unregisteredEndpointReturnsNotFound) {

  try {
    client_.get("http://localhost:7999/api/v1/foobar")
        .on("error", [](request::Error &&err) -> void {
          FAIL() << "Could not connect to API Server: "
                 << err.message;
        }).on("response", [](request::Response &&res) -> void {
          EXPECT_EQ(404, res.statusCode);
          EXPECT_NE(std::string::npos, res.str().find("Not a valid resource")) << "Found "
                    "instead: " << res.str();
        }).end();
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}


TEST_F(ApiServerTest, getValidEndpointReturns200) {
  try {
    client_.get("http://localhost:7999/api/v1/test")
        .on("error", [](request::Error &&err) {
          FAIL() << "Could not connect to API Server: "
                 << err.message;
        }).on("response", [this](request::Response &&res) {
          EXPECT_EQ("application/json", res.headers["content-type"]);
          EXPECT_EQ(200, res.statusCode);
        }).end();
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}


TEST_F(ApiServerTest, postValidEndpointReturns201) {
  try {
    client_.post("http://localhost:7999/api/v1/test", "{\"test\": \"here\"}")
        .on("error", [](request::Error &&err) {
          FAIL() << "Could not connect to API Server: "
                 << err.message;
        }).on("response", [this](request::Response &&res) {
          EXPECT_TRUE(res.good());
          EXPECT_EQ(201, res.statusCode);
        }).end();
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}


TEST_F(ApiServerTest, getBody) {
  try {
    client_.get("http://localhost:7999/api/v1/body")
        .on("error", [](request::Error &&err) {
          FAIL() << "Could not connect to API Server: "
                 << err.message;
        }).on("response", [this](request::Response &&res) {
          EXPECT_EQ("application/json", res.headers["content-type"]);
          EXPECT_EQ(200, res.statusCode);
          EXPECT_EQ("Response Body", res.str());
        }).end();
  } catch (const std::exception &e) {
    FAIL() << e.what();
  }
}


// TODO: SimpleHttpRequest does not support sending query args, need to find a workaround.
TEST_F(ApiServerTest, DISABLED_queryArgs) {
    server_->AddGet("query", [](const Request &request) -> Response {
        auto query = request.GetQueryArg("abc");
        if (query == "test") {
          return Response::ok();
        }
        return Response::bad_request("Invalid query arg: '" + query + "'");
    });

    try {
        client_.get("http://localhost:7999/api/v1/query?abc=test&def=value")
                .on("error", [](request::Error &&err) {
                    FAIL() << "Could not connect to API Server: "
                           << err.message;
                }).on("response", [this](request::Response &&res) {
                    EXPECT_EQ(200, res.statusCode) << res.str();
                }).end();
    } catch (const std::exception &e) {
        FAIL() << e.what();
    }
}


TEST_F(ApiServerTest, setHeaders) {

    server_->AddGet("headers", [](const Request &request) {
        auto response = Response::ok();
        response.AddHeader("Content-Type", "plain/text");
        response.AddHeader("x-custom", "custom-header:value");
        return response;
    });

    try {
        client_.get("http://localhost:7999/api/v1/headers")
                .on("error", [](request::Error &&err) {
                    FAIL() << "Could not connect to API Server: "
                           << err.message;
                }).on("response", [this](request::Response &&res) {
                    EXPECT_EQ("plain/text", res.headers["content-type"]);
                    EXPECT_EQ(200, res.statusCode);
                    EXPECT_EQ("custom-header:value", res.headers["x-custom"]);
                }).end();
    } catch (const std::exception &e) {
        FAIL() << e.what();
    }
}


TEST_F(ApiServerTest, postContentType) {

    server_->AddPost("entity", [](const Request &request) -> Response {
        if (request.body() != "Some random value") {
          return Response::bad_request("Unexpected body: " + request.body());
        }

        auto response = Response::created();
        response.AddHeader("Content-Type", "plain/text");
        response.set_body("plain text body");
        return response;
    });

    try {
        client_.post("http://localhost:7999/api/v1/entity", "Some random value")
                .on("error", [](request::Error &&err) {
                    FAIL() << "Could not connect to API Server: "
                           << err.message;
                }).on("response", [this](request::Response &&res) {
                    EXPECT_EQ("plain/text", res.headers["content-type"]);
                    EXPECT_EQ(201, res.statusCode);
                    EXPECT_EQ("plain text body", res.str());
                }).end();
    } catch (const std::exception &e) {
        FAIL() << e.what();
    }
}


