// wuproxy.c
// 源码链接: https://github.com/dongyusheng/csdn-code/blob/master/ZeroMQ/wuproxy.c
// https://blog.51cto.com/u_15346415/category7
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zmq.h>

#define ASYNC_MSGQ_FE   "tcp://127.0.0.1:5555"
#define ASYNC_MSGQ_BE   "tcp://127.0.0.1:5556"

#define SYNC_MSGQ_FE    "tcp://127.0.0.1:5557"
#define SYNC_MSGQ_BE    "tcp://127.0.0.1:5558"

#define ASYNC_MSGQ_FE_MON "inproc://ASYNC_MSGQ_FE_MON"
#define ASYNC_MSGQ_BE_MON "inproc://ASYNC_MSGQ_BE_MON"

char *zmq_strevent (int event);

void monitor_async_fe_event (void *monitor)
{
    char        local[512];
    char        remote[512];
    uint8_t    *data;
    uint16_t    event;
    size_t      size;
    int         value;

    while (1) {
        zmq_msg_t msg;

        zmq_msg_init (&msg);
        if (zmq_msg_recv (&msg, monitor, 0) == -1) {
            return;
        }

        data  = (uint8_t *) zmq_msg_data (&msg);
        event = *(uint16_t *) (data);
        value = *(uint32_t *) (data + 2);

        if (!zmq_msg_more (&msg)) {
            continue;
        }
        zmq_msg_init (&msg);
        if (zmq_msg_recv (&msg, monitor, 0) == -1) {
            continue;
        }

        data = (uint8_t *) zmq_msg_data (&msg);
        size = zmq_msg_size (&msg);
        memcpy (local, data, size);
        local[size] = 0;
        printf ("EVENT: %-37s%5d  %.*s", zmq_strevent(event), value, (int) size,
                local);

        if (!zmq_msg_more (&msg)) {
            printf ("\r\n");
            continue;
        }
        zmq_msg_init (&msg);
        if (zmq_msg_recv (&msg, monitor, 0) == -1) {
            continue;
        }

        data = (uint8_t *) zmq_msg_data (&msg);
        size = zmq_msg_size (&msg);
        memcpy (remote, data, size);
        remote[size] = 0;

        printf ("  %.*s\r\n", (int) size, remote);
    }
}

void start_monitor_async_msgq_fe (void *context, void *socket)
{
    void *client_mon = zmq_socket (context, ZMQ_PAIR);
    zmq_socket_monitor (socket, ASYNC_MSGQ_FE_MON, ZMQ_EVENT_ALL);
    /* zmq_socket_monitor_versioned (socket, ASYNC_MSGQ_FE_MON, ZMQ_EVENT_ALL,
                                       2,
                                  ZMQ_PAIR);*/
    zmq_connect (client_mon, ASYNC_MSGQ_FE_MON);
    zmq_threadstart (monitor_async_fe_event, client_mon);
}

void msgq_fe_thread_async (void *context1)
{
    // 1.创建新的上下文
    void *context = zmq_ctx_new ();

    // 2.前端套接字, 用于连接内部的天气服务器
    void *frontend = zmq_socket (context, ZMQ_XSUB);
    zmq_bind (frontend, ASYNC_MSGQ_FE);

    start_monitor_async_msgq_fe (context, frontend);

    // 3.后端套接字, 用来处理外部的订阅者的请求
    void *backend = zmq_socket (context, ZMQ_XPUB);
    zmq_bind (backend, ASYNC_MSGQ_BE);

    // 4.持续运行代理
    zmq_proxy (frontend, backend, NULL);

    // 5.关闭套接字、清除上下文
    zmq_close (frontend);
    zmq_close (backend);
}

void msgq_be_thread_sync (void *context1)
{
    // 1.创建新的上下文
    void *context = zmq_ctx_new ();

    // 2.前端套接字, 用于连接内部的天气服务器
    void *frontend = zmq_socket (context, ZMQ_XSUB);
    zmq_bind (frontend, SYNC_MSGQ_FE);

    // 3.后端套接字, 用来处理外部的订阅者的请求
    void *backend = zmq_socket (context, ZMQ_XPUB);
    zmq_bind (backend, SYNC_MSGQ_BE);

    // 4.持续运行代理
    zmq_proxy (frontend, backend, NULL);

    // 5.关闭套接字、清除上下文
    zmq_close (frontend);
    zmq_close (backend);
}

#if 1

int main ()
{
    zmq_threadstart (msgq_fe_thread_async, NULL);
    msgq_be_thread_sync(NULL);
    return 0;
}

#else

int main ()
{
    // 1.创建新的上下文
    void *context = zmq_ctx_new ();

    // 2.前端套接字, 用于连接内部的天气服务器
    void *frontend = zmq_socket (context, ZMQ_XSUB);
    zmq_bind (frontend, "tcp://127.0.0.1:5555");

    // 3.后端套接字, 用来处理外部的订阅者的请求
    void *backend = zmq_socket (context, ZMQ_XPUB);
    zmq_bind (backend, "tcp://127.0.0.1:5556");

    // 4.持续运行代理
    zmq_proxy (frontend, backend, NULL);

    // 5.关闭套接字、清除上下文
    zmq_close (frontend);
    zmq_close (backend);
    zmq_ctx_destroy (context);

    return 0;
}

#endif
