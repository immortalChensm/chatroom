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
    FIOBJ method;

//    const char *type = fiobj_type_name(request->method);
//    printf("%s\n",type);
//
    //fio_str_info_s data = fiobj_obj2cstr(request->method);
//    printf("%s\n",data.data);
//
//    const char *type1 = fiobj_type_name(request->query);
//    printf("%s\n",type1);
//
//    fio_str_info_s data1 = fiobj_obj2cstr(request->query);
//    printf("%s\n",data1.data);
//
//    const char *type2 = fiobj_type_name(request->body);
//    printf("%s\n",type2);
//
//    fio_str_info_s body = fiobj_data_read(request->body, fiobj_data_len(request->body));
//    printf("%s\n",body.data);
//    char *data = "hello,world";
//    //创建字符串对象
//    FIOBJ test = fiobj_str_new(data,strlen(data));
////    FIOBJ test = fiobj_str_move(data,strlen(data), sizeof(data));
//    char *data1 = "中国";
//    //追加字符串
//    int len = fiobj_str_write(test,data1,strlen(data1));
//    //追加数值字符
//    len = fiobj_str_write_i(test,123456);
//
//    //获取数据类型
//    if(fiobj_type_is(test,FIOBJ_T_STRING)){
//        printf("这是字符串对象1");
//    }
//    if(FIOBJ_TYPE_IS(test,FIOBJ_T_STRING)){
//        printf("这是字符串对象2");
//    }
//
//    //将内容写入到字符串对象中
//    len = fiobj_str_printf(test,"%s-%d-%s\n","中国人",100,"美国人");
//    if (test){
//        fio_str_info_s data = fiobj_obj2cstr(test);
//        printf("%s-%d\n",data.data,data.len);
//    }
//
//    //创建一个缓存字符串对象
//    char *word = "打倒一切黑恶势力";
//    FIOBJ test1 = fiobj_str_buf(sizeof(word));
//    len = fiobj_str_write(test1,word,strlen(word));
//    va_list b="haha";
//    len = fiobj_str_vprintf(test1,"%s=%d\n",b);
//    fio_str_info_s datat = fiobj_obj2cstr(test1);
//    printf("%s-%d\n",datat.data,datat.len);

    //读取一个文件的内容
//    const char *file = "../web/about.html";
//    len = fiobj_str_readfile(test1,file,0,0);
//
//    //字符串对象拼接
//    char *test2_data = "6666";
//    FIOBJ test2 = fiobj_str_new(test2_data,strlen(test2_data));
//    len = fiobj_str_concat(test1,test2);
//    if (len){
//            fio_str_info_s datat = fiobj_obj2cstr(test1);
//            printf("%s-%d\n",datat.data,datat.len);
//    }
//
//    //得到字符串的hash值
//    uint64_t hash = fiobj_str_hash(test1);
//    uint64_t hash1 = fiobj_hash_string("bb",2);
//    printf("%ld,%ld\n",hash,hash1);
//
//    size_t si = fiobj_str_capa(test1);
//    printf("test1字符串的容量大小=%d\n",si);

    //重置字符串的容量
    //char *more = "我要加钱";
    //fiobj_str_resize(test1, sizeof(more));

    //冻结字符串防止更新


    //清空字符的内容
    //fiobj_str_clear(test1);
    //fio_str_info_s datat = fiobj_obj2cstr(test1);
   // printf("字符串清空后%s-%d\n",datat.data,datat.len);

    //释放不必要的内存
    //fiobj_str_compact(test1);

    FIOBJ data = fiobj_str_buf(300);
    char *text = "welcome to use it\n";
    int len = fiobj_str_write(data,text,strlen(text));
    printf("写进%d-%s\n",len,text);
    //冻结字符串
    fiobj_str_freeze(data);

    len = fiobj_str_write(data,text,strlen(text));
    printf("冻结后返回0=%d\n",len);

    //检测是否对象
    int is = fiobj_is_true(data);
    printf("is=%d\n",is);
    //释放内存
    fiobj_free(data);

    http_set_cookie(request, .name = "my_cookie", .name_len = 9, .value = "data",
    .value_len = 4);
    http_set_header(request, HTTP_HEADER_CONTENT_TYPE,
                    http_mimetype_find("txt", 3));
    http_set_header(request, HTTP_HEADER_X_DATA, fiobj_str_new("my data", 7));
    http_send_body(request, "Hello World!\r\n", 14);
}