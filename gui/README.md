# IP Inventory UI MVP

Single-screen vanilla JavaScript UI for the implemented IP Inventory REST API.

## Files

- `index.html`
- `style.css`
- `app.js`

No npm. No build step. No framework.

## Supported API endpoints

The UI uses only these endpoints:

- `POST /ip-inventory/ip-pool`
- `POST /ip-inventory/reserve-ip`
- `POST /ip-inventory/assign-ip-serviceId`
- `POST /ip-inventory/terminate-ip-serviceId`
- `POST /ip-inventory/serviceId-change`
- `GET /ip-inventory/serviceId?serviceId=...`
- `GET /ip-inventory/all-reserved-ips`

There is no `/state` endpoint.

## Important limitation

The API has an endpoint for all reserved IPs, but it does not have an endpoint for all assigned IPs.

Therefore the Assigned IPs panel queries:

```text
GET /ip-inventory/serviceId?serviceId=...
```

for service IDs known to the UI. Known service IDs are collected from:

- reserve operations
- assign operations
- terminate operations
- rename operations
- services returned by `GET /ip-inventory/all-reserved-ips`
- browser local storage

A full assigned-state view for unknown historical services is impossible without an API endpoint that lists all assigned IPs or all services.

## Run

Serve the files from the same origin as the C++ API, or use a static server:

```bash
cd ip-inventory-ui-mvp-v2
python3 -m http.server 3000
```

Then open:

```text
http://127.0.0.1:3000
```

If the API is on another origin, set the API base field in the UI, for example:

```text
http://127.0.0.1:8080
```

The backend must allow CORS if the UI and API use different origins.
