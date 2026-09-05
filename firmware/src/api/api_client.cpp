#include "api/api_client.h"
#include "config.h"

APIClient::APIClient() : m_port(8787) {}

void APIClient::begin(const char* host, uint16_t port, const char* apiKey) {
    m_host = host;
    m_port = port;
    m_apiKey = apiKey;
}

bool APIClient::fetchMetrics(SystemMetrics& outMetrics) {
    // ----- DEBUG -----
    String url = "http://" + m_host + ":" + String(m_port) + "/api/v1/status";
    Serial.printf("[APIClient] Trying %s\n", url.c_str());
    WiFiClient testClient;
    if (!testClient.connect(m_host.c_str(), m_port)) {
        Serial.println("[APIClient] TCP connect failed (host unreachable)");
        outMetrics.valid = false;
        return false;
    }
    testClient.stop(); // connection succeeded
    // ------------------
    
    m_http.begin(url);
    m_http.setTimeout(HTTP_TIMEOUT_MS);
    
    if (m_apiKey.length() > 0) {
        m_http.addHeader("X-API-Key", m_apiKey);
    }

    int httpCode = m_http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[APIClient] HTTP GET failed, code: %d\n", httpCode);
        m_http.end();
        outMetrics.valid = false;
        return false;
    }

    WiFiClient* stream = m_http.getStreamPtr();
    DynamicJsonDocument doc(16384);
    DeserializationError err = deserializeJson(doc, *stream);
    m_http.end();

    if (err) {
        Serial.printf("[APIClient] JSON parsing error: %s\n", err.c_str());
        outMetrics.valid = false;
        return false;
    }

    // Parse Server
    JsonObject server = doc["server"];
    outMetrics.server.online = server["online"] | true;
    outMetrics.server.hostname = server["hostname"].as<String>();
    outMetrics.server.os = server["os"].as<String>();
    outMetrics.server.version = server["version"].as<String>();

    // Parse Uptime
    JsonObject uptime = doc["uptime"];
    outMetrics.uptime.seconds = uptime["seconds"] | 0.0f;
    outMetrics.uptime.formatted = uptime["formatted"].as<String>();

    // Parse CPU
    JsonObject cpu = doc["cpu"];
    outMetrics.cpu.usage_percent = cpu["usage_percent"] | 0.0f;
    outMetrics.cpu.load_1m = cpu["load_1m"] | 0.0f;
    outMetrics.cpu.cores = cpu["cores"] | 4;

    // Parse Memory
    JsonObject memory = doc["memory"];
    outMetrics.memory.usage_percent = memory["usage_percent"] | 0.0f;
    outMetrics.memory.used_bytes = memory["used_bytes"] | 0ULL;
    outMetrics.memory.total_bytes = memory["total_bytes"] | 0ULL;

    // Parse Temperature
    JsonObject temp = doc["temperature"];
    outMetrics.temperature.cpu_celsius = temp["cpu_celsius"] | 0.0f;
    outMetrics.temperature.available = temp["available"] | false;

    // Parse Storage
    JsonObject storage = doc["storage"];
    outMetrics.storage.pools.clear();
    JsonArray pools = storage["pools"];
    for (JsonObject p : pools) {
        StoragePoolData pool;
        pool.name = p["name"].as<String>();
        pool.usage_percent = p["usage_percent"] | 0.0f;
        pool.used_bytes = p["used_bytes"] | 0ULL;
        pool.total_bytes = p["total_bytes"] | 0ULL;
        outMetrics.storage.pools.push_back(pool);
    }

    outMetrics.storage.disks.clear();
    JsonArray disks = storage["disks"];
    for (JsonObject d : disks) {
        DiskData disk;
        disk.name = d["name"].as<String>();
        disk.model = d["model"].as<String>();
        disk.capacity_bytes = d["capacity_bytes"] | 0ULL;
        disk.temp_celsius = d["temperature_celsius"] | 0.0f;
        disk.has_temp = !d["temperature_celsius"].isNull();
        disk.health = d["health"] | "OK";
        outMetrics.storage.disks.push_back(disk);
    }

    // Parse Docker
    JsonObject docker = doc["docker"];
    outMetrics.docker.total = docker["total"] | 0;
    outMetrics.docker.running = docker["running"] | 0;
    outMetrics.docker.stopped = docker["stopped"] | 0;
    
    outMetrics.docker.containers.clear();
    JsonArray containers = docker["containers"];
    for (JsonObject c : containers) {
        ContainerData container;
        container.name = c["name"].as<String>();
        container.status = c["status"].as<String>();
        container.image = c["image"].as<String>();
        outMetrics.docker.containers.push_back(container);
    }

    // Parse Network
    JsonObject network = doc["network"];
    outMetrics.network.interfaces.clear();
    float sum_rx = 0.0f;
    float sum_tx = 0.0f;
    JsonArray ifaces = network["interfaces"];
    for (JsonObject net : ifaces) {
        NetworkInterfaceData n;
        n.name = net["name"].as<String>();
        n.rx_rate_bytes_sec = net["rx_rate_bytes_sec"] | 0.0f;
        n.tx_rate_bytes_sec = net["tx_rate_bytes_sec"] | 0.0f;
        sum_rx += n.rx_rate_bytes_sec;
        sum_tx += n.tx_rate_bytes_sec;
        outMetrics.network.interfaces.push_back(n);
    }
    outMetrics.network.total_rx_mb_s = sum_rx / (1024.0f * 1024.0f);
    outMetrics.network.total_tx_mb_s = sum_tx / (1024.0f * 1024.0f);

    // Parse Health
    JsonObject health = doc["health"];
    outMetrics.health.status = health["status"].as<String>();

    outMetrics.valid = true;
    outMetrics.last_updated_ms = millis();
    return true;
}
