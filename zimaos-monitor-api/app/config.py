from typing import Optional
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    app_name: str = "ZimaOS Monitor API"
    version: str = "1.0.0"
    debug: bool = False
    
    monitor_port: int = 8787
    monitor_api_key: Optional[str] = None
    
    # Host root mounts for docker container environment
    host_proc: str = "/host/proc"
    host_sys: str = "/host/sys"
    host_etc: str = "/host/etc"
    docker_socket: str = "unix:///var/run/docker.sock"
    
    # ZimaOS local endpoint if available
    zimaos_host: str = "http://127.0.0.1:80"
    
    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding="utf-8",
        extra="ignore"
    )


settings = Settings()
