//
// Created by 1655664358@qq.com on 2019/12/3.
//

#include "http.h"


void on_request(http_s *request);


FIOBJ HTTP_HEADER_X_DATA;



int main(void) {

    HTTP_HEADER_X_DATA = fiobj_str_new("X-Data", 6);

    http_listen("2347", NULL, .on_request = on_request, .max_clients=10240,.log = 1,.public_folder="../web");

    fio_start(.threads = 2,.workers=3);

    fiobj_free(HTTP_HEADER_X_DATA);
}


void on_request(http_s *request) {


    //创建一个IO流
//    FIOBJ io = fiobj_data_newstr();
//    printf("io=%d\n",io);
//
//    //往IO流写入数据
//    int len = fiobj_data_write(io,fiobj_obj2cstr(request->body).data,fiobj_obj2cstr(request->body).len);
//    printf("len=%d\n",len);
//
//    //IO流长度
//    len = fiobj_data_len(io);
//    printf("len=%d\n",len);
//    //fio_str_info_s data = fiobj_data_read(io,len);
////    fio_str_info_s data = fiobj_data_read2ch(io,2);
//    //读取IO流内容
//    fio_str_info_s data = fiobj_data_read(io,0);
//    printf("body=%s,len=%d\n",data.data,data.len);
//
//    //IO流当前位置
//    int pos = fiobj_data_pos(io);
//    printf("pos=%d\n",pos);
//
//    len = fiobj_data_len(io);
//    printf("io len=%d\n",len);
//
//    //指定IO流位置
//    fiobj_data_seek(io,10);
//    data = fiobj_data_read(io,0);
//    printf("seek read content=%s\n",data.data);
//
//    data = fiobj_data_pread(io,0,fiobj_data_len(io));
//    printf("pread=%s\n",data.data);

//    FIOBJ io = fiobj_data_newstr();
//    char *content = "hello,world\n";
//    int len = fiobj_data_write(io,content,strlen(content));
//    printf("len=%d\n",len);
//    char *word = "你好，世界\n";
//    len = fiobj_data_puts(io,word,strlen(word));
//
//    fio_str_info_s data = fiobj_data_read(io,0);
//    printf("%s\n",data.data);

    FIOBJ lio = fiobj_data_newtmpfile();
    char *content = "hello,world";
    FIOBJ str = fiobj_str_new(content,strlen(content));
    int len = fiobj_data_write(lio,fiobj_obj2cstr(str).data,fiobj_obj2cstr(str).len);
    printf("len=%d\n",len);

    FIOBJ slicio = fiobj_data_slice(lio,0,3);
    fio_str_info_s data = fiobj_data_read(slicio,0);
    printf("%s\n",data.data);

    data = fiobj_data_gets(lio);
    printf("data=%s\n",data.data);


    http_set_cookie(request, .name = "my_cookie", .name_len = 9, .value = "data",
    .value_len = 4);
    http_set_header(request, HTTP_HEADER_CONTENT_TYPE,
                    http_mimetype_find("txt", 3));
    http_set_header(request, HTTP_HEADER_X_DATA, fiobj_str_new("my data", 7));

    printf("query=%s\n",fiobj_obj2cstr(request->query).data);
    http_send_body(request, "Hello World!\r\n", 14);
}