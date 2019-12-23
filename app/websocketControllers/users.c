//
// Created by 1655664358@qq.com on 2019/12/12.
//

#include "http.h"
#include "../httpControllers/base.h"
/*********************************************************************
 *
 * websocket 业务模块
 * 功能：负责接受websocket数据并处理
 *********************************************************************/

static void talkHandler(request *req)
{

    ws_s *ws = req->ws;
    fio_str_info_s msg = req->msg;
    if (msg.len<2){
        websocket_response("请输点东西吧",1,"",ws);
        return;
    }
    uint8_t is_text = req->is_text;

    //解析数据
    cJSON *data=NULL;
    data = cJSON_Parse(msg.data);
    cJSON *params;

    FIOBJ value = fiobj_hash_new();//接受数据
    FIOBJ fields = fiobj_ary_new();//表单字段

    fiobj_ary_push(fields,fiobj_str_new("loginId",7));
    fiobj_ary_push(fields,fiobj_str_new("word",4));
    fiobj_ary_push(fields,fiobj_str_new("time",4));

    if (!data) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
    //接受数据
    params = cJSON_GetObjectItem(data,"data");
    //用户的内容
    if (params&&params->valuestring&&strlen(params->valuestring)<10000) {
        fiobj_hash_set(value, fiobj_str_new("word", 4),
                       fiobj_str_new(params->valuestring, strlen(params->valuestring)));
    }else{

        websocket_response("请输点东西吧",1,"",ws);
        return;
    }
    //用户的账号
    params = cJSON_GetObjectItem(data,"loginId");

    if (params&&params->valuestring) {
        fiobj_hash_set(value, fiobj_str_new("loginId", 7),
                       fiobj_str_new(params->valuestring, strlen(params->valuestring)));
        fiobj_hash_set(value, fiobj_str_new("time", 4), fiobj_num_new(time(NULL)));
    }else{

        websocket_response("你账号都没有玩什么",9,"",ws);
        return;
    }

    FIOBJ str = fiobj_str_copy((FIOBJ)websocket_udata_get(ws));
    fiobj_str_write(str, msg.data, msg.len);
    // publish  广播发送
    fio_publish(.channel = CHAT_CANNEL, .message = fiobj_obj2cstr(str));
    //将消息入库保存 检测一下该账号是否存在
    char condition[30] = {0};
    sprintf(condition,"`id`=%s",params->valuestring);
    printf("%s\n",condition);
    FIOBJ userProfile = db("users")->where(condition)->value("id");
    //可以插入
    if (fiobj_obj2num(fiobj_hash_get(userProfile,fiobj_str_new("result",6))) == 1){
        printf("insert\n");
        db("group_chat")->insert((table_data){.fields=fields,.value=value});
    }

    fiobj_free(value);
    fiobj_free(fields);
    cJSON_Delete(data);


}

static void recentTalkHandler(request *req)
{
    printf("拉取最近的聊天记录20条并给当前用户发送 \n");
    ws_s *ws = req->ws;
    fio_str_info_s msg = req->msg;
    if (msg.len<2){
        websocket_response("请输点东西吧",1,"",ws);
        return;
    }
    uint8_t is_text = req->is_text;
    if (msg.data) {
        cJSON *data = cJSON_Parse(msg.data);
        cJSON *params;
        //取start
        params = cJSON_GetObjectItem(data, "start");
        int start = params->valueint&&params->valueint>0? params->valueint : 1;
        //每页条数
        params = cJSON_GetObjectItem(data, "count");
        int count = params->valueint&&params->valueint>0? params->valueint : 20;
        int page;

        page = (start-1)*count;

        char sql[200] = {0};
        sprintf(sql,
                "SELECT * FROM (SELECT a.loginId,a.word,a.time,b.login FROM group_chat a LEFT JOIN users b ON a.loginId=b.id ORDER BY a.time DESC LIMIT %d,%d) AS temp ORDER BY time ASC",
                page, count);

        //printf("%s\n",sql);
        FIOBJ result = db(NULL)->query(sql, SELECT);
        char total_sql[100] = {0};
        sprintf(total_sql,"SELECT COUNT(id) AS count FROM group_chat");


        if (fiobj_obj2num(fiobj_hash_get(result, fiobj_str_new("result", 6))) == 1) {
            FIOBJ columns = fiobj_hash_get(result, fiobj_str_new("columns", 7));

            FIOBJ rows = fiobj_hash_get(result, fiobj_str_new("rows", 4));
            FIOBJ temp;
            /*************json*******************/

            cJSON *root, *fmt, *ary;
            char *out;
            root = cJSON_CreateObject();
            ary = cJSON_CreateArray();

            /*************json*******************/

            for (int j = 0; j < fiobj_ary_count(rows); ++j) {//行
                temp = fiobj_ary_index(rows, j);
                fmt = cJSON_CreateObject();
                for (int k = 0; k < fiobj_ary_count(temp); ++k) {//列
                    cJSON_AddStringToObject(fmt, fiobj_obj2cstr(fiobj_ary_index(columns, k)).data,
                                            fiobj_obj2cstr(fiobj_ary_index(temp, k)).data);
                }
                cJSON_AddItemToArray(ary, fmt);
            }
            cJSON_AddItemToObject(root, "data", ary);
            //获取总记录
            FIOBJ total = db(NULL)->query(total_sql, SELECT);
            rows = fiobj_hash_get(total, fiobj_str_new("rows", 4));
            temp = fiobj_ary_index(fiobj_ary_index(rows,0), 0);

            cJSON_AddStringToObject(root, "count", fiobj_obj2cstr(temp).data);
            out = cJSON_Print(root);
            websocket_response(out,2,"请求成功",ws);
            fiobj_free(result);
            fiobj_free(columns);
            fiobj_free(rows);
            cJSON_Delete(root);
            free(out);

        }
    }
}

static void heartBeatHandler(request *req)
{
    ws_s *ws = req->ws;
    fio_str_info_s msg = req->msg;
    websocket_write(
            ws, (fio_str_info_s){.data = msg.data, .len = msg.len},
            1);
}