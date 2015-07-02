#!/usr/bin/env python

import os
import subprocess
import simplejson as json
import re
from config import prefix

alias_ovs2st = {
        "dl_vlan": "vlan_vid",
        "dl_vlan_pcp": "vlan_pcp",
        "dl_src": "eth_src",
        "dl_dst": "eth_dst",
        "dl_type": "eth_type",
        "nw_src": "ipv4_src",
        "nw_dst": "ipv4_dst",
        "nw_proto": "ip_proto",
        "nw_tos": "ip_dscp",
        "nw_ecn": "ip_ecn",
        "ipv6_label": "ipv6_flabel",
        "ng_target": "ipv6_nd_target",
        "nd_ssl": "ipv6_nd_ssl",
        "nd_tll": "ipv6_nd_tll",
        "tun_id": "tunnel_id",
        }

alias_st2ovs = {
        "vlan_vid": "dl_vlan",
        "vlan_pcp": "dl_vlan_pcp",
        "eth_src": "dl_src",
        "eth_dst": "dl_dst",
        "eth_type": "dl_type",
        "ipv4_src": "nw_src",
        "ipv4_dst": "nw_dst",
        "ip_proto": "nw_proto",
        "ip_dscp": "nw_tos",
        "ip_ecn": "nw_ecn",
        "ipv6_flabel": "ipv6_label",
        "ipv6_nd_target": "ng_target",
        "ipv6_nd_ssl": "nd_ssl",
        "ipv6_nd_tll": "nd_tll",
        "tunnel_id": "tun_id",
        }

abbreviates = [ "ip", "icmp", "tcp", "udp", "arp", "rarp",
                "ipv6", "tcp6", "udp6", "icmp6", "mpls" ]

class ofctlParser(object):
    def __init__(self, raw):
        self.raw = raw
        self.flow = dict(
                match_fields = {},
                counters = {},
                )

    def _preprocess(self):
        self.flow["actions"] = re.search("actions=([^ ]+)", self.raw).group(1)
        self._params = {}
        prefix = self.raw[:self.raw.find("actions")]
        pairs = re.split(", |,| ", prefix.strip())
        for pair in pairs:
            name, sep, value = pair.partition("=")
            if name:
                self._params[name] = value

    def convertShort(self, short):
        if short == "ip":
            self._shorthands("0x0800", None)
        if short == "icmp":
            self._shorthands("0x0800", "1")
        if short == "tcp":
            self._shorthands("0x0800", "6")
        if short == "udp":
            self._shorthands("0x0800", "17")
        if short == "arp":
            self._shorthands("0x0806", None)
        if short == "rarp":
            self._shorthands("0x8035", None)
        if short == "ipv6":
            self._shorthands("0x86dd", None)
        if short == "tcp6":
            self._shorthands("0x86dd", "6")
        if short == "udp6":
            self._shorthands("0x86dd", "17")
        if short == "icmp6":
            self._shorthands("0x86dd", "58")
        if short == "mpls":
            self._shorthands("0x8847", None)

    def _shorthands(self, dl_type, nw_proto):
        self.flow["match_fields"]["eth_type"] = dl_type
        if nw_proto:
            self.flow["match_fields"]["ip_proto"] = nw_proto

    @staticmethod
    def aliasForCmd(field):
        return alias_st2ovs[field] if field in alias_st2ovs else field

    def convert2json(self):
        self._preprocess()
        # Priority
        if "priority" in self._params:
            self.flow["priority"] = int(self._params["priority"])
            del self._params["priority"]
        else:
            self.flow["priority"] = 32768

        # Cookie
        self.flow["cookie"] = self._params["cookie"]
        del self._params["cookie"]

        # Table
        self.flow["table"] = self._params["table"]
        del self._params["table"]

        # Counters
        self.flow["counters"]["bytes"] = self._params["n_bytes"]
        self.flow["counters"]["packets"] = self._params["n_packets"]
        del self._params["n_bytes"]
        del self._params["n_packets"]

        # Duration
        self.flow["duration"] = self._params["duration"]
        del self._params["duration"]

        # Idel Age
        if "idle_age" in self._params:
            self.flow["idle_age"] = self._params["idle_age"]
            del self._params["idle_age"]

        # Hard Age
        if "hard_age" in self._params:
            self.flow["hard_age"] = self._params["hard_age"]
            del self._params["hard_age"]

        # Send Flow Remove
        if "send_flow_rem" in self._params:
            self.flow["send_flow_rem"] = True
            del self._params["send_flow_rem"]

        for key, val in self._params.iteritems():
            if key in abbreviates:
                self.convertShort(key)
            elif key in alias_ovs2st:
                self.flow["match_fields"][alias_ovs2st[key]] = val
            else:
                self.flow["match_fields"][key] = val

class SimpleCtrl(object):
    def __init__(self, sw):
        self.switch = sw

    def get_tables(self):
        response = {}
        tmp = [{"id":i, "flows":[]} for i in range(0, 256)]

        command = ['ovs-ofctl', 'dump-flows', self.switch]
        child = subprocess.Popen(command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env={"PATH":prefix})
        ret, err = child.communicate()
        if err:
            response["ret"] = -1
            response["msg"] = err.partition(":")[2]
        else:
            response["ret"] = 0
            for raw_flow in ret.splitlines()[1:]:
                if not re.match('OFPST_FLOW', raw_flow):
                    json_flow = self._string_to_json(raw_flow)
                    table_index = int(json_flow["table"])
                    tmp[table_index]["flows"].append(json_flow)
            response["tables"] = filter(lambda item: True if item["flows"] else False, tmp)

        return json.dumps(response)

    def get_flows(self, tableid):
        """
        ovs-ofctl dump-flows switch
        """
        response = {}
        command = ['ovs-ofctl', 'dump-flows', self.switch]
        command.append('table=%d' % tableid)
        child = subprocess.Popen(command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env={"PATH":prefix})
        ret, err = child.communicate()
        if err:
            response["ret"] = -1
            response["msg"] = err.partition(":")[2]
        else:
            response["ret"] = 0
            response["table"] = {}
            response["table"]["id"] = tableid
            response["table"]["flows"] = []
            for raw_flow in ret.splitlines()[1:]:
                if not re.match('OFPST_FLOW', raw_flow):
                    response["table"]["flows"].append(self._string_to_json(raw_flow))

        return json.dumps(response)

    def add_flow(self, data):
        """
        ovs-ofctl add-flow switch flow
        """
        flow = json.loads(data)
        response = {}
        command = ['ovs-ofctl', 'add-flow', self.switch]
        command.append(flow["flow"])
        child = subprocess.Popen(command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env={"PATH":prefix})
        ret, err = child.communicate()
        if err:
            response["ret"] = -1
            response["msg"] = err.partition(":")[2]
            return json.dumps(response)
        else:
            return self.get_flows(int(flow["table"]))

    def mod_flow(self, data):
        """
        ovs-ofctl --strict mod-flows switch flow
        """
        flow = json.loads(data)
        response = {}
        command = ['ovs-ofctl', '--strict', 'mod-flows', self.switch]
        command.append("{fields},actions={actions}".format(
            fields=self._flow_json_to_string(flow),
            actions=flow["actions"]))
        child = subprocess.Popen(command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env={"PATH":prefix})
        ret, err = child.communicate()
        if err:
            response["ret"] = -1
            response["msg"] = err.partition(":")[2]
            return json.dumps(response)
        else:
            return self.get_flows(int(flow["table"]))

    def del_flow(self, data):
        """
        ovs-ofctl --strict del-flows switch flow
        """
        flow = json.loads(data)
        response = {}
        command = ['ovs-ofctl', '--strict', 'del-flows', self.switch]
        command.append(self._flow_json_to_string(flow, False))
        child = subprocess.Popen(command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env={"PATH":prefix})
        ret, err = child.communicate()
        if err:
            response["ret"] = -1
            response["msg"] = err.partition(":")[2]
            return json.dumps(response)
        else:
            return self.get_flows(int(flow["table"]))

    def del_flows(self, table_id):
        """
        ovs-ofctl --strict del-flows switch table=0
        """
        response = {}
        command = ['ovs-ofctl', 'del-flows', self.switch, 'table={0}'.format(table_id)]
        child = subprocess.Popen(command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env={"PATH":prefix})
        ret, err = child.communicate()
        if err:
            response["ret"] = -1
            response["msg"] = err.partition(":")[2]
            return json.dumps(response)
        else:
            return self.get_flows(int(table_id))

    def add_flows(self, data):
        flows = json.loads(data)
        response = {}
        command = ['ovs-ofctl', 'add-flow', self.switch, '-']
        child = subprocess.Popen(command, stdout=subprocess.PIPE,
                stdin=subprocess.PIPE, stderr=subprocess.PIPE,
                env={"PATH":prefix})
        ret, err = child.communicate(input=flows["flows"])
        if err:
            response["ret"] = -1
            response["msg"] = err.partition(":")[2]
            return json.dumps(response)
        else:
            return self.get_tables()

    def bundle(self, data):
        messages = json.loads(data)
        response = {}
        command = ['ovs-ofctl', 'bundle', self.switch]
        bundle_str = 'open:bundle_id=1,'
        add_msg_str = 'add:bundle_id=1,message={0},'
        if messages.get('flow'):
            bundle_str += add_msg_str.format(messages.get('flow'))
        if messages.get('group'):
            bundle_str += add_msg_str.format(messages.get('group'))
        if messages.get('meter'):
            bundle_str += add_msg_str.format(messages.get('meter'))
        if messages.get('port'):
            bundle_str += add_msg_str.format(messages.get('port'))
        bundle_str += 'close:bundle_id=1,commit:bundle_id=1'
        command.append(bundle_str)
        child = subprocess.Popen(command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env={"PATH":prefix})
        ret, err = child.communicate()
        if err:
            response["ret"] = -1
            response["msg"] = err.partition(":")[2]
            return json.dumps(response)
        else:
            response["ret"] = 0
            return json.dumps(response)

    def _flow_json_to_string(self, flow, set_cookie=True):
        fields = []
        if flow["priority"] is not None:
            fields.append("priority={0}".format(flow["priority"]))

        if set_cookie:
            if flow["cookie"] is not None:
                fields.append("cookie={0}".format(flow["cookie"]))

        if flow["table"] is not None:
            fields.append("table={0}".format(flow["table"]))
        else:
            fields.append("table=0")

        fields.extend(map(lambda x:"{0}={1}".format(ofctlParser.aliasForCmd(x[0]),x[1]),
                filter(lambda x: True if x[1] is not None else False, flow["match_fields"].items())))

        return ",".join(fields)

    def _string_to_json(self, raw):
        parser = ofctlParser(raw)
        parser.convert2json()
        return parser.flow

if __name__ == "__main__":
    myctrl = SimpleCtrl("br0")
    data = {
        "flow": "add-flow in_port=1,actions=output:2",
        "meter": "add-meter meter=1,kbps,band=type=drop,rate=3000",
        "port": "mod-port 13 down"
    }

    print myctrl.bundle(json.dumps(data))
    #print myctrl.del_flow(data)
    #flows = json.loads(myctrl.get_flows())["flows"]
    #for flow in flows:
        #print myctrl._flow_json_to_string(flow)
