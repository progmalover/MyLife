#!/usr/bin/env python

import os
import pam
import web
import ovsdb
import ofctrl
import simplejson as json

urls = (
    '/', 'Index',
    '/login', 'Login',
    '/logout', 'Logout',
    '/availableports', 'SwitchPorts',
    '/swconf', 'SwitchConf',
    '/switchinfo', 'SwitchInfo',
    '/helptext', 'HelpText',
# All Bridges
    '/bridges', 'Bridges',
    '/bridges/add', 'Bridges',
# A single Bridge
    '/bridges/([\w:.-]+)', 'Bridge',
    '/bridges/([\w:.-]+)/(update|del)', 'Bridge',
# Controllers
    '/bridges/([\w:.-]+)/controllers', 'Controller',
    '/bridges/([\w:.-]+)/controllers/(update|del|add)', 'Controller',
# Normal Ports
    '/bridges/([\w:.-]+)/ports', 'Ports',
    '/bridges/([\w:.-]+)/ports/(\d+)/(update|del|add)', 'Ports',
# Tunnels
    '/bridges/([\w:.-]+)/tunnels', 'Tunnel',
    '/bridges/([\w:.-]+)/tunnels/([\w:.-]+)/(update|del|add)', 'Tunnel',
# Bonds
    '/bridges/([\w:.-]+)/bonds', 'Bond',
    '/bridges/([\w:.-]+)/bonds/([\w:.-]+)/(update|del|add)', 'Bond',
# Mirrors
    '/bridges/([\w:.-]+)/mirrors', 'Mirror',
    '/bridges/([\w:.-]+)/mirrors/([\w:.-]+)/(update|del|add)', 'Mirror',
# NetFlow
    '/bridges/([\w:.-]+)/netflow', 'NetFlow',
    '/bridges/([\w:.-]+)/netflow/(update|del|add)', 'NetFlow',
# sFlow
    '/bridges/([\w:.-]+)/sflow', 'sFlow',
    '/bridges/([\w:.-]+)/sflow/(update|del|add)', 'sFlow',
# Queue
    '/bridges/([\w:.-]+)/queues', 'Queues',
    '/bridges/([\w:.-]+)/queues/add', 'Queues',
    '/bridges/([\w:.-]+)/queues/(\w{8})/(update|del)', 'Queue',
# Qos
    '/bridges/([\w:.-]+)/qos', 'QoSes',
    '/bridges/([\w:.-]+)/qos/add', 'QoSes',
    '/bridges/([\w:.-]+)/qos/(\w{8})/(update|del)', 'QoS',
# Group Table
    '/bridges/([\w:.-]+)/groups', 'Group',
    '/bridges/([\w:.-]+)/groups/(\d+)/(update|del|add)', 'Group',
# Flows
    '/bridges/([\w:.-]+)/tables', 'Tables',
    '/bridges/([\w:.-]+)/tables/(\d+)/flows', 'Flows',
    '/bridges/([\w:.-]+)/tables/(\d+)/flows/(update|add|del|clear)', 'Flows',
    '/bridges/([\w:.-]+)/flows', 'FlowHandler',
# Meter Table
    '/bridges/([\w:.-]+)/meters', 'Meter',
    '/bridges/([\w:.-]+)/meters/(\d+)/(update|del|add)', 'Meter',
# Bundle control message
    '/bridges/([\w:.-]+)/ofbundle', 'Bundle',
    '/(.*)', 'Default'
)

app = web.application(urls, globals())

if web.config.get('_session') is None:
    session = web.session.Session(app, web.session.DiskStore('sessions'), initializer={'loggedin': False})
    web.config._session = session
else:
    session = web.config._session

render = web.template.render(os.path.join(os.path.dirname(os.getcwd()), 'frontend/'))

class Index(object):
    def GET(self):
        # redirect to layout template
        if session.loggedin:
            return render.index()
        else:
            return render.login()

class Login(object):
    def GET(self):
        getInput = web.input(username="", password="")
        if pam.authenticate(getInput.username, getInput.password):
            session.loggedin = True
            return 'success'
        else:
            return 'fail'

class Logout(object):
    def GET(self):
        session.loggedin = False
        raise web.seeother("/")

class SwitchPorts(object):
    def GET(self):
        return ovsdb.get_available_ports()

class SwitchInfo(object):
    def GET(self):
        return ovsdb.get_switch_resource()

class SwitchConf(object):
    def GET(self):
        """
        GET /swconf
        """
        return ovsdb.get_switch_conf()

    def POST(self):
        """
        POST /swconf
        """
        data = web.data()
        return ovsdb.update_switch_conf(data)

class HelpText(object):
    def GET(self):
        return ovsdb.get_help_text()

class Bridges(object):
    def GET(self):
        """
        GET /bridges
        """
        return ovsdb.fast_get_bridges()

    def POST(self):
        """
        POST /bridges/add?name=br0
        """
        getInput = web.input()
        # TODO, elaborate add_bridge
        return ovsdb.add_bridge(str(getInput.name))

class Bridge(object):
    def GET(self, name):
        """
        GET /bridges/br0
        """
        return ovsdb.get_bridge(name)

    def POST(self, name, op):
        """
        POST /bridges/br0/update
        POST /bridges/br0/del
        """
        data = web.data()
        if op == "update":
            # TODO, elaborate update_bridge
            return ovsdb.update_bridge(name, data)
        elif op == "del":
            # TODO, elaborate del_bridge
            return ovsdb.del_bridge(name)

class Controller(object):
    def GET(self, name):
        """
        GET /bridges/br0/controllers
        """
        return ovsdb.get_controllers(name)

    def POST(self, name, op):
        """
        POST /bridges/br0/controllers/update
        POST /bridges/br0/controllers/add
        POST /bridges/br0/controllers/del
        """
        data = web.data()
        if op == "update":
            return ovsdb.update_controller(name, data)
        elif op == "del":
            return ovsdb.del_controller(name, data)
        elif op == "add":
            return ovsdb.add_controller(name, data)

class Ports(object):
    def GET(self, brname):
        """
        GET /bridges/br0/ports
        """
        return ovsdb.get_ports(brname)

    def POST(self, brname, portname, op):
        """
        POST /bridges/br0/ports/eth0/update
        POST /bridges/br0/ports/eth0/add
        POST /bridges/br0/ports/eth0/del
        """
        data = web.data()
        if op == "update":
            return ovsdb.update_port(brname, data)
        elif op == "del":
            return ovsdb.del_port(brname, data)
        elif op == "add":
            return ovsdb.add_port(brname, data)

class Tunnel(object):
    def GET(self, brname):
        """
        GET /bridges/br0/tunnels
        """
        return ovsdb.get_tunnels(brname)

    def POST(self, brname, tulname, op):
        """
        POST /bridges/br0/tunnels/gre0/update
        POST /bridges/br0/tunnels/gre0/add
        POST /bridges/br0/tunnels/gre0/del
        """
        data = web.data()
        if op == "update":
            return ovsdb.update_tunnel(brname, data)
        elif op == "del":
            return ovsdb.del_tunnel(brname, data)
        elif op == "add":
            return ovsdb.add_tunnel(brname, data)

class Bond(object):
    def GET(self, brname):
        """
        GET /bridges/br0/bonds
        """
        return ovsdb.get_bonds(brname)

    def POST(self, brname, bondname, op):
        """
        POST /bridges/br0/bonds/lag0/update
        POST /bridges/br0/bonds/lag0/add
        POST /bridges/br0/bonds/lag0/del
        """
        data = web.data()
        if op == "update":
            return ovsdb.update_bond(brname, data)
        elif op == "del":
            return ovsdb.del_bond(brname, data)
        elif op == "add":
            return ovsdb.add_bond(brname, data)

class Mirror(object):
    def GET(self, brname):
        """
        GET /bridges/br0/mirrors
        """
        return ovsdb.get_mirrors(brname)

    def POST(self, brname, mirrorname, op):
        """
        POST /bridges/br0/mirrors/M1/update
        POST /bridges/br0/mirrors/M1/add
        POST /bridges/br0/mirrors/M1/del
        """
        data = web.data()
        if op == "update":
            return ovsdb.update_mirror(brname, data)
        elif op == "del":
            return ovsdb.del_mirror(brname, data)
        elif op == "add":
            return ovsdb.add_mirror(brname, data)

class NetFlow(object):
    def GET(self, brname):
        """
        GET /bridges/br0/netflow
        """
        return ovsdb.get_netflows(brname)

    def POST(self, brname, op):
        """
        POST /bridges/br0/netflow/update
        POST /bridges/br0/netflow/add
        POST /bridges/br0/netflow/del
        """
        data = web.data()
        if op == "update":
            return ovsdb.update_netflow(brname, data)
        elif op == "del":
            return ovsdb.del_netflow(brname, data)
        elif op == "add":
            return ovsdb.add_netflow(brname, data)

class sFlow(object):
    def GET(self, brname):
        """
        GET /bridges/br0/sflow
        """
        return ovsdb.get_sflow(brname)

    def POST(self, brname, op):
        """
        POST /bridges/br0/sflow/update
        POST /bridges/br0/sflow/add
        POST /bridges/br0/sflow/del
        """
        data = web.data()
        if op == "update":
            return ovsdb.update_sflow(brname, data)
        elif op == "del":
            return ovsdb.del_sflow(brname, data)
        elif op == "add":
            return ovsdb.add_sflow(brname, data)

class Queues(object):
    def GET(self, brname):
        """
        GET /bridges/br0/queues
        """
        return ovsdb.get_queues()

    def POST(self, brname):
        """
        POST /bridges/br0/queues/add
        """
        data = web.data()
        return ovsdb.add_queue(data)

class Queue(object):
    def GET(self):
        pass

    def POST(self, brname, uuid, op):
        """
        POST /bridges/br0/queues/00000000/update
        POST /bridges/br0/queues/00000000/del
        """
        data = web.data()
        if op == "update":
            return ovsdb.update_queue(data)
        elif op == "del":
            return ovsdb.del_queue(data)

class QoSes(object):
    def GET(self, brname):
        """
        GET /bridges/br0/qos
        """
        return ovsdb.get_all_qos()

    def POST(self, brname):
        """
        POST /bridges/br0/qos/add
        """
        data = web.data()
        return ovsdb.add_qos(data)

class QoS(object):
    def GET(self):
        pass

    def POST(self, brname, uuid, op):
        """
        POST /bridges/br0/qos/00000000/update
        POST /bridges/br0/qos/00000000/del
        """
        data = web.data()
        if op == "update":
            return ovsdb.update_qos(data)
        elif op == "del":
            return ovsdb.del_qos(data)

class Group(object):
    def GET(self, brname):
        return ovsdb.get_groups(brname)

    def POST(self, name, gid, op):
        data = web.data()
        if op == "update":
            return ovsdb.update_group(name, data)
        elif op == "del":
            return ovsdb.del_group(name, data)
        elif op == "add":
            return ovsdb.create_group(name, data)

    def DELETE(self, brname):
        return ovsdb.del_groups(brname)

class Meter(object):
    def GET(self, brname):
        return ovsdb.get_meters(brname)

    def POST(self, name, gid, op):
        data = web.data()
        if op == "update":
            return ovsdb.update_meter(name, data)
        elif op == "del":
            return ovsdb.del_meter(name, data)
        elif op == "add":
            return ovsdb.create_meter(name, data)

    def DELETE(self, brname):
        return ovsdb.del_meters(brname)

class Tables():
    def GET(self, brname):
        """
        GET /bridges/br0/tables
        """
        wrapper = ofctrl.SimpleCtrl(brname)
        return wrapper.get_tables()

class Flows():
    def GET(self, brname, tid):
        """
        GET /bridges/br0/tables/0/flows
        """
        wrapper = ofctrl.SimpleCtrl(brname)
        return wrapper.get_flows(int(tid))

    def POST(self, brname, tid, op):
        """
        POST /bridges/br0/tables/0/flows/update
        POST /bridges/br0/tables/0/flows/add
        POST /bridges/br0/tables/0/flows/del
        POST /bridges/br0/tables/0/flows/clear
        """
        data = web.data()
        ofctl_wrapper = ofctrl.SimpleCtrl(brname)

        if op == "update":
            return ofctl_wrapper.mod_flow(data)
        elif op == "del":
            return ofctl_wrapper.del_flow(data)
        elif op == "add":
            return ofctl_wrapper.add_flow(data)
        elif op == "clear":
            return ofctl_wrapper.del_flows(tid)

class FlowHandler():
    def GET(self, brname):
        """
        GET /bridges/br0/flows
        """
        wrapper = ofctrl.SimpleCtrl(brname)
        allflows = []
        resp = json.loads(wrapper.get_tables())
        if resp["ret"] == 0:
            for table in resp["tables"]:
                for flow in table["flows"]:
                    allflows.append("{fields},actions={actions}".format(
            fields=wrapper._flow_json_to_string(flow),
            actions=flow["actions"]))

        web.header('Content-Type', 'text/plain')
        web.header('Content-disposition', 'attachment; filename=flows.txt')
        return "\n".join(allflows)

    def POST(self, brname):
        """
        POST /bridges/br0/flows
        """
        data = web.data()
        wrapper = ofctrl.SimpleCtrl(brname)
        return wrapper.add_flows(data)

class Bundle():
    def POST(self, brname):
        """
        POST /bridges/br0/ofbundle
        """
        data = web.data()
        ofctl_wrapper = ofctrl.SimpleCtrl(brname)

        return ofctl_wrapper.bundle(data)

class Default():
    def GET(self, whatever):
        """
        Handle page-not found error
        """
        return render.error()


if __name__ == "__main__":
    app.run()
        #if name:
            #bridge = json.loads(ovsdb.get_bridge(str(name)))
            #return self.render.bridge(bridge)
