import logging
import docker
from typing import List
from app.config import settings
from app.models import ContainerInfo, DockerStatus

logger = logging.getLogger("docker_collector")


def get_docker_status() -> DockerStatus:
    total = 0
    running = 0
    stopped = 0
    containers_list: List[ContainerInfo] = []

    candidate_sockets = [
        settings.docker_socket,
        "unix:///var/run/docker.sock",
        "unix:///host/var/run/docker.sock",
        "unix:///run/docker.sock"
    ]

    client = None
    for socket_url in candidate_sockets:
        try:
            client = docker.DockerClient(base_url=socket_url, timeout=3)
            # Test ping
            client.ping()
            break
        except Exception as e:
            client = None

    if client:
        try:
            raw_containers = client.containers.list(all=True)
            for c in raw_containers:
                total += 1
                status_str = c.status.lower()
                if status_str == "running":
                    running += 1
                else:
                    stopped += 1

                image_name = ""
                if c.image and c.image.tags:
                    image_name = c.image.tags[0]
                elif c.image:
                    image_name = c.image.short_id

                state_status = c.attrs.get("State", {}).get("Status", c.status)
                
                containers_list.append(
                    ContainerInfo(
                        name=c.name.lstrip("/"),
                        status=state_status.upper() if state_status.lower() == "running" else state_status,
                        image=image_name,
                        uptime=c.status
                    )
                )
            client.close()
        except Exception as e:
            logger.warning(f"Error fetching docker containers: {e}")

    return DockerStatus(
        total=total,
        running=running,
        stopped=stopped,
        containers=containers_list
    )
