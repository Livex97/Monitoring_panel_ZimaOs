import os
import psutil
from typing import List
from app.config import settings
from app.models import StoragePool, DiskStatus, StorageStatus


def get_storage_pools() -> List[StoragePool]:
    pools: List[StoragePool] = []
    seen_mounts = set()

    partitions = psutil.disk_partitions(all=False)
    
    # Also check /host/proc/mounts or /proc/mounts for real host mounts
    host_mounts = []
    mounts_file = os.path.join(settings.host_proc, "mounts")
    if not os.path.exists(mounts_file):
        mounts_file = "/proc/mounts"
        
    if os.path.exists(mounts_file):
        try:
            with open(mounts_file, "r") as f:
                for line in f:
                    parts = line.split()
                    if len(parts) >= 3:
                        device, target, fstype = parts[0], parts[1], parts[2]
                        if fstype in ("ext4", "zfs", "btrfs", "xfs", "fuse.rclone", "overlay"):
                            host_mounts.append((target, fstype))
        except Exception:
            pass

    for part in partitions:
        mount = part.mountpoint
        # Filter out container internal mounts
        if (mount.startswith("/etc/") or mount in ("/etc/resolv.conf", "/etc/hostname", "/etc/hosts")
                or mount.startswith("/proc") or mount.startswith("/sys") or mount.startswith("/dev")
                or "/docker" in mount or "/overlay" in mount):
            continue
        if mount in seen_mounts:
            continue

        try:
            usage = psutil.disk_usage(mount)
            seen_mounts.add(mount)

            name = "POOL 1" if (mount == "/" or "DATA" in mount) else os.path.basename(mount.rstrip("/")) or mount

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

    if not pools:
        # Fallback to root disk usage
        try:
            usage = psutil.disk_usage("/")
            pools.append(
                StoragePool(
                    name="POOL 1",
                    filesystem="ext4",
                    mount_point="/",
                    total_bytes=usage.total,
                    used_bytes=usage.used,
                    free_bytes=usage.free,
                    usage_percent=round(usage.percent, 1)
                )
            )
        except Exception:
            pass

    return pools


def get_disk_temperature(dev_name: str) -> float:
    # Try reading sysfs hwmon for block device or NVMe device
    sys_block_dirs = [
        os.path.join(settings.host_sys, f"block/{dev_name}/device/hwmon"),
        os.path.join(settings.host_sys, f"class/nvme/{dev_name}/device/hwmon"),
        f"/sys/block/{dev_name}/device/hwmon",
        f"/sys/class/nvme/{dev_name}/device/hwmon"
    ]

    for hwmon_base in sys_block_dirs:
        if os.path.exists(hwmon_base):
            try:
                for hw_item in os.listdir(hwmon_base):
                    hw_path = os.path.join(hwmon_base, hw_item)
                    if os.path.isdir(hw_path):
                        for f_name in os.listdir(hw_path):
                            if f_name.startswith("temp") and f_name.endswith("_input"):
                                temp_file = os.path.join(hw_path, f_name)
                                with open(temp_file, "r") as f:
                                    val = float(f.read().strip()) / 1000.0
                                    if 0 < val < 100:
                                        return round(val, 1)
            except Exception:
                pass

    return None


def get_disks() -> List[DiskStatus]:
    disks: List[DiskStatus] = []
    
    block_dirs = [
        os.path.join(settings.host_sys, "block"),
        "/sys/block"
    ]

    for block_dir in block_dirs:
        if os.path.exists(block_dir):
            try:
                for dev in sorted(os.listdir(block_dir)):
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

                        temp_val = get_disk_temperature(dev)

                        disks.append(
                            DiskStatus(
                                name=dev,
                                model=model,
                                capacity_bytes=capacity,
                                temperature_celsius=temp_val,
                                health="OK",
                                smart_status="PASSED"
                            )
                        )
                if disks:
                    break
            except Exception:
                pass

    return disks


def get_storage_status() -> StorageStatus:
    return StorageStatus(
        pools=get_storage_pools(),
        disks=get_disks()
    )
