### Reference?

1. http://blog.csdn.net/chenhuajie123/article/details/9202395
2. http://ivory.idyll.org/articles/wsgi-intro/what-is-wsgi.html
3. http://wsgi.tutorial.codepoint.net/


### Introduce?

CGI is common gateway interface between web server and third party application.
The web server response for receive L7 message from network, most is HTTP
message. The third party application response for process the packet follow
customer's logic.

How the server notice third party application and what message should take is
CGI does. Mostly CGI use enviroment to transform property of this flow, and
use cgi interface to call third party application.

WSGI is the interface between web server and python web application.

Refer to the graph from <1>, wsgiserver run threads in thread pool, and all
thread use wsgi_app layer to talk to application which is run by app. The
interface thay talk is WSGI.


### Server, app, middleware in wsgi_server_1?

Refer <2>.

The PEP333 is the spec of WSGI, it specifies three role: server, application or
application framwork, middleware.

In WSGI spec, server side is HTTP server, need to provider two: "environ" directory and "start_response" function, the dispatch code like this:

def server_side_function(self):
    iter = app(environ, start_response)
    for data in iter:
        # as HTTP message is HTML format, so client will compose it to an
        # complete message, UDP or TCP as transport layer is all OK.
        send_data_to_client(data)

In WSGI spec, web framework/app is application server written by python (Java or C++ did not use WSGI), framework/app must implement by a class/object or a function. If it's a class/object, it must have __init__ and __call__ method with these param: an environ object and an callable function named start_response. If it's a function, its param must be an environ object and an callable function named start_response. Framework/app must call start_response before return/yield any data, this start_response function is post filtered function used by server after framework/app process data and before transport data to client. And framework/app must return data in iterable, which is like [ page ] or [ header, body, page ], it's list. It's like this:

def simple_app(environ, start_function):
    status = '200 OK'
    response_header = [('Content-type','text/plain')]
    body = 'Hello World!\n' # HTML format
    start_response(status, response_header) # start to compose HTTP message
    return [body];

In WSGI spec, middleware is not neccessary, of it has, it must obay both server and framework/app spec like this:

class Middleware:
    def __init__(self, app):
        self.app = app

    def __call__(self, environ, start_function):
        for data in self.app(environ, start_function):
            return data.upper() # change lower str to upper str, optional clause.

So for framework/app and middleware defined upper, server side code could be change into this:

def server_side_function():
    server(simple_app) # call simple_app function

or

def server_side_function():
    middleware = Middleware(simple_app)
    server(middleware) # call Middleware function

Code like this:

#!/usr/bin/env python
import middleware as app
from scgiserver import server_application

PRIFIX = ''
PORT = 8888

server_application(app, PRIFIX, PORT)


### wsgi_server_2?

Refer <3>.
