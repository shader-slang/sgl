import pytest
from kali import Timer
import time


def test_now():
    delta_ns = 10000000
    t0 = Timer.now()
    time.sleep(delta_ns / 1000000000.0)
    t1 = Timer.now()
    assert t1 > t0 + delta_ns / 2


def test_delta():
    assert Timer.delta_ns(0, 1000000000) == 1000000000.0
    assert Timer.delta_us(0, 1000000000) == 1000000.0
    assert Timer.delta_ms(0, 1000000000) == 1000.0
    assert Timer.delta_s(0, 1000000000) == 1.0


def test_timer():
    delta_ns = 10000000
    delta_us = delta_ns / 1000.0
    delta_ms = delta_us / 1000.0
    delta_s = delta_ms / 1000.0
    timer = Timer()
    time.sleep(delta_ns / 1000000000.0)
    assert timer.elapsed_ns() > delta_ns / 2
    assert timer.elapsed_us() > delta_us / 2
    assert timer.elapsed_ms() > delta_ms / 2
    assert timer.elapsed_s() > delta_s / 2


if __name__ == "__main__":
    pytest.main([__file__])
