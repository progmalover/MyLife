#!/usr/bin/python
import simplejson as json
import subprocess
import clientWrapper as dbclient
import ctlWrapper as vsctl
import ctlWrapper as ofctl
import re
from config import infofile, portconfig

class ovsdb_uuid:
    """
    expect ['uuid', 'af749e9c-464a-4af1-a381-7bdd2ac6be45']
    """
    def __init__(self, notation):
        self.uuid = notation[1]

    @property
    def uuid(self):
        return self.uuid

class ovsdb_set:
    """
    expect ['set', []]
    """
    def __init__(self, notation):
        self.elements = []
        if _is_type_set(notation):
            for ele in notation[1]:
                if _is_type_uuid(ele):
                    self.elements.append(ovsdb_uuid(ele).uuid)
                else:
                    self.elements.append(ele)
        else:
            if _is_type_uuid(notation):
                self.elements.append(ovsdb_uuid(notation).uuid)
            else:
                self.elements.append(notation)

    @property
    def elements(self):
        return self.elements

class ovsdb_map:
    """
    expect ['map', []]
    """
    def __init__(self, notation):
        self.pairs = {}
        for pair in notation[1]:
            self.pairs[pair[0]] = pair[1][1] if _is_type_uuid(pair[1]) else pair[1]

    @property
    def pairs(self):
        return self.pairs

class target:
    """
    expect 'ptcp:[6633][:192.168.1.1]' or 'tcp:192.168.1.1[:80]'
    """
    def __init__(self, con):
        self.con = con.split(":")
        self._ip = None
        self._port = None
        self._protocol = None
        for x in self.con:
            if x.isalpha():
                self._protocol = x
            elif x.isdigit():
                self._port = int(x)
            elif x:
                self._ip = x

    @property
    def ip(self):
        return self._ip

    @property
    def port(self):
        return self._port

    @property
    def protocol(self):
        return self._protocol

def _is_type_set(column):
    return isinstance(column, list) and column[0] == 'set'

def _is_type_map(column):
    return isinstance(column, list) and column[0] == 'map'

def _is_type_uuid(column):
    return isinstance(column, list) and column[0] == 'uuid'

def _record_to_response(record, column):
    if _is_type_set(record[column]):
        if record[column][1]:
            return record[column][1]
        else:
            return None
    else:
        if _is_type_uuid(record[column]):
            return record[column][1]
        else:
            return record[column]

def _iface_record_helper(uuid):
    con = dbclient.connect()
    cur = con.cursor()
    transact = '["Open_vSwitch", {"op":"select", "table":"Port", \
            "where":[["_uuid", "==", ["uuid", "%s"]]]}]'\
            % uuid
    cur.execute(transact)
    port_record = cur.fetchone()
    # to get Interface record
    # FIXME assuming one-to-one mapping between iface and port
    iface_uuid = ovsdb_uuid(port_record["interfaces"]).uuid
    transact = '["Open_vSwitch", {"op":"select", "table":"Interface", \
            "where":[["_uuid", "==", ["uuid", "%s"]]]}]'\
            % iface_uuid
    cur.execute(transact)
    iface_record = cur.fetchone()
    return port_record, iface_record

def get_bridges():
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["bridges"] = []
            cur = con.cursor()
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[]}]'
            cur.execute(transact)
            for record in cur.fetchall():
                response["bridges"].append(json.loads(_get_bridge(record)))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def fast_get_bridges():
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["bridges"] = []
            cur = con.cursor()
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[]}]'
            cur.execute(transact)
            for record in cur.fetchall():
                response["bridges"].append(json.loads(_fast_get_bridge(record)))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def fast_get_bridge(brname):
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            cur = con.cursor()
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]]}]' % brname
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                response["bridge"] = json.loads(_fast_get_bridge(row));
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def _fast_get_bridge(record):
    """
    Return a concise bridge JSON.
    """
    response = {}
    response["name"] = record["name"]
    response["datapath_id"] = _record_to_response(record, "datapath_id")
    response["fail_mode"] = _record_to_response(record, "fail_mode")
    response["protocols"] = _record_to_response(record, "protocols")
    response["stp_config"] = {}
    other = ovsdb_map(record["other_config"]).pairs
    response["stp_config"]["stp_enable"] = _record_to_response(record, \
            "stp_enable")

    if "datapath_id" in other:
        response["datapath_id"] = other["datapath_id"]

    return json.dumps(response)

def update_bridge(brname, data):
    new_br = json.loads(data)
    commands = ['--', 'set', 'Bridge', new_br['name']]
    nullify = []

    commands.extend(['other_config:datapath-id={id}'.format(id=new_br['datapath_id'])])

    if new_br['fail_mode']:
        commands.extend(['fail_mode={0}'.format(new_br['fail_mode'])])
    else:
        nullify.extend(['--', 'clear', 'Bridge', new_br['name'], 'fail_mode'])

    if new_br['protocols']:
        commands.extend(['protocols={0}'.format(new_br['protocols'])])
    else:
        nullify.extend(['--', 'clear', 'Bridge', new_br['name'], 'protocols'])

    commands.extend(nullify)

    ret, err = vsctl.execute(commands)
    return fast_get_bridge(brname)

def get_all_qos():
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["qoses"] = []
            cur = con.cursor()
            transact = '["Open_vSwitch", {"op":"select", "table":"QoS", \
                    "where":[]}]'
            cur.execute(transact)
            for record in cur.fetchall():
                response["qoses"].append(json.loads(_get_qos(record)))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def get_qos(uuid):
    """
    A helper function for retrieving QoS data.
    """
    response = {"uuid":uuid, "queues":[]}

    con = dbclient.connect()
    cur = con.cursor()
    transact = '["Open_vSwitch", {"op":"select", "table":"QoS", \
            "where":[["_uuid", "==", ["uuid", "%s"]]]}]'\
            % uuid
    cur.execute(transact)
    record = cur.fetchone()
    if record:
        response["queues"].extend(_get_qos(record))
    return response

def _get_qos(record):
    response = []

    queues = ovsdb_map(record["queues"]).pairs

    for queue_id, queue_uuid in queues.items():
        queue = {"queue_id":queue_id}
        queue.update(get_queue(queue_uuid))
        response.append(queue)

    return response

def get_queue(uuid):
    con = dbclient.connect()
    cur = con.cursor()
    transact = '["Open_vSwitch", {"op":"select", "table":"Queue", \
            "where":[["_uuid", "==", ["uuid", "%s"]]]}]'\
            % uuid
    cur.execute(transact)
    record = cur.fetchone()
    if record:
        return _get_queue(record)

def _del_qos(uuid):
    commands = ['--', 'destroy', 'QoS', uuid]
    return commands

def _clear_qos(uuid):
    commands = ['--', 'clear', 'QoS', uuid, 'queues']
    return commands

def purge_qos(uuid):
    commands = []
    con = dbclient.connect()
    cur = con.cursor()
    transact = '["Open_vSwitch", {"op":"select", "table":"QoS", \
            "where":[["_uuid", "==", ["uuid", "%s"]]]}]'\
            % uuid
    cur.execute(transact)
    record = cur.fetchone()
    if record:
        queues = ovsdb_map(record["queues"]).pairs
        for queue_uuid in queues.values():
            commands.extend(['--', 'destroy', 'queue', queue_uuid])
    return commands

def get_queues():
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["queues"] = []
            cur = con.cursor()
            transact = '["Open_vSwitch", {"op":"select", "table":"Queue", \
                    "where":[]}]'
            cur.execute(transact)
            for record in cur.fetchall():
                response["queues"].append(json.loads(_get_queue(record)))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def _get_queue(record):
    response = {}

    config = ovsdb_map(record["other_config"]).pairs
    response["min_rate"] = int(config["min-rate"]) \
            if "min-rate" in config else None
    response["max_rate"] = int(config["max-rate"]) \
            if "max-rate" in config else None
    response["burst"] = int(config["burst"]) \
            if "burst" in config else None

    return response

def update_queue(data):
    nq = json.loads(data)
    commands = []
    nullify = []

    if nq['dscp']:
        commands.extend(['dscp=%d' % nq['dscp']])
    else:
        nullify.extend(['--', 'clear', 'Queue', nq['uuid'], 'dscp'])

    if nq['other_config']['min-rate']:
        commands.extend(['other_config:min-rate=%d' % nq['other_config']['min-rate']])
    else:
        nullify.extend(['--', 'remove', 'Queue', nq['uuid'], 'other_config', 'min-rate'])

    if nq['other_config']['max-rate']:
        commands.extend(['other_config:max-rate=%d' % nq['other_config']['max-rate']])
    else:
        nullify.extend(['--', 'remove', 'Queue', nq['uuid'], 'other_config', 'max-rate'])

    if nq['other_config']['burst']:
        commands.extend(['other_config:burst=%d' % nq['other_config']['burst']])
    else:
        nullify.extend(['--', 'remove', 'Queue', nq['uuid'], 'other_config', 'burst'])

    if nq['other_config']['priority']:
        commands.extend(['other_config:priority=%d' % nq['other_config']['priority']])
    else:
        nullify.extend(['--', 'remove', 'Queue', nq['uuid'], 'other_config', 'priority'])

    if commands:
        commands = ['set', 'Queue', nq['uuid']] + commands
    commands.extend(nullify)
    ret, err = vsctl.execute(commands)
    return get_queues()

def add_queue(data):
    nq = json.loads(data)
    commands = ['create', 'Queue']

    if nq['dscp']:
        commands.extend(['dscp=%d' % nq['dscp']])

    if nq['other_config']['min-rate']:
        commands.extend(['other_config:min-rate=%d' % nq['other_config']['min-rate']])

    if nq['other_config']['max-rate']:
        commands.extend(['other_config:max-rate=%d' % nq['other_config']['max-rate']])

    if nq['other_config']['burst']:
        commands.extend(['other_config:burst=%d' % nq['other_config']['burst']])

    if nq['other_config']['priority']:
        commands.extend(['other_config:priority=%d' % nq['other_config']['priority']])

    ret, err = vsctl.execute(commands)
    return get_queues()

def del_queue(data):
    q = json.loads(data)
    commands = ['destroy', 'Queue', q['uuid']]
    ret, err = vsctl.execute(commands)
    return get_queues()

def get_bonds(brname):
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["bonds"] = []
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["ports"]}]'\
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                records = row["ports"]
                response["bonds"] += json.loads(
                        _get_logical_ports(ovsdb_set(records).elements, \
                                ["webovs8_lag"]))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)


def update_bond(brname, data):
    bond = json.loads(data)
    commands = []

    if bond["vlan_config"]["vlan_mode"] == 'access':
        commands.extend([
            '--', 'set', 'Port', bond['name'], 'vlan_mode=access',
            '--', 'set', 'Port', bond['name'], 'tag=%d' %
            bond['vlan_config']['tag'],
            '--', 'clear', 'Port', bond['name'], 'trunks'])
    elif bond["vlan_config"]["vlan_mode"] == 'trunk':
        commands.extend([
            '--', 'set', 'Port', bond['name'], 'vlan_mode=trunk',
            '--', 'set', 'Port', bond['name'], 'tag=%d' %
            bond['vlan_config']['tag'],
            '--', 'set', 'Port', bond['name'], 'trunks=%s' %
                        str(bond['vlan_config']['trunks'])])

    commands.extend([
        '--', 'set', 'Interface', bond['name'], 'options:members=%s' %
        ','.join(bond['options']['members'])])

    if bond["options"]["link_speed"]:
        commands.extend(['--', 'set', 'Interface', bond['name'],
                "options:link_speed={0}".format(bond["options"]["link_speed"])])

    if bond['options']['lag_type'] == 'lacp':
        #commands.extend([
            #'--', 'set', 'Interface', bond['name'],
            #'options:lag_type=lacp'])

        if bond['options']['lacp-system-id']:
            commands.extend([
                '--', 'set', 'Interface', bond['name'],
                'options:lacp-system-id=%s' % bond['options']['lacp-system-id']])

        if bond['options']['lacp-time']:
            commands.extend([
                '--', 'set', 'Interface', bond['name'],
                'options:lacp-time=%s' % bond['options']['lacp-time']])

        if bond['options']['lacp-system-priority']:
            commands.extend([
                '--', 'set', 'Interface', bond['name'],
                'options:lacp-system-priority=%s' % bond['options']['lacp-system-priority']])

        if bond['options']['lacp-mode']:
            commands.extend([
                '--', 'set', 'Interface', bond['name'],
                'options:lacp-mode=%s' % bond['options']['lacp-mode']])
    elif bond['options']['lag_type'] == 'static':
        pass
        #commands.extend([
            #'--', 'set', 'Interface', bond['name'],
            #'options:lag_type=static'])

    ret, err = vsctl.execute(commands)
    return get_bonds(brname)

def del_bond(brname, data):
    bond = json.loads(data)
    commands = ['del-port', brname, bond['name']]
    ret, err = vsctl.execute(commands)
    return get_bonds(brname)

def add_bond(brname, data):
    response = {}
    new_bond = json.loads(data)
    commands = ['--', 'add-port', brname, new_bond['name'], '--',
            'set', 'Interface', new_bond['name'], 'type=webovs8_lag']
    bond_config = []

    if new_bond["vlan_config"]["vlan_mode"] == 'access':
        bond_config.extend([
            '--', 'set', 'Port', new_bond['name'], 'vlan_mode=access',
            '--', 'set', 'Port', new_bond['name'], 'tag=%d' %
            new_bond['vlan_config']['tag']])
    elif new_bond["vlan_config"]["vlan_mode"] == 'trunk':
        commands.extend([
            '--', 'set', 'Port', new_bond['name'], 'vlan_mode=trunk',
            '--', 'set', 'Port', new_bond['name'], 'tag=%d' %
            new_bond['vlan_config']['tag'],
            '--', 'set', 'Port', new_bond['name'], 'trunks=%s' %
                        str(new_bond['vlan_config']['trunks'])])

    if new_bond['options']['members']:
        bond_config.extend([
            '--', 'set', 'Interface', new_bond['name'], 'options:members=%s' % ','.join(new_bond['options']['members'])])
    #
    bond_config.extend(['--', 'set', 'Interface', new_bond['name'], 'options:link_speed=auto'])

    if new_bond['options']['lag_type'] == 'lacp':
        bond_config.extend([
            '--', 'set', 'Interface', new_bond['name'],
            'options:lag_type=lacp'])

        if new_bond['options']['lacp-system-id']:
            bond_config.extend([
                '--', 'set', 'Interface', new_bond['name'],
                'options:lacp-system-id=%s' % new_bond['options']['lacp-system-id']])

        if new_bond['options']['lacp-time']:
            bond_config.extend([
                '--', 'set', 'Interface', new_bond['name'],
                'options:lacp-time=%s' % new_bond['options']['lacp-time']])

        if new_bond['options']['lacp-system-priority']:
            bond_config.extend([
                '--', 'set', 'Interface', new_bond['name'],
                'options:lacp-system-priority=%s' % new_bond['options']['lacp-system-priority']])

        if new_bond['options']['lacp-mode']:
            bond_config.extend([
                '--', 'set', 'Interface', new_bond['name'],
                'options:lacp-mode=%s' % new_bond['options']['lacp-mode']])
    elif new_bond['options']['lag_type'] == 'static':
        bond_config.extend([
            '--', 'set', 'Interface', new_bond['name'],
            'options:lag_type=static'])

    commands.extend(bond_config)
    ret, err = vsctl.execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    return get_bonds(brname)

def get_tunnels(brname):
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["tunnels"] = []
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["ports"]}]'\
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                records = row["ports"]
                response["tunnels"] += json.loads(
                        _get_logical_ports(ovsdb_set(records).elements,
                            ["webovs8_gre", "webovs8_l2gre"]))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def update_tunnel(brname, data):
    tul = json.loads(data)
    commands = []

# Remote IP and Local IP are required
    commands.extend([
        '--', 'set', 'Interface', tul['name'], 'options:remote_ip=%s' %
        tul['options']['remote_ip']])
    commands.extend([
        '--', 'set', 'Interface', tul['name'], 'options:local_ip=%s' %
        tul['options']['local_ip']])
# SRC/DST MAC are optional
    if tul['options']['src_mac']:
        commands.extend([
            '--', 'set', 'Interface', tul['name'], 'options:src_mac=%s' %
            tul['options']['src_mac']])
    else:
        commands.extend([
            '--', 'remove', 'Interface', tul['name'], 'options', 'src_mac'])

    if tul['options']['dst_mac']:
        commands.extend([
            '--', 'set', 'Interface', tul['name'], 'options:dst_mac=%s' %
            tul['options']['dst_mac']])
    else:
        commands.extend([
            '--', 'remove', 'Interface', tul['name'], 'options', 'dst_mac'])

    if tul['options']['vlan']:
        commands.extend([
            '--', 'set', 'Interface', tul['name'], 'options:vlan=%s' %
            tul['options']['vlan']])
    else:
        commands.extend([
            '--', 'remove', 'Interface', tul['name'], 'options', 'vlan'])
# Egress port is required
    commands.extend([
        '--', 'set', 'Interface', tul['name'], 'options:egress_port=%s' %
        tul['options']['egress_port']])
# L2GRE port's key are required
    if tul['options']['l2gre_key']:
        commands.extend([
            '--', 'set', 'Interface', tul['name'], 'options:l2gre_key=%s' %
            tul['options']['l2gre_key']])

    ret, err = vsctl.execute(commands)
    return get_tunnels(brname)

def del_tunnel(brname, data):
    tul = json.loads(data)
    commands = ['del-port', brname, tul["name"]]
    ret, err = vsctl.execute(commands)
    return get_tunnels(brname)

def add_tunnel(brname, data):
    response = {}
    tul = json.loads(data)
    name = 'gre{0}'.format(tul['name']) if tul['type'] == 'webovs8_gre' else 'l2gre{0}'.format(tul['name'])
    commands = ['--', 'add-port', brname, name, '--',
            'set', 'Interface', name, 'type={0}'.format(tul['type'])]
    gre_config = []

# Remote IP and Local IP are required
    gre_config.extend([
        '--', 'set', 'Interface', name, 'options:remote_ip=%s' %
        tul['options']['remote_ip']])
    gre_config.extend([
        '--', 'set', 'Interface', name, 'options:local_ip=%s' %
        tul['options']['local_ip']])
# SRC/DST MAC are optional
    if tul['options']['src_mac']:
        gre_config.extend([
            '--', 'set', 'Interface', name, 'options:src_mac=%s' %
            tul['options']['src_mac']])
    if tul['options']['dst_mac']:
        gre_config.extend([
            '--', 'set', 'Interface', name, 'options:dst_mac=%s' %
            tul['options']['dst_mac']])
    if tul['options']['vlan']:
        gre_config.extend([
            '--', 'set', 'Interface', name, 'options:vlan=%s' %
            tul['options']['vlan']])
# Egress port is required
    gre_config.extend([
        '--', 'set', 'Interface', name, 'options:egress_port=%s' %
        tul['options']['egress_port']])
# L2GRE port's key are required
    if tul['options']['key']:
        gre_config.extend([
            '--', 'set', 'Interface', name, 'options:l2gre_key=%s' %
            tul['options']['key']])


    commands.extend(gre_config)
    ret, err = vsctl.execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    return get_tunnels(brname)

def get_bridge(brname):
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["bridge"] = None
            cur = con.cursor()
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]]}]' % brname
            cur.execute(transact)
            record = cur.fetchone()
            if record:
                response["bridge"] = json.loads(_get_bridge(record))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def get_bond(brname, bondname):
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["bond"] = None
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["ports"]}]' \
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                records = row["ports"]
                for uuid in ovsdb_set(records).elements:
                    transact = '["Open_vSwitch", {"op":"select", \
                            "table":"Port", \
                            "where":[["_uuid", "==", ["uuid", "%s"]],\
                            ["name", "==", "%s"]]}]' % (uuid, bondname)
                    cur.execute(transact)
                    record = cur.fetchone()
                    if record:
                        response["bond"] = json.loads(
                                _get_logical_port(uuid, ["webovs8_lag" ]))
                        break
        except InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        #DEBUG
        return json.dumps(response, indent=' '*2)

def get_tunnel(brname, tunnelname):
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["tunnel"] = None
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["ports"]}]' \
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                records = row["ports"]
                for uuid in ovsdb_set(records).elements:
                    transact = '["Open_vSwitch", {"op":"select", \
                            "table":"Port", \
                            "where":[["_uuid", "==", ["uuid", "%s"]],\
                            ["name", "==", "%s"]]}]' % (uuid, tunnelname)
                    cur.execute(transact)
                    record = cur.fetchone()
                    if record:
                        response["tunnel"] = json.loads(
                                _get_logical_port(uuid, ["webovs8_gre"]))
                        break
        except InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        #DEBUG
        return json.dumps(response, indent=' '*2)

def _get_bridge(record):
    """
    Caller provides this function with a record. So always assumes this
    function will succeed.
    """
    response = {}
    controller_uuids = ovsdb_set(record["controller"])
    port_uuids = ovsdb_set(record["ports"])
    sflow_uuid = ovsdb_set(record["sflow"])
    netflow_uuid = ovsdb_set(record["netflow"])
    mirror_uuids = ovsdb_set(record["mirrors"])
    response["name"] = record["name"]
    response["datapath_id"] = _record_to_response(record, "datapath_id")
    response["fail_mode"] = _record_to_response(record, "fail_mode")
    response["protocols"] = _record_to_response(record, "protocols")
    response["flood_vlans"] = _record_to_response(record, "flood_vlans")
    response["controller"] = json.loads(_get_controllers(controller_uuids.\
            elements))

    response.update(_get_all_ports(port_uuids.elements))

    response["sFlow"] = json.loads(_get_sflow(sflow_uuid.elements))
    response["NetFlow"] = json.loads(_get_netflow(netflow_uuid.elements))
    response["Mirrors"] = json.loads(_get_mirrors(mirror_uuids.elements))
    response["stp_config"] = {}
    other = ovsdb_map(record["other_config"]).pairs
    response["stp_config"]["stp_enable"] = _record_to_response(record, \
            "stp_enable")
    response["stp_config"]["stp-system-id"] = other["stp-system-id"] \
            if "stp-system-id" in other else None
    response["stp_config"]["stp-priority"] = other["stp-priority"] \
            if "stp-priority" in other else None
    response["stp_config"]["stp-hello-time"] = other["stp-hello-time"] \
            if "stp-hello-time" in other else None
    response["stp_config"]["stp-max-age"] = other["stp-max-age"] \
            if "stp-max-age" in other else None
    response["stp_config"]["stp-forward-delay"] = other["stp-forward-delay"] \
            if "stp-forward-delay" in other else None
    response["status"] = {}
    status = ovsdb_map(record["status"]).pairs
    response["status"]["stp_bridge_id"] = status["stp_bridge_id"] \
            if "stp_bridge_id" in status else None
    response["status"]["stp_designated_root"] = status["stp_designated_root"]\
            if "stp_designated_root" in status else None
    response["status"]["stp_root_path_cost"] = status["stp_root_path_cost"] \
            if "stp_root_path_cost" in status else None
    response["other"] = {}
    response["other"]["forward-bpdu"] = other["forward-bpdu"] \
            if "forward-bpdu" in other else None

    if "datapath_id" in other:
        response["datapath_id"] = other["datapath_id"]

    return json.dumps(response)

def _get_sflow(uuids):
    response = {}
    for uuid in uuids:
        con = dbclient.connect()
        cur = con.cursor()
        transact = '["Open_vSwitch", {"op":"select", "table":"sFlow", \
                "where":[["_uuid", "==", ["uuid", "%s"]]]}]'\
                % uuid
        cur.execute(transact)
        record = cur.fetchone()
        response["agent"] = _record_to_response(record, "agent")
        response["header"] = _record_to_response(record, "header")
        response["polling"] = _record_to_response(record, "polling")
        response["sampling"] = _record_to_response(record, "sampling")
        response["targets"] = []
        for tar in record["targets"].split(","):
            if tar:
                t = target(tar)
                response["targets"].append(dict(ip=t.ip, port=t.port))
    return json.dumps(response)

def _get_netflow(uuids):
    response = {}
    for uuid in uuids:
        con = dbclient.connect()
        cur = con.cursor()
        transact = '["Open_vSwitch", {"op":"select", "table":"NetFlow", \
                "where":[["_uuid", "==", ["uuid", "%s"]]]}]'\
                % uuid
        cur.execute(transact)
        record = cur.fetchone()
        response["active_timeout"] = _record_to_response(record, "active_timeout")
        response["targets"] = []
        for tar in record["targets"].split(","):
            if tar:
                t = target(tar)
                response["targets"].append(dict(ip=t.ip, port=t.port))
    return json.dumps(response)

def _get_mirrors(uuids):
    response = []
    for uuid in uuids:
        response.append(json.loads(_get_mirror(uuid)))
    return json.dumps(response)

def _get_mirror(uuid):
    response = {}
    con = dbclient.connect()
    cur = con.cursor()
    transact = '["Open_vSwitch", {"op":"select", "table":"Mirror", \
            "where":[["_uuid", "==", ["uuid", "%s"]]]}]'\
            % uuid
    cur.execute(transact)
    for record in cur.fetchall():
        response["name"] = _record_to_response(record, "name")
        response["select_all"] = _record_to_response(record, "select_all")
        response["select_src_port"] = []
        for port in ovsdb_set(record["select_src_port"]).elements:
            response["select_src_port"].append(port)
        response["select_dst_port"] = []
        for port in ovsdb_set(record["select_dst_port"]).elements:
            response["select_dst_port"].append(port)
        response["select_vlan"] = []
        for vlan in ovsdb_set(record["select_vlan"]).elements:
            response["select_vlan"].append(vlan)
        response["statistics"] = {}
        response["statistics"]["tx_bytes"] = 0
        response["statistics"]["tx_packets"] = 0
        for key, val in ovsdb_map(record["statistics"]).pairs.iteritems():
            response["statistics"][key] = val
        response["output_port"] = _record_to_response(record, "output_port")
        response["output_vlan"] = _record_to_response(record, "output_vlan")
# replace port uuid with port name
    tmp = []
    for uuid in response["select_src_port"]:
        tmp = [json.loads(_get_port(uuid))["name"] \
                for uuid in response["select_src_port"]]
    response["select_src_port"] = tmp
    tmp = []
    for uuid in response["select_dst_port"]:
        tmp = [json.loads(_get_port(uuid))["name"]\
                for uuid in response["select_dst_port"]]
    response["select_dst_port"] = tmp
    tmp = []
    if response["output_port"]:
        tmp = json.loads(_get_port(response["output_port"]))["name"]
    response["output_port"] = tmp
    return json.dumps(response)

def get_controllers(brname):
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["controllers"] = []
            transact = '["Open_vSwitch", {"op":"select","table":"Bridge",\
                    "where":[["name","==","%s"]],"columns":["controller"]}]'\
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                records = row["controller"]
                response["controllers"] += json.loads(
                        _get_controllers(ovsdb_set(records).elements))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def _get_controllers(uuids=[]):
    # caller should guarantee that bridge name is in the database
    response = []
    for uuid in uuids:
        response.append(_get_controller(uuid))
    return json.dumps(response)

def _get_controller(uuid):
    response = {}

    con = dbclient.connect()
    cur = con.cursor()
    transact = '["Open_vSwitch", {"op":"select", "table":"Controller", \
            "where":[["_uuid", "==", ["uuid", "%s"]]]}]'  % uuid
    cur.execute(transact)
    record = cur.fetchone()
    # refer to documentation for the controller object JSON schema
    t = target(record["target"])
    response["uuid"] = _record_to_response(record, "_uuid")
    response["ip"] = t.ip
    response["port"] = t.port
    response["protocol"] = t.protocol
    response["connection_mode"] = _record_to_response(record, \
            "connection_mode")
    response["max_backoff"] = _record_to_response(record, "max_backoff")
    response["inactivity_probe"] = _record_to_response(record, \
            "inactivity_probe")
    response["enable_async_messages"] = _record_to_response(record, \
            "enable_async_messages")
    response["controller_rate_limit"] = _record_to_response(record, \
            "controller_rate_limit")
    response["controller_burst_limit"] = _record_to_response(record, \
            "controller_burst_limit")
    response["in_band"] = {}
    response["in_band"]["local_ip"] = _record_to_response(record, "local_ip")
    response["in_band"]["local_netmask"] = _record_to_response(record, \
            "local_netmask")
    response["in_band"]["local_gateway"] = _record_to_response(record, \
            "local_gateway")
    response["is_connected"] = record["is_connected"]
    response["role"] = _record_to_response(record, "role")
    response["status"] = {}
    status = ovsdb_map(record["status"])
    response["status"]["state"] = status.pairs.get("state", None)
    return response

def get_ports(brname):
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["ports"] = []
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["ports"]}]'\
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                records = row["ports"]
                response["ports"] += json.loads(
                        _get_ports(ovsdb_set(records).elements))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def get_port(brname, portname):
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["port"] = None
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["ports"]}]'\
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                records = row["ports"]
                for uuid in ovsdb_set(records).elements:
                    transact = '["Open_vSwitch", {"op":"select", \
                            "table":"Port", \
                            "where":[["_uuid", "==", ["uuid", "%s"]], \
                            ["name", "==", "%s"]]}]' % (uuid, portname)
                    cur.execute(transact)
                    record = cur.fetchone()
                    if record:
                        response["port"] = json.loads(_get_port(uuid))
                        break
        except InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        #DEBUG
        return json.dumps(response, indent=' '*2)

def _get_ports(uuids=[]):
    response = []
    for uuid in uuids:
        port = _get_port(uuid)
        # _get_port should return "{}" if uuid is not a physical port
        if json.loads(port):
            response.append(json.loads(port))
    return json.dumps(response)

def _get_port(uuid):
    response = {}

    port_record, iface_record = _iface_record_helper(uuid)

    if not iface_record["type"] or iface_record["type"] == "webovs8" or iface_record["type"] == "pronto":
        response["name"] = port_record["name"]
        response["mac"] = _record_to_response(port_record, "mac")
        response["qos"] = _record_to_response(port_record, "qos")
        # QoS
        if response["qos"]:
            response["qos"] = get_qos(response["qos"])
        else:
            response["qos"] = {"uuid":None, "queues":None}
        response["ofport"] = iface_record["ofport"]
        response["type"] = "phy"
        response["vlan_config"] = {}
        response["vlan_config"]["vlan_mode"] =\
                _record_to_response(port_record, "vlan_mode")
        response["vlan_config"]["tag"] =\
                _record_to_response(port_record, "tag")
        response["vlan_config"]["trunks"] =\
                _record_to_response(port_record, "trunks")
        response["status"] = {}
        # Status
        status = ovsdb_map(port_record["status"]).pairs
        response["status"]["stp_port_id"] = status["stp_port_id"]\
                if "stp_port_id" in status else None
        response["status"]["stp_state"] = status["stp_state"]\
                if "stp_state" in status else None
        response["status"]["stp_sec_in_state"] = status["stp_sec_in_state"]\
                if "stp_sec_in_state" in status else None
        response["status"]["stp_role"] = status["stp_role"]\
                if "stp_role" in status else None
        response["admin_state"] = _record_to_response(iface_record,
                "admin_state")
        response["link_state"] = _record_to_response(iface_record,\
                "link_state")
        response["duplex"] = _record_to_response(iface_record, "duplex")
        response["mtu"] = _record_to_response(iface_record, "mtu")
        response["link_speed"] = _record_to_response(iface_record, "link_speed")
        response["options"] = {}
        # Options
        options = ovsdb_map(iface_record["options"]).pairs
        response["options"]["link_speed"] = options["link_speed"]\
                if "link_speed" in options else None
        response["options"]["flow_ctl"] = options["flow_ctl"]\
                if "flow_ctl" in options else None
        response["options"]["lacp-port-id"] = int(options["lacp-port-id"])\
                if "lacp-port-id" in options else None
        response["options"]["lacp-port-priority"] = \
                int(options["lacp-port-priority"]) if "lacp-port-priority" in \
                options else None
        response["options"]["lacp-aggregation-key"] = \
                int(options["lacp-aggregation-key"]) if "lacp-aggregation-key" \
                in options else None
        # Statistics
        response["statistics"] = {}
        response["statistics"]["counters"] = {}
        response["statistics"]["receive_errors"] = {}
        response["statistics"]["transmit_errors"] = {}
        response["statistics"]["stp_counters"] = {}
        statistics = ovsdb_map(iface_record["statistics"]).pairs
# counter
        response["statistics"]["counters"]["rx_packets"] = \
                statistics["rx_packets"] if "rx_packets" in statistics else None
        response["statistics"]["counters"]["rx_bytes"] = \
                statistics["rx_bytes"] if "rx_bytes" in statistics else None
        response["statistics"]["counters"]["tx_packets"] = \
                statistics["tx_packets"] if "tx_packets" in statistics else None
        response["statistics"]["counters"]["tx_bytes"] = \
                statistics["tx_bytes"] if "tx_bytes" in statistics else None
# receive_errors
        response["statistics"]["receive_errors"]["rx_dropped"] = \
                statistics["rx_dropped"] if "rx_dropped" in statistics else None
        response["statistics"]["receive_errors"]["rx_frame_err"] = \
                statistics["rx_frame_err"] if "rx_frame_err" in statistics else None
        response["statistics"]["receive_errors"]["rx_over_err"] = \
                statistics["rx_over_err"] if "rx_over_err" in statistics else None
        response["statistics"]["receive_errors"]["rx_crc_err"] = \
                statistics["rx_crc_err"] if "rx_crc_err" in statistics else None
        response["statistics"]["receive_errors"]["rx_errors"] = \
                statistics["rx_errors"] if "rx_errors" in statistics else None
# transmit_errors
        response["statistics"]["transmit_errors"]["tx_dropped"] = \
                statistics["tx_dropped"] if "tx_dropped" in statistics else None
        response["statistics"]["transmit_errors"]["collisions"] = \
                statistics["collisions"] if "collisions" in statistics else None
        response["statistics"]["transmit_errors"]["tx_errors"] = \
                statistics["tx_errors"] if "tx_errors" in statistics else None
# stp counters
        stp_counter = ovsdb_map(port_record["statistics"]).pairs
        response["statistics"]["stp_counters"]["stp_tx_count"] = \
                stp_counter["stp_tx_count"] \
                if "stp_tx_count" in stp_counter else None
        response["statistics"]["stp_counters"]["stp_rx_count"] = \
                stp_counter["stp_rx_count"] \
                if "stp_rx_count" in stp_counter else None
        response["statistics"]["stp_counters"]["stp_error_count"] = \
                stp_counter["stp_error_count"] \
                if "stp_error_count" in stp_counter else None

    return json.dumps(response)

def update_port(brname, data):
    response = {}
    port = json.loads(data)
    commands = []
    if port["vlan_config"]["vlan_mode"] == 'access':
        commands = ['--', 'set', 'Port', port['name'], 'vlan_mode=access',
                '--', 'set', 'Port', port['name'], 'tag=%d' % \
                        port['vlan_config']['tag'],
                        '--', 'clear', 'Port', port['name'], 'trunks']
    elif port["vlan_config"]["vlan_mode"] == 'trunk':
        commands = ['--', 'set', 'Port', port['name'], 'vlan_mode=trunk',
                '--', 'set', 'Port', port['name'], 'tag=%d' % \
                        port['vlan_config']['tag']]
        if port['vlan_config']['trunks']:
            commands.extend(['--', 'set', 'Port', port['name'], 'trunks=%s' % \
                        ','.join(map(lambda x: str(x), port['vlan_config']['trunks']))])
        else:
            commands.extend(['--', 'clear', 'Port', port['name'], 'trunks'])

    if port["options"]["link_speed"]:
        commands.extend(['--', 'set', 'Interface', port['name'],
                "options:link_speed={0}".format(port["options"]["link_speed"])])
    else:
        commands.extend(['--', 'remove', 'Interface', port['name'],
            'options', 'link_speed'])

    if port["options"]["flow_ctl"]:
        commands.extend(['--', 'set', 'Interface', port['name'],
                "options:flow_ctl={0}".format(port["options"]["flow_ctl"])])

    # LACP configure update
    if port["options"]["lacp-port-id"] is not None:
        commands.extend(['--', 'set', 'Interface', port['name'],
                "options:lacp-port-id={0}".format(port["options"]["lacp-port-id"])])
    else:
        commands.extend(['--', 'remove', 'Interface', port['name'],
            'options', 'lacp-port-id'])

    if port["options"]["lacp-port-priority"] is not None:
        commands.extend(['--', 'set', 'Interface', port['name'],
                "options:lacp-port-priority={0}".format(port["options"]["lacp-port-priority"])])
    else:
        commands.extend(['--', 'remove', 'Interface', port['name'],
            'options', 'lacp-port-priority'])

    if port["options"]["lacp-aggregation-key"] is not None:
        commands.extend(['--', 'set', 'Interface', port['name'],
                "options:lacp-aggregation-key={0}".format(port["options"]["lacp-aggregation-key"])])
    else:
        commands.extend(['--', 'remove', 'Interface', port['name'],
            'options', 'lacp-aggregation-key'])

    # QoS update
    if port["qos"]["uuid"] is None:
        if port["qos"]["queues"] is not None:
            commands.extend(add_queues(port["name"], port["qos"]["queues"]))
    else:
        # Destroy queues related to this QoS
        if port["qos"]["queues"] is None:
            commands.extend(['--', 'clear', 'Port', port['name'], 'qos'])
            commands.extend(_del_qos(port["qos"]["uuid"]))
            commands.extend(purge_qos(port["qos"]["uuid"]))
        elif port["qos"]["queues"] == []:
            commands.extend(_clear_qos(port["qos"]["uuid"]))
            commands.extend(purge_qos(port["qos"]["uuid"]))
        else:
            commands.extend(add_queues(port["name"], port["qos"]["queues"],
                qosuuid=port["qos"]["uuid"]))
            commands.extend(purge_qos(port["qos"]["uuid"]))

    ret, err = vsctl.execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_port(brname, port['name'])

def add_queues(name, queues, qosuuid=None):
    commands = []
    queuemap = ""
    if qosuuid is None:
        # Create queues one by one
        for queue in queues:
            queuemap += "{id}=@q{id},".format(id=queue["queue_id"])
            commands.extend(['--', '--id=@q{0}'.format(queue["queue_id"]),
                'create', 'Queue'])
            if "min_rate" in queue and queue["min_rate"] is not None:
                commands.append('other-config:min-rate={0}'.format(queue\
                        ["min_rate"]))
            if "max_rate" in queue and queue["max_rate"] is not None:
                commands.append('other-config:max-rate={0}'.format(queue\
                        ["max_rate"]))
            if "burst" in queue and queue["burst"] is not None:
                commands.append('other-config:burst={0}'.format(queue\
                        ["burst"]))
        # Create QoS
        commands.extend(['--', 'set', 'Port', name, 'qos=@newqos',
            '--', '--id=@newqos', 'create', 'QoS', 'type=PRONTO_STRICT'])
        if queuemap:
            commands.append('queues={0}'.format(queuemap))
    else:
        # Create queues one by one
        for queue in queues:
            queuemap += "{id}=@q{id},".format(id=queue["queue_id"])
            commands.extend(['--', '--id=@q{0}'.format(queue["queue_id"]),
                'create', 'Queue'])
            if "min_rate" in queue and queue["min_rate"] is not None:
                commands.append('other-config:min-rate={0}'.format(queue\
                        ["min_rate"]))
            if "max_rate" in queue and queue["max_rate"] is not None:
                commands.append('other-config:max-rate={0}'.format(queue\
                        ["max_rate"]))
            if "burst" in queue and queue["burst"] is not None:
                commands.append('other-config:burst={0}'.format(queue\
                        ["burst"]))
        # Attach to QoS
        commands.extend(['--', 'set', 'Port', name, 'qos={0}'.format(\
                qosuuid), '--', 'set', 'QoS', qosuuid])
        if queuemap:
            commands.append('queues={0}'.format(queuemap))

    return commands

def del_port(brname, data):
    response = {}
    port = json.loads(data)
    commands = ['--', 'del-port', brname, port["name"]]
    extra_commands = []

    con = dbclient.connect()
    transact = '["Open_vSwitch", {"op":"select", "table":"Port", \
            "where":[["name", "==", "%s"]], "columns":["qos"]}]' % port["name"]
    cur = con.cursor()
    cur.execute(transact)
    row = cur.fetchone()
    if row:
        record = row["qos"]
        if _is_type_uuid(record):
            uuid = record[1]
            extra_commands.extend(_del_qos(uuid))
            extra_commands.extend(purge_qos(uuid))

    # There are references to QoS in the Port table, so we have to do this in
    # two steps.
    ret, err = vsctl.execute(commands)
    if extra_commands:
        ret, err = vsctl.execute(extra_commands)

    if err:
        response["ret"] = -1
        response["msg"] = err
    else:
        response["ret"] = 0
    return json.dumps(response)

def add_port(brname, data):
    response = {}
    port = json.loads(data)
    commands = ['--', 'add-port', brname, port["name"], '--',
            'set', 'Interface', port["name"], 'type=webovs8']

    vlan_config = []

    if port['vlan_config']['vlan_mode'] == 'access':
        vlan_config = ['--', 'set', 'Port', port["name"], 'vlan_mode=access',
                '--', 'set', 'Port', port["name"],
                'tag=%d' % port['vlan_config']['tag']]
    elif port['vlan_config']['vlan_mode'] == 'trunk':
        vlan_config = ['--', 'set', 'Port', port["name"], 'vlan_mode=trunk',
                '--', 'set', 'Port', port["name"],
                'tag=%d' % port['vlan_config']['tag']]
        if port['vlan_config']['trunks']:
                vlan_config.extend(['--', 'set', 'Port', port["name"],
                'trunks=%s' % ','.join(map(lambda x: str(x), port['vlan_config']['trunks']))])

    commands.extend(vlan_config)

    commands.extend(['--', 'set', 'Interface', port['name'],
                'options:flow_ctl={0}'.format(port['options']['flow_ctl']), 'options:link_speed=auto'])
    ret, err = vsctl.execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_port(brname, port["name"])

def _get_logical_ports(uuids, types):
    response = []
    for uuid in uuids:
        port = _get_logical_port(uuid, types)
        # _get_port should return "{}" if uuid is not a physical port
        if json.loads(port):
            response.append(json.loads(port))
    return json.dumps(response)

def _get_logical_port(uuid, types):
    response = {}
    port_record, iface_record = _iface_record_helper(uuid)

    if iface_record["type"] in types:
        response["name"] = port_record["name"]
        response["ofport"] = iface_record["ofport"]
        response["type"] = iface_record["type"]
        response["vlan_config"] = {}
        response["vlan_config"]["vlan_mode"] =\
                _record_to_response(port_record, "vlan_mode")
        response["vlan_config"]["tag"] =\
                _record_to_response(port_record, "tag")
        response["vlan_config"]["trunks"] =\
                _record_to_response(port_record, "trunks")
        response["link_speed"] = _record_to_response(iface_record, "link_speed")
        response["options"] = {}
        response["admin_state"] = _record_to_response(iface_record,
                "admin_state")
        response["link_state"] = _record_to_response(iface_record,\
                "link_state")
        response["mac_in_use"] = _record_to_response(iface_record,
                                                     "mac_in_use")
        # Options
        options = ovsdb_map(iface_record["options"]).pairs
        response["options"]["link_speed"] = options["link_speed"]\
                if "link_speed" in options else None
        response["options"]["lag_type"] = options["lag_type"]\
                if "lag_type" in options else None
        response["options"]["members"] = options["members"].split(",")\
                if "members" in options else None
        response["options"]["lacp-system-id"] = options["lacp-system-id"]\
                if "lacp-system-id" in options else response["mac_in_use"]
        response["options"]["lacp-system-priority"] = \
                options["lacp-system-priority"] \
                if "lacp-system-priority" in options else None
        response["options"]["lacp-time"] = options["lacp-time"]\
                if "lacp-time" in options else None
        response["options"]["lacp-mode"] = options["lacp-mode"]\
                if "lacp-mode" in options else None
        response["options"]["lacp-port-id"] = options["lacp-port-id"]\
                if "lacp-port-id" in options else None
        response["options"]["lacp-port-priority"] = \
                options["lacp-port-priority"] \
                if "lacp-port-priority" in options else None
        response["options"]["lacp-aggregation-key"] = \
                options["lacp-aggregation-key"] \
                if "lacp-aggregation-key" in options else None
        response["options"]["remote_ip"] = options["remote_ip"]\
                if "remote_ip" in options else None
        response["options"]["local_ip"] = options["local_ip"]\
                if "local_ip" in options else None
        response["options"]["vlan"] = int(options["vlan"])\
                if "vlan" in options else None
        response["options"]["src_mac"] = options["src_mac"]\
                if "src_mac" in options else None
        response["options"]["dst_mac"] = options["dst_mac"]\
                if "dst_mac" in options else None
        response["options"]["egress_port"] = options["egress_port"]\
                if "egress_port" in options else None
        response["options"]["l2gre_key"] = int(options["l2gre_key"])\
                if "l2gre_key" in options else None
# Statistics
        response["statistics"] = {}
        response["statistics"]["counters"] = {}
        response["statistics"]["receive_errors"] = {}
        response["statistics"]["transmit_errors"] = {}
        statistics = ovsdb_map(iface_record["statistics"]).pairs
# counter
        response["statistics"]["counters"]["rx_packets"] = \
                statistics["rx_packets"] if "rx_packets" in statistics else None
        response["statistics"]["counters"]["rx_bytes"] = \
                statistics["rx_bytes"] if "rx_bytes" in statistics else None
        response["statistics"]["counters"]["tx_packets"] = \
                statistics["tx_packets"] if "tx_packets" in statistics else None
        response["statistics"]["counters"]["tx_bytes"] = \
                statistics["tx_bytes"] if "tx_bytes" in statistics else None
# receive_errors
        response["statistics"]["receive_errors"]["rx_dropped"] = \
                statistics["rx_dropped"] if "rx_dropped" in statistics else None
        response["statistics"]["receive_errors"]["rx_frame_err"] = \
                statistics["rx_frame_err"] if "rx_frame_err" in statistics else None
        response["statistics"]["receive_errors"]["rx_over_err"] = \
                statistics["rx_over_err"] if "rx_over_err" in statistics else None
        response["statistics"]["receive_errors"]["rx_crc_err"] = \
                statistics["rx_crc_err"] if "rx_crc_err" in statistics else None
        response["statistics"]["receive_errors"]["rx_errors"] = \
                statistics["rx_errors"] if "rx_errors" in statistics else None
# transmit_errors
        response["statistics"]["transmit_errors"]["tx_dropped"] = \
                statistics["tx_dropped"] if "tx_dropped" in statistics else None
        response["statistics"]["transmit_errors"]["collisions"] = \
                statistics["collisions"] if "collisions" in statistics else None
        response["statistics"]["transmit_errors"]["tx_errors"] = \
                statistics["tx_errors"] if "tx_errors" in statistics else None

    return json.dumps(response)

def _get_all_ports(uuids=[]):
    response = {
            "ports" : [],
            "lags" : [],
            "tunnels" : []
            }
    tunnel_types = ['webovs8_gre', 'pronto_gre', 'webovs8_l2gre']

    for uuid in uuids:
        port_record, iface_record = _iface_record_helper(uuid)
        if not iface_record["type"] or \
                iface_record["type"] == "webovs8" or \
                iface_record["type"] == "pronto":
            response["ports"].append(json.loads(_get_port(uuid)))
        elif iface_record["type"] in tunnel_types:
            response["tunnels"].append(json.loads(_get_logical_port(uuid, tunnel_types)))
        elif iface_record["type"] == "webovs8_lag":
            response["lags"].append(json.loads(_get_logical_port(uuid, ["webovs8_lag"])))

    return response

def get_sflow(brname):
    """
    If Bridge brname doesn't have an sflow record, return None.
    """
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["sflow"] = None
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["sflow"]}]' \
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                record = row["sflow"]
                if record[1]:
                    response["sflow"] = json.loads(_get_sflow(ovsdb_set(record).elements))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def update_sflow(brname, data):
    sf = json.loads(data)
    commands = ['--', 'set', 'sFlow', brname]
    clear = []

    if sf['agent']:
        commands.extend(['agent=%s' % sf['agent']])
    else:
        clear.extend(['--', 'clear', 'sflow', brname, 'agent'])

    if sf['header']:
       commands.extend(['header=%d' % sf['header']])
    else:
        clear.extend(['--', 'clear', 'sflow', brname, 'header'])

    if sf['polling']:
       commands.extend(['polling=%d' % sf['polling']])
    else:
        clear.extend(['--', 'clear', 'sflow', brname, 'polling'])

    if sf['sampling']:
       commands.extend(['sampling=%d' % sf['sampling']])
    else:
        clear.extend(['--', 'clear', 'sflow', brname, 'sampling'])

    targets = ''
    for target in sf['targets']:
        targets += target['ip'] + ':' + str(target['port']) + ','
    commands.extend(['targets=%s' % ('\"' + targets + '\"')])

    commands.extend(clear)

    ret, err = vsctl.execute(commands)
    return get_sflow(brname)

def del_sflow(brname, data):
    sf = json.loads(data)
    commands = ['clear', 'Bridge', brname, 'sflow']
    ret, err = vsctl.execute(commands)
    return get_sflow(brname)

def add_sflow(brname, data):
    sf = json.loads(data)
    commands = ['--', 'set', 'Bridge', brname, 'sflow=@sf',
            '--', '--id=@sf', 'create', 'sFlow']

    if sf['agent']:
        commands.extend(['agent=%s' % sf['agent']])

    if sf['header']:
       commands.extend(['header=%d' % sf['header']])

    if sf['polling']:
       commands.extend(['polling=%d' % sf['polling']])

    if sf['sampling']:
       commands.extend(['sampling=%d' % sf['sampling']])

    targets = ''
    for target in sf['targets']:
        targets += target['ip'] + ':' + str(target['port']) + ','
    commands.extend(['targets=%s' % ('\"' + targets + '\"')])

    ret, err = vsctl.execute(commands)
    return get_sflow(brname)

def get_netflow(brname):
    """
    If Bridge brname doesn't have an newflow record, return None.
    """
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["netflow"] = None
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["netflow"]}]' \
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                record = row["netflow"]
                if record[1]:
                    response["netflow"] = json.loads(_get_netflow(ovsdb_set(record).elements))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def update_netflow(brname, data):
    nf = json.loads(data)
    commands = ['--', 'set', 'NetFlow', brname]

    commands.extend(['active_timeout=%d' % nf['active_timeout']])

    targets = ''
    for target in nf['targets']:
        targets += target['ip'] + ':' + str(target['port']) + ','
    commands.extend(['targets=%s' % ('\"' + targets + '\"')])

    ret, err = vsctl.execute(commands)
    return get_netflow(brname)

def del_netflow(brname, data):
    nf = json.loads(data)
    commands = ['clear', 'Bridge', brname, 'netflow']
    ret, err = vsctl.execute(commands)
    return get_netflow(brname)

def add_netflow(brname, data):
    nf = json.loads(data)
    commands = ['--', 'set', 'Bridge', brname, 'netflow=@nf',
            '--', '--id=@nf', 'create', 'NetFlow']

    commands.extend(['active_timeout=%d' % nf['active_timeout']])

    targets = ''
    for target in nf['targets']:
        targets += target['ip'] + ':' + str(target['port']) + ','
    commands.extend(['targets=%s' % ('\"' + targets + '\"')])

    ret, err = vsctl.execute(commands)
    return get_netflow(brname)

def get_mirrors(brname):
    """
    If Bridge brname doesn't contain any mirror records, return an empty list.
    """
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["mirrors"] = []
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["mirrors"]}]' \
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            row = cur.fetchone()
            if row:
                records = row["mirrors"]
                response["mirrors"] = json.loads(_get_mirrors(ovsdb_set(records).elements))
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        return json.dumps(response)

def get_mirror(brname, mirname):
    """
    If Bridge brname doesn't contain any mirror named mirname, return None.
    """
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            response["mirror"] = None
            transact = '["Open_vSwitch", {"op":"select", "table":"Bridge", \
                    "where":[["name", "==", "%s"]], "columns":["mirrors"]}]' \
                    % brname
            cur = con.cursor()
            cur.execute(transact)
            record = cur.fetchone()
            if record:
                records = record["mirrors"]
                for uuid in ovsdb_set(records).elements:
                    transact = '["Open_vSwitch", {"op":"select", "table":"Mirror", \
                        "where":[["_uuid", "==", ["uuid", "%s"]], \
                        ["name", "==", "%s"]]}]' % (uuid, mirname)
                    cur.execute(transact)
                    record = cur.fetchone()
                    if record:
                        response["mirror"] = json.loads(_get_mirror(uuid))
                        break
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except:
            print "error"
        return json.dumps(response)

def update_mirror(brname, data):
    mirror = json.loads(data)
    commands = []
    if mirror['output_port']:
        ids = port_ids(mirror['select_src_port'],
                mirror['select_dst_port'],
                mirror['output_port'])
        commands.extend(ids)
        if not mirror['select_src_port']:
            commands.extend([
                '--', 'clear', 'Mirror', mirror['name'], 'select_src_port'])
        if not mirror['select_dst_port']:
            commands.extend([
                '--', 'clear', 'Mirror', mirror['name'], 'select_dst_port'])
        # Clear VLAN section
        commands.extend([
            '--', 'clear', 'Mirror', mirror['name'], 'select_vlan',
            '--', 'clear', 'Mirror', mirror['name'], 'output_vlan'])
        commands.extend([
            '--', 'set', 'Mirror', mirror['name']])
        if mirror['select_src_port']:
            commands.append('select_src_port=%s' %
                ','.join(add_prefix(mirror['select_src_port'])))
        if mirror['select_dst_port']:
            commands.append('select_dst_port=%s' %
                ','.join(add_prefix(mirror['select_dst_port'])))
        commands.append('output_port=%s' %
            add_prefix(mirror['output_port']))
    elif mirror['output_vlan']:
        # Clear Port section
        commands.extend([
            '--', 'clear', 'Mirror', mirror['name'], 'select_src_port',
            '--', 'clear', 'Mirror', mirror['name'], 'select_dst_port',
            '--', 'clear', 'Mirror', mirror['name'], 'output_port'])
        commands.extend([
            '--', 'set', 'Mirror', mirror['name'],
            'name=%s' % mirror['name'],
            'select_vlan=%s' % mirror['select_vlan'],
            'output_vlan=%s' % mirror['output_vlan']])

    ret, err = vsctl.execute(commands)
    return get_mirrors(brname)

def del_mirror(brname, data):
    mirror = json.loads(data)
    commands = ['--', '--id=@tmp', 'get', 'Mirror', mirror['name'],
            '--', 'remove', 'Bridge', brname, 'mirrors', '@tmp']
    ret, err = vsctl.execute(commands)
    return get_mirrors(brname)

def add_prefix(input):
    if isinstance(input, str) or isinstance(input, unicode):
        return ','.join(map(lambda x: '@'+x if x else x, input.split(',')))
    else:
        return map(lambda x: '@'+x, input)

def port_ids(*args):
    ports = set([])
    for x in args:
        if x:
            if isinstance(x, list):
                ports.update(x)
            elif isinstance(x, str) or isinstance(x, unicode):
                ports.update(x.split(','))
    return reduce(lambda x, y: x + y, map(lambda p: ['--', '--id=@%s' % p, 'get', 'Port', p], ports))

def add_mirror(brname, data):
    mirror = json.loads(data)
    commands = ['--', 'add', 'Bridge', brname, 'mirrors', '@tmp']
    if mirror['output_port']:
        ids = port_ids(mirror['select_src_port'],
                mirror['select_dst_port'],
                mirror['output_port'])
        commands.extend(ids)
        commands.extend([
            '--', '--id=@tmp', 'create', 'mirror',
            'name=%s' % mirror['name']])
        if mirror['select_src_port']:
            commands.append('select_src_port=%s' %
                ','.join(add_prefix(mirror['select_src_port'])))
        if mirror['select_dst_port']:
            commands.append('select_dst_port=%s' %
                ','.join(add_prefix(mirror['select_dst_port'])))
        commands.append('output_port=%s' %
            add_prefix(mirror['output_port']))
    elif mirror['output_vlan']:
        commands.extend([
            '--', '--id=@tmp', 'create', 'mirror',
            'name=%s' % mirror['name'],
            'select_vlan=%s' % mirror['select_vlan'],
            'output_vlan=%s' % mirror['output_vlan']])

    ret, err = vsctl.execute(commands)
    return get_mirrors(brname)

def add_bridge(br_name):
    protocols = 'OpenFlow10,OpenFlow12,OpenFlow13,OpenFlow14'
    commands = ['--', 'add-br', br_name, '--', \
            'set', 'Bridge', br_name, 'datapath_type=webovs8', '--', \
            'set', 'Bridge', br_name, 'fail_mode=standalone', '--', \
            'set', 'Bridge', br_name, 'protocols={0}'.format(protocols)]
    ret, err = vsctl.execute(commands)
    return fast_get_bridges()

def del_bridge(br_name):
    command = ['--', 'del-br', br_name]
    ret, err = vsctl.execute(command)
    return fast_get_bridges()

def add_controller(br_name, data):
    newctrl = json.loads(data)

    if newctrl["protocol"] == 'tcp' or newctrl["protocol"] == 'ssl':
        # assume newctrl["ip"] is not None
        if not newctrl["port"]:
            newctrl["port"] = 6633
        target = ":".join([newctrl["protocol"], newctrl["ip"], str(newctrl["port"])])
    elif newctrl["protocol"] == 'ptcp' or newctrl["protocol"] == 'pssl':
        if not newctrl["port"]:
            newctrl["port"] = 6633
        # newctrl["ip"] not used
        target = ":".join([newctrl["protocol"], str(newctrl["port"])])
    target = 'target=%s' % ('\"' + target + '\"')
    commands = ['--', 'add', 'Bridge', br_name, 'controller', '@new', '--',\
            '--id=@new', 'create', 'Controller', target]

    if newctrl["connection_mode"]:
        connection_mode = 'connection_mode=%s' % newctrl["connection_mode"]
        commands.append(connection_mode)

    ret, err = vsctl.execute(commands)
    return get_controllers(br_name)

def del_controller(br_name, data):
    newctrl = json.loads(data)
    commands = ['--', 'remove', 'Bridge', br_name, 'controller', \
            newctrl["uuid"]]
    ret, err = vsctl.execute(commands)
    return get_controllers(br_name)

def update_controller(br_name, data):
    newctrl = json.loads(data)
    if newctrl["protocol"] == 'tcp' or newctrl["protocol"] == 'ssl':
        # assume newctrl["ip"] is not None
        if not newctrl["port"]:
            newctrl["port"] = 6633
        target = ":".join([newctrl["protocol"], newctrl["ip"], str(newctrl["port"])])
    elif newctrl["protocol"] == 'ptcp' or newctrl["protocol"] == 'pssl':
        if not newctrl["port"]:
            newctrl["port"] = 6633
        # newctrl["ip"] not used
        target = ":".join([newctrl["protocol"], str(newctrl["port"])])
    target = 'target=%s' % ('\"' + target + '\"')

    commands = ['--', 'set', 'Controller', newctrl["uuid"], target]

    if newctrl["connection_mode"]:
        connection_mode = 'connection_mode=%s' % newctrl["connection_mode"]
        commands.append(connection_mode)
    # TODO: more columns to set
    ret, err = vsctl.execute(commands)
    return get_controllers(br_name)

def get_groups(br_name):
    response = {}
    desc_command = ['dump-groups', br_name]
    ret, err = ofctl.of_execute(desc_command)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)

    response["ret"] = 0
    response["groups"] = []
    # Regex
    groups = re.compile("group_id=\d+,type=\w+,[^\s]+")
    basic = re.compile("group_id=(\d+),type=(\w+)")
    buckets = re.compile(r"bucket=(?:watch_port:(\d+),)?(?:watch_group:(\d+),)?actions=([^=]*)(?=,bucket)")
    last_bucket = re.compile(r"bucket=(?:watch_port:(\d+),)?(?:watch_group:(\d+),)?actions=([^=]*)$")
    for group in groups.findall(ret):
        new_group = {}
        m = basic.search(group)
        group_id = m.group(1)
        group_type = m.group(2)
        new_group["id"] = int(group_id)
        new_group["type"] = group_type
        new_group["buckets"] = []
        for watch_port, watch_group, actions in buckets.findall(group):
            bucket = {}
            bucket["watch_port"] = int(watch_port) if watch_port else None
            bucket["watch_group"] = int(watch_group) if watch_group else None
            bucket["actions_list"] = actions
            new_group["buckets"].append(bucket)
        for watch_port, watch_group, actions in last_bucket.findall(group):
            bucket = {}
            bucket["watch_port"] = int(watch_port) if watch_port else None
            bucket["watch_group"] = int(watch_group) if watch_group else None
            bucket["actions_list"] = actions
            new_group["buckets"].append(bucket)
        response["groups"].append(new_group)

    for group in response["groups"]:
        # Group Statistics
        stats_command = ['dump-group-stats', br_name, 'group_id={0}'.format(group["id"])]
        ret, err = ofctl.of_execute(stats_command)

        # Regex
        stats = re.compile("group_id=\d+,[^\s]+")
        counters = re.compile("duration=(.*?),ref_count=(\d+),packet_count=(\d+),byte_count=(\d+)")

        stat = stats.search(ret)

        m = counters.search(stat.group(0))
        # group = response["groups"][index]
        group["counters"] = {}
        group["duration"] = m.group(1)
        group["ref_count"] = m.group(2)
        group["counters"]["packet_count"] = m.group(3)
        group["counters"]["byte_count"] = m.group(4)

    return json.dumps(response)


def _group_type(type):
    if type == "fast failover":
        return "fast_failover"
    else:
        return type

def create_group(br_name, data):
    response = {}
    group = json.loads(data)
    commands = ["add-group", br_name]
    params = "group_id={0},type={1},".format(group["id"], _group_type(group["type"]))
    for bucket in group["buckets"]:
        if bucket["watch_port"] is not None and bucket["watch_group"] is not None:
            params += "bucket=watch_port:{1},watch_group:{0},{2},".format(bucket["watch_group"], bucket["watch_port"], bucket["actions_list"])
        elif bucket["watch_port"] is not None:
            params += "bucket=watch_port:{0},{1},".format(bucket["watch_port"], bucket["actions_list"])
        elif bucket["watch_group"] is not None:
            params += "bucket=watch_group:{0},{1},".format(bucket["watch_group"],bucket["actions_list"])
        else:
            params += "bucket={actions},".format(actions=bucket["actions_list"])
    commands.append(params)

    ret, err = ofctl.of_execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_groups(br_name)

def update_group(br_name, data):
    response = {}
    group = json.loads(data)
    commands = ["mod-group", br_name]
    params = "group_id={0},type={1},".format(group["id"], _group_type(group["type"]))
    for bucket in group["buckets"]:
        if bucket["watch_port"] is not None and bucket["watch_group"] is not None:
            params += "bucket=watch_port:{1},watch_group:{0},{2},".format(bucket["watch_group"], bucket["watch_port"], bucket["actions_list"])
        elif bucket["watch_port"] is not None:
            params += "bucket=watch_port:{0},{1},".format(bucket["watch_port"], bucket["actions_list"])
        elif bucket["watch_group"] is not None:
            params += "bucket=watch_group:{0},{1},".format(bucket["watch_group"],bucket["actions_list"])
        else:
            params += "bucket={actions},".format(actions=bucket["actions_list"])
    commands.append(params)

    ret, err = ofctl.of_execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_groups(br_name)

def del_group(br_name, data):
    response = {}
    group = json.loads(data)
    commands = ["del-groups", br_name, "group_id={0}".format(group["id"])]

    ret, err = ofctl.of_execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_groups(br_name)

def del_groups(br_name):
    response = {}
    commands = ["del-groups", br_name]

    ret, err = ofctl.of_execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_groups(br_name)

def get_meters(br_name):
    response = {}
    cfg_command = ['dump-meters', br_name]
    ret, err = ofctl.of_execute(cfg_command)
    ret, err = err, ret
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)

    response["ret"] = 0
    response["meters"] = []
    # Regex
    meters  = re.compile("meter.*?bands=\ntype.*?\n")
    basic = re.compile("meter=(\d+)(?: (kbps))?(?: (burst))?(?: (stats))?")
    bands = re.compile("type=([\w_]+) rate=(\d+)(?: burst_size=(\d+))?(?: prec_level=(\d+))?")
    for meter in meters.findall(ret):
        entry = {}
        m = basic.search(meter)
        entry_id = m.group(1)
        entry_flags = ( m.group(i) for i in range(2, 5) )
        entry["id"] = int(entry_id)
        entry["flags"] = {
                "KBPS":False,
                "BURST":False,
                "STATS":False
                }
        for f in entry_flags:
            if f:
                entry["flags"][f.upper()] = True
        entry["bands"] = []
        # band
        for band in bands.findall(meter):
            new_band = {"counters":{}}
            new_band["type"] = band[0]
            new_band["rate"] = int(band[1])
            if band[2]:
                new_band["burst"] = int(band[2])
            if band[3]:
                new_band["prec_level"] = int(band[3])

            entry["bands"].append(new_band)
        response["meters"].append(entry)

    # Statistics
    stats_command = ["meter-stats", br_name]
    ret, err = ofctl.of_execute(stats_command)
    ret, err = err, ret
    if err:
        response["ret"] = -1
        response["msg"] = err
        del response["meters"]
        return json.dumps(response)

    # Regex
    stats = re.compile("meter:.*?bands:\n0:.*?\n")
    mcounter = re.compile("flow_count:(\d+) packet_in_count:(\d+) byte_in_count:(\d+) duration:([\w.]+)")
    bandscnt = re.compile("0: packet_count:(\d+) byte_count:(\d+)")
    index = 0
    for stat in stats.findall(ret):
        mcnt_match = mcounter.search(stat)
        entry = response["meters"][index]
        entry["counters"] = {}
        entry["counters"]["flow_count"] = int(mcnt_match.group(1))
        entry["counters"]["packet_count"] = int(mcnt_match.group(2))
        entry["counters"]["byte_count"] = int(mcnt_match.group(3))
        entry["counters"]["duration"] = mcnt_match.group(4)
        i = 0
        band_cnt = bandscnt.search(stat)
        entry["bands"][i]["counters"]["packet_count"] = int(band_cnt.group(1))
        entry["bands"][i]["counters"]["byte_count"] = int(band_cnt.group(2))
        index += 1

    return json.dumps(response)

def create_meter(br_name, data):
    meter = json.loads(data)
    response = {}
    commands = ["add-meter", br_name]

    tmp = []
    for k, v in meter["flags"].iteritems():
        if v:
            tmp.append(k.lower())
    flags = ",".join(tmp)

    params = "meter={0},{1},band=type={2},rate={3}".format(
            meter["id"], flags, meter["bands"][0]["type"].lower(),
            meter["bands"][0]["rate"])
            #meter["bands"][0]["type_specific"])

    if "BURST" in meter["flags"] and meter["flags"]["BURST"]:
        if "burst" in meter["bands"][0]:
            params += ",burst_size={0}".format(meter["bands"][0]["burst"])
    if meter["bands"][0]["type"] == "dscp_remark":
        if "prec_level" in meter["bands"][0]:
            params += ",prec_level={0}".format(meter["bands"][0]["prec_level"])
    commands.append(params)

    ret, err = ofctl.of_execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_meters(br_name)

def update_meter(br_name, data):
    meter = json.loads(data)
    response = {}
    commands = ["mod-meter", br_name]

    tmp = []
    for k, v in meter["flags"].iteritems():
        if v:
            tmp.append(k.lower())
    flags = ",".join(tmp)

    params = "meter={0},{1},band=type={2},rate={3}".format(
            meter["id"], flags, meter["bands"][0]["type"].lower(),
            meter["bands"][0]["rate"])
            #meter["bands"][0]["type_specific"])

    if "BURST" in meter["flags"] and meter["flags"]["BURST"]:
        if "burst" in meter["bands"][0]:
            params += ",burst_size={0}".format(meter["bands"][0]["burst"])
    if meter["bands"][0]["type"] == "dscp_remark":
        if "prec_level" in meter["bands"][0]:
            params += ",prec_level={0}".format(meter["bands"][0]["prec_level"])
    commands.append(params)

    ret, err = ofctl.of_execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_meters(br_name)

def del_meter(br_name, data):
    meter = json.loads(data)
    response = {}
    commands = ["del-meter", br_name, "meter={0}".format(meter["id"])]

    ret, err = ofctl.of_execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_meters(br_name)

def del_meters(br_name):
    response = {}
    commands = ["del-meters", br_name]

    ret, err = ofctl.of_execute(commands)
    if err:
        response["ret"] = -1
        response["msg"] = err
        return json.dumps(response)
    else:
        return get_meters(br_name)

def get_available_ports():
    response = {}

    response["ports"] = []
    port_pattern = re.compile('gigabit-ethernet "(.*)"')
    with open("/webovs/bin/webovs_default.boot") as f:
        raw_ports = port_pattern.findall(f.read())

    def port_item (port):
        port_prefix = port[:2]
        if port_prefix == 'ge':
            port_type = '1 GbE'
        elif port_prefix == 'te':
            port_type = '10 GbE'
        elif port_prefix == 'qe':
            port_type = '40 GbE'
        return {"name":port, "type":port_type}

    if len(raw_ports) == 48:
        # 3780
        ports = map(port_item, raw_ports[:48])
    elif len(raw_ports) == 52:
        # 3290/3295
        ports = map(port_item, raw_ports)
    elif len(raw_ports) == 68:
        # 3922
        ports = map(port_item, raw_ports[:48] + raw_ports[-4:])
    elif len(raw_ports) == 78:
        # 5712
        ports = map(port_item, raw_ports[:48] + raw_ports[-6:])
    elif len(raw_ports) == 80:
        # 5101
        ports = map(port_item, raw_ports[:40] + raw_ports[-8:])
    elif len(raw_ports) == 128:
        # 5401/6701
        ports = map(port_item, raw_ports[-32:])

    response["ports"].extend(ports)
    response["ret"] = 0
    return json.dumps(response)

def infer_tech_spec_from_ports(default_boot):
    port_pattern = re.compile('gigabit-ethernet "(.*)"')
    raw_ports = port_pattern.findall(default_boot)

    if len(raw_ports) == 48:
        return "48 x 1 GbE SFP+ port"
    elif len(raw_ports) == 52:
        return "48 x 1 GbE RJ45 port base unit, 4 x 10 GbE SFP+ uplinks"
    elif len(raw_ports) == 68:
        return "48 x 10 GbE SFP+ port base unit, 4 x 40 GbE QSFP+ uplinks"
    elif len(raw_ports) == 78: # 5712
        return "48 x 10 GbE QSFP+ port base unit, 6 x 40 GbE QSFP+ uplinks"
    elif len(raw_ports) == 80: # 5101
        return "40 x 10 GbE SFP+ port base unit, 8 x 40 GbE QSFP+ uplinks"
    elif len(raw_ports) == 128: # 5401/6701
        return "32 x 40 GbE QSFP+ port base unit"

def get_switch_resource():
    response = {}
    lsb_release = open(infofile, 'r').readlines()
# Switch Model
    info_model = lsb_release[3].split(' ')[3]
# Tech Specs
    with open("/webovs/bin/webovs_default.boot") as f:
        info_tech_spec = infer_tech_spec_from_ports(f.read())
# PicOS version
    info_picos_version = lsb_release[2].split('=')[1][:-1]
# software revision
    info_soft_revision = lsb_release[5].split('=')[1][:-1]
# This management system version
    info_webgui_version = "1.0"
# OVS version
    p = subprocess.Popen(['ovs-vswitchd', '--version'], stdout=subprocess.PIPE, env={'PATH':'/ovs/sbin'})
    info_ovs_version = p.stdout.readlines()[0].split(' ')[3][:-1]
# Maximum flow numbers
    info_max_flows = '2048'
# Storage capacity
    df = subprocess.Popen('df -Ph'.split(), stdout=subprocess.PIPE)
    df_output = df.stdout.readlines()
    size = df_output[1].split()[1]
    avail = df_output[1].split()[3]
    info_storage = '{avail} / {size} (Avail / Total)'.format(size=size, avail=avail)
# Mac Address
    ifconfig = subprocess.Popen(['ifconfig', 'eth0'], stdout=subprocess.PIPE)
    ifconfig_output = ifconfig.stdout.readlines()
    info_mac_addr = ifconfig_output[0].split()[4]
# IP Address
    info_ip_addr = ifconfig_output[1].split()[1].split(':')[1]
# Gateway
    route = subprocess.Popen(['route'], stdout=subprocess.PIPE)
    route_output = route.stdout.readlines()
    for line in route_output:
        fields = line.split()
        if fields[0] == 'default':
            info_gateway = fields[1]
            break
# OVSDB config file location
    db_loc = subprocess.Popen('find /ovs -name ovs-vswitchd.conf.db'.split(), stdout=subprocess.PIPE)
    info_ovsdb_path = db_loc.stdout.read()[:-1]
# CPU load
    cpu_load = open('/proc/stat', 'r').readline()
    user, nice, system, idle = tuple(map(lambda x: int(x), cpu_load.split()[1:5]))
    info_cpu_load = 100.0 * (user + nice + system)/(user + nice + system + idle)
    response["ret"] = 0
    response["switch_info"] = []
    response["switch_info"].append({"desc":"Switch Model", "value":info_model})
    response["switch_info"].append({"desc":"Tech Specs", "value":info_tech_spec})
    response["switch_info"].append({"desc":"PicOS Version", "value":info_picos_version})
    response["switch_info"].append({"desc":"Software Revision", "value":info_soft_revision})
    response["switch_info"].append({"desc":"Web Interface Version", "value":info_webgui_version})
    response["switch_info"].append({"desc":"Open vSwitch Version", "value":info_ovs_version})
    response["switch_info"].append({"desc":"Maximum Flow Numbers", "value":info_max_flows})
    response["switch_info"].append({"desc":"Storage Capacity", "value":info_storage})
    response["switch_info"].append({"desc":"MAC Address", "value":info_mac_addr})
    response["switch_info"].append({"desc":"IP Address", "value":info_ip_addr})
    response["switch_info"].append({"desc":"Gateway", "value":info_gateway})
    response["switch_info"].append({"desc":"OVSDB config file location", "value":info_ovsdb_path})
    response["switch_info"].append({"desc":"CPU Load", "value":"{0:.2f}%".format(info_cpu_load)})
    return json.dumps(response)

def update_switch_conf(data):
    response = {}
    new_conf = json.loads(data)
    commands = []
    # match mode
    commands.extend(['--', 'set-match-mode'])
    match_mode_config = []
    if new_conf.get("mac_mode"):
        match_mode_config.append("mac={0}-{1}".format(
                new_conf["mac_low_prio"],
                new_conf["mac_high_prio"]))
    if new_conf.get("ip_mode"):
        match_mode_config.append("ip={0}-{1}".format(
                new_conf["ip_low_prio"],
                new_conf["ip_high_prio"]))
    if new_conf.get("arp_tpa_mode"):
        match_mode_config.append("arp_tpa={0}-{1}".format(
                new_conf["arp_low_prio"],
                new_conf["arp_high_prio"]))
    if not match_mode_config:
        match_mode_config.append("default")
    commands.append(','.join(match_mode_config))

    # egress mode
    commands.extend(['--', 'enable-egress-mode'])
    if new_conf.get( "egress" ):
        commands.append('true')
    else:
        commands.append('false')
    # cos map mode
    commands.extend(['--', 'set-cos-map'])
    if new_conf.get( "cos_map" ):
        commands.append('true')
    else:
        commands.append('false')
    # combinated action mode
    commands.extend(['--', 'set-combinated-mode'])
    if new_conf.get( "combinated" ):
        commands.append('true')
    else:
        commands.append('false')

    ret, err = vsctl.execute(commands)

    if err:
        response["ret"] = -1
        response["msg"] = err
    else:
        response["ret"] = 0
    return json.dumps(response)

def _get_match_mode(uuid):
    response = {}
    con = dbclient.connect()
    cur = con.cursor()
    transact = '["Open_vSwitch", {"op":"select", "table":"webovs_match_mode", \
            "where":[["_uuid", "==", ["uuid", "%s"]]]}]' % uuid
    cur.execute(transact)
    record = cur.fetchone()
    if record:
        response['mode'] = record['name']
        response['low'] = record['priority_low']
        response['high'] = record['priority_high']
    return response

def get_switch_conf():
    # {
    #     switch_conf: {
    #         egress_mode: False,
    #         cos_map: False,
    #         combinated_mode: False,
    #         match_mode: [
    #             {
    #                 mode: 'mac',
    #                 low: 10,
    #                 high: 1000
    #             },
    #             {
    #                 mode: 'ip',
    #                 low: 10000,
    #                 high: 20000
    #             }
    #         ]
    #     },
    #     ret: 0
    # }
    response = {}
    try:
        con = dbclient.connect()
    except dbclient.DatabaseError as e:
        response["ret"] = e.ret
        response["msg"] = e.msg
        return json.dumps(response)
    else:
        try:
            response["ret"] = 0
            cur = con.cursor()
            transact = '["Open_vSwitch", {"op":"select", "table":"webovs8", \
                    "where":[]}]'
            cur.execute(transact)
            record = cur.fetchone()
            if record:
                switch_conf = {}
                switch_conf["egress"] = record['egress_mode_enable']
                switch_conf["cos_map"] = record['cos_map_enable']
                switch_conf["combinated"] = record['combinate_actions_enable']
                match_mode = map(
                        _get_match_mode,
                        ovsdb_set(record['match_mode']).elements)
                for mode in match_mode:
                    if mode["mode"] == 'mac':
                        switch_conf["mac_mode"] = True
                        switch_conf["mac_low_prio"] = mode["low"]
                        switch_conf["mac_high_prio"] = mode["high"]
                    elif mode["mode"] == 'ip':
                        switch_conf["ip_mode"] = True
                        switch_conf["ip_low_prio"] = mode["low"]
                        switch_conf["ip_high_prio"] = mode["high"]
                    elif mode["mode"] == 'arp_tpa':
                        switch_conf["arp_tpa_mode"] = True
                        switch_conf["arp_low_prio"] = mode["low"]
                        switch_conf["arp_high_prio"] = mode["high"]
                response["switch_conf"] = switch_conf
            else:
                response["switch_conf"] = {
                    "egress": False,
                    "cos_map": False,
                    "combinated": False,
                    "mac_mode": False,
                    "ip_mode": False,
                    "arp_tpa_mode": False,
                    "mac_low_prio": None,
                    "mac_high_prio": None,
                    "ip_low_prio": None,
                    "ip_high_prio": None,
                    "arp_low_prio": None,
                    "arp_high_prio": None
                }
        except dbclient.InterfaceError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        except dbclient.ProgrammingError as e:
            response["ret"] = e.ret
            response["msg"] = e.msg
        finally:
            return json.dumps(response)

def get_help_text():
    response = {}
    response["ret"] = 0
    ht_fp = open("helptext.json", "r")
    help_text = json.load(ht_fp)
    response["help_text"] = help_text
    return json.dumps(response)

if __name__ == "__main__":
    data = {
        "mac_mode": True,
        "ip_mode": False,
        "arp_tpa_mode": False,
        "egress": False,
        "cos_map": False,
        "combinated": False,
        "mac_low_prio": 111,
        "mac_high_prio": 111
    }
    print update_switch_conf(json.dumps(data))
