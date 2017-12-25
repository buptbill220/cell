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
        self.req = ugc.PostUGCReqMsg();
        self.key = 99999989;
        pass;

    def mock(self, uid, session, ugc_id, msid, org_uid, post_type):
        self.req.session_token = session;
        self.req.uid = int(uid);
        self.req.ugc_id = ugc_id;
        self.req.msid = msid;
        self.req.org_uid = org_uid;
        self.req.post_type = post_type;
        self.req.geo.latitude = 40.018141 + random.random() * 2;
        self.req.geo.longitude = -105.274858 + random.random();

    def genK(self):
        req_str = self.Serialize();
        return (99999989 + len(req_str) + 2) / 3;

    def Serialize(self):
        return self.req.SerializeToString()
        pass

    def Deserialize(self, responseStr):
        self.resp = ugc.PostUGCRespMsg()
        self.resp.ParseFromString(responseStr)
        pb_dict = protobuf2dict.load(self.resp);
        return pb_dict

if __name__ == '__main__':
    if (len(sys.argv) != 7):
        print "Usage: mock_ugc_post.py <uid> <session_token> ugc_id msid org_uid post_type";

    uid = sys.argv[1];
    session_token = sys.argv[2];
    ugc_id = int(sys.argv[3]);
    msid = int(sys.argv[4]);
    org_uid = int(sys.argv[5]);
    post_type = int(sys.argv[6])

    Mock = UserInit();
    Mock.mock(uid, session_token, ugc_id, msid, org_uid, post_type);
    key = Mock.genK();
    print "Key: ", key
    server = "127.0.0.1"

    print "Server: ", server
    print "Reuqest:========================"
    print Mock.req.__str__();

    tp_str = '\x68\x00';
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
