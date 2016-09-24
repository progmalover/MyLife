#!/usr/bin/env python
# -*- coding: utf-8 -*-

from libs.shark_installer import SharkInstaller as Installer

def main():
    installer = Installer()
    if(installer != None):
        installer.run()

if __name__ == "__main__":
    main()
