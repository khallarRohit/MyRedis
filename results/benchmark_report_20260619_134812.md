# MyRedis vs Docker Redis Benchmark Report
**Date**: 2026-06-19 13:48:12
**Requests per Test**: 50000

## 1. Non-Pipeline Throughput

### SET
| Concurrency | Real Redis rps | MyRedis rps | Ratio | Winner |
|------------:|---------------:|------------:|------:|--------|
| 10 |    130,208.3 |    15,777.9 |  12.1% | Redis |
| 50 |    152,439.0 |    19,282.7 |  12.6% | Redis |
| 100 |    156,739.8 |    17,927.6 |  11.4% | Redis |

### GET
| Concurrency | Real Redis rps | MyRedis rps | Ratio | Winner |
|------------:|---------------:|------------:|------:|--------|
| 10 |    144,092.2 |    15,893.2 |  11.0% | Redis |
| 50 |    151,975.7 |    19,920.3 |  13.1% | Redis |
| 100 |    147,058.8 |    19,327.4 |  13.1% | Redis |

## 2. Pipeline Throughput (Concurrency = 50)

### SET
| Pipeline | Real Redis rps | MyRedis rps | Ratio | Winner |
|---------:|---------------:|------------:|------:|--------|
| 1 |    152,905.2 |    18,754.7 |  12.3% | Redis |
| 16 |  1,612,903.2 |    63,613.2 |   3.9% | Redis |
| 64 |  2,174,608.8 |    76,687.9 |   3.5% | Redis |

### GET
| Pipeline | Real Redis rps | MyRedis rps | Ratio | Winner |
|---------:|---------------:|------------:|------:|--------|
| 1 |    151,057.4 |    19,216.0 |  12.7% | Redis |
| 16 |  1,960,784.4 |    57,770.1 |   2.9% | Redis |
| 64 |  3,125,999.8 |    75,661.1 |   2.4% | Redis |
