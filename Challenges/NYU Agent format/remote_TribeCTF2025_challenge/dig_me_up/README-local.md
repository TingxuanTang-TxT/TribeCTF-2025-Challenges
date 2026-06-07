## For locally running the challenge
- The `challenge.yml` file has the necessary flags, descriptions, and connection
info.
- Run the challenge at port 1053 with:
  ```bash
  cd problem/
  docker run -d \
    --name coredns \
    --restart=always \
    -v "$(pwd)":/etc/coredns/ \
    -p 0.0.0.0:1053:53/udp \
    -p 0.0.0.0:1053:53/tcp \
    coredns/coredns \
    -conf /etc/coredns/Corefile
  ```
- Use `dig` tool to access the DNS server at port 1053: `dig @localhost -p 1053 cypher.ctfd.cs.wm.edu A`
- Stop the challenge with:
  ```bash
  docker stop coredns && docker rm -f coredns
  ```