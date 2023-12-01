#pragma once

namespace kali {

// bitmap.h

class Bitmap;

// blob.h

class Blob;

// file_stream.h

class EOFException;
class FileStream;

// input.h

struct KeyboardEvent;
struct MouseEvent;
struct GamepadEvent;
struct GamepadState;

// logger.h

class LoggerOutput;
class ConsoleLoggerOutput;
class FileLoggerOutput;
class DebugConsoleLoggerOutput;
class Logger;

// memory_mapped_file_stream.h

class MemoryMappedFileStream;

// memory_mapped_file.h

class MemoryMappedFile;

// memory_stream.h

class MemoryStream;

// object.h

class Object;
template<typename>
class ref;
template<typename>
class breakable_ref;

// short_vector.h

template<typename T, size_t N>
class ShortVector;

// stream.h

class Stream;

// struct.h

class Struct;
class StructConverter;

// timer.h

class Timer;

// window.h

class Window;

} // namespace kali
