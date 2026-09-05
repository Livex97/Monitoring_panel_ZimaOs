import httpx
from typing import Dict, Any, Optional
from app.config import settings


async def fetch_zimaos_system_info() -> Optional[Dict[str, Any]]:
    """
    Attempts to fetch hardware/system metrics from ZimaOS local OpenAPI if reachable.
    """
    url = f"{settings.zimaos_host}/v1/sys/hardware"
    try:
        async with httpx.AsyncClient(timeout=2.0) as client:
            resp = await client.get(url)
            if resp.status_code == 200:
                return resp.json()
    except Exception:
        pass
    return None
