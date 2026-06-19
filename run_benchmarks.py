#!/usr/bin/env python3
"""
MyRedis vs Real Redis comprehensive benchmark driver for Windows/Docker.
"""

import subprocess
import sys
import os
import statistics
from datetime import datetime
from pathlib import Path

# --- Configuration ---
# Your C++ Server (Running directly on Windows)
MYREDIS_HOST = "host.docker.internal"
MYREDIS_PORT = 6112

# Real Redis (Running in Docker)
DOCKER_REDIS_HOST = "127.0.0.1"
DOCKER_REDIS_PORT = 6379 

# The Docker command to execute redis-benchmark
BENCH_PREFIX = ["docker", "run", "--rm", "--network", "host", "redis", "redis-benchmark"]

# Testing Parameters
REQUESTS = 50000          # Total requests per test
PIPELINE_REQUESTS = 100000 
REPEATS = 3               # Number of times to repeat each test (for median accuracy)

# Only testing commands currently implemented in Dispatcher
COMMANDS = ["SET", "GET"]
CONCURRENCIES = [10, 50, 100]
PIPELINES = [1, 16, 64]

class BenchmarkRunner:
    def __init__(self):
        self.results_dir = Path(__file__).parent / "results"
        self.results_dir.mkdir(parents=True, exist_ok=True)
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.outfile = self.results_dir / f"benchmark_report_{ts}.md"
        self.out = []

    def write(self, line: str = "") -> None:
        self.out.append(line)
        print(line)

    def flush(self) -> None:
        text = "\n".join(self.out)
        self.outfile.write_text(text + "\n")
        print(f"\n[+] Results saved to {self.outfile}")

    def _run_bench_csv(self, host: str, port: int, cmd: str, concurrency: int, pipeline: int = 1) -> float:
        # e.g., docker run --rm --network host redis redis-benchmark -h 127.0.0.1 -p 6112 -n 50000 -c 50 -P 1 -t SET --csv
        n_req = PIPELINE_REQUESTS if pipeline > 1 else REQUESTS
        args = BENCH_PREFIX + [
            "-h", host, "-p", str(port),
            "-n", str(n_req), "-c", str(concurrency),
            "-P", str(pipeline), "-t", cmd, "--csv"
        ]
        
        try:
            result = subprocess.run(args, capture_output=True, timeout=60)
            stdout_text = result.stdout.decode('utf-8', errors='replace')
            
            for line in stdout_text.strip().split('\n'):
                line = line.strip()
                if not line or line.startswith('"test'): continue
                
                parts = line.split(",")
                if len(parts) >= 2:
                    try:
                        return float(parts[1].strip('"')) # Extract RPS from CSV format
                    except ValueError:
                        pass
        except subprocess.TimeoutExpired:
            print(f"[!] Timeout on {cmd} at port {port}")
        return 0.0

    def _bench_rps_repeated(self, host: str, port: int, cmd: str, concurrency: int, pipeline: int = 1) -> float:
        vals = []
        for _ in range(REPEATS):
            v = self._run_bench_csv(host, port, cmd, concurrency, pipeline)
            if v > 0:
                vals.append(v)
        return statistics.median(vals) if vals else 0.0

    def run_throughput_tests(self):
        self.write("# MyRedis vs Docker Redis Benchmark Report")
        self.write(f"**Date**: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        self.write(f"**Requests per Test**: {REQUESTS}")
        self.write("\n## 1. Non-Pipeline Throughput")

        for cmd in COMMANDS:
            self.write(f"\n### {cmd}")
            self.write("| Concurrency | Real Redis rps | MyRedis rps | Ratio | Winner |")
            self.write("|------------:|---------------:|------------:|------:|--------|")

            for c in CONCURRENCIES:
                redis_rps = self._bench_rps_repeated(DOCKER_REDIS_HOST, DOCKER_REDIS_PORT, cmd, c, 1)
                my_rps = self._bench_rps_repeated(MYREDIS_HOST, MYREDIS_PORT, cmd, c, 1)
                
                ratio = (my_rps / redis_rps * 100) if redis_rps > 0 else 0.0
                winner = "**MyRedis**" if my_rps > redis_rps else "Redis"
                
                self.write(f"| {c} | {redis_rps:>12,.1f} | {my_rps:>11,.1f} | {ratio:>5.1f}% | {winner} |")

    def run_pipeline_tests(self):
        self.write("\n## 2. Pipeline Throughput (Concurrency = 50)")
        
        for cmd in COMMANDS:
            self.write(f"\n### {cmd}")
            self.write("| Pipeline | Real Redis rps | MyRedis rps | Ratio | Winner |")
            self.write("|---------:|---------------:|------------:|------:|--------|")

            for p in PIPELINES:
                redis_rps = self._bench_rps_repeated(DOCKER_REDIS_HOST, DOCKER_REDIS_PORT, cmd, 50, p)
                my_rps = self._bench_rps_repeated(MYREDIS_HOST, MYREDIS_PORT, cmd, 50, p)
                
                ratio = (my_rps / redis_rps * 100) if redis_rps > 0 else 0.0
                winner = "**MyRedis**" if my_rps > redis_rps else "Redis"
                
                self.write(f"| {p} | {redis_rps:>12,.1f} | {my_rps:>11,.1f} | {ratio:>5.1f}% | {winner} |")


if __name__ == "__main__":
    runner = BenchmarkRunner()
    runner.run_throughput_tests()
    runner.run_pipeline_tests()
    runner.flush()