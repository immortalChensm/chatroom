//
// Created by 1655664358@qq.com on 2019/12/14.
//

#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>
#include "database.h"
typedef struct table_data{
    FIOBJ fields;
    FIOBJ value;
}table_data;
typedef struct limit{
    int start;
    int end;
}limit;

typedef enum {INSERT=1,DELETE,UPDATE,SELECT}query_type;

typedef struct database{
   int (*insert)(table_data data);
   FIOBJ (*selects)(char *fields,limit rows);
   FIOBJ (*query)(char *sql_string,query_type type);
   int (*update)(table_data data);
   int (*delete)(char *data);
   struct database *(*where)(char *condition);
   FIOBJ (*first)();
   FIOBJ (*value)(char *field);
   struct database *(*orderBy)(char *order);

   FIOBJ tableName;
   FIOBJ sql;
   FIOBJ condition;
   FIOBJ order;
}model;
model DB;

typedef struct database_config
{
    char *host;
    char *user;
    char *pwd;
    char *db;
    int port;
}database_config;
database_config config;

MYSQL           mysql;
MYSQL_RES       *res = NULL;
MYSQL_ROW       row;
MYSQL_FIELD *fields_col;
char            *query_str = NULL;
int             rc, i,fields_num;
int             rows_field;
static int mysqlHandle(MYSQL *mysql);

static void get_mysql_config()
{
    FILE *f;long len;char *data;
    char *route_file = "../config/database.json";
    if(access(route_file,F_OK)==-1){
        fprintf(stderr,"database.json is not existing,please create it and add some config^_^from 勺颠颠\n");
        return;
    }
    f=fopen(route_file,"rb");
    if (f){
        fseek(f,0,SEEK_END);
        len=ftell(f);
        fseek(f,0,SEEK_SET);
        data=(char*)malloc(len+1);
        fread(data,1,len,f);
        if (data){
            cJSON *config=cJSON_Parse(data);
            if (!config) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
            cJSON *params = cJSON_GetObjectItem(config,"host");
            printf("%s\n",params->valuestring);
        }
    }else{
        fprintf(stderr,"database.json can not read,please create it and add some config^_^from 勺颠颠\n");
        return;
    }

    fclose(f);
}

FIOBJ value(char *field)
{
    FIOBJ data = fiobj_hash_new();
    FIOBJ sql = fiobj_str_tmp();
    if (fiobj_obj2cstr(DB.tableName).len==0){
        fprintf(stderr,"table not exist\n");
        fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
        return data;
    }
    if (fiobj_obj2cstr(DB.tableName).len&&fiobj_obj2cstr(DB.condition).len){
        fiobj_str_printf(sql,"SELECT %s FROM %s WHERE %s LIMIT 1",field,fiobj_obj2cstr(DB.tableName).data,fiobj_obj2cstr(DB.condition).data);
        DB.sql = fiobj_str_copy(sql);
    }else{
        fiobj_str_printf(sql,"SELECT %s FROM %s LIMIT 1",field,fiobj_obj2cstr(DB.tableName).data);
        DB.sql = fiobj_str_copy(sql);
    }
    if(mysqlHandle(&mysql)){
        rc = mysql_real_query(&mysql, fiobj_obj2cstr(DB.sql).data, fiobj_obj2cstr(DB.sql).len);
        fiobj_free(DB.condition);
        fiobj_free(DB.tableName);
        fiobj_free(DB.sql);
        if (0 != rc) {
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
            return data;
        }
        res = mysql_store_result(&mysql);
        if (NULL == res) {
            printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
            fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
            return data;
        }
        row = mysql_fetch_row(res);//取一行数据
        if (row){
            fiobj_hash_set(data,fiobj_str_new("data",4),fiobj_str_new(row[0],strlen(row[0])));
            fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(1));
            return data;
        }else{
            fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
            return data;
        }

    }
}

FIOBJ first()
{
    FIOBJ data = fiobj_hash_new();
    FIOBJ sql = fiobj_str_tmp();
    if (fiobj_obj2cstr(DB.tableName).len==0){
        fprintf(stderr,"table not exist\n");
        fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
        return data;
    }
    if (fiobj_obj2cstr(DB.tableName).len&&fiobj_obj2cstr(DB.condition).len){
        fiobj_str_printf(sql,"SELECT * FROM %s WHERE %s LIMIT 1",fiobj_obj2cstr(DB.tableName).data,fiobj_obj2cstr(DB.condition).data);
        DB.sql = fiobj_str_copy(sql);
    }else{
        fiobj_str_printf(sql,"SELECT * FROM %s LIMIT 1",fiobj_obj2cstr(DB.tableName).data);
        DB.sql = fiobj_str_copy(sql);
    }
    if(mysqlHandle(&mysql)){
        rc = mysql_real_query(&mysql, fiobj_obj2cstr(DB.sql).data, fiobj_obj2cstr(DB.sql).len);
        fiobj_free(DB.condition);
        fiobj_free(DB.tableName);
        fiobj_free(DB.sql);
        if (0 != rc) {
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
            return data;
        }
        res = mysql_store_result(&mysql);
        if (NULL == res) {
            printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
            fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
            return data;
        }
        fields_col = mysql_fetch_fields(res);//获取列字段数组
        fields_num = mysql_num_fields(res);//获取列数
        rows_field = mysql_num_rows(res);//获取行数

        FIOBJ columns = fiobj_ary_new();
        FIOBJ rows_data = fiobj_ary_new();
        for(int i=0;i<fields_num;i++){
            fiobj_ary_push(columns,fiobj_str_new(fields_col[i].name,strlen(fields_col[i].name)));
        }
        //列数组
        fiobj_hash_set(data,fiobj_str_new("columns",7),columns);
        row = mysql_fetch_row(res);//取一行数据
        for (i = 0; i <fields_num; i++) {
            fiobj_ary_push(rows_data,fiobj_str_new(row[i],strlen(row[i])));
        }
        //行数组
        fiobj_hash_set(data,fiobj_str_new("rows",4),rows_data);
        fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(1));
        return data;
    }
}

FIOBJ query(char *sql_string,query_type type)
{
    //get_mysql_config();

    FIOBJ sql = fiobj_str_tmp();
    FIOBJ data = fiobj_hash_new();
    fiobj_str_printf(sql,"%s",sql_string);

    DB.sql = fiobj_str_copy(sql);
    if(mysqlHandle(&mysql)){
        rc = mysql_real_query(&mysql, fiobj_obj2cstr(DB.sql).data, fiobj_obj2cstr(DB.sql).len);
        fiobj_free(DB.sql);
        fiobj_free(sql);
        if (0 != rc) {
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
            return data;
        }
        switch (type){
            case INSERT://insert
                rows_field = mysql_affected_rows(&mysql);
                if (rows_field){
                    fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(mysql_insert_id(&mysql)));
                }else{
                    fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
                }
                break;
            case DELETE://delete
                rows_field = mysql_affected_rows(&mysql);
                if (rows_field){
                    fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(1));
                }else{
                    printf("mysql_real_query(): %s\n", mysql_error(&mysql));
                    fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
                }
                break;
            case UPDATE://update
                rows_field = mysql_affected_rows(&mysql);
                if (rows_field){
                    fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(1));

                }else{
                    printf("mysql_real_query(): %s\n", mysql_error(&mysql));
                    fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
                }
                break;
            case SELECT://select
                res = mysql_store_result(&mysql);
                if (NULL == res) {
                    printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
                    fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
                }

                fields_col = mysql_fetch_fields(res);//获取列字段数组
                fields_num = mysql_num_fields(res);//获取列数
                rows_field = mysql_num_rows(res);//获取行数

                FIOBJ columns = fiobj_ary_new();
                FIOBJ rows_data = fiobj_ary_new();
                for(int i=0;i<fields_num;i++){
                    fiobj_ary_push(columns,fiobj_str_new(fields_col[i].name,strlen(fields_col[i].name)));
                }
                fiobj_hash_set(data,fiobj_str_new("columns",7),columns);
                int k=0;
                while ((row = mysql_fetch_row(res))) {
                    FIOBJ temp = fiobj_ary_new();//每一行
                    for (i = 0; i <fields_num; i++) {
                        fiobj_ary_push(temp,fiobj_str_new(row[i],strlen(row[i])));
                    }
                    fiobj_ary_push(rows_data,temp);
                    k++;

                }
                k=0;
                fiobj_hash_set(data,fiobj_str_new("rows",4),rows_data);
                fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(1));
                break;
        }
        return data;

    }
}
FIOBJ selects(char *fields,limit rows)
{
    //select field from table where condition limit x,y
    FIOBJ data = fiobj_hash_new();
    if (fiobj_obj2cstr(DB.tableName).len==0){
        fprintf(stderr,"table not exist\n");
        fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
        return data;
    }
    FIOBJ sql = fiobj_str_tmp();
    if (fiobj_obj2cstr(DB.tableName).len&&fiobj_obj2cstr(DB.condition).len&&fiobj_obj2cstr(DB.order).len&&rows.start&&rows.end){
        fiobj_str_printf(sql,"SELECT %s FROM %s WHERE %s ORDER BY %s LIMIT %d,%d",fields,fiobj_obj2cstr(DB.tableName).data,fiobj_obj2cstr(DB.condition).data,fiobj_obj2cstr(DB.order).data,rows.start,rows.end);
        DB.sql = fiobj_str_copy(sql);
    }else if (fiobj_obj2cstr(DB.tableName).len&&fiobj_obj2cstr(DB.condition).len&&rows.start){
        fiobj_str_printf(sql,"SELECT %s FROM %s WHERE %s LIMIT %d",fields,fiobj_obj2cstr(DB.tableName).data,fiobj_obj2cstr(DB.condition).data,rows.start);
        DB.sql = fiobj_str_copy(sql);
    }else if (fiobj_obj2cstr(DB.tableName).len&&fiobj_obj2cstr(DB.condition).len){
        fiobj_str_printf(sql,"SELECT %s FROM %s WHERE %s",fields,fiobj_obj2cstr(DB.tableName).data,fiobj_obj2cstr(DB.condition).data);
        DB.sql = fiobj_str_copy(sql);
    }else{
        fiobj_str_printf(sql,"SELECT %s FROM %s ",fields,fiobj_obj2cstr(DB.tableName).data);
        DB.sql = fiobj_str_copy(sql);
    }

    if(mysqlHandle(&mysql)){
        rc = mysql_real_query(&mysql, fiobj_obj2cstr(DB.sql).data, fiobj_obj2cstr(DB.sql).len);
        fiobj_free(DB.condition);
        fiobj_free(DB.tableName);
        fiobj_free(DB.sql);
        if (0 != rc) {
            printf("mysql_real_query(): %s\n", mysql_error(&mysql));
            fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
            return data;
        }
        res = mysql_store_result(&mysql);
        if (NULL == res) {
            printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
            fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(-1));
            return data;
        }
        fields_col = mysql_fetch_fields(res);//获取列字段数组
        fields_num = mysql_num_fields(res);//获取列数
        rows_field = mysql_num_rows(res);//获取行数

        FIOBJ columns = fiobj_ary_new();
        FIOBJ rows_data = fiobj_ary_new();
        for(int i=0;i<fields_num;i++){
            fiobj_ary_push(columns,fiobj_str_new(fields_col[i].name,strlen(fields_col[i].name)));
        }
        fiobj_hash_set(data,fiobj_str_new("columns",7),columns);
        int k=0;
        while ((row = mysql_fetch_row(res))) {
            FIOBJ temp = fiobj_ary_new();//每一行
            for (i = 0; i <fields_num; i++) {
                fiobj_ary_push(temp,fiobj_str_new(row[i],strlen(row[i])));
            }
            fiobj_ary_push(rows_data,temp);
            k++;

        }
        fiobj_hash_set(data,fiobj_str_new("rows",4),rows_data);
        fiobj_hash_set(data,fiobj_str_new("result",6),fiobj_num_new(1));
        k=0;
        return data;
    }

}
int insert(table_data data)
{
    int i=0;
    FIOBJ sql = fiobj_str_buf(1);
    FIOBJ fields = fiobj_str_buf(1);
    FIOBJ val = fiobj_str_buf(1);
    FIOBJ temp_field;
    FIOBJ temp_value;
    int ret;
    if (fiobj_obj2cstr(DB.tableName).len){
        for (i;i<fiobj_ary_count(data.fields);i++){
            //字段处理
            temp_field = fiobj_ary_index(data.fields,i);
            fiobj_str_write(fields,"`",1);
            fiobj_str_write(fields,fiobj_obj2cstr(temp_field).data,fiobj_obj2cstr(temp_field).len);
            fiobj_str_write(fields,"`",1);
            if (i<fiobj_ary_count(data.fields)-1){
                fiobj_str_write(fields,",",1);
            }
            //字段的值处理
            temp_value = fiobj_hash_get(data.value,temp_field);
            fiobj_str_write(val,"'",1);

//            mysql_real_escape_string_quote(&mysql,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).len,'\'');
  //          mysql_real_escape_string_quote(&mysql,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).len,'\"');
    //        mysql_real_escape_string_quote(&mysql,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).len,'`');
      //      mysql_real_escape_string_quote(&mysql,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).len,'/');
        //    mysql_real_escape_string_quote(&mysql,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).len,'<');
          //  mysql_real_escape_string_quote(&mysql,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).len,'>');


            fiobj_str_write(val,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).len);
            fiobj_str_write(val,"'",1);
            if (i<fiobj_ary_count(data.fields)-1){
                fiobj_str_write(val,",",1);
            }
        }

        fiobj_str_printf(sql,"INSERT INTO %s(%s) VALUES(%s)",fiobj_obj2cstr(DB.tableName).data,fiobj_obj2cstr(fields).data,fiobj_obj2cstr(val).data);
        DB.sql = fiobj_str_copy(sql);
        printf("%s\n",fiobj_obj2cstr(sql).data);
        if(mysqlHandle(&mysql)){
            rc = mysql_real_query(&mysql, fiobj_obj2cstr(DB.sql).data, fiobj_obj2cstr(DB.sql).len);

            fiobj_free(DB.condition);
            fiobj_free(DB.tableName);
            fiobj_free(DB.sql);
            fiobj_free(sql);
            fiobj_free(fields);
            fiobj_free(val);
            fiobj_free(temp_value);
            fiobj_free(temp_field);

            if (0 != rc) {
                printf("mysql_real_query(): %s\n", mysql_error(&mysql));
                return -1;
            }
            rows_field = mysql_affected_rows(&mysql);
            if (rows_field){
                return mysql_insert_id(&mysql);
            }
        }
    }
}

model *where(char *condition)
{
    if (condition){
        DB.condition = fiobj_str_copy(fiobj_str_new(condition,strlen(condition)));
    }
    return &DB;
}


model *orderBy(char *order)
{
    if (order){
        DB.order = fiobj_str_copy(fiobj_str_new(order,strlen(order)));
    }
    return &DB;
}

int delete(char *condition)
{
    FIOBJ sql = fiobj_str_tmp();
    if (fiobj_obj2cstr(DB.tableName).len&&condition){
        fiobj_str_printf(sql,"DELETE FROM %s WHERE %s",fiobj_obj2cstr(DB.tableName).data,condition);
        DB.sql = fiobj_str_copy(sql);
    }else if (fiobj_obj2cstr(DB.tableName).len&&DB.condition){
        fiobj_str_printf(sql,"DELETE FROM %s WHERE %s",fiobj_obj2cstr(DB.tableName).data,fiobj_obj2cstr(DB.condition).data);
        DB.sql = fiobj_str_copy(sql);
    }else if (fiobj_obj2cstr(DB.tableName).len){
        fiobj_str_printf(sql,"DELETE FROM %s",fiobj_obj2cstr(DB.tableName).data);
        DB.sql = fiobj_str_copy(sql);
    }
    if(fiobj_obj2cstr(DB.sql).len){
        if(mysqlHandle(&mysql)){
            rc = mysql_real_query(&mysql, fiobj_obj2cstr(DB.sql).data, fiobj_obj2cstr(DB.sql).len);
            fiobj_free(DB.condition);
            fiobj_free(DB.tableName);
            fiobj_free(DB.sql);
            fiobj_free(sql);
            if (0 != rc) {
                printf("mysql_real_query(): %s\n", mysql_error(&mysql));
                return -1;
            }
            rows_field = mysql_affected_rows(&mysql);
            if (rows_field){
                return 1;
            }else{
                printf("mysql_real_query(): %s\n", mysql_error(&mysql));
                return -1;
            }
        }
    }
}

int update(table_data data)
{
    FIOBJ sql = fiobj_str_tmp();
    FIOBJ update = fiobj_str_buf(sizeof(data));
    FIOBJ temp_field;
    FIOBJ temp_value;
    int i=0;
    if (fiobj_obj2cstr(DB.tableName).len){
        for (i;i<fiobj_ary_count(data.fields);i++){
            //字段
            temp_field = fiobj_ary_index(data.fields,i);

            fiobj_str_write(update,fiobj_obj2cstr(temp_field).data,fiobj_obj2cstr(temp_field).len);
            fiobj_str_write(update,"='",2);
            //字段的值处理
            temp_value = fiobj_hash_get(data.value,temp_field);
            mysql_real_escape_string_quote(&mysql,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).len,'\'');
            fiobj_str_write(update,fiobj_obj2cstr(temp_value).data,fiobj_obj2cstr(temp_value).len);
            fiobj_str_write(update,"'",1);
            if (i<fiobj_ary_count(data.fields)-1){
                fiobj_str_write(update,",",1);
            }

        }
        if (fiobj_obj2cstr(DB.condition).len>0){
            fiobj_str_printf(sql,"UPDATE %s SET %s WHERE %s",fiobj_obj2cstr(DB.tableName).data,fiobj_obj2cstr(update).data,fiobj_obj2cstr(DB.condition).data);
        }else{
            fprintf(stderr,"update condition params must be given\n");
            return -1;
        }

        DB.sql = fiobj_str_copy(sql);
        if(fiobj_obj2cstr(DB.sql).len){
            if(mysqlHandle(&mysql)){
                rc = mysql_real_query(&mysql, fiobj_obj2cstr(DB.sql).data, fiobj_obj2cstr(DB.sql).len);
                fiobj_free(DB.condition);
                fiobj_free(DB.tableName);
                fiobj_free(DB.sql);
                fiobj_free(sql);
                fiobj_free(update);
                fiobj_free(temp_value);
                fiobj_free(temp_field);
                if (0 != rc) {
                    printf("mysql_real_query(): %s\n", mysql_error(&mysql));
                    return -1;
                }
                rows_field = mysql_affected_rows(&mysql);
                if (rows_field){
                    return 1;
                }else{
                    printf("mysql_real_query(): %s\n", mysql_error(&mysql));
                    return -1;
                }
            }

        }
    }
}

static int mysqlHandle(MYSQL *mysql)
{

    if (mysql_ping(&*mysql)==0){
        //printf("不需要重新连接了，直接返回\n");
        return 1;
    }
    if (NULL == mysql_init(&*mysql)) {
        printf("mysql_init(): %s\n", mysql_error(&*mysql));
        return -1;
    }
    if (NULL == mysql_real_connect(&*mysql,
                                   "localhost",
                                   "sdfdsf",
                                   "sdfds*",
                                   "sdf",
                                   0,
                                   NULL,
                                   0)) {
        printf("mysql_real_connect(): %s\n", mysql_error(&*mysql));
        return -1;
    }
    my_bool reconnect = 0;
    mysql_options(&*mysql, MYSQL_OPT_RECONNECT, &reconnect);
    printf("1. Connected MySQL successful! \n");
    query_str = "set NAMES utf8mb4";
    rc = mysql_real_query(&*mysql, query_str, strlen(query_str));
    if (0 != rc) {
        printf("charset error: %s\n", mysql_error(&*mysql));
        return -1;
    }
//    printf("1. charset successful! \n");
//    res = mysql_store_result(&mysql);
//    if (NULL == res) {
//        printf("mysql_restore_result(): %s\n", mysql_error(&mysql));
//        return -1;
//    }
//    rows = mysql_num_rows(res);
//    printf("The total rows is: %d\n", rows);
//    fields = mysql_num_fields(res);
//    printf("The total fields is: %d\n", fields);
//    while ((row = mysql_fetch_row(res))) {
//        for (i = 0; i < fields; i++) {
//            printf("%s\t", row[i]);
//        }
//        printf("\n");
//    }
    //mysql_close(&mysql);
    return 1;
}

model *db(char *table)
{
    if (table){
        DB.tableName = fiobj_str_copy(fiobj_str_new(table,strlen(table)));
    }
    DB.where=where;
    DB.update = update;
    DB.insert = insert;
    DB.delete = delete;
    DB.selects = selects;
    DB.orderBy = orderBy;
    DB.query = query;
    DB.first = first;
    DB.value = value;
    return &DB;
}
