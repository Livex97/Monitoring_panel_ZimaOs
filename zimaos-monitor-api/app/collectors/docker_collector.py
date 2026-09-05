import docker
from typing import List
from app.config import settings
from app.models import ContainerInfo, DockerStatus


def get_docker_status() -> DockerStatus:
    total = 0
    running = 0
    stopped = 0
    containers_list: List[ContainerInfo] = []

    try:
        client = docker.DockerClient(base_url=settings.docker_socket, timeout=3)
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

            containers_list.append(
                ContainerInfo(
                    name=c.name.lstrip("/"),
                    status=c.status,
                    image=image_name,
                    uptime=c.attrs.get("State", {}).get("Status", c.status)
                )
            )
        client.close()
    except Exception:
        # Docker engine not available / socket not mounted
        pass

    return DockerStatus(
        total=total,
        running=running,
        stopped=stopped,
        containers=containers_list
    )
