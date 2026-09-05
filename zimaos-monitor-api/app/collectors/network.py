import time
import psutil
from typing import Dict, Tuple, List
from app.models import NetworkInterface, NetworkStatus

# State for rate calculation
_last_sample_time: float = 0.0
_last_counters: Dict[str, Tuple[int, int]] = {}


def get_network_status() -> NetworkStatus:
    global _last_sample_time, _last_counters

    now = time.time()
    dt = now - _last_sample_time if _last_sample_time > 0 else 0.0
    _last_sample_time = now

    current_counters = psutil.net_io_counters(pernic=True)
    interfaces: List[NetworkInterface] = []

    for name, stats in current_counters.items():
        # Ignore loopback and virtual docker/veth interfaces
        if name == "lo" or name.startswith("veth") or name.startswith("docker") or name.startswith("br-"):
            continue

        rx_bytes = stats.bytes_recv
        tx_bytes = stats.bytes_sent

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
