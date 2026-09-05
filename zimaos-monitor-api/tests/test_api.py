from fastapi.testclient import TestClient
from app.main import app
from app.config import settings

client = TestClient(app)


def test_health_endpoint():
    response = client.get("/health")
    assert response.status_code == 200
    data = response.json()
    assert data["status"] == "healthy"


def test_version_endpoint():
    response = client.get("/api/v1/version")
    assert response.status_code == 200
    data = response.json()
    assert "version" in data
    assert data["schema_version"] == "1.0"


def test_status_endpoint():
    response = client.get("/api/v1/status")
    assert response.status_code == 200
    data = response.json()
    assert data["schema_version"] == "1.0"
    assert "server" in data
    assert "cpu" in data
    assert "memory" in data
    assert "storage" in data
    assert "docker" in data
    assert "network" in data
    assert "health" in data


def test_sub_endpoints():
    r_sys = client.get("/api/v1/system")
    assert r_sys.status_code == 200

    r_storage = client.get("/api/v1/storage")
    assert r_storage.status_code == 200

    r_docker = client.get("/api/v1/docker")
    assert r_docker.status_code == 200

    r_network = client.get("/api/v1/network")
    assert r_network.status_code == 200


def test_api_key_auth(monkeypatch):
    # Enable API Key requirement
    monkeypatch.setattr(settings, "monitor_api_key", "secret123")

    # Unauthorized request
    r_unauth = client.get("/api/v1/status")
    assert r_unauth.status_code == 401

    # Invalid key
    r_invalid = client.get("/api/v1/status", headers={"X-API-Key": "wrong"})
    assert r_invalid.status_code == 401

    # Valid key
    r_valid = client.get("/api/v1/status", headers={"X-API-Key": "secret123"})
    assert r_valid.status_code == 200
