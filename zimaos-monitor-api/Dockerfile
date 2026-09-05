FROM python:3.11-slim

LABEL maintainer="ZimaOS Monitor Panel"
LABEL description="Lightweight Read-Only Monitoring API for ZimaOS NAS Host"

WORKDIR /app

# Install security updates and system tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY app ./app

ENV MONITOR_PORT=8787
EXPOSE 8787

HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8787/health || exit 1

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8787"]
