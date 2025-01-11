# Quick Base58 _(qbase58)_

<p align="center">

  <a href="https://github.com/qvecs/qbase58/actions?query=workflow%3ABuild">
    <img src="https://github.com/qvecs/qbase58/workflows/Build/badge.svg">
  </a>

  <a href="https://opensource.org/licenses/MIT">
    <img src="https://img.shields.io/badge/License-MIT-blue.svg">
  </a>
</p>

A quick base58 encoder/decoder implementation written in C, with Python bindings.

## Install

```
pip install qbase58
```

## Usage

```python
import qbase58

string = b"Hello, World!"

encoded = qbase58.encode(string)
print(f"Encoded: {encoded}")

decoded = qbase58.decode(encoded)
print(f"Decoded: {decoded}")
```

## Benchmark

Comparing with the two other Python base58 libraries:

* [base58](https://pypi.org/project/base58/)
* [based58](https://pypi.org/project/based58/)

```bash
--- Data Set: SMALL (size=100 bytes) ---
base58   => avg: 45.1845 ms (std: 2.7698 ms) over 1000 iterations
based58  => avg: 12.7324 ms (std: 0.1445 ms) over 1000 iterations
qbase58  => avg: 6.5189 ms (std: 0.0414 ms) over 1000 iterations

--- Data Set: MEDIUM (size=1000 bytes) ---
base58   => avg: 763.2404 ms (std: 3.9402 ms) over 500 iterations
based58  => avg: 646.2393 ms (std: 2.4316 ms) over 500 iterations
qbase58  => avg: 320.7714 ms (std: 1.1277 ms) over 500 iterations

--- Data Set: LARGE (size=5000 bytes) ---
base58   => avg: 319.6193 ms (std: 1.5274 ms) over 10 iterations
based58  => avg: 320.4021 ms (std: 1.2545 ms) over 10 iterations
qbase58  => avg: 162.0134 ms (std: 0.7824 ms) over 10 iterations
```

See `scripts/benchmark.py` for benchmarking details.