import os
import re
import time
import socket
import psutil
from typing import Optional
from app.config import settings
from app.models import ServerStatus, UptimeInfo, CPUStatus, MemoryStatus, TemperatureStatus


def format_uptime(seconds: float) -> str:
    total_seconds = int(seconds)
    days = total_seconds // 86400
    hours = (total_seconds % 86400) // 3600
    minutes = (total_seconds % 3600) // 60
    return f"{days}d {hours:02d}h {minutes:02d}m"


def get_server_status() -> ServerStatus:
    hostname = socket.gethostname()
    
    # Try reading real host hostname from /host/proc/sys/kernel/hostname or /proc/sys/kernel/hostname or /host/etc/hostname
    hostname_candidates = [
        os.path.join(settings.host_proc, "sys/kernel/hostname"),
        "/proc/sys/kernel/hostname",
        os.path.join(settings.host_etc, "hostname"),
        "/etc/hostname"
    ]
    for h_path in hostname_candidates:
        if os.path.exists(h_path):
            try:
                with open(h_path, "r") as f:
                    content = f.read().strip()
                    if content and not re.match(r"^[0-9a-f]{12}$", content):
                        hostname = content
                        break
            except Exception:
                pass

    if re.match(r"^[0-9a-f]{12}$", hostname):
        hostname = "ZimaOS-NAS"

    os_name = "ZimaOS"
    version = "1.0.0"
    
    # Try reading host os-release
    os_release_candidates = [
        os.path.join(settings.host_etc, "zimaos-release"),
        os.path.join(settings.host_etc, "os-release"),
        "/etc/os-release"
    ]
    for os_path in os_release_candidates:
        if os.path.exists(os_path):
            try:
                with open(os_path, "r") as f:
                    content = f.read()
                    name_found = None
                    ver_found = None
                    for line in content.splitlines():
                        if line.startswith("PRETTY_NAME="):
                            name_found = line.split("=", 1)[1].strip('"\'')
                        elif line.startswith("NAME=") and not name_found:
                            name_found = line.split("=", 1)[1].strip('"\'')
                        elif line.startswith("VERSION_ID="):
                            ver_found = line.split("=", 1)[1].strip('"\'')
                        elif line.startswith("VERSION=") and not ver_found:
                            ver_found = line.split("=", 1)[1].strip('"\'')
                    if name_found:
                        os_name = name_found
                    if ver_found:
                        version = ver_found
                    if os_name != "Debian GNU/Linux 13 (trixie)":
                        break
            except Exception:
                pass

    return ServerStatus(
        online=True,
        hostname=hostname,
        os=os_name,
        version=version
    )


def get_uptime_info() -> UptimeInfo:
    uptime_seconds = 0.0
    # Try reading /host/proc/uptime or /proc/uptime
    uptime_candidates = [
        os.path.join(settings.host_proc, "uptime"),
        "/proc/uptime"
    ]
    for u_path in uptime_candidates:
        if os.path.exists(u_path):
            try:
                with open(u_path, "r") as f:
                    content = f.read().split()
                    if content:
                        uptime_seconds = float(content[0])
                        break
            except Exception:
                pass

    if uptime_seconds == 0.0:
        boot_time = psutil.boot_time()
        uptime_seconds = time.time() - boot_time

    return UptimeInfo(
        seconds=round(uptime_seconds, 1),
        formatted=format_uptime(uptime_seconds)
    )


def get_cpu_status() -> CPUStatus:
    usage = psutil.cpu_percent(interval=None)
    try:
        load_1, load_5, load_15 = os.getloadavg()
    except Exception:
        load_1, load_5, load_15 = 0.0, 0.0, 0.0

    cores = psutil.cpu_count(logical=True) or 1

    freq_mhz: Optional[float] = None
    try:
        freq = psutil.cpu_freq()
        if freq and freq.current:
            freq_mhz = round(freq.current, 1)
    except Exception:
        pass

    return CPUStatus(
        usage_percent=round(usage, 1),
        load_1m=round(load_1, 2),
        load_5m=round(load_5, 2),
        load_15m=round(load_15, 2),
        cores=cores,
        frequency_mhz=freq_mhz
    )


def get_memory_status() -> MemoryStatus:
    mem = psutil.virtual_memory()
    return MemoryStatus(
        total_bytes=mem.total,
        used_bytes=mem.used,
        free_bytes=mem.free,
        available_bytes=mem.available,
        usage_percent=round(mem.percent, 1)
    )


def get_temperature_status() -> TemperatureStatus:
    cpu_temp: Optional[float] = None
    sys_temp: Optional[float] = None
    available = False

    try:
        temps = psutil.sensors_temperatures()
        if temps:
            for sensor_name, entries in temps.items():
                for entry in entries:
                    if entry.current is not None:
                        label = (entry.label or sensor_name).lower()
                        if "cpu" in label or "core" in label or "package" in label:
                            if cpu_temp is None:
                                cpu_temp = float(entry.current)
                        elif "sys" in label or "board" in label or "ambient" in label:
                            if sys_temp is None:
                                sys_temp = float(entry.current)
    except Exception:
        pass

    # Fallback to sysfs thermal zones in /host/sys/class/thermal or /sys/class/thermal
    if cpu_temp is None:
        thermal_dirs = [
            os.path.join(settings.host_sys, "class/thermal"),
            "/sys/class/thermal"
        ]
        for thermal_dir in thermal_dirs:
            if os.path.exists(thermal_dir):
                try:
                    for zone in os.listdir(thermal_dir):
                        if zone.startswith("thermal_zone"):
                            temp_path = os.path.join(thermal_dir, zone, "temp")
                            if os.path.exists(temp_path):
                                with open(temp_path, "r") as f:
                                    val = float(f.read().strip()) / 1000.0
                                    if 0 < val < 120:  # Reasonable temperature range
                                        cpu_temp = val
                                        break
                    if cpu_temp is not None:
                        break
                except Exception:
                    pass

    if cpu_temp is not None or sys_temp is not None:
        available = True

    return TemperatureStatus(
        cpu_celsius=round(cpu_temp, 1) if cpu_temp is not None else None,
        system_celsius=round(sys_temp, 1) if sys_temp is not None else None,
        available=available
    )
