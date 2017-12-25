#!/usr/ali/bin/python
##-*- coding:utf-8 -*-

import common_data_pb2 as common_data
import initial_pb2 as initial
import httplib
import protobuf2dict
import sys
import time
import random

class UserInit:
    def __init__(self):
        self.req = initial.InitReqMsg();
        self.key = 99999989;
        pass;

    def mock(self):
        self.req.device_id = "device_id_" + str(random.randint(3,4654646));
        self.req.device_type = common_data.DT_ANDROID;
        self.req.app_key = "key";
        self.req.app_ver = "ver";

    def genK(self):
        req_str = self.Serialize();
        return (99999989 + len(req_str) + 2) / 3;

    def Serialize(self):
        return self.req.SerializeToString()
        pass

    def Deserialize(self, responseStr):
        self.resp = initial.InitRespMsg()
        self.resp.ParseFromString(responseStr)
        pb_dict = protobuf2dict.load(self.resp);
        return pb_dict

if __name__ == '__main__':
    Mock = UserInit();
    Mock.mock();
    key = Mock.genK();
    print "Key: ", key
    server = "127.0.0.1"

    print "Server: ", server
    print "Reuqest:========================"
    print Mock.req.__str__();

    tp_str = '\x01\x00';
    pb_str = Mock.Serialize();

    conn = httplib.HTTPConnection(server, 80);
    headers = {"Content-Type": "application/x-protobuf"};
    conn.request('POST', '/user?k=' + str(key), tp_str+pb_str, headers);

    response = conn.getresponse();
    print "HTTP Response Status: %d" % response.status;

    if  response.status > 200:
        print "Reason: " + response.reason;

    resStr = response.read();
    Mock.Deserialize(resStr);

    print "Response:=======================";
    print Mock.resp.__str__();
