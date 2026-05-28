FROM python:3.12-slim AS builder

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential make \
    && rm -rf /var/lib/apt/lists/*

COPY . .
RUN make clean && make app

FROM python:3.12-slim AS runner

WORKDIR /app

COPY pyproject.toml uv.lock* ./
RUN pip install --no-cache-dir streamlit

COPY app.py preprocess.py ./
COPY data/README.md data/README.md
COPY --from=builder /app/app ./app

EXPOSE 8501

CMD ["streamlit", "run", "app.py", "--server.address=0.0.0.0", "--server.port=8501"]
