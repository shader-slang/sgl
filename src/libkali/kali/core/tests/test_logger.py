import pytest
import os
import tempfile
from kali import (
    Logger,
    LoggerOutput,
    ConsoleLoggerOutput,
    FileLoggerOutput,
    LogLevel,
    LogFrequency,
)


class CustomLoggerOutput(LoggerOutput):
    def __init__(self):
        super().__init__()
        self.clear()

    def clear(self):
        self.messages = []

    def write(self, level, name, msg):
        self.messages.append((level, name, msg))


def test_logger():
    logger = Logger(level=LogLevel.debug, name="test", use_default_outputs=False)
    output = CustomLoggerOutput()
    logger.add_output(output)

    messages = [
        (LogLevel.none, "plain text"),
        (LogLevel.debug, "debug message"),
        (LogLevel.info, "info message"),
        (LogLevel.warn, "warn message"),
        (LogLevel.error, "error message"),
        (LogLevel.fatal, "fatal message"),
    ]

    for level in [
        LogLevel.debug,
        LogLevel.info,
        LogLevel.warn,
        LogLevel.error,
        LogLevel.fatal,
    ]:
        logger.level = level
        logger.name = f"test_{level}"

        for msg in messages:
            output.clear()
            logger.log(msg[0], msg[1])
            if msg[0] == LogLevel.none or msg[0] >= level:
                assert len(output.messages) == 1
                assert output.messages[0][0] == msg[0]
                assert output.messages[0][1] == f"test_{level}"
                assert output.messages[0][2] == msg[1]
            else:
                assert len(output.messages) == 0


def test_logger_frequency():
    logger = Logger(level=LogLevel.info, name="test", use_default_outputs=False)
    output = CustomLoggerOutput()
    logger.add_output(output)

    logger.log(LogLevel.info, "repeated", LogFrequency.once)
    assert len(output.messages) == 1

    logger.log(LogLevel.info, "repeated", LogFrequency.once)
    assert len(output.messages) == 1

    logger.log(LogLevel.info, "repeated 2", LogFrequency.once)
    assert len(output.messages) == 2

    logger.log(LogLevel.info, "repeated 2", LogFrequency.once)
    assert len(output.messages) == 2


def _test_console_output():
    output = ConsoleLoggerOutput(colored=False)
    logger = Logger(level=LogLevel.debug, name="test", use_default_outputs=False)
    logger.add_output(output)
    logger.log(LogLevel.none, "plain message")
    logger.debug("debug message")
    logger.info("info message")
    logger.warn("warn message")
    logger.error("error message")
    logger.fatal("fatal message")


def test_console_output(capfd):
    _test_console_output()
    out, err = capfd.readouterr()
    # TODO: for some reason all output ends up in stdout or stderr although only error/fatal should be in stderr
    lines = err.splitlines() if len(err) > len(out) else out.splitlines()
    assert len(lines) == 6
    assert lines[0].startswith("plain message")
    assert lines[1].startswith("[DEBUG] (test) debug message")
    assert lines[2].startswith("[INFO] (test) info message")
    assert lines[3].startswith("[WARN] (test) warn message")
    assert lines[4].startswith("[ERROR] (test) error message")
    assert lines[5].startswith("[FATAL] (test) fatal message")


def test_file_output():
    with tempfile.TemporaryDirectory() as tmpdir:
        path = os.path.join(tmpdir, "test.log")
        output = FileLoggerOutput(path)
        logger = Logger(level=LogLevel.debug, name="test", use_default_outputs=False)
        logger.add_output(output)
        logger.log(LogLevel.none, "plain message")
        logger.debug("debug message")
        logger.info("info message")
        logger.warn("warn message")
        logger.error("error message")
        logger.fatal("fatal message")
        del output
        del logger
        lines = open(path, "r").readlines()
        assert len(lines) == 6
        assert lines[0].startswith("plain message")
        assert lines[1].startswith("[DEBUG] (test) debug message")
        assert lines[2].startswith("[INFO] (test) info message")
        assert lines[3].startswith("[WARN] (test) warn message")
        assert lines[4].startswith("[ERROR] (test) error message")
        assert lines[5].startswith("[FATAL] (test) fatal message")


if __name__ == "__main__":
    pytest.main([__file__, "-s"])
