#!/usr/bin/env python
"""Python PAM non-interactive authentication."""

import PAM


def authenticate(username, password):
    """Return True if is a valid user, return False otherwise."""
    def pam_conv(auth, query_list, userData):
        return [(password, 0)]
    res = True
    service = 'login'

    auth = PAM.pam()
    auth.start(service)
    auth.set_item(PAM.PAM_USER, username)
    auth.set_item(PAM.PAM_CONV, pam_conv)
    try:
        auth.authenticate()
        auth.acct_mgmt()
    except PAM.error, resp:
        res = False
    except:
        res = False

    return res

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 3:
        print 'Usage: pam.py user pass'
        sys.exit(1)
    else:
        print authenticate(sys.argv[1], sys.argv[2])
