#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uv.h"
#include "db_wrapper.h"

using namespace std;

struct Connect_Handle
{
	char* ip;
	int port;
	char* db_name;
	char* uname;
	char* pwd;
	void* context;
	void(*open_cb)(const char* err, void* context, void* udata);
	char* err;
	void* udata;
};
struct Mysql_Context
{
	MYSQL* sql_handle;
	int is_close;
	uv_mutex_t lock;
};
struct Query_Handle
{
	void* context;
	char* sql;
	void(*query_cb)(const char* err, MYSQL_RES* result,void* udata);
	char* err;
	void* udata;
	MYSQL_RES* result_vec;
};

void close_work_cb(uv_work_t* req)
{
	Mysql_Context* context = (Mysql_Context*)req->data;
	uv_mutex_lock(&context->lock);
	MYSQL* mysql_context = (MYSQL*)context->sql_handle;
	mysql_close(mysql_context);
	uv_mutex_unlock(&context->lock);
}
void query_work_cb(uv_work_t* req)
{
	Query_Handle* query_handle = (Query_Handle*)req->data;
	Mysql_Context* context = (Mysql_Context*)query_handle->context;
	uv_mutex_lock(&context->lock);
	int ret = mysql_query(context->sql_handle, query_handle->sql);
	if (ret != 0)
	{
		query_handle->err = strdup(mysql_error(context->sql_handle));
		query_handle->result_vec = NULL;
		uv_mutex_unlock(&context->lock);
		printf("%s\n", mysql_error(context->sql_handle));
		return;
	}
	query_handle->err = NULL;
	MYSQL_RES *result = mysql_store_result(context->sql_handle);
	query_handle->result_vec = result;
	uv_mutex_unlock(&context->lock);
}
void connect_work_cb(uv_work_t* req)
{
	Connect_Handle* handle = (Connect_Handle*)req->data;
	MYSQL* pConnect = mysql_init(NULL);
	MYSQL* sql = mysql_real_connect(pConnect, handle->ip, handle->uname, handle->pwd, handle->db_name, handle->port, NULL, 0);
	
	if (sql!=NULL)
	{
		Mysql_Context* context = (Mysql_Context*)malloc(sizeof(Mysql_Context));
		memset(context, 0, sizeof(Mysql_Context));
		context->sql_handle = sql;
		handle->context = context;
		uv_mutex_init(&context->lock);
	}
	else
	{
		handle->context = NULL;
		handle->err = strdup(mysql_error(sql));
	}
}

void close_after_cb(uv_work_t* req, int status)
{
	Mysql_Context* mysql_context = (Mysql_Context*)req->data;
	if (mysql_context)
	{
		free(mysql_context);
	}

	free(req);
}

void query_after_cb(uv_work_t* req, int status)
{
	Query_Handle* query_handle = (Query_Handle*)req->data;
	query_handle->query_cb(query_handle->err, query_handle->result_vec,query_handle->udata);
	if (query_handle->err)
	{
		free(query_handle->err);
	}
	if (query_handle->sql)
	{
		free(query_handle->sql);
	}
	/*if (query_handle->context)
	{
		free(query_handle->context);
	}*/
	if (query_handle->result_vec)
	{
		mysql_free_result(query_handle->result_vec);
		query_handle->result_vec = NULL;
	}
	free(query_handle);
	free(req);

}

void connect_after_cb(uv_work_t* req,int status)
{
	Connect_Handle* handle = (Connect_Handle*)req->data;
	handle->open_cb(handle->err, handle->context,handle->udata);
	if (handle->ip)
	{
		free(handle->ip);
	}
	if (handle->db_name)
	{
		free(handle->db_name);
	}
	if (handle->pwd)
	{
		free(handle->pwd);
	}
	if (handle->uname)
	{
		free(handle->uname);
	}
	if (handle->err)
	{
		free(handle->err);
	}
	/*if (handle->context)
	{
		free(handle->context);
	}*/
	free(handle);
	free(req);
}

void mysql_wrapper::connect(char* ip, int port, char* db_name, char* uname, char* pwd, 
	void(*open_cb)(const char* err, void* context,void* udata), void* udata)
{
	uv_work_t* work = (uv_work_t*)malloc(sizeof(uv_work_t));
	memset(work, 0, sizeof(uv_work_t));
	Connect_Handle* handle = (Connect_Handle*)malloc(sizeof(Connect_Handle));
	memset(handle, 0, sizeof(Connect_Handle));

	handle->ip = strdup(ip);
	handle->port = port;
	handle->db_name = strdup(db_name);
	handle->uname = strdup(uname);
	handle->pwd = strdup(pwd);
	handle->open_cb = open_cb;
	handle->udata = udata;
	work->data = handle;
	uv_queue_work(uv_default_loop(), work, connect_work_cb, connect_after_cb);
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void mysql_wrapper::close(void* context)
{
	Mysql_Context* mysql_context = (Mysql_Context*)context;
	if (mysql_context->is_close)
	{
		return;
	}
	uv_work_t* work = (uv_work_t*)malloc(sizeof(uv_work_t));
	memset(work, 0, sizeof(uv_work_t));

	mysql_context->is_close = 1;
	work->data = mysql_context;
	uv_queue_work(uv_default_loop(), work, close_work_cb, close_after_cb);

}

void mysql_wrapper::query(void* context, char* sql, void(*query_cb)(const char* err, MYSQL_RES* result,void* udata),void* udata)
{
	Mysql_Context* mysql_context = (Mysql_Context*)context;
	if (mysql_context->is_close)
	{
		return;
	}
	uv_work_t* work = (uv_work_t*)malloc(sizeof(uv_work_t));
	memset(work, 0, sizeof(uv_work_t));
	Query_Handle* query_handle = (Query_Handle*)malloc(sizeof(Query_Handle));
	memset(query_handle, 0, sizeof(Query_Handle));
	query_handle->context = mysql_context;
	query_handle->sql = strdup(sql);
	query_handle->query_cb = query_cb;
	query_handle->udata = udata;
	work->data = query_handle;
	uv_queue_work(uv_default_loop(), work, query_work_cb, query_after_cb);
}