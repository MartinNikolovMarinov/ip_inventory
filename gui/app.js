const STORAGE_KEYS = {
    apiBase: "ipInventory.apiBase",
    services: "ipInventory.services"
};

const apiBaseInput = document.getElementById("apiBase");
const lastResponseEl = document.getElementById("lastResponse");
const availableRows = document.getElementById("availableRows");
const reservedRows = document.getElementById("reservedRows");
const assignedRows = document.getElementById("assignedRows");

apiBaseInput.value = localStorage.getItem(STORAGE_KEYS.apiBase) || "";

document.getElementById("saveApiBase").addEventListener("click", () => {
    localStorage.setItem(STORAGE_KEYS.apiBase, apiBaseInput.value.trim());
});

document.getElementById("refresh").addEventListener("click", refreshUi);

document.getElementById("addPoolForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const declaredType = document.getElementById("poolIpType").value;
    const ipAddresses = parseLines(document.getElementById("poolIps").value).map((ip) => ({
        ip,
        ipType: declaredType === "Auto" ? inferIpType(ip) : declaredType
    }));

    await postJson("/ip-inventory/ip-pool", { ipAddresses }, true);
    await refreshUi();
});

document.getElementById("reserveForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const serviceId = document.getElementById("reserveServiceId").value.trim();
    const ipType = document.getElementById("reserveIpType").value;
    rememberService(serviceId);

    const response = await postJson("/ip-inventory/reserve-ip", { serviceId, ipType }, true);
    if (response.ipAddresses) {
        document.getElementById("assignServiceId").value = serviceId;
        document.getElementById("assignIps").value = response.ipAddresses.map((item) => item.ip).join("\n");
    }
    await refreshUi();
});

document.getElementById("assignForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const serviceId = document.getElementById("assignServiceId").value.trim();
    const ipAddresses = parseLines(document.getElementById("assignIps").value).map((ip) => ({ ip }));
    rememberService(serviceId);

    await postJson("/ip-inventory/assign-ip-serviceId", { serviceId, ipAddresses }, true);
    await refreshUi();
});

document.getElementById("terminateForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const serviceId = document.getElementById("terminateServiceId").value.trim();
    const ipAddresses = parseLines(document.getElementById("terminateIps").value).map((ip) => ({ ip }));
    rememberService(serviceId);

    await postJson("/ip-inventory/terminate-ip-serviceId", { serviceId, ipAddresses }, true);
    await refreshUi();
});

document.getElementById("renameForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const serviceIdOld = document.getElementById("serviceIdOld").value.trim();
    const serviceId = document.getElementById("serviceIdNew").value.trim();

    await postJson("/ip-inventory/serviceId-change", { serviceIdOld, serviceId }, true);
    forgetService(serviceIdOld);
    rememberService(serviceId);
    await refreshUi();
});

async function refreshUi() {
    await Promise.all([
        loadAvailableIps(),
        loadReservedIps(),
        loadAssignedIpsForKnownServices()
    ]);
}

async function loadAvailableIps() {
    try {
        const data = await getJson("/ip-inventory/available-ips");
        const items = data.ipAddresses || [];
        availableRows.innerHTML = items.map((item) => row([
            item.ip,
            item.ipType
        ])).join("") || emptyRow(2);
    }
    catch (error) {
        availableRows.innerHTML = errorRow(2, error.message);
    }
}

async function loadReservedIps() {
    try {
        const data = await getJson("/ip-inventory/all-reserved-ips");
        const items = data.reservedIps || [];
        reservedRows.innerHTML = items.map((item) => row([
            item.serviceId,
            item.ip,
            item.ipType,
            formatUnixTime(item.expirationTime)
        ])).join("") || emptyRow(4);

        for (const item of items) {
            rememberService(item.serviceId, false);
        }
    }
    catch (error) {
        reservedRows.innerHTML = errorRow(4, error.message);
    }
}

async function loadAssignedIpsForKnownServices() {
    const services = getKnownServices();
    const rows = [];

    for (const serviceId of services) {
        try {
            const data = await getJson(`/ip-inventory/serviceId?serviceId=${encodeURIComponent(serviceId)}`);
            for (const item of data.ipAddresses || []) {
                rows.push(row([serviceId, item.ip, item.ipType]));
            }
        }
        catch (error) {
            const message = error.message.toLowerCase();
            if (!message.includes("service not found")) {
                rows.push(errorRow(3, `${serviceId}: ${error.message}`));
            }
        }
    }

    assignedRows.innerHTML = rows.join("") || emptyRow(3);
}

async function postJson(path, payload, writeLastResponse = false) {
    const response = await fetch(`${getApiBase()}${path}`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
    });
    return readResponse(response, writeLastResponse);
}

async function getJson(path, writeLastResponse = false) {
    const response = await fetch(`${getApiBase()}${path}`, { method: "GET" });
    return readResponse(response, writeLastResponse);
}

async function readResponse(response, writeLastResponse) {
    const text = await response.text();
    let data;

    try {
        data = text ? JSON.parse(text) : {};
    }
    catch {
        data = { raw: text };
    }

    if (writeLastResponse) {
        writeLastResponseCard(response.status, data);
    }

    if (!response.ok || data.statusCode === "1") {
        throw new Error(data.statusMessage || `HTTP ${response.status}`);
    }

    return data;
}

function getApiBase() {
    return (localStorage.getItem(STORAGE_KEYS.apiBase) || apiBaseInput.value || "").replace(/\/$/, "");
}

function parseLines(value) {
    return value.split(/\r?\n/).map((line) => line.trim()).filter(Boolean);
}

function inferIpType(ip) {
    return ip.includes(":") ? "IPv6" : "IPv4";
}

function formatUnixTime(value) {
    if (value === undefined || value === null || value === "") return "";
    const date = new Date(Number(value) * 1000);
    if (Number.isNaN(date.getTime())) return String(value);
    return `${date.toLocaleString()} (${value})`;
}

function writeLastResponseCard(statusCode, data) {
    lastResponseEl.textContent = `Status: ${statusCode}\n${JSON.stringify(data, null, 2)}`;
}

function row(values) {
    return `<tr>${values.map((value) => `<td>${escapeHtml(String(value ?? ""))}</td>`).join("")}</tr>`;
}

function emptyRow(cols) {
    return `<tr><td colspan="${cols}">No data.</td></tr>`;
}

function errorRow(cols, message) {
    return `<tr><td colspan="${cols}">Error: ${escapeHtml(message)}</td></tr>`;
}

function escapeHtml(value) {
    return value
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

function getKnownServices() {
    try {
        return JSON.parse(localStorage.getItem(STORAGE_KEYS.services) || "[]");
    }
    catch {
        return [];
    }
}

function setKnownServices(services) {
    localStorage.setItem(STORAGE_KEYS.services, JSON.stringify([...new Set(services)].filter(Boolean).sort()));
}

function rememberService(serviceId, persist = true) {
    if (!serviceId) return;
    const services = getKnownServices();
    if (!services.includes(serviceId)) {
        services.push(serviceId);
        if (persist) setKnownServices(services);
        else localStorage.setItem(STORAGE_KEYS.services, JSON.stringify([...new Set(services)].sort()));
    }
}

function forgetService(serviceId) {
    setKnownServices(getKnownServices().filter((item) => item !== serviceId));
}

refreshUi();
