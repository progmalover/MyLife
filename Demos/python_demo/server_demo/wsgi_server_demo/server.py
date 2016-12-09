#!/usr/bin/env python
import middleware as app
from scgiserver import serve_application

PREFIX=''
PORT=4101

serve_application(app, PREFIX, PORT)
