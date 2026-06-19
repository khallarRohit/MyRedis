# MyRedis vs Docker Redis Benchmark Report
**Date**: 2026-06-19 13:38:04
**Requests per Test**: 50000

## 1. Non-Pipeline Throughput

### SET
| Concurrency | Real Redis rps | MyRedis rps | Ratio | Winner |
|------------:|---------------:|------------:|------:|--------|
| 10 |    121,359.2 |         0.0 |   0.0% | Redis |
| 50 |    131,233.6 |         0.0 |   0.0% | Redis |
| 100 |    134,048.3 |         0.0 |   0.0% | Redis |

### GET
| Concurrency | Real Redis rps | MyRedis rps | Ratio | Winner |
|------------:|---------------:|------------:|------:|--------|
| 10 |    146,198.8 |         0.0 |   0.0% | Redis |
| 50 |    147,492.6 |         0.0 |   0.0% | Redis |
| 100 |    152,905.2 |         0.0 |   0.0% | Redis |

## 2. Pipeline Throughput (Concurrency = 50)

### SET
| Pipeline | Real Redis rps | MyRedis rps | Ratio | Winner |
|---------:|---------------:|------------:|------:|--------|
| 1 |    147,058.8 |         0.0 |   0.0% | Redis |
| 16 |  1,538,461.6 |         0.0 |   0.0% | Redis |
| 64 |  2,174,608.8 |         0.0 |   0.0% | Redis |

### GET
| Pipeline | Real Redis rps | MyRedis rps | Ratio | Winner |
|---------:|---------------:|------------:|------:|--------|
| 1 |    147,929.0 |         0.0 |   0.0% | Redis |
| 16 |  1,923,076.9 |         0.0 |   0.0% | Redis |
| 64 |  2,942,117.5 |         0.0 |   0.0% | Redis |
