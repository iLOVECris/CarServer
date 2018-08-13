#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <hiredis.h>
#ifdef WIN32
#define NO_QFORKIMPL //这一行必须加才能正常使用
#include <Win32_Interop/win32fixes.h>
#pragma comment(lib,"hiredis.lib")
#pragma comment(lib,"Win32_Interop.lib")
#endif

#include "redis_wrapper.h"
#include "uv.h"
using namespace std;

struct Connect_Handle
{
	char* ip;
	int port;
	void* context;
	void(*open_cb)(const char* err, void* context,void* udata);
	char* err;
	void* udata;
};
struct Redis_Context
{
	redisContext* redis_handle;
	int is_close;
	uv_mutex_t lock;
};
struct Query_Handle
{
	void* context;
	char* cmd;
	void(*query_cb)(const char* err, redisReply* result,void* udata);
	char* err;
	void* udata;
	redisReply* result;
};

void RedisClose_work_cb(uv_work_t* req)
{
	Redis_Context* redis_context = (Redis_Context*)req->data;
	uv_mutex_lock(&redis_context->lock);
	redisFree(redis_context->redis_handle);
	redis_context->redis_handle = NULL;
	uv_mutex_unlock(&redis_context->lock);
}
void RedisQuery_work_cb(uv_work_t* req)
{
	Query_Handle* query_handle = (Query_Handle*)req->data;
	Redis_Context* context = (Redis_Context*)query_handle->context;
	uv_mutex_lock(&context->lock);
	query_handle->err = NULL;
	redisReply* reply_0 = (redisReply*)redisCommand(context->redis_handle, query_handle->cmd);
	query_handle->result = reply_0;
	uv_mutex_unlock(&context->lock);
}
void RedisConnect_work_cb(uv_work_t* req)
{
	Connect_Handle* handle = (Connect_Handle*)req->data;
	
	struct timeval timeout = { 5, 0 }; // 1.5 seconds
	redisContext* c = redisConnectWithTimeout((char*)handle->ip, handle->port, timeout);
	if (c->err)
	{
		handle->context = NULL;
		handle->err = strdup(c->errstr);
		redisFree(c);
	}
	else
	{
		Redis_Context* context = (Redis_Context*)malloc(sizeof(Redis_Context));
		memset(context, 0, sizeof(Redis_Context));
		context->redis_handle = c;
		handle->context = context;
		handle->err = NULL;
		uv_mutex_init(&context->lock);
	}
}

void RedisClose_after_cb(uv_work_t* req, int status)
{
	Redis_Context* mysql_context = (Redis_Context*)req->data;
	free(mysql_context);
	free(req);
	printf("close redis");
}

void RedisQuery_after_cb(uv_work_t* req, int status)
{
	Query_Handle* query_handle = (Query_Handle*)req->data;
	query_handle->query_cb(query_handle->err, query_handle->result,query_handle->udata);
	if (query_handle->err)
	{
		free(query_handle->err);
	}
	if (query_handle->cmd)
	{
		free(query_handle->cmd);
	}
	if (query_handle->result)
	{
		delete query_handle->result;
	}
	free(query_handle);
	free(req);

}

void RedisConnect_after_cb(uv_work_t* req,int status)
{
	Connect_Handle* handle = (Connect_Handle*)req->data;
	handle->open_cb(handle->err, handle->context,handle->udata);
	if (handle->ip)
	{
		free(handle->ip);
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

void redis_wrapper::connect(char* ip, int port,void(*open_cb)(const char* err, void* context,void* udata),void* udata)
{
	uv_work_t* work = (uv_work_t*)malloc(sizeof(uv_work_t));
	memset(work, 0, sizeof(uv_work_t));
	Connect_Handle* handle = (Connect_Handle*)malloc(sizeof(Connect_Handle));
	memset(handle, 0, sizeof(Connect_Handle));

	handle->ip = strdup(ip);
	handle->port = port;
	handle->udata = udata;
	handle->open_cb = open_cb;
	work->data = handle;
	uv_queue_work(uv_default_loop(), work, RedisConnect_work_cb, RedisConnect_after_cb);
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void redis_wrapper::CloseRedisConnect(void* context)
{
	Redis_Context* redis_context = (Redis_Context*)context;
	if (redis_context->is_close)
	{
		return;
	}
	uv_work_t* work = (uv_work_t*)malloc(sizeof(uv_work_t));
	memset(work, 0, sizeof(uv_work_t));

	redis_context->is_close = 1;
	work->data = context;
	uv_queue_work(uv_default_loop(), work, RedisClose_work_cb, RedisClose_after_cb);

}

void redis_wrapper::query(void* context, char* cmd, void(*query_cb)(const char* err, redisReply* result,void* udata),void* udata)
{
	Redis_Context* redis_context = (Redis_Context*)context;
	if (redis_context->is_close)
	{
		return;
	}
	uv_work_t* work = (uv_work_t*)malloc(sizeof(uv_work_t));
	memset(work, 0, sizeof(uv_work_t));
	Query_Handle* query_handle = (Query_Handle*)malloc(sizeof(Query_Handle));
	memset(query_handle, 0, sizeof(Query_Handle));
	query_handle->context = redis_context;
	query_handle->cmd = strdup(cmd);
	query_handle->udata = udata;
	query_handle->query_cb = query_cb;
	work->data = query_handle;
	uv_queue_work(uv_default_loop(), work, RedisQuery_work_cb, RedisQuery_after_cb);
}