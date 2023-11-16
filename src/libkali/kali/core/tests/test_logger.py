import pytest
import os
import tempfile
import kali
from kali import Logger, LoggerOutput, ConsoleLoggerOutput, FileLoggerOutput, LogLevel


class CustomLoggerOutput(LoggerOutput):
    def __init__(self):
        super().__init__()
        self.clear()

    def clear(self):
        self.messages = []

    def write(self, level, module, msg):
        self.messages.append((level, module, msg))


def test_logger():
    kali.log_info("just an info")

    logger = Logger(level=LogLevel.info, module="test", use_default_outputs=False)
    # output = ConsoleLoggerOutput(colored=True)
    output = CustomLoggerOutput()
    print(output)
    logger.add_output(output)
    output.write(LogLevel.info, "test", "a message")

    assert logger.level == LogLevel.info

    pass


def _test_console_output():
    output = ConsoleLoggerOutput(colored=False)
    logger = Logger(level=LogLevel.debug, module="test", use_default_outputs=False)
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
        logger = Logger(level=LogLevel.debug, module="test", use_default_outputs=False)
        logger.add_output(output)
        logger.log(LogLevel.none, "plain message")
        logger.debug("debug message")
        logger.info("info message")
        logger.warn("warn message")
        logger.error("error message")
        logger.fatal("fatal message")
        del output
        del logger
        with open(path, "r") as f:
            lines = f.readlines()
            assert len(lines) == 6
            assert lines[0].startswith("plain message")
            assert lines[1].startswith("[DEBUG] (test) debug message")
            assert lines[2].startswith("[INFO] (test) info message")
            assert lines[3].startswith("[WARN] (test) warn message")
            assert lines[4].startswith("[ERROR] (test) error message")
            assert lines[5].startswith("[FATAL] (test) fatal message")


if __name__ == "__main__":
    pytest.main([__file__, "-s"])
