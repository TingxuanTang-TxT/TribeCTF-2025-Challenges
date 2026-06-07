# Dig Me Up! 

This is a DNS based challenge. It requires running a CoreDNS server on a server with the following two files. 

> 

Connection Info: <Server IP> <Server Port>

> dig @<Server IP> -p <Server Port> cypher.ctfd.cs.wm.edu TXT


## Remove the existing container first
```bash
docker stop coredns && docker rm -f coredns
```

## Run with Corefile
```bash
docker run -d \
  --name coredns \
  --restart=always \
  -v "$(pwd)":/etc/coredns/ \
  -p 0.0.0.0:1053:53/udp \
  -p 0.0.0.0:1053:53/tcp \
  coredns/coredns \
  -conf /etc/coredns/Corefile
```