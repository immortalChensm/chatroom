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
#include "../model/db/database.c"

static void indexAction(request *req)
{
    http_s *h = req->h;
    //http_parse_body(h);
    http_parse_query(h);
    char *fields[] = {"name","email"};
    char *value[] = {fiobj_obj2cstr(fiobj_hash_get(h->params,fiobj_str_new("name",4))).data,"shaodiandian@qq.com"};
    //printf("%s\n",value[0]);
    //printf(fiobj_obj2cstr(fiobj_hash_get(h->params,fiobj_str_new("name",4))).data);
    if (fiobj_type_is(h->params,FIOBJ_T_HASH)){
        if (fiobj_hash_haskey(h->params,fiobj_str_new("name",4))){
//            printf("name=%s\n",fiobj_obj2cstr(fiobj_hash_get(h->params,fiobj_str_new("name",4))).data);
//            printf("query=%s\n",fiobj_obj2cstr(h->query).data);
            //printf("query=%s\n",value[0]);
            //db("users")->where("`userId`=76")->update((table_data){.fields=fields,.value=value,.rows=2});

//            db("users")->where("1")->selects("userId,name,email",(limit){.start=1,.end=10});
            //db("users")->where("`userId`<20 AND name='老油条'")->selects("userId,name",(limit){.start=10});
            FIOBJ result;
            //result = db("users")->where("`userId`<20 AND `userId`>1")->selects("userId,name",(limit){.start=20});
            result = db(NULL)->query("select * from users limit 10",4);
            //db(NULL)->query("insert into users(name,email) values('shao','1655664358@qq.com')",1);
            //printf("result=%d\n",fiobj_type_is(result,FIOBJ_T_HASH));
            FIOBJ ret = fiobj_hash_get(result,fiobj_str_new("result",6));
            printf("%d\n",fiobj_obj2num(ret));

            FIOBJ columns = fiobj_hash_get(result,fiobj_str_new("columns",7));
            FIOBJ temp;
            if (fiobj_type_is(columns,FIOBJ_T_ARRAY)){
                for (int j = 0; j <fiobj_ary_count(columns) ; ++j) {
                    temp = fiobj_ary_index(columns,j);
                    printf("%s\t",fiobj_obj2cstr(temp).data);
                }
            }
            printf("\n");
            FIOBJ rows = fiobj_hash_get(result,fiobj_str_new("rows",4));
            if (fiobj_type_is(rows,FIOBJ_T_ARRAY)){
                for (int j = 0; j <fiobj_ary_count(rows) ; ++j) {
                    for (int k = 0; k <fiobj_ary_count(fiobj_ary_index(rows,j)) ; ++k) {
                        temp = fiobj_ary_index(fiobj_ary_index(rows,j),k);
                        printf("%s\n",fiobj_obj2cstr(temp).data);
                    }
                }
            }
//
//            FIOBJ ary = fiobj_ary_new();
//            fiobj_ary_push(ary,fiobj_str_new("hello",5));
//            fiobj_ary_push(ary,fiobj_str_new("world",5));
//
//            FIOBJ row1 = fiobj_ary_new();
//            fiobj_ary_push(row1,fiobj_str_new("china",5));
//            fiobj_ary_push(row1,fiobj_str_new("japanese",8));
//
//            fiobj_ary_push(ary,row1);
//            for (int l = 0; l < fiobj_ary_count(ary); ++l) {
//                FIOBJ data = fiobj_ary_index(ary,l);
//                if (fiobj_type_is(data,FIOBJ_T_ARRAY)){
//                    for (int j = 0; j < fiobj_ary_count(data); ++j) {
//                        FIOBJ child = fiobj_ary_index(data,j);
//                        printf("%s\n",fiobj_obj2cstr(child).data);
//                    }
//                }else{
//                    printf("%s\n",fiobj_obj2cstr(data).data);
//                }
//
//            }


            //printf("%s\n",fiobj_obj2cstr(data).data);
            //printf("%s\n",fiobj_obj2cstr(data).data);

            //free(result);
            //db("users")->where("`userId`=78")->delete(NULL);
            //db("users")->insert((table_data){.fields=fields,.value=value,.rows=2});
            //db("users")->where("`userId`=1 AND name='老油条'")->selects("userId,name,email",(limit){.start=10});
            //db("users")->selects("userId,name,email",(limit){.start=0,.end=0});
            //int line[0] = {10};
            //db("users")->where("1")->selects("userId,name,email",line);

        }
    }
    //insert("users",(table_data){.fields=fields,.value=value});
    //delete("users","`id`='jack'");
    //where("id=100");
    //update("users",(table_data){.fields=fields,.value=value});
    //where("`id`=1")->update("users",(table_data){.fields=fields,.value=value}));
    //db("users")->where("id=1")->update((table_data){.fields=fields,.value=value});
    //db("users")->insert((table_data){.fields=fields,.value=value,.rows=2});
    //db("users")->delete("`userId`=77");
    //db("users")->where("`userId`=76")->update((table_data){.fields=fields,.value=value,.rows=2});
//    db("users")->delete("`id`=1 AND name='jack'");
    //db("users")->where("`id`=1 AND `name`='tom'")->delete(NULL);
    http_send_body(h,"hello,japanese",14);
//    create_tcp_client("www.999999.live",9501,"api/contact/us","GET",NULL);
    //create_tcp_client("www.999999.live",9501,"api/community/hot","POST","start=1&count=2");
//    curl_request("http://47.110.136.62/index.php/community",NULL);
    //char *data = curl_request("http://www.999999.live",NULL);
    //printf("%s\n",data);
    //db();

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


static void gitLoginAction(request *req)
{
    http_s *h = req->h;
    http_parse_query(h);
    FIOBJ code = fiobj_str_new("code",4);
    if(fiobj_hash_haskey(h->params,code)){
        printf("%s\n",fiobj_obj2cstr(fiobj_hash_get(h->params,code)).data);
        char *uri = "https://github.com/login/oauth/access_token";
        char postData[1024];
        sprintf(postData,"client_id=%s&client_secret=%s&code=%s",CLIETN_ID,CLIENT_SECRET,fiobj_obj2cstr(fiobj_hash_get(h->params,code)).data);
        char *data = curl_request(uri,postData);
        char *access_token = strtok(data,"&");
        if (access_token){
            char access_uri[512];
            sprintf(access_uri,"https://api.github.com/user?%s",access_token);
            printf("access_uri=%s\n",access_uri);
            char *user_info = curl_request(access_uri,NULL);
            printf("user_info=%s\n",user_info);
            http_set_header(h,fiobj_str_new("Content-Type",12),fiobj_str_new("text/json",9));
            http_send_body(h,user_info,strlen(user_info));
        }
    }
    printf("%s\n",fiobj_obj2cstr(h->query).data);
    http_send_body(h,"hello,test",10);
}