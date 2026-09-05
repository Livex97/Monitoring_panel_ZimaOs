from fastapi import FastAPI, Header, HTTPException, Security, status
from fastapi.middleware.cors import CORSMiddleware
from app.config import settings
from app.models import (
    SystemMetricsResponse,
    CPUStatus,
    MemoryStatus,
    StorageStatus,
    DockerStatus,
    NetworkStatus,
    ServerStatus
)
from app.services.monitor import collect_system_metrics
from app.collectors.system import get_cpu_status, get_memory_status, get_server_status
from app.collectors.storage import get_storage_status
from app.collectors.docker_collector import get_docker_status
from app.collectors.network import get_network_status

app = FastAPI(
    title=settings.app_name,
    version=settings.version,
    description="Lightweight Read-Only Monitoring API for ZimaOS NAS Host"
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["GET"],
    allow_headers=["*"],
)


def verify_api_key(x_api_key: str = Header(None, alias="X-API-Key")):
    if settings.monitor_api_key:
        if not x_api_key or x_api_key != settings.monitor_api_key:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Invalid or missing X-API-Key header"
            )


@app.get("/health", tags=["Health"])
def health_check():
    return {"status": "healthy", "app": settings.app_name, "version": settings.version}


@app.get("/api/v1/version", tags=["Info"])
def get_version():
    return {"version": settings.version, "schema_version": "1.0"}


@app.get("/api/v1/status", response_model=SystemMetricsResponse, tags=["Metrics"])
def get_full_status(api_key: None = Security(verify_api_key)):
    return collect_system_metrics()


@app.get("/api/v1/system", tags=["Metrics"])
def get_system(api_key: None = Security(verify_api_key)):
    return {
        "server": get_server_status(),
        "cpu": get_cpu_status(),
        "memory": get_memory_status()
    }


@app.get("/api/v1/storage", response_model=StorageStatus, tags=["Metrics"])
def get_storage(api_key: None = Security(verify_api_key)):
    return get_storage_status()


@app.get("/api/v1/docker", response_model=DockerStatus, tags=["Metrics"])
def get_docker(api_key: None = Security(verify_api_key)):
    return get_docker_status()


@app.get("/api/v1/network", response_model=NetworkStatus, tags=["Metrics"])
def get_network(api_key: None = Security(verify_api_key)):
    return get_network_status()


if __name__ == "__main__":
    import uvicorn
    uvicorn.run("main:app", host="0.0.0.0", port=settings.monitor_port, reload=settings.debug)
