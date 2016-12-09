#!/usr/bin/env python

class Middleware:
    def __init__(self, app):
        self.app = app

    def __call__(self, environ, start_function):
        for data in self.app(environ, start_function):
            return data.upper() # change lower str to upper str, optional clause.
