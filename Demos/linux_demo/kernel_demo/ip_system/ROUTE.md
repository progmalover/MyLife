### Reference?

1. http://linux-ip.net/html/index.html
2. http://www.cnblogs.com/gunl/archive/2010/09/14/1826234.html


### arp?

Refer to <2>.


### ROUTE and IPTABLES?

Refer to ./netfilter_iptables/README.md


### ip rule?

Ip rule is for Policy Routing, which is stored in RPDB, has priority(0 is highest). This is before ROUTE.


### multi route table?

Refer to <2>, notice that route match NOT use priority but use LPM, so for kernel, all route in different table will combine and LPM search.

enum rt_class_t
{
    RT_TABLE_UNSPEC=0,
    RT_TABLE_DEFAULT=253,
    RT_TABLE_MAIN=254,
    RT_TABLE_LOCAL=255,
    __RT_TABLE_MAX
};

Defualt table: kernel manage this table for default route and nat route.
Main table: unicast route table.
Local table: broadcast route and local route.


### Routing process?

if packet.routeCacheLookupKey in routeCache :
    route = routeCache[ packet.routeCacheLookupKey ]
else
    for rule in rpdb :
        if packet.rpdbLookupKey in rule :
            routeTable = rule[ lookupTable ]
            if packet.routeLookupKey in routeTable :
                route = route_table[ packet.routeLookup_key ]


### 
