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
        self.req = initial.LoginReqMsg();
        self.key = 99999989;
        pass;

    def mock(self, phone, passwd):
        self.req.usr_info = phone;
        self.req.password = passwd;

    def genK(self):
        req_str = self.Serialize();
        return (99999989 + len(req_str) + 2) / 3;

    def Serialize(self):
        return self.req.SerializeToString()
        pass

    def Deserialize(self, responseStr):
        self.resp = initial.LoginRespMsg()
        self.resp.ParseFromString(responseStr)
        pb_dict = protobuf2dict.load(self.resp);
        return pb_dict

if __name__ == '__main__':
    if (len(sys.argv) != 3):
        print "Usage: mock_user_login.py <phone> <password>";

    phone = sys.argv[1];
    passwd = sys.argv[2];

    Mock = UserInit();
    Mock.mock(phone, passwd);
    key = Mock.genK();
    print "Key: ", key
    server = "127.0.0.1"

    print "Server: ", server
    print "Reuqest:========================"
    print Mock.req.__str__();

    tp_str = '\x03\x00';
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
