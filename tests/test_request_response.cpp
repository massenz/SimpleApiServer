// Copyright (c) 2017 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com) on 7/23/17.


#include <gtest/gtest.h>

#include "api/rest/ApiServer.hpp"

#include "tests.h"

using namespace api::rest;

Response retResp() {
  auto ret = Response::ok();
  ret.AddHeader("simple", "value");
  return ret;
}


TEST(TestRequestResponse, testCreateResponse) {
  auto response = Response::ok();
  ASSERT_EQ(200, response.status_code());
}

TEST(TestRequestResponse, testCreateRequest) {
  Request req("this is a test");
  ASSERT_EQ("this is a test", req.body());
  ASSERT_EQ(0, req.query_args().size());
}


TEST(TestRequestResponse, testHeaders) {
  auto response = Response::created();
  response.AddHeader("Content-Type", "text/plain");

  ASSERT_EQ("text/plain", response.GetHeader("Content-Type"));
  ASSERT_EQ("", response.GetHeader("x-foo"));

  response.AddHeader("x-foo", "custom-header=211;x-len:776");
  ASSERT_EQ("custom-header=211;x-len:776", response.GetHeader("x-foo"));
}


TEST(TestRequestResponse, testQueryArgs) {
  Request request;
  request.AddQueryArg("test", "value");
  request.AddQueryArg("page", "2");

  ASSERT_EQ("value", request.GetQueryArg("test"));
  ASSERT_EQ("2", request.GetQueryArg("page"));
  ASSERT_EQ("", request.GetQueryArg("non-exist"));
}


TEST(TestRequestResponse, testCopyConstructor) {
  auto response = Response::created();
  response.AddHeader("Content-Type", "text/plain");
  response.AddHeader("Authorization", "user=foo:password=bar");
  response.AddHeader("x-foo", "custom-header=211;x-len:776");

  Response other(response);

  ASSERT_EQ(201, other.status_code());
  ASSERT_EQ("CREATED", other.reason());
  ASSERT_EQ("", other.body());

  ASSERT_EQ("text/plain", other.GetHeader("Content-Type"));
  ASSERT_EQ("user=foo:password=bar", other.GetHeader("Authorization"));
  ASSERT_EQ("custom-header=211;x-len:776", other.GetHeader("x-foo"));
}

TEST(TestRequestResponse, trivial) {
  Response response = retResp();
  ASSERT_EQ(200, response.status_code());
  ASSERT_EQ("value", response.GetHeader("simple"));
}
