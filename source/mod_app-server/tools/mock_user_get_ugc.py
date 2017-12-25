#!/usr/ali/bin/python
##-*- coding:utf-8 -*-

import common_data_pb2 as common_data
import ugc_pb2 as ugc
import httplib
import protobuf2dict
import sys
import time
import random

class UserInit:
    def __init__(self):
        self.req = ugc.GetUGCListReqMsg();
        self.key = 99999989;
        pass;

    def mock(self, uid, session, type):
        self.req.uid = int(uid);
        self.req.session_token = session;
        self.req.ugc_list_type = int(type);
        self.req.last_id = random.randint(0, 100);
        self.req.need_ugc_num = random.randint(1,5);

    def genK(self):
        req_str = self.Serialize();
        return (99999989 + len(req_str) + 2) / 3;

    def Serialize(self):
        return self.req.SerializeToString()
        pass

    def Deserialize(self, responseStr):
        self.resp = ugc.GetUGCListRespMsg()
        self.resp.ParseFromString(responseStr)
        pb_dict = protobuf2dict.load(self.resp);
        return pb_dict

if __name__ == '__main__':
    if (len(sys.argv) != 4):
        print "Usage: mock_user_get_ugc.py <uid> <session_token> <type>";

    uid = sys.argv[1];
    session = sys.argv[2];
    type = sys.argv[3];

    Mock = UserInit();
    Mock.mock(uid, session, type);
    key = Mock.genK();
    print "Key: ", key
    server = "127.0.0.1"

    print "Server: ", server
    print "Reuqest:========================"
    print Mock.req.__str__();

    tp_str = '\x07\x00';
    pb_str = Mock.Serialize();

    conn = httplib.HTTPConnection(server, 81);
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
