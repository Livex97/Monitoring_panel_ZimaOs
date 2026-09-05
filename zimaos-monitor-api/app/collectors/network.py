import os
import time
import psutil
from typing import Dict, Tuple, List
from app.config import settings
from app.models import NetworkInterface, NetworkStatus

_last_sample_time: float = 0.0
_last_counters: Dict[str, Tuple[int, int]] = {}


def read_net_dev_counters() -> Dict[str, Tuple[int, int]]:
    counters: Dict[str, Tuple[int, int]] = {}
    
    net_dev_paths = [
        os.path.join(settings.host_proc, "net/dev"),
        "/proc/net/dev"
    ]

    for p in net_dev_paths:
        if os.path.exists(p):
            try:
                with open(p, "r") as f:
                    lines = f.readlines()[2:]
                    for line in lines:
                        if ":" in line:
                            iface, data = line.split(":", 1)
                            iface = iface.strip()
                            cols = data.split()
                            if len(cols) >= 9:
                                rx_bytes = int(cols[0])
                                tx_bytes = int(cols[8])
                                counters[iface] = (rx_bytes, tx_bytes)
                if counters:
                    return counters
            except Exception:
                pass

    # Fallback to psutil
    try:
        current_counters = psutil.net_io_counters(pernic=True)
        for name, stats in current_counters.items():
            counters[name] = (stats.bytes_recv, stats.bytes_sent)
    except Exception:
        pass

    return counters


def get_network_status() -> NetworkStatus:
    global _last_sample_time, _last_counters

    now = time.time()
    dt = now - _last_sample_time if _last_sample_time > 0 else 0.0
    _last_sample_time = now

    current_counters = read_net_dev_counters()
    interfaces: List[NetworkInterface] = []

    for name, (rx_bytes, tx_bytes) in current_counters.items():
        if name == "lo" or name.startswith("veth") or name.startswith("docker") or name.startswith("br-"):
            continue

        rx_rate = 0.0
        tx_rate = 0.0

        if dt > 0 and name in _last_counters:
            prev_rx, prev_tx = _last_counters[name]
            rx_diff = rx_bytes - prev_rx if rx_bytes >= prev_rx else 0
            tx_diff = tx_bytes - prev_tx if tx_bytes >= prev_tx else 0
            rx_rate = round(rx_diff / dt, 1)
            tx_rate = round(tx_diff / dt, 1)

        _last_counters[name] = (rx_bytes, tx_bytes)

        interfaces.append(
            NetworkInterface(
                name=name,
                rx_bytes=rx_bytes,
                tx_bytes=tx_bytes,
                rx_rate_bytes_sec=rx_rate,
                tx_rate_bytes_sec=tx_rate
            )
        )

    return NetworkStatus(interfaces=interfaces)
