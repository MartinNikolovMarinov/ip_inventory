curl -X POST http://localhost:8080/ip-inventory/ip-pool \
  -H "Content-Type: application/json" \
  -d '{
    "ipAddresses": [
      {
        "ip": "95.44.73.19",
        "ipType": "IPv4"
      },
      {
        "ip": "2a01:05a9:01a4:095c:0000:0000:0000:0001",
        "ipType": "IPv6"
      }
    ]
  }'
