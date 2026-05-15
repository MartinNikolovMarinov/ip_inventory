#!/bin/bash

echo
echo
echo "Add 2 IPs to the inventory pool"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/ip-pool \
  -H "Content-Type: application/json" \
  -d '{
    "ipAddresses": [
      {
        "ip": "1.1.1.1",
        "ipType": "IPv4"
      },
      {
        "ip": "2a01:5a9:1a4:95c::1",
        "ipType": "IPv6"
      }
    ]
  }'

echo
echo
echo "Reserve an IPv4 for service-a"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/reserve-ip \
  -H "Content-Type: application/json" \
  -d '{
  "serviceId": "service-a",
  "ipType": "IPv4"
  }'

echo
echo
echo "Reserve should fail because there are no more IPv4s available"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/reserve-ip \
  -H "Content-Type: application/json" \
  -d '{
  "serviceId": "service-a",
  "ipType": "IPv4"
  }'

echo
echo
echo "Assign a service to a list of ips where one does not exist"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/assign-ip-serviceId \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-a",
    "ipAddresses": [
      {
        "ip": "1.1.1.1"
      },
      {
        "ip": "95.44.73.19"
      }
    ]
  }'

echo
echo
echo "Assign a service to a list of ips where one is not reserved"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/assign-ip-serviceId \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-a",
    "ipAddresses": [
      {
        "ip": "1.1.1.1"
      },
      {
        "ip": "2a01:5a9:1a4:95c::1"
      }
    ]
  }'

echo
echo
echo "Assign a service to an ip that hasa a port defined"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/assign-ip-serviceId \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-a",
    "ipAddresses": [
      {
        "ip": "1.1.1.1:80"
      },
    ]
  }'

echo
echo
echo "Assign a service that does not exist"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/assign-ip-serviceId \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-b",
    "ipAddresses": [
      {
        "ip": "1.1.1.1"
      }
    ]
  }'

echo
echo
echo "Assign an Ip with invalid ip string"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/assign-ip-serviceId \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-a",
    "ipAddresses": [
      {
        "ip": "llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll"
      }
    ]
  }'

echo
echo
echo "Assign an Ip for service-a"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/assign-ip-serviceId \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-a",
    "ipAddresses": [
      {
        "ip": "1.1.1.1"
      }
    ]
  }'

echo
echo
echo "Reserve an IPv6 for service-b"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/reserve-ip \
  -H "Content-Type: application/json" \
  -d '{
  "serviceId": "service-b",
  "ipType": "IPv6"
  }'

echo
echo
echo "Get all reserved ips"
echo
curl -i "http://127.0.0.1:8080/ip-inventory/all-reserved-ips"

echo
echo
echo "Get assigned IPs for service-a"
echo
curl -i "http://127.0.0.1:8080/ip-inventory/serviceId?serviceId=service-a"

echo
echo
echo "Terminate ip 1.1.1.1 for service-a"
echo
curl -i -X POST http://127.0.0.1:8080/ip-inventory/terminate-ip-serviceId \
  -H "Content-Type: application/json" \
  -d '{
    "serviceId": "service-a",
    "ipAddresses": [
      {
        "ip": "1.1.1.1"
      }
    ]
  }'

echo
echo
echo "Get assigned IPs for service-a"
echo
curl -i "http://127.0.0.1:8080/ip-inventory/serviceId?serviceId=service-a"
