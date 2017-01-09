### What's sourth/north bond interface?

Refer https://www.sdxcentral.com/sdn/definitions/southbound-interface-api/ and https://www.sdxcentral.com/sdn/definitions/north-bound-interfaces-api/ .

SDN controller use north bond interface API to talk to app user. API is like create l2 network, create l3 network, etc.

SDN controller use sourth bond interface API to implement network demand to dataplan. API is openflow, ovsdb, netconf, company own, protocol related like BGP.
