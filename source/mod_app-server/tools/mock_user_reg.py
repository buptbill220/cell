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
        self.req = initial.RegReqMsg();
        self.key = 99999989;
        pass;

    def mock(self, uid, session):
        self.req.session_token = session;
        self.req.uid = int(uid);
        self.req.phone = "12348" + str(random.randint(1,343456));
        self.req.email = "buptbill220@gmail.com";
        self.req.gender = common_data.GR_MALE;
        self.req.birthday = "1988-01-25 00:00:00";
        self.req.avada_url = "img";
        self.req.avada_size = "1000x435";
        self.req.usr_name = "test_name";
        self.req.password = "123456";
        self.req.reg_source = common_data.RS_NATIVE;

    def genK(self):
        req_str = self.Serialize();
        return (99999989 + len(req_str) + 2) / 3;

    def Serialize(self):
        return self.req.SerializeToString()
        pass

    def Deserialize(self, responseStr):
        self.resp = initial.RegRespMsg()
        self.resp.ParseFromString(responseStr)
        pb_dict = protobuf2dict.load(self.resp);
        return pb_dict

if __name__ == '__main__':
    if (len(sys.argv) != 3):
        print "Usage: mock_user_reg.py <uid> <session_token>";

    uid = sys.argv[1];
    session_token = sys.argv[2];

    Mock = UserInit();
    Mock.mock(uid, session_token);
    key = Mock.genK();
    print "Key: ", key
    server = "127.0.0.1"

    print "Server: ", server
    print "Reuqest:========================"
    print Mock.req.__str__();

    tp_str = '\x02\x00';
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
