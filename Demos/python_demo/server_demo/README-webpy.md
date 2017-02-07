### Reference?

1. http://blog.csdn.net/chenhuajie123/article/details/9202395
2. webpy/docs/build/html/index.html
3. Garfielt Blog


### How to use webpy?

Webpy is python library, like ACE, is called by user apps. After call webpy's lib (which is web), then webpy start to work (such as create worker thread).

BTW, you could just think python is same as shell, when start by `python xxx.py`, it's same like shell command, os start a process to handle it. So don't ask how to start two python, just start two terminal is OK.

BTW, gdb could attach process by `gdb -p `, pdb could not. You could use `top -H -p xxxx` to see which thread and what's titles, pdb could not.


### Code flow of webpy?

Like <1> said, developer call init of application.py, and register url maps and env, refer to application::__init__ . Then application::__init__ call application::run, this function switch wsgi server and register middleware into wsgi server. If you read "wsgi_server_demo", it's the same.

Then WSGI server start thread pool to listen at PORT, if request comes, new thread will be create to process. After basic process of HTTP message parse, then use WSGI interface to notice application.

Application get the request by processor, then deliver request to handler class, then handler class will process this, GET or POST or etc.

So there are several layers:
<ur>
    <li>network layer: httpserver.py, based on SimpleHTTPServer and BaseHTTPServer. When socket connection comes from /wsgiserver folder, this is to dispatch connection into /static folder or upper.</li>
    <li>network layer: /wsgiserver folder, thread pool is here. When socket connection comes, here create thread and process this socket, use wsgi_app talk to httpserver.py . So this layer response for socket layer work, act as server.</li>
    <li>middleware: wsgi.py, response for select SGI/WSGI/etc mode when start up.</li>
    <li>application: application.py, this is to dispatch request by URL mapping. webapi.py, session.py, form.py, template.py, db.py is all util of application.py .</li>
</ur>


### webpy code read?

Refer to:
https://github.com/batmancn/webpy.git    mylife-0.38
