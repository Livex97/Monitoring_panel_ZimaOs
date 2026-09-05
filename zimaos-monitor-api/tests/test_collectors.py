from app.collectors.system import (
    get_server_status,
    get_uptime_info,
    get_cpu_status,
    get_memory_status,
    get_temperature_status
)
from app.collectors.storage import get_storage_status
from app.collectors.docker_collector import get_docker_status
from app.collectors.network import get_network_status


def test_system_collectors():
    server = get_server_status()
    assert server.online is True
    assert isinstance(server.hostname, str)

    uptime = get_uptime_info()
    assert uptime.seconds >= 0

    cpu = get_cpu_status()
    assert 0 <= cpu.usage_percent <= 100
    assert cpu.cores >= 1

    mem = get_memory_status()
    assert mem.total_bytes > 0
    assert 0 <= mem.usage_percent <= 100

    temp = get_temperature_status()
    assert isinstance(temp.available, bool)


def test_storage_collector():
    storage = get_storage_status()
    assert isinstance(storage.pools, list)
    assert isinstance(storage.disks, list)


def test_docker_collector():
    docker = get_docker_status()
    assert docker.total >= 0
    assert docker.running >= 0
    assert docker.stopped >= 0


def test_network_collector():
    net = get_network_status()
    assert isinstance(net.interfaces, list)
