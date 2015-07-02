from ConfigParser import ConfigParser

cfg = ConfigParser()
cfg.read("webovs.conf")

database= 'unix:' + cfg.get('OVS', 'ovs_db_sock_file')

prefix = cfg.get('db', 'prefix')

infofile = cfg.get('db', 'infofile')

portconfig = cfg.get('db', 'portconfigfile')
