# refer to http://image.diku.dk/shark/sphinx_pages/build/html/rest_sources/getting_started/installation.html

# for Shark,
# 1. ccmake .
# 2. make
# 3. cd doc    ccmake .   make doc
#

#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libs.shark_installer import SharkInstaller as Installer

def main():
    installer = Installer()
    if(installer != None):
        installer.run()

if __name__ == "__main__":
    main()
