#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include <Arduino.h>
#include <vector>

struct ServerData {
    bool online = false;
    String hostname = "ZimaOS-NAS";
    String os = "ZimaOS";
    String version = "1.0";
};

struct UptimeData {
    float seconds = 0;
    String formatted = "0d 00h 00m";
};

struct CpuData {
    float usage_percent = 0.0;
    float load_1m = 0.0;
    float load_5m = 0.0;
    float load_15m = 0.0;
    int cores = 4;
    float frequency_mhz = 0.0;
};

struct MemoryData {
    uint64_t total_bytes = 0;
    uint64_t used_bytes = 0;
    uint64_t free_bytes = 0;
    float usage_percent = 0.0;
};

struct TempData {
    float cpu_celsius = 0.0;
    float system_celsius = 0.0;
    bool available = false;
};

struct StoragePoolData {
    String name = "POOL 1";
    String filesystem = "ext4";
    uint64_t total_bytes = 0;
    uint64_t used_bytes = 0;
    uint64_t free_bytes = 0;
    float usage_percent = 0.0;
};

struct DiskData {
    String name = "sda";
    String model = "Disk";
    uint64_t capacity_bytes = 0;
    float temp_celsius = 0.0;
    bool has_temp = false;
    String health = "OK";
};

struct StorageData {
    std::vector<StoragePoolData> pools;
    std::vector<DiskData> disks;
    String raid_status = "HEALTHY";
    String raid_type = "RAID 1 Mirroring";
};

struct ContainerData {
    String name;
    String status = "UP";
    String image;
    String uptime;
};

struct DockerData {
    int total = 0;
    int running = 0;
    int stopped = 0;
    std::vector<ContainerData> containers;
};

struct NetworkInterfaceData {
    String name = "eth0";
    uint64_t rx_bytes = 0;
    uint64_t tx_bytes = 0;
    float rx_rate_bytes_sec = 0.0;
    float tx_rate_bytes_sec = 0.0;
};

struct NetworkData {
    std::vector<NetworkInterfaceData> interfaces;
    float total_rx_mb_s = 0.0;
    float total_tx_mb_s = 0.0;
};

struct HealthData {
    String status = "healthy";
    std::vector<String> warnings;
    std::vector<String> errors;
};

struct SystemMetrics {
    bool valid = false;
    uint32_t last_updated_ms = 0;
    ServerData server;
    UptimeData uptime;
    CpuData cpu;
    MemoryData memory;
    TempData temperature;
    StorageData storage;
    DockerData docker;
    NetworkData network;
    HealthData health;
};

#endif // DATA_MODEL_H
