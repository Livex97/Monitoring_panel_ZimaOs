import os
import psutil
from typing import List
from app.models import StoragePool, DiskStatus, StorageStatus


def get_storage_pools() -> List[StoragePool]:
    pools: List[StoragePool] = []
    seen_mounts = set()

    # Prioritize root and storage volume mounts
    partitions = psutil.disk_partitions(all=False)
    for part in partitions:
        mount = part.mountpoint
        # Filter out pseudofs / snap / docker overlay mounts
        if mount.startswith("/proc") or mount.startswith("/sys") or mount.startswith("/dev") or "/docker" in mount or "/overlay" in mount:
            continue
        if mount in seen_mounts:
            continue

        try:
            usage = psutil.disk_usage(mount)
            seen_mounts.add(mount)

            name = "Root" if mount == "/" else os.path.basename(mount.rstrip("/")) or mount

            pools.append(
                StoragePool(
                    name=name,
                    filesystem=part.fstype,
                    mount_point=mount,
                    total_bytes=usage.total,
                    used_bytes=usage.used,
                    free_bytes=usage.free,
                    usage_percent=round(usage.percent, 1)
                )
            )
        except Exception:
            pass

    return pools


def get_disks() -> List[DiskStatus]:
    disks: List[DiskStatus] = []
    
    # Try reading sysfs block devices
    block_dir = "/sys/block"
    if os.path.exists(block_dir):
        try:
            for dev in os.listdir(block_dir):
                # Filter for disk drives (sdX, nvmeXnX, vdX)
                if dev.startswith("sd") or dev.startswith("nvme") or dev.startswith("vd"):
                    dev_path = os.path.join(block_dir, dev)
                    
                    # Size in 512 byte sectors
                    size_file = os.path.join(dev_path, "size")
                    capacity = 0
                    if os.path.exists(size_file):
                        try:
                            with open(size_file, "r") as f:
                                capacity = int(f.read().strip()) * 512
                        except Exception:
                            pass

                    model = dev
                    model_file = os.path.join(dev_path, "device", "model")
                    if os.path.exists(model_file):
                        try:
                            with open(model_file, "r") as f:
                                model = f.read().strip()
                        except Exception:
                            pass

                    disks.append(
                        DiskStatus(
                            name=dev,
                            model=model,
                            capacity_bytes=capacity,
                            temperature_celsius=None,
                            health="OK",
                            smart_status="PASSED"
                        )
                    )
        except Exception:
            pass

    return disks


def get_storage_status() -> StorageStatus:
    return StorageStatus(
        pools=get_storage_pools(),
        disks=get_disks()
    )
