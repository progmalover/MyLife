# only file support 'with', but in python, socket/database is all file-like
# so thay can also use 'with'

# without 'with'
set things up
    try:
        do something
    finally:
        tear things down

# with 'with'
with open("x.txt") as f:
    data = f.read()
    do something with data

# that's because file object equepped __enter__ and __exit__ method,
# refer to http://effbot.org/zone/python-with-statement.htm
