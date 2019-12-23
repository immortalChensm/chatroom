//
// Created by 1655664358@qq.com on 2019/12/11.
//

#include "http.h"
#include "base.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../model/db/database.c"

static void *task(void *arg)
{
    for (int j = 0; j <10 ; ++j) {
        printf("%d\n",j);
        sleep(1);
    }
    int ret = 1;
    printf("线程%d马上结束\n",pthread_self());
    pthread_exit((void*)&ret);
}
static void indexAction(request *req)
{
    http_s *h = req->h;
    http_send_body(h,"hello,index",11);

//    //创建分离线程处理耗时任务
//    pthread_t tid;
//    pthread_attr_t attr;
//    pthread_attr_init(&attr);
//    //设置线程为分离线程  线程结束时自动释放占用的资源
//    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
//    pthread_create(&tid,&attr,task,NULL);
//    void *res;
//    pthread_join(tid,&res);
//    printf("线程%d=%d运行结束\n",tid,*(int*)res);
}

static void getAllUsers(request *req)
{
    http_s *h = req->h;
    char sql[50] = {0};
    sprintf(sql,"SELECT login,avatar_url from users");
    FIOBJ result = db(NULL)->query(sql, SELECT);
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
        out = cJSON_Print(root);
        http_response(h, 200, out);
        fiobj_free(result);
        fiobj_free(columns);
        fiobj_free(rows);
        cJSON_Delete(root);
        free(out);

    }
}

static void loginAction(request *req)
{
    http_s *h = req->h;
    http_send_body(h,"hello,login",11);
}

static void testAction(request *req)
{
    http_s *h = req->h;
    http_parse_body(h);
    if (fiobj_type_is(h->params,FIOBJ_T_HASH)){

        if (fiobj_hash_haskey(h->params,fiobj_str_new("file",4))){

            FIOBJ file = fiobj_hash_get(h->params,fiobj_str_new("file",4));
            printf("%s\n",fiobj_type_name(file));

            if (fiobj_hash_haskey(file,fiobj_str_new("type",4))){
                FIOBJ type = fiobj_hash_get(file,fiobj_str_new("type",4));
                printf("type=%s\n",fiobj_obj2cstr(type).data);
            }
            if (fiobj_hash_haskey(file,fiobj_str_new("name",4))){
                FIOBJ name = fiobj_hash_get(file,fiobj_str_new("name",4));
                printf("type=%s\n",fiobj_obj2cstr(name).data);
            }
            if (fiobj_hash_haskey(file,fiobj_str_new("data",4))){
                FIOBJ data = fiobj_hash_get(file,fiobj_str_new("data",4));
                printf("data=%s\n",fiobj_obj2cstr(data).data);
                FILE *image;
                FIOBJ fileName = fiobj_str_new("../web/",7);
                fiobj_str_concat(fileName,fiobj_hash_get(file,fiobj_str_new("name",4)));
                image = fopen(fiobj_obj2cstr(fileName).data,"wb");
                fwrite(fiobj_obj2cstr(data).data,fiobj_obj2cstr(data).len,1,image);
                fclose(image);
            }
        }

    }
    http_send_body(h,"hello,test",10);
}

static void *addGithubUser(void *args)
{
    cJSON *params;
    char *user = (char*)args;
    char *table_fields[] = {"login","email","avatar_url","html_url","name","company","blog","location","bio","public_repos","followers","following","created_at","id"};

    FIOBJ data = fiobj_hash_new();//接受数据
    FIOBJ fields = fiobj_ary_new();//表单字段

    cJSON *profile=cJSON_Parse(user);
    if (!profile) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
    params = cJSON_GetObjectItem(profile,"id");

    //尝试取一下数据，存在则更新/否则保存
    char condition[50] = {0};
    sprintf(condition,"`id`=%d",params->valueint);
    FIOBJ userProfile = db("users")->where(condition)->value("id");
    for (int j = 0; j <14 ; ++j) {
        fiobj_ary_push(fields,fiobj_str_new(table_fields[j],strlen(table_fields[j])));
        params = cJSON_GetObjectItem(profile,table_fields[j]);
        if (strcasecmp("public_repos",table_fields[j])==0||strcasecmp("followers",table_fields[j])==0||strcasecmp("following",table_fields[j])==0||strcasecmp("id",table_fields[j])==0){
            fiobj_hash_set(data,fiobj_str_new(table_fields[j],strlen(table_fields[j])),fiobj_num_new(params->valueint?params->valueint:0));
        }else{
            if (params->valuestring&&strlen(params->valuestring)){
                fiobj_hash_set(data,fiobj_str_new(table_fields[j],strlen(table_fields[j])),fiobj_str_new(params->valuestring,strlen(params->valuestring)));
            }else{
                fiobj_hash_set(data,fiobj_str_new(table_fields[j],strlen(table_fields[j])),fiobj_str_new("null",4));
            }

        }
    }

    if (fiobj_obj2num(fiobj_hash_get(userProfile,fiobj_str_new("result",6))) == 1){
        //printf("update=%s\n",fiobj_obj2cstr(fiobj_hash_get(userProfile,fiobj_str_new("data",4))).data);
        db("users")->where(condition)->update((table_data){.fields=fields,.value=data});
    }else{
        //printf("insert\n");
        db("users")->insert((table_data){.fields=fields,.value=data});
    }
    fiobj_free(fields);
    fiobj_free(data);
    cJSON_Delete(profile);
    //free(user);
    user = NULL;
    pthread_exit((void*)0);
}

static void gitLoginAction(request *req)
{
    http_s *h = req->h;
    http_parse_query(h);
    FIOBJ code = fiobj_str_new("code",4);
    if(fiobj_hash_haskey(h->params,code)){
        char *uri = "https://github.com/login/oauth/access_token";
        char postData[1024];
        sprintf(postData,"client_id=%s&client_secret=%s&code=%s",CLIETN_ID,CLIENT_SECRET,fiobj_obj2cstr(fiobj_hash_get(h->params,code)).data);
        FIOBJ data = curl_request(uri,postData);
        char *access_token = strtok(fiobj_obj2cstr(fiobj_hash_get(data,fiobj_str_new("data",4))).data,"&");
        fiobj_free(data);
        if (access_token){
            char access_uri[512];
            sprintf(access_uri,"https://api.github.com/user?%s",access_token);

            FIOBJ user_info = curl_request(access_uri,NULL);

            if (fiobj_obj2num(fiobj_hash_get(user_info,fiobj_str_new("code",4)))==2) {

                //char *user = fiobj_obj2cstr(fiobj_hash_get(user_info,fiobj_str_new("data",4))).data;
                //fiobj_free(user_info);

                cJSON *profile = cJSON_Parse(fiobj_obj2cstr(fiobj_hash_get(user_info,fiobj_str_new("data",4))).data);
                cJSON *params = NULL;
                params = cJSON_GetObjectItem(profile, "message");
                if (params && strcasecmp(params->valuestring, "Requires authentication") == 0) {
                    http_response(h, 300, "需要授权");
                    fiobj_free(user_info);
                    return;
                }
                //addGithubUser((void*)user_info);
                //创建分离线程处理耗时任务
                if (fiobj_obj2cstr(fiobj_hash_get(user_info,fiobj_str_new("data",4))).len) {
                    http_response(h, 200, fiobj_obj2cstr(fiobj_hash_get(user_info,fiobj_str_new("data",4))).data);
                    pthread_t tid;
                    pthread_attr_t attr;
                    pthread_attr_init(&attr);
                    //设置线程为分离线程  线程结束时自动释放占用的资源
                    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                    pthread_create(&tid, &attr, addGithubUser, (void *)fiobj_obj2cstr(fiobj_hash_get(user_info,fiobj_str_new("data",4))).data);
                    void *res;
                    pthread_join(tid, &res);
                    fiobj_free(user_info);
                }
            }else{

                http_response(h, 300, "需要授权");
                return;
            }

        }
    }else{
        http_response(h,400,"登录错误");
    }

}

