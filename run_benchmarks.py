#!/usr/bin/env python3
"""
MyRedis vs Real Redis comprehensive benchmark driver.
Expanded for Advanced Data Structures & Memory Stress Testing.
"""

import subprocess
import statistics
from datetime import datetime
from pathlib import Path

# --- Configuration ---

MYREDIS_HOST = "host.docker.internal"
MYREDIS_PORT = 6112


DOCKER_REDIS_HOST = "127.0.0.1"
DOCKER_REDIS_PORT = 6379    

BENCH_PREFIX = ["docker", "run", "--rm", "--network", "host", "redis", "redis-benchmark"]

# Testing Parameters
REQUESTS = 50000          
PIPELINE_REQUESTS = 100000 
REPEATS = 3               

# Expanded Command Suite for newly implemented data structures
COMMANDS = [
    "SET", "GET", "MSET",              # Strings
    "LPUSH", "RPUSH", "LPOP", "RPOP",  # Lists
    "HSET",                            # Hashes
    "SADD",                            # Sets
    "ZADD"                             # Sorted Sets
]

CONCURRENCIES = [10, 50, 100]
PIPELINES = [1, 16, 64]
PAYLOAD_SIZES = [32, 256, 1024]        # Bytes

class BenchmarkRunner:
    def __init__(self):
        self.results_dir = Path(__file__).parent / "results"
        self.results_dir.mkdir(parents=True, exist_ok=True)
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.outfile = self.results_dir / f"benchmark_report_full_{ts}.md"
        self.out = []

    def write(self, line: str = "") -> None:
        self.out.append(line)
        print(line)

    def flush(self) -> None:
        text = "\n".join(self.out)
        self.outfile.write_text(text + "\n")
        print(f"\n[+] Comprehensive results saved to {self.outfile}")

    def _run_bench_csv(self, host: str, port: int, cmd: str, concurrency: int, pipeline: int = 1, payload: int = 3) -> float:
        n_req = PIPELINE_REQUESTS if pipeline > 1 else REQUESTS
        
        # redis-benchmark expects lowercase for the -t argument
        target_cmd = cmd.lower()
        
        args = BENCH_PREFIX + [
            "-h", host, "-p", str(port),
            "-n", str(n_req), "-c", str(concurrency),
            "-P", str(pipeline), "-d", str(payload), 
            "-t", target_cmd, "--csv"
        ]
        
        try:
            result = subprocess.run(args, capture_output=True, timeout=90)
            stdout_text = result.stdout.decode('utf-8', errors='replace')
            
            for line in stdout_text.strip().split('\n'):
                line = line.strip()
                if not line or line.startswith('"test'): continue
                
                parts = line.split(",")
                if len(parts) >= 2:
                    try:
                        return float(parts[1].strip('"')) 
                    except ValueError:
                        pass
        except subprocess.TimeoutExpired:
            print(f"[!] Timeout on {cmd} at port {port}")
        return 0.0

    def _bench_rps_repeated(self, host: str, port: int, cmd: str, concurrency: int, pipeline: int = 1, payload: int = 3) -> float:
        vals = []
        for _ in range(REPEATS):
            v = self._run_bench_csv(host, port, cmd, concurrency, pipeline, payload)
            if v > 0:
                vals.append(v)
        return statistics.median(vals) if vals else 0.0

    def run_throughput_tests(self):
        self.write("# MyRedis vs Docker Redis - Comprehensive Benchmark")
        self.write(f"**Date**: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        self.write(f"**Base Requests**: {REQUESTS}")
        self.write("\n## 1. Advanced Data Structure Throughput (Non-Pipelined)")
        self.write("> Tests lock contention and memory allocation overhead on complex types.")

        for cmd in COMMANDS:
            self.write(f"\n### Command: {cmd}")
            self.write("| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |")
            self.write("|------------:|-----------------:|--------------:|------:|--------|")

            for c in CONCURRENCIES:
                redis_rps = self._bench_rps_repeated(DOCKER_REDIS_HOST, DOCKER_REDIS_PORT, cmd, c, 1)
                my_rps = self._bench_rps_repeated(MYREDIS_HOST, MYREDIS_PORT, cmd, c, 1)
                
                ratio = (my_rps / redis_rps * 100) if redis_rps > 0 else 0.0
                winner = "**MyRedis**" if my_rps > redis_rps else "Redis"
                
                self.write(f"| {c} | {redis_rps:>14,.1f} | {my_rps:>13,.1f} | {ratio:>5.1f}% | {winner} |")

    def run_pipeline_tests(self):
        self.write("\n## 2. Pipelined I/O Parsing (Concurrency = 50)")
        self.write("> Tests the raw socket ingestion and RESP parser efficiency.")
        
        # Testing pipeline efficiency primarily on SET/GET as they isolate parser speed from data structure logic
        for cmd in ["SET", "GET", "HSET", "LPUSH"]:
            self.write(f"\n### Command: {cmd}")
            self.write("| Pipeline | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |")
            self.write("|---------:|-----------------:|--------------:|------:|--------|")

            for p in PIPELINES:
                redis_rps = self._bench_rps_repeated(DOCKER_REDIS_HOST, DOCKER_REDIS_PORT, cmd, 50, p)
                my_rps = self._bench_rps_repeated(MYREDIS_HOST, MYREDIS_PORT, cmd, 50, p)
                
                ratio = (my_rps / redis_rps * 100) if redis_rps > 0 else 0.0
                winner = "**MyRedis**" if my_rps > redis_rps else "Redis"
                
                self.write(f"| {p} | {redis_rps:>14,.1f} | {my_rps:>13,.1f} | {ratio:>5.1f}% | {winner} |")

    def run_payload_stress_tests(self):
        self.write("\n## 3. Payload Memory Stress Test (SET)")
        self.write("> Tests the C++ memory allocator against Redis jemalloc using varying string sizes.")
        
        self.write("\n| Payload (Bytes) | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |")
        self.write("|----------------:|-----------------:|--------------:|------:|--------|")

        for size in PAYLOAD_SIZES:
            redis_rps = self._bench_rps_repeated(DOCKER_REDIS_HOST, DOCKER_REDIS_PORT, "SET", 50, 1, size)
            my_rps = self._bench_rps_repeated(MYREDIS_HOST, MYREDIS_PORT, "SET", 50, 1, size)
            
            ratio = (my_rps / redis_rps * 100) if redis_rps > 0 else 0.0
            winner = "**MyRedis**" if my_rps > redis_rps else "Redis"
            
            self.write(f"| {size} | {redis_rps:>14,.1f} | {my_rps:>13,.1f} | {ratio:>5.1f}% | {winner} |")


if __name__ == "__main__":
    runner = BenchmarkRunner()
    
    print("[*] Starting Phase 1: Data Structure Throughput...")
    runner.run_throughput_tests()
    
    print("\n[*] Starting Phase 2: Pipeline I/O Parsers...")
    runner.run_pipeline_tests()
    
    print("\n[*] Starting Phase 3: Memory Stress Tests...")
    runner.run_payload_stress_tests()
    
    runner.flush()