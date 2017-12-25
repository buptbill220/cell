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
        self.req = ugc.UploadUGCReqMsg();
        self.key = 99999989;
        pass;

    def mock(self, uid, session):
        self.req.session_token = session;
        self.req.uid = int(uid);
        self.req.ugc.message_meta.content = u"mock测试" + str(random.randint(1,24324));
        #self.req.ugc.message_meta.image.append(u"a.img;b.img");
        self.req.ugc.message_meta.img_size.append("100x23;4x444");
        self.req.ugc.geo.latitude = 40.018141 + random.random() * 2;
        self.req.ugc.geo.longitude = -105.274858 + random.random();

    def genK(self):
        req_str = self.Serialize();
        return (99999989 + len(req_str) + 2) / 3;

    def Serialize(self):
        return self.req.SerializeToString()
        pass

    def Deserialize(self, responseStr):
        self.resp = ugc.UploadUGCRespMsg()
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

    tp_str = '\x65\x00';
    pb_str = Mock.Serialize();

    conn = httplib.HTTPConnection(server, 80);
    headers = {"Content-Type": "application/x-protobuf"};
    conn.request('POST', '/ugc?k=' + str(key), tp_str+pb_str, headers);

    response = conn.getresponse();
    print "HTTP Response Status: %d" % response.status;

    if  response.status > 200:
        print "Reason: " + response.reason;

    resStr = response.read();
    Mock.Deserialize(resStr);

    print "Response:=======================";
    print Mock.resp.__str__();
