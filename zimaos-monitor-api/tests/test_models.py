from app.models import (
    ServerStatus,
    CPUStatus,
    MemoryStatus,
    TemperatureStatus,
    StoragePool,
    DockerStatus,
    SystemMetricsResponse
)


def test_server_status_defaults():
    s = ServerStatus()
    assert s.online is True
    assert s.hostname == "zima-nas"
    assert s.os == "ZimaOS"


def test_temperature_status_missing():
    t = TemperatureStatus(cpu_celsius=None, system_celsius=None, available=False)
    assert t.cpu_celsius is None
    assert t.available is False


def test_storage_pool_serialization():
    pool = StoragePool(
        name="Pool1",
        mount_point="/mnt/pool1",
        total_bytes=1000,
        used_bytes=500,
        free_bytes=500,
        usage_percent=50.0
    )
    assert pool.name == "Pool1"
    assert pool.usage_percent == 50.0
