//
// Created by 1655664358@qq.com on 2019/12/12.
//
#include "http.h"
#include "base.h"

static void groupsAction(request *req)
{
    http_s *h = req->h;
    http_send_body(h,"hello,group",11);
}

static void addGroupAction(request *req)
{
    http_s *h = req->h;
    http_send_body(h,"hello,addGroup",14);
}

static void removeGroupAction(request *req)
{
    http_s *h = req->h;
    http_send_body(h,"hello,remove",13);
}
