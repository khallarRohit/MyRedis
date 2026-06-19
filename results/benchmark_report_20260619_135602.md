# MyRedis vs Docker Redis Benchmark Report
**Date**: 2026-06-19 13:56:02
**Requests per Test**: 50000

## 1. Non-Pipeline Throughput

### SET
| Concurrency | Real Redis rps | MyRedis rps | Ratio | Winner |
|------------:|---------------:|------------:|------:|--------|
| 10 |    145,348.8 |    13,196.1 |   9.1% | Redis |
| 50 |    152,439.0 |    17,736.8 |  11.6% | Redis |
| 100 |    143,678.2 |    18,089.7 |  12.6% | Redis |

### GET
| Concurrency | Real Redis rps | MyRedis rps | Ratio | Winner |
|------------:|---------------:|------------:|------:|--------|
| 10 |    141,643.1 |    15,898.2 |  11.2% | Redis |
| 50 |    151,515.1 |    17,099.9 |  11.3% | Redis |
| 100 |    137,741.0 |    18,228.2 |  13.2% | Redis |

## 2. Pipeline Throughput (Concurrency = 50)

### SET
| Pipeline | Real Redis rps | MyRedis rps | Ratio | Winner |
|---------:|---------------:|------------:|------:|--------|
| 1 |    150,602.4 |    17,012.6 |  11.3% | Redis |
| 16 |  1,562,499.9 |    59,701.5 |   3.8% | Redis |
| 64 |  2,128,340.5 |    65,666.4 |   3.1% | Redis |

### GET
| Pipeline | Real Redis rps | MyRedis rps | Ratio | Winner |
|---------:|---------------:|------------:|------:|--------|
| 1 |    146,627.6 |    17,409.5 |  11.9% | Redis |
| 16 |  1,754,386.0 |    59,594.8 |   3.4% | Redis |
| 64 |  2,858,057.2 |    75,310.2 |   2.6% | Redis |
