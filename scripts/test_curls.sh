# Health check
curl -i http://127.0.0.1:8080/health

# Add IPs to the pool
curl -i -X POST http://127.0.0.1:8080/ip-inventory/ip-pool \
  -H "Content-Type: application/json" \
  -d '{
    "ipAddresses": [
      {
        "ip": "95.44.73.19",
        "ipType": "IPv4"
      },
      {
        "ip": "2a01:5a9:1a4:95c::1",
        "ipType": "IPv6"
      }
    ]
  }'

# Reserve an IPv4 address for a service
curl -i -X POST http://127.0.0.1:8080/ip-inventory/reserve-ip \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-a",
    "ipType": "IPv4"
  }'

# Assign IPs to a service
curl -i -X POST http://127.0.0.1:8080/ip-inventory/assign-ip-serviceId \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-a",
    "ipAddresses": [
      {
        "ip": "95.44.73.19"
      },
      {
        "ip": "2a01:5a9:1a4:95c::1"
      }
    ]
  }'

# Get IPs assigned to a service
curl -i "http://127.0.0.1:8080/ip-inventory/serviceId?serviceId=service-a"

# Change a service id
curl -i -X POST http://127.0.0.1:8080/ip-inventory/serviceId-change \
  -H "Content-Type: application/json" \
  -d '{
    "serviceIdOld": "service-a",
    "serviceId": "service-b"
  }'

# Terminate an IP assignment
curl -i -X POST http://127.0.0.1:8080/ip-inventory/terminate-ip-serviceId \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-b",
    "ipAddresses": [
      {
        "ip": "95.44.73.19"
      }
    ]
  }'

# Check the docs endpoint
curl -sS -I http://127.0.0.1:8080/docs

# Check the OpenAPI spec endpoint
curl -sS -I http://127.0.0.1:8080/openapi.yaml
