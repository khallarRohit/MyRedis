# MyRedis vs Docker Redis Benchmark Report
**Date**: 2026-06-19 14:02:54
**Requests per Test**: 50000

## 1. Non-Pipeline Throughput

### SET
| Concurrency | Real Redis rps | MyRedis rps | Ratio | Winner |
|------------:|---------------:|------------:|------:|--------|
| 10 |    144,927.5 |    12,976.9 |   9.0% | Redis |
| 50 |    154,798.8 |    16,756.0 |  10.8% | Redis |
| 100 |    156,250.0 |    17,966.2 |  11.5% | Redis |

### GET
| Concurrency | Real Redis rps | MyRedis rps | Ratio | Winner |
|------------:|---------------:|------------:|------:|--------|
| 10 |    135,135.1 |    13,164.8 |   9.7% | Redis |
| 50 |    145,348.8 |    16,463.6 |  11.3% | Redis |
| 100 |    127,226.5 |    15,169.9 |  11.9% | Redis |

## 2. Pipeline Throughput (Concurrency = 50)

### SET
| Pipeline | Real Redis rps | MyRedis rps | Ratio | Winner |
|---------:|---------------:|------------:|------:|--------|
| 1 |    139,664.8 |    15,179.1 |  10.9% | Redis |
| 16 |  1,538,461.6 |    66,137.6 |   4.3% | Redis |
| 64 |  2,222,933.2 |   107,909.4 |   4.9% | Redis |

### GET
| Pipeline | Real Redis rps | MyRedis rps | Ratio | Winner |
|---------:|---------------:|------------:|------:|--------|
| 1 |    147,492.6 |    17,265.2 |  11.7% | Redis |
| 16 |  1,818,181.9 |    88,105.7 |   4.8% | Redis |
| 64 |  3,031,272.8 |   112,017.9 |   3.7% | Redis |
