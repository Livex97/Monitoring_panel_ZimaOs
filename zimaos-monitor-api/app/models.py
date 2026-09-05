from typing import List, Optional
from pydantic import BaseModel, Field


class ServerStatus(BaseModel):
    online: bool = True
    hostname: str = "zima-nas"
    os: str = "ZimaOS"
    version: Optional[str] = "1.0.0"


class UptimeInfo(BaseModel):
    seconds: float = 0.0
    formatted: str = "0d 00h 00m"


class CPUStatus(BaseModel):
    usage_percent: float = 0.0
    load_1m: float = 0.0
    load_5m: float = 0.0
    load_15m: float = 0.0
    cores: int = 1
    frequency_mhz: Optional[float] = None


class MemoryStatus(BaseModel):
    total_bytes: int = 0
    used_bytes: int = 0
    free_bytes: int = 0
    available_bytes: int = 0
    usage_percent: float = 0.0


class TemperatureStatus(BaseModel):
    cpu_celsius: Optional[float] = None
    system_celsius: Optional[float] = None
    available: bool = False


class StoragePool(BaseModel):
    name: str
    filesystem: str = "unknown"
    mount_point: str
    total_bytes: int = 0
    used_bytes: int = 0
    free_bytes: int = 0
    usage_percent: float = 0.0


class DiskStatus(BaseModel):
    name: str
    model: str = "Unknown Disk"
    capacity_bytes: int = 0
    temperature_celsius: Optional[float] = None
    health: str = "OK"
    smart_status: Optional[str] = "PASSED"


class StorageStatus(BaseModel):
    pools: List[StoragePool] = Field(default_factory=list)
    disks: List[DiskStatus] = Field(default_factory=list)


class ContainerInfo(BaseModel):
    name: str
    status: str  # e.g., "running", "exited", "created"
    image: str = ""
    uptime: Optional[str] = None


class DockerStatus(BaseModel):
    total: int = 0
    running: int = 0
    stopped: int = 0
    containers: List[ContainerInfo] = Field(default_factory=list)


class NetworkInterface(BaseModel):
    name: str
    rx_bytes: int = 0
    tx_bytes: int = 0
    rx_rate_bytes_sec: float = 0.0
    tx_rate_bytes_sec: float = 0.0


class NetworkStatus(BaseModel):
    interfaces: List[NetworkInterface] = Field(default_factory=list)


class HealthStatus(BaseModel):
    status: str = "healthy"  # "healthy", "warning", "critical"
    warnings: List[str] = Field(default_factory=list)
    errors: List[str] = Field(default_factory=list)


class SystemMetricsResponse(BaseModel):
    schema_version: str = "1.0"
    timestamp: str
    server: ServerStatus
    uptime: UptimeInfo
    cpu: CPUStatus
    memory: MemoryStatus
    temperature: TemperatureStatus
    storage: StorageStatus
    docker: DockerStatus
    network: NetworkStatus
    health: HealthStatus
