# MyRedis vs Docker Redis - Comprehensive Benchmark
**Date**: 2026-07-16 08:13:26
**Base Requests**: 50000

## 1. Advanced Data Structure Throughput (Non-Pipelined)
> Tests lock contention and memory allocation overhead on complex types.

### Command: SET
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      134,408.6 |      96,774.1 |  72.0% | Redis |
| 50 |      147,492.6 |      110,029.4 |  74.6% | Redis |
| 100 |      145,772.6 |      107,142.8 |  73.5% | Redis |

### Command: GET
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      134,770.9 |      102,020.8 |  75.7% | Redis |
| 50 |      149,700.6 |      115,567.9 |  77.2% | Redis |
| 100 |      152,905.2 |      119,724.4 |  78.3% | Redis |

### Command: MSET
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      161,812.3 |      99,514.5 |   61.5% | Redis |
| 50 |      168,918.9 |      105,405.3 |  62.4% | Redis |
| 100 |      171,232.9 |      106,678.0 |  62.3% | Redis |

### Command: LPUSH
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      148,368.0 |      86,498.5 |   58.3% | Redis |
| 50 |      149,700.6 |      87,874.2 |   58.7% | Redis |
| 100 |      156,250.0 |      93,593.7 |   59.9% | Redis |

### Command: RPUSH
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      146,627.6 |      83,724.3 |   57.1% | Redis |
| 50 |      151,515.1 |      90,151.4 |   59.5% | Redis |
| 100 |      153,846.2 |      91,383.7 |   59.4% | Redis |

### Command: LPOP
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      145,772.6 |      79,154.5 |   54.3% | Redis |
| 50 |      155,279.5 |      83,540.2 |   53.8% | Redis |
| 100 |      154,321.0 |      85,802.4 |   55.6% | Redis |

### Command: RPOP
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      127,551.0 |      70,408.0 |   55.2% | Redis |
| 50 |      140,845.1 |      76,197.0 |   54.1% | Redis |
| 100 |      152,439.0 |      82,012.1 |   53.8% | Redis |

### Command: HSET
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      148,368.0 |      66,172.1 |   44.6% | Redis |
| 50 |      149,253.7 |      66,268.6 |   44.4% | Redis |
| 100 |      155,763.2 |      52,093.4 |   45.0% | Redis |

### Command: SADD
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      145,772.6 |      71,865.8 |   49.3% | Redis |
| 50 |      150,150.1 |      75,225.2 |   50.1% | Redis |
| 100 |      155,279.5 |      79,037.2 |   50.9% | Redis |

### Command: ZADD
| Concurrency | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|------------:|-----------------:|--------------:|------:|--------|
| 10 |      148,809.5 |      58,035.7 |   39.0% | Redis |
| 50 |      149,700.6 |      58,532.9 |   39.1% | Redis |
| 100 |      155,279.5 |      59,361.7 |   38.2% | Redis |

## 2. Pipelined I/O Parsing (Concurrency = 50)
> Tests the raw socket ingestion and RESP parser efficiency.

### Command: SET
| Pipeline | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|---------:|-----------------:|--------------:|------:|--------|
| 1 |      151,515.1 |      106,060.5 |   70.1% | Redis |
| 16 |    1,449,275.4 |      169,565.5 |   11.7% | Redis |
| 64 |    2,273,454.5 |      195,517.0 |   8.6% | Redis |

### Command: GET
| Pipeline | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|---------:|-----------------:|--------------:|------:|--------|
| 1 |      149,700.6 |      109,880.2 |   73.4% | Redis |
| 16 |    1,785,714.2 |      230,357.1 |   12.9% | Redis |
| 64 |    2,703,567.5 |      213,581.8 |   7.9% | Redis |

### Command: HSET
| Pipeline | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|---------:|-----------------:|--------------:|------:|--------|
| 1 |      153,374.2 |      73,312.8 |   47.8% | Redis |
| 16 |    1,612,903.2 |      95,161.2 |   5.9% | Redis |
| 64 |    2,000,640.0 |      96,030.0 |   4.8% | Redis |

### Command: LPUSH
| Pipeline | Real Redis (rps) | MyRedis (rps) | Ratio | Winner |
|---------:|-----------------:|--------------:|------:|--------|
| 1 |      155,763.2 |      91,588.7 |   58.8% | Redis |
| 16 |    1,515,151.5 |      116,666.6 |   7.7% | Redis |
| 64 |    2,000,640.0 |      132,042.2 |   6.6% | Redis |

