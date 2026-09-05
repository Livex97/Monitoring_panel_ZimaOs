from datetime import datetime, timezone
from typing import List
from app.models import SystemMetricsResponse, HealthStatus
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


def evaluate_health(
    cpu_percent: float,
    ram_percent: float,
    storage_pools: list,
    docker_status: object
) -> HealthStatus:
    status = "healthy"
    warnings: List[str] = []
    errors: List[str] = []

    if cpu_percent > 90.0:
        status = "warning"
        warnings.append(f"High CPU utilization: {cpu_percent}%")
    
    if ram_percent > 90.0:
        status = "warning"
        warnings.append(f"High RAM utilization: {ram_percent}%")

    for pool in storage_pools:
        if pool.usage_percent > 90.0:
            status = "warning"
            warnings.append(f"Storage pool '{pool.name}' usage critical: {pool.usage_percent}%")

    if getattr(docker_status, "stopped", 0) > 0:
        warnings.append(f"{docker_status.stopped} Docker container(s) stopped")

    if cpu_percent > 98.0 or ram_percent > 98.0:
        status = "critical"
        errors.append("Host resources critically exhausted")

    return HealthStatus(
        status=status,
        warnings=warnings,
        errors=errors
    )


def collect_system_metrics() -> SystemMetricsResponse:
    now_iso = datetime.now(timezone.utc).isoformat()
    
    server = get_server_status()
    uptime = get_uptime_info()
    cpu = get_cpu_status()
    memory = get_memory_status()
    temperature = get_temperature_status()
    storage = get_storage_status()
    docker = get_docker_status()
    network = get_network_status()

    health = evaluate_health(
        cpu_percent=cpu.usage_percent,
        ram_percent=memory.usage_percent,
        storage_pools=storage.pools,
        docker_status=docker
    )

    return SystemMetricsResponse(
        schema_version="1.0",
        timestamp=now_iso,
        server=server,
        uptime=uptime,
        cpu=cpu,
        memory=memory,
        temperature=temperature,
        storage=storage,
        docker=docker,
        network=network,
        health=health
    )
