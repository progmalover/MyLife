#!/usr/bin/env python

def simple_app(environ, start_function):
    status = '200 OK'
    response_header = [('Content-type','text/plain')]
    body = 'Hello World!\n' # HTML format
    start_response(status, response_header) # start to compose HTTP message
    return [body];
