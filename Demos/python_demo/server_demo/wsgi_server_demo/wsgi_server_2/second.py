#!/usr/bin/env python

'''
Lesson 3
'''

from wsgiref.simple_server import make_server
from cgi import parse_qs, escape

html = """
<html>
<body>
   <form method="get" action="">
        <p>
           Age: <input type="text" name="age" value="%(age)s">
        </p>
        <p>
            Hobbies:
            <input
                name="hobbies" type="checkbox" value="software"
                %(checked-software)s
            > Software
            <input
                name="hobbies" type="checkbox" value="tunning"
                %(checked-tunning)s
            > Auto Tunning
        </p>
        <p>
            <input type="submit" value="Submit">
        </p>
    </form>
    <p>
        Age: %(age)s<br>
        Hobbies: %(hobbies)s
    </p>
</body>
</html>
"""

def application (environ, start_response):

    # Returns a dictionary in which the values are lists
    # parse_qs is parse quarry string, it's used for parse
    # quarry(environ['QUERY_STRING']) from request URL into
    # map = {'age':['38', '42'], ...}.
    d = parse_qs(environ['QUERY_STRING'])

    # As there can be more than one value for a variable then
    # a list is provided as a default value.
    #
    # https://docs.python.org/2.7/library/stdtypes.html#mapping-types-dict
    # get(key[, default])
    age = d.get('age', [''])[0] # Returns the first age value
    hobbies = d.get('hobbies', []) # Returns a list of hobbies

    # Always escape user input to avoid script injection
    # http://www.cnblogs.com/xuxn/archive/2011/08/12/parse-html-escape-characters-in-python.html
    # As <> will be html injection, so escape it.
    age = escape(age)
    hobbies = [escape(hobby) for hobby in hobbies]

    # Fill the above html template by age etc, return html will add this:
    # Age: 34343
    # Hobbies: software, tunning
    response_body = html % {
        'checked-software': ('', 'checked')['software' in hobbies],
        'checked-tunning': ('', 'checked')['tunning' in hobbies],
        'age': age or 'Empty',
        'hobbies': ', '.join(hobbies or ['No Hobbies?'])
    }

    status = '200 OK'

    # Now content type is text/html
    response_headers = [
        ('Content-Type', 'text/html'),
        ('Content-Length', str(len(response_body)))
    ]

    start_response(status, response_headers)
    # return [response_body]
    return response_body

httpd = make_server('10.10.50.236', 8051, application)

# Now it is serve_forever() in instead of handle_request()
httpd.serve_forever()
