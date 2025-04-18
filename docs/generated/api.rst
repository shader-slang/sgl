Core
----

.. py:class:: sgl.DataType

    Base class: :py:class:`enum.Enum`
    
    
    
    .. py:attribute:: sgl.DataType.void
        :type: DataType
        :value: DataType.void
    
    .. py:attribute:: sgl.DataType.bool
        :type: DataType
        :value: DataType.bool
    
    .. py:attribute:: sgl.DataType.int8
        :type: DataType
        :value: DataType.int8
    
    .. py:attribute:: sgl.DataType.int16
        :type: DataType
        :value: DataType.int16
    
    .. py:attribute:: sgl.DataType.int32
        :type: DataType
        :value: DataType.int32
    
    .. py:attribute:: sgl.DataType.int64
        :type: DataType
        :value: DataType.int64
    
    .. py:attribute:: sgl.DataType.uint8
        :type: DataType
        :value: DataType.uint8
    
    .. py:attribute:: sgl.DataType.uint16
        :type: DataType
        :value: DataType.uint16
    
    .. py:attribute:: sgl.DataType.uint32
        :type: DataType
        :value: DataType.uint32
    
    .. py:attribute:: sgl.DataType.uint64
        :type: DataType
        :value: DataType.uint64
    
    .. py:attribute:: sgl.DataType.float16
        :type: DataType
        :value: DataType.float16
    
    .. py:attribute:: sgl.DataType.float32
        :type: DataType
        :value: DataType.float32
    
    .. py:attribute:: sgl.DataType.float64
        :type: DataType
        :value: DataType.float64
    


----

.. py:class:: sgl.Object

    Base class for all reference counted objects.
    


----

.. py:class:: sgl.Bitmap

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:method:: __init__(self, pixel_format: sgl.Bitmap.PixelFormat, component_type: sgl.Struct.Type, width: int, height: int, channel_count: int = 0, channel_names: collections.abc.Sequence[str] = []) -> None
    
    .. py:method:: __init__(self, data: ndarray[device='cpu'], pixel_format: sgl.Bitmap.PixelFormat | None = None, channel_names: collections.abc.Sequence[str] | None = None) -> None
        :no-index:
    
    .. py:method:: __init__(self, path: str | os.PathLike) -> None
        :no-index:
    
    .. py:class:: sgl.Bitmap.PixelFormat
    
        Base class: :py:class:`enum.Enum`
        
        
        
        .. py:attribute:: sgl.Bitmap.PixelFormat.y
            :type: PixelFormat
            :value: PixelFormat.y
        
        .. py:attribute:: sgl.Bitmap.PixelFormat.ya
            :type: PixelFormat
            :value: PixelFormat.ya
        
        .. py:attribute:: sgl.Bitmap.PixelFormat.r
            :type: PixelFormat
            :value: PixelFormat.r
        
        .. py:attribute:: sgl.Bitmap.PixelFormat.rg
            :type: PixelFormat
            :value: PixelFormat.rg
        
        .. py:attribute:: sgl.Bitmap.PixelFormat.rgb
            :type: PixelFormat
            :value: PixelFormat.rgb
        
        .. py:attribute:: sgl.Bitmap.PixelFormat.rgba
            :type: PixelFormat
            :value: PixelFormat.rgba
        
        .. py:attribute:: sgl.Bitmap.PixelFormat.multi_channel
            :type: PixelFormat
            :value: PixelFormat.multi_channel
        
    .. py:class:: sgl.Bitmap.ComponentType
        :canonical: sgl.Struct.Type
        
        Alias class: :py:class:`sgl.Struct.Type`
        
    .. py:class:: sgl.Bitmap.FileFormat
    
        Base class: :py:class:`enum.Enum`
        
        
        
        .. py:attribute:: sgl.Bitmap.FileFormat.unknown
            :type: FileFormat
            :value: FileFormat.unknown
        
        .. py:attribute:: sgl.Bitmap.FileFormat.auto
            :type: FileFormat
            :value: FileFormat.auto
        
        .. py:attribute:: sgl.Bitmap.FileFormat.png
            :type: FileFormat
            :value: FileFormat.png
        
        .. py:attribute:: sgl.Bitmap.FileFormat.jpg
            :type: FileFormat
            :value: FileFormat.jpg
        
        .. py:attribute:: sgl.Bitmap.FileFormat.bmp
            :type: FileFormat
            :value: FileFormat.bmp
        
        .. py:attribute:: sgl.Bitmap.FileFormat.tga
            :type: FileFormat
            :value: FileFormat.tga
        
        .. py:attribute:: sgl.Bitmap.FileFormat.hdr
            :type: FileFormat
            :value: FileFormat.hdr
        
        .. py:attribute:: sgl.Bitmap.FileFormat.exr
            :type: FileFormat
            :value: FileFormat.exr
        
    .. py:property:: pixel_format
        :type: sgl.Bitmap.PixelFormat
    
        The pixel format.
        
    .. py:property:: component_type
        :type: sgl.Struct.Type
    
        The component type.
        
    .. py:property:: pixel_struct
        :type: sgl.Struct
    
        Struct describing the pixel layout.
        
    .. py:property:: width
        :type: int
    
        The width of the bitmap in pixels.
        
    .. py:property:: height
        :type: int
    
        The height of the bitmap in pixels.
        
    .. py:property:: pixel_count
        :type: int
    
        The total number of pixels in the bitmap.
        
    .. py:property:: channel_count
        :type: int
    
        The number of channels in the bitmap.
        
    .. py:property:: channel_names
        :type: list[str]
    
        The names of the channels in the bitmap.
        
    .. py:property:: srgb_gamma
        :type: bool
    
        True if the bitmap is in sRGB gamma space.
        
    .. py:method:: has_alpha(self) -> bool
    
        Returns true if the bitmap has an alpha channel.
        
    .. py:property:: bytes_per_pixel
        :type: int
    
        The number of bytes per pixel.
        
    .. py:property:: buffer_size
        :type: int
    
        The total size of the bitmap in bytes.
        
    .. py:method:: empty(self) -> bool
    
        True if bitmap is empty.
        
    .. py:method:: clear(self) -> None
    
        Clears the bitmap to zeros.
        
    .. py:method:: vflip(self) -> None
    
        Vertically flip the bitmap.
        
    .. py:method:: split(self) -> list[tuple[str, sgl.Bitmap]]
    
        Split bitmap into multiple bitmaps, each containing the channels with
        the same prefix.
        
        For example, if the bitmap has channels `albedo.R`, `albedo.G`,
        `albedo.B`, `normal.R`, `normal.G`, `normal.B`, this function will
        return two bitmaps, one containing the channels `albedo.R`,
        `albedo.G`, `albedo.B` and the other containing the channels
        `normal.R`, `normal.G`, `normal.B`.
        
        Common pixel formats (e.g. `y`, `rgb`, `rgba`) are automatically
        detected and used for the split bitmaps.
        
        Any channels that do not have a prefix will be returned in the bitmap
        with the empty prefix.
        
        Returns:
            Returns a list of (prefix, bitmap) pairs.
        
    .. py:method:: convert(self, pixel_format: sgl.Bitmap.PixelFormat | None = None, component_type: sgl.Struct.Type | None = None, srgb_gamma: bool | None = None) -> sgl.Bitmap
    
    .. py:method:: write(self, path: str | os.PathLike, format: sgl.Bitmap.FileFormat = FileFormat.auto, quality: int = -1) -> None
    
    .. py:method:: write_async(self, path: str | os.PathLike, format: sgl.Bitmap.FileFormat = FileFormat.auto, quality: int = -1) -> None
    
    .. py:staticmethod:: read_multiple(paths: Sequence[str | os.PathLike], format: sgl.Bitmap.FileFormat = FileFormat.auto) -> list[sgl.Bitmap]
    
        Load a list of bitmaps from multiple paths. Uses multi-threading to
        load bitmaps in parallel.
        


----

.. py:class:: sgl.Struct

    Base class: :py:class:`sgl.Object`
    
    Structured data definition.
    
    This class is used to describe a structured data type layout. It is
    used by the StructConverter class to convert between different
    layouts.
    
    .. py:method:: __init__(self, pack: bool = False, byte_order: sgl.Struct.ByteOrder = ByteOrder.host) -> None
    
        Constructor.
        
        Parameter ``pack``:
            If true, the struct will be packed.
        
        Parameter ``byte_order``:
            Byte order of the struct.
        
    .. py:class:: sgl.Struct.Type
    
        Base class: :py:class:`enum.Enum`
        
        Struct field type.
        
        .. py:attribute:: sgl.Struct.Type.int8
            :type: Type
            :value: Type.int8
        
        .. py:attribute:: sgl.Struct.Type.int16
            :type: Type
            :value: Type.int16
        
        .. py:attribute:: sgl.Struct.Type.int32
            :type: Type
            :value: Type.int32
        
        .. py:attribute:: sgl.Struct.Type.int64
            :type: Type
            :value: Type.int64
        
        .. py:attribute:: sgl.Struct.Type.uint8
            :type: Type
            :value: Type.uint8
        
        .. py:attribute:: sgl.Struct.Type.uint16
            :type: Type
            :value: Type.uint16
        
        .. py:attribute:: sgl.Struct.Type.uint32
            :type: Type
            :value: Type.uint32
        
        .. py:attribute:: sgl.Struct.Type.uint64
            :type: Type
            :value: Type.uint64
        
        .. py:attribute:: sgl.Struct.Type.float16
            :type: Type
            :value: Type.float16
        
        .. py:attribute:: sgl.Struct.Type.float32
            :type: Type
            :value: Type.float32
        
        .. py:attribute:: sgl.Struct.Type.float64
            :type: Type
            :value: Type.float64
        
    .. py:class:: sgl.Struct.Flags
    
        Base class: :py:class:`enum.IntFlag`
        
        Struct field flags.
        
        .. py:attribute:: sgl.Struct.Flags.none
            :type: Flags
            :value: Flags.none
        
        .. py:attribute:: sgl.Struct.Flags.normalized
            :type: Flags
            :value: Flags.normalized
        
        .. py:attribute:: sgl.Struct.Flags.srgb_gamma
            :type: Flags
            :value: Flags.srgb_gamma
        
        .. py:attribute:: sgl.Struct.Flags.default
            :type: Flags
            :value: Flags.default
        
    .. py:class:: sgl.Struct.ByteOrder
    
        Base class: :py:class:`enum.Enum`
        
        Byte order.
        
        .. py:attribute:: sgl.Struct.ByteOrder.little_endian
            :type: ByteOrder
            :value: ByteOrder.little_endian
        
        .. py:attribute:: sgl.Struct.ByteOrder.big_endian
            :type: ByteOrder
            :value: ByteOrder.big_endian
        
        .. py:attribute:: sgl.Struct.ByteOrder.host
            :type: ByteOrder
            :value: ByteOrder.host
        
    .. py:class:: sgl.Struct.Field
    
        Struct field.
        
        .. py:property:: name
            :type: str
        
            Name of the field.
            
        .. py:property:: type
            :type: sgl.Struct.Type
        
            Type of the field.
            
        .. py:property:: flags
            :type: sgl.Struct.Flags
        
            Field flags.
            
        .. py:property:: size
            :type: int
        
            Size of the field in bytes.
            
        .. py:property:: offset
            :type: int
        
            Offset of the field in bytes.
            
        .. py:property:: default_value
            :type: float
        
            Default value.
            
        .. py:method:: is_integer(self) -> bool
        
            Check if the field is an integer type.
            
        .. py:method:: is_unsigned(self) -> bool
        
            Check if the field is an unsigned type.
            
        .. py:method:: is_signed(self) -> bool
        
            Check if the field is a signed type.
            
        .. py:method:: is_float(self) -> bool
        
            Check if the field is a floating point type.
            
    .. py:method:: append(self, field: sgl.Struct.Field) -> sgl.Struct
    
        Append a field to the struct.
        
    .. py:method:: append(self, name: str, type: sgl.Struct.Type, flags: sgl.Struct.Flags = Flags.none, default_value: float = 0.0, blend: collections.abc.Sequence[tuple[float, str]] = []) -> sgl.Struct
        :no-index:
    
        Append a field to the struct.
        
        Parameter ``name``:
            Name of the field.
        
        Parameter ``type``:
            Type of the field.
        
        Parameter ``flags``:
            Field flags.
        
        Parameter ``default_value``:
            Default value.
        
        Parameter ``blend``:
            List of blend weights/names.
        
        Returns:
            Reference to the struct.
        
    .. py:method:: has_field(self, name: str) -> bool
    
        Check if a field with the specified name exists.
        
    .. py:method:: field(self, name: str) -> sgl.Struct.Field
    
        Access field by name. Throws if field is not found.
        
    .. py:property:: size
        :type: int
    
        The size of the struct in bytes (with padding).
        
    .. py:property:: alignment
        :type: int
    
        The alignment of the struct in bytes.
        
    .. py:property:: byte_order
        :type: sgl.Struct.ByteOrder
    
        The byte order of the struct.
        
    .. py:staticmethod:: type_size(arg: sgl.Struct.Type, /) -> int
    
        Get the size of a type in bytes.
        
    .. py:staticmethod:: type_range(arg: sgl.Struct.Type, /) -> tuple[float, float]
    
        Get the numeric range of a type.
        
    .. py:staticmethod:: is_integer(arg: sgl.Struct.Type, /) -> bool
    
        Check if ``type`` is an integer type.
        
    .. py:staticmethod:: is_unsigned(arg: sgl.Struct.Type, /) -> bool
    
        Check if ``type`` is an unsigned type.
        
    .. py:staticmethod:: is_signed(arg: sgl.Struct.Type, /) -> bool
    
        Check if ``type`` is a signed type.
        
    .. py:staticmethod:: is_float(arg: sgl.Struct.Type, /) -> bool
    
        Check if ``type`` is a floating point type.
        


----

.. py:class:: sgl.StructConverter

    Base class: :py:class:`sgl.Object`
    
    Struct converter.
    
    This helper class can be used to convert between structs with
    different layouts.
    
    .. py:method:: __init__(self, src: sgl.Struct, dst: sgl.Struct) -> None
    
        Constructor.
        
        Parameter ``src``:
            Source struct definition.
        
        Parameter ``dst``:
            Destination struct definition.
        
    .. py:property:: src
        :type: sgl.Struct
    
        The source struct definition.
        
    .. py:property:: dst
        :type: sgl.Struct
    
        The destination struct definition.
        
    .. py:method:: convert(self, input: bytes) -> bytes
    


----

.. py:class:: sgl.Timer

    High resolution CPU timer.
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: reset(self) -> None
    
        Reset the timer.
        
    .. py:method:: elapsed_s(self) -> float
    
        Elapsed seconds since last reset.
        
    .. py:method:: elapsed_ms(self) -> float
    
        Elapsed milliseconds since last reset.
        
    .. py:method:: elapsed_us(self) -> float
    
        Elapsed microseconds since last reset.
        
    .. py:method:: elapsed_ns(self) -> float
    
        Elapsed nanoseconds since last reset.
        
    .. py:staticmethod:: delta_s(start: int, end: int) -> float
    
        Compute elapsed seconds between two time points.
        
    .. py:staticmethod:: delta_ms(start: int, end: int) -> float
    
        Compute elapsed milliseconds between two time points.
        
    .. py:staticmethod:: delta_us(start: int, end: int) -> float
    
        Compute elapsed microseconds between two time points.
        
    .. py:staticmethod:: delta_ns(start: int, end: int) -> float
    
        Compute elapsed nanoseconds between two time points.
        
    .. py:staticmethod:: now() -> int
    
        Current time point in nanoseconds since epoch.
        


----

.. py:class:: sgl.SHA1

    Helper to compute SHA-1 hash.
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, data: bytes) -> None
        :no-index:
    
    .. py:method:: __init__(self, str: str) -> None
        :no-index:
    
    .. py:method:: update(self, data: bytes) -> sgl.SHA1
    
        Update hash by adding the given data.
        
        Parameter ``data``:
            Data to hash.
        
        Parameter ``len``:
            Length of data in bytes.
        
    .. py:method:: update(self, str: str) -> sgl.SHA1
        :no-index:
    
        Update hash by adding the given string.
        
        Parameter ``str``:
            String to hash.
        
    .. py:method:: digest(self) -> bytes
    
        Return the message digest.
        
    .. py:method:: hex_digest(self) -> str
    
        Return the message digest as a hex string.
        


----

Constants
---------

.. py:data:: sgl.SGL_VERSION
    :type: str
    :value: "0.15.0"



----

.. py:data:: sgl.SGL_VERSION_MAJOR
    :type: int
    :value: 0



----

.. py:data:: sgl.SGL_VERSION_MINOR
    :type: int
    :value: 15



----

.. py:data:: sgl.SGL_VERSION_PATCH
    :type: int
    :value: 0



----

.. py:data:: sgl.SGL_GIT_VERSION
    :type: str
    :value: "commit: 19981de / branch: fix-doc (local changes)"



----

Logging
-------

.. py:class:: sgl.LogLevel

    Base class: :py:class:`enum.IntEnum`
    
    Log level.
    
    .. py:attribute:: sgl.LogLevel.none
        :type: LogLevel
        :value: LogLevel.none
    
    .. py:attribute:: sgl.LogLevel.debug
        :type: LogLevel
        :value: LogLevel.debug
    
    .. py:attribute:: sgl.LogLevel.info
        :type: LogLevel
        :value: LogLevel.info
    
    .. py:attribute:: sgl.LogLevel.warn
        :type: LogLevel
        :value: LogLevel.warn
    
    .. py:attribute:: sgl.LogLevel.error
        :type: LogLevel
        :value: LogLevel.error
    
    .. py:attribute:: sgl.LogLevel.fatal
        :type: LogLevel
        :value: LogLevel.fatal
    


----

.. py:class:: sgl.LogFrequency

    Base class: :py:class:`enum.Enum`
    
    Log frequency.
    
    .. py:attribute:: sgl.LogFrequency.always
        :type: LogFrequency
        :value: LogFrequency.always
    
    .. py:attribute:: sgl.LogFrequency.once
        :type: LogFrequency
        :value: LogFrequency.once
    


----

.. py:class:: sgl.Logger

    
    
    .. py:method:: __init__(self, level: sgl.LogLevel = LogLevel.info, name: str = '', use_default_outputs: bool = True) -> None
    
        Constructor.
        
        Parameter ``level``:
            The log level to use (messages with level >= this will be logged).
        
        Parameter ``name``:
            The name of the logger.
        
        Parameter ``use_default_outputs``:
            Whether to use the default outputs (console + debug console on
            windows).
        
    .. py:property:: level
        :type: sgl.LogLevel
    
        The log level.
        
    .. py:property:: name
        :type: str
    
        The name of the logger.
        
    .. py:method:: add_console_output(self, colored: bool = True) -> sgl.LoggerOutput
    
        Add a console logger output.
        
        Parameter ``colored``:
            Whether to use colored output.
        
        Returns:
            The created logger output.
        
    .. py:method:: add_file_output(self, path: str | os.PathLike) -> sgl.LoggerOutput
    
        Add a file logger output.
        
        Parameter ``path``:
            The path to the log file.
        
        Returns:
            The created logger output.
        
    .. py:method:: add_debug_console_output(self) -> sgl.LoggerOutput
    
        Add a debug console logger output (Windows only).
        
        Returns:
            The created logger output.
        
    .. py:method:: add_output(self, output: sgl.LoggerOutput) -> None
    
        Add a logger output.
        
        Parameter ``output``:
            The logger output to add.
        
    .. py:method:: use_same_outputs(self, other: sgl.Logger) -> None
    
        Use the same outputs as the given logger.
        
        Parameter ``other``:
            Logger to copy outputs from.
        
    .. py:method:: remove_output(self, output: sgl.LoggerOutput) -> None
    
        Remove a logger output.
        
        Parameter ``output``:
            The logger output to remove.
        
    .. py:method:: remove_all_outputs(self) -> None
    
        Remove all logger outputs.
        
    .. py:method:: log(self, level: sgl.LogLevel, msg: str, frequency: sgl.LogFrequency = LogFrequency.always) -> None
    
        Log a message.
        
        Parameter ``level``:
            The log level.
        
        Parameter ``msg``:
            The message.
        
        Parameter ``frequency``:
            The log frequency.
        
    .. py:method:: debug(self, msg: str) -> None
    
    .. py:method:: info(self, msg: str) -> None
    
    .. py:method:: warn(self, msg: str) -> None
    
    .. py:method:: error(self, msg: str) -> None
    
    .. py:method:: fatal(self, msg: str) -> None
    
    .. py:method:: debug_once(self, msg: str) -> None
    
    .. py:method:: info_once(self, msg: str) -> None
    
    .. py:method:: warn_once(self, msg: str) -> None
    
    .. py:method:: error_once(self, msg: str) -> None
    
    .. py:method:: fatal_once(self, msg: str) -> None
    
    .. py:staticmethod:: get() -> sgl.Logger
    
        Returns the global logger instance.
        


----

.. py:class:: sgl.LoggerOutput

    Base class: :py:class:`sgl.Object`
    
    Abstract base class for logger outputs.
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: write(self, level: sgl.LogLevel, name: str, msg: str) -> None
    
        Write a log message.
        
        Parameter ``level``:
            The log level.
        
        Parameter ``module``:
            The module name.
        
        Parameter ``msg``:
            The message.
        


----

.. py:class:: sgl.ConsoleLoggerOutput

    Base class: :py:class:`sgl.LoggerOutput`
    
    Logger output that writes to the console. Error messages are printed
    to stderr, all other messages to stdout. Messages are optionally
    colored.
    
    .. py:method:: __init__(self, colored: bool = True) -> None
    


----

.. py:class:: sgl.FileLoggerOutput

    Base class: :py:class:`sgl.LoggerOutput`
    
    Logger output that writes to a file.
    
    .. py:method:: __init__(self, path: str | os.PathLike) -> None
    


----

.. py:class:: sgl.DebugConsoleLoggerOutput

    Base class: :py:class:`sgl.LoggerOutput`
    
    Logger output that writes to the debug console (Windows only).
    
    .. py:method:: __init__(self) -> None
    


----

.. py:function:: sgl.log(level: sgl.LogLevel, msg: str, frequency: sgl.LogFrequency = LogFrequency.always) -> None

    Log a message.
    
    Parameter ``level``:
        The log level.
    
    Parameter ``msg``:
        The message.
    
    Parameter ``frequency``:
        The log frequency.
    


----

.. py:function:: sgl.log_debug(msg: str) -> None



----

.. py:function:: sgl.log_debug_once(msg: str) -> None



----

.. py:function:: sgl.log_info(msg: str) -> None



----

.. py:function:: sgl.log_info_once(msg: str) -> None



----

.. py:function:: sgl.log_warn(msg: str) -> None



----

.. py:function:: sgl.log_warn_once(msg: str) -> None



----

.. py:function:: sgl.log_error(msg: str) -> None



----

.. py:function:: sgl.log_error_once(msg: str) -> None



----

.. py:function:: sgl.log_fatal(msg: str) -> None



----

.. py:function:: sgl.log_fatal_once(msg: str) -> None



----

Windowing
---------

.. py:class:: sgl.CursorMode

    Base class: :py:class:`enum.Enum`
    
    Mouse cursor modes.
    
    .. py:attribute:: sgl.CursorMode.normal
        :type: CursorMode
        :value: CursorMode.normal
    
    .. py:attribute:: sgl.CursorMode.hidden
        :type: CursorMode
        :value: CursorMode.hidden
    
    .. py:attribute:: sgl.CursorMode.disabled
        :type: CursorMode
        :value: CursorMode.disabled
    


----

.. py:class:: sgl.WindowMode

    Base class: :py:class:`enum.Enum`
    
    Window modes.
    
    .. py:attribute:: sgl.WindowMode.normal
        :type: WindowMode
        :value: WindowMode.normal
    
    .. py:attribute:: sgl.WindowMode.minimized
        :type: WindowMode
        :value: WindowMode.minimized
    
    .. py:attribute:: sgl.WindowMode.fullscreen
        :type: WindowMode
        :value: WindowMode.fullscreen
    


----

.. py:class:: sgl.Window

    Base class: :py:class:`sgl.Object`
    
    Window class.
    
    Platform independent class for managing a window and handle input
    events.
    
    .. py:method:: __init__(self, width: int = 1024, height: int = 1024, title: str = 'sgl', mode: sgl.WindowMode = WindowMode.normal, resizable: bool = True) -> None
    
        Constructor.
        
        Parameter ``width``:
            Width of the window in pixels.
        
        Parameter ``height``:
            Height of the window in pixels.
        
        Parameter ``title``:
            Title of the window.
        
        Parameter ``mode``:
            Window mode.
        
        Parameter ``resizable``:
            Whether the window is resizable.
        
    .. py:property:: width
        :type: int
    
        The width of the window in pixels.
        
    .. py:property:: height
        :type: int
    
        The height of the window in pixels.
        
    .. py:method:: resize(self, width: int, height: int) -> None
    
        Resize the window.
        
        Parameter ``width``:
            The new width of the window in pixels.
        
        Parameter ``height``:
            The new height of the window in pixels.
        
    .. py:property:: title
        :type: str
    
        The title of the window.
        
    .. py:method:: close(self) -> None
    
        Close the window.
        
    .. py:method:: should_close(self) -> bool
    
        True if the window should be closed.
        
    .. py:method:: process_events(self) -> None
    
        Process any pending events.
        
    .. py:method:: set_clipboard(self, text: str) -> None
    
        Set the clipboard content.
        
    .. py:method:: get_clipboard(self) -> str | None
    
        Get the clipboard content.
        
    .. py:property:: cursor_mode
        :type: sgl.CursorMode
    
        The mouse cursor mode.
        
    .. py:property:: on_resize
        :type: collections.abc.Callable[[int, int], None]
    
        Event handler to be called when the window is resized.
        
    .. py:property:: on_keyboard_event
        :type: collections.abc.Callable[[sgl.KeyboardEvent], None]
    
        Event handler to be called when a keyboard event occurs.
        
    .. py:property:: on_mouse_event
        :type: collections.abc.Callable[[sgl.MouseEvent], None]
    
        Event handler to be called when a mouse event occurs.
        
    .. py:property:: on_gamepad_event
        :type: collections.abc.Callable[[sgl.GamepadEvent], None]
    
        Event handler to be called when a gamepad event occurs.
        
    .. py:property:: on_gamepad_state
        :type: collections.abc.Callable[[sgl.GamepadState], None]
    
        Event handler to be called when the gamepad state changes.
        
    .. py:property:: on_drop_files
        :type: collections.abc.Callable[[list[str]], None]
    
        Event handler to be called when files are dropped onto the window.
        


----

.. py:class:: sgl.MouseButton

    Base class: :py:class:`enum.Enum`
    
    Mouse buttons.
    
    .. py:attribute:: sgl.MouseButton.left
        :type: MouseButton
        :value: MouseButton.left
    
    .. py:attribute:: sgl.MouseButton.middle
        :type: MouseButton
        :value: MouseButton.middle
    
    .. py:attribute:: sgl.MouseButton.right
        :type: MouseButton
        :value: MouseButton.right
    
    .. py:attribute:: sgl.MouseButton.unknown
        :type: MouseButton
        :value: MouseButton.unknown
    


----

.. py:class:: sgl.KeyModifierFlags

    Base class: :py:class:`enum.Enum`
    
    Keyboard modifier flags.
    
    .. py:attribute:: sgl.KeyModifierFlags.none
        :type: KeyModifierFlags
        :value: KeyModifierFlags.none
    
    .. py:attribute:: sgl.KeyModifierFlags.shift
        :type: KeyModifierFlags
        :value: KeyModifierFlags.shift
    
    .. py:attribute:: sgl.KeyModifierFlags.ctrl
        :type: KeyModifierFlags
        :value: KeyModifierFlags.ctrl
    
    .. py:attribute:: sgl.KeyModifierFlags.alt
        :type: KeyModifierFlags
        :value: KeyModifierFlags.alt
    


----

.. py:class:: sgl.KeyModifier

    Base class: :py:class:`enum.Enum`
    
    Keyboard modifiers.
    
    .. py:attribute:: sgl.KeyModifier.shift
        :type: KeyModifier
        :value: KeyModifier.shift
    
    .. py:attribute:: sgl.KeyModifier.ctrl
        :type: KeyModifier
        :value: KeyModifier.ctrl
    
    .. py:attribute:: sgl.KeyModifier.alt
        :type: KeyModifier
        :value: KeyModifier.alt
    


----

.. py:class:: sgl.KeyCode

    Base class: :py:class:`enum.Enum`
    
    Keyboard key codes.
    
    .. py:attribute:: sgl.KeyCode.space
        :type: KeyCode
        :value: KeyCode.space
    
    .. py:attribute:: sgl.KeyCode.apostrophe
        :type: KeyCode
        :value: KeyCode.apostrophe
    
    .. py:attribute:: sgl.KeyCode.comma
        :type: KeyCode
        :value: KeyCode.comma
    
    .. py:attribute:: sgl.KeyCode.minus
        :type: KeyCode
        :value: KeyCode.minus
    
    .. py:attribute:: sgl.KeyCode.period
        :type: KeyCode
        :value: KeyCode.period
    
    .. py:attribute:: sgl.KeyCode.slash
        :type: KeyCode
        :value: KeyCode.slash
    
    .. py:attribute:: sgl.KeyCode.key0
        :type: KeyCode
        :value: KeyCode.key0
    
    .. py:attribute:: sgl.KeyCode.key1
        :type: KeyCode
        :value: KeyCode.key1
    
    .. py:attribute:: sgl.KeyCode.key2
        :type: KeyCode
        :value: KeyCode.key2
    
    .. py:attribute:: sgl.KeyCode.key3
        :type: KeyCode
        :value: KeyCode.key3
    
    .. py:attribute:: sgl.KeyCode.key4
        :type: KeyCode
        :value: KeyCode.key4
    
    .. py:attribute:: sgl.KeyCode.key5
        :type: KeyCode
        :value: KeyCode.key5
    
    .. py:attribute:: sgl.KeyCode.key6
        :type: KeyCode
        :value: KeyCode.key6
    
    .. py:attribute:: sgl.KeyCode.key7
        :type: KeyCode
        :value: KeyCode.key7
    
    .. py:attribute:: sgl.KeyCode.key8
        :type: KeyCode
        :value: KeyCode.key8
    
    .. py:attribute:: sgl.KeyCode.key9
        :type: KeyCode
        :value: KeyCode.key9
    
    .. py:attribute:: sgl.KeyCode.semicolon
        :type: KeyCode
        :value: KeyCode.semicolon
    
    .. py:attribute:: sgl.KeyCode.equal
        :type: KeyCode
        :value: KeyCode.equal
    
    .. py:attribute:: sgl.KeyCode.a
        :type: KeyCode
        :value: KeyCode.a
    
    .. py:attribute:: sgl.KeyCode.b
        :type: KeyCode
        :value: KeyCode.b
    
    .. py:attribute:: sgl.KeyCode.c
        :type: KeyCode
        :value: KeyCode.c
    
    .. py:attribute:: sgl.KeyCode.d
        :type: KeyCode
        :value: KeyCode.d
    
    .. py:attribute:: sgl.KeyCode.e
        :type: KeyCode
        :value: KeyCode.e
    
    .. py:attribute:: sgl.KeyCode.f
        :type: KeyCode
        :value: KeyCode.f
    
    .. py:attribute:: sgl.KeyCode.g
        :type: KeyCode
        :value: KeyCode.g
    
    .. py:attribute:: sgl.KeyCode.h
        :type: KeyCode
        :value: KeyCode.h
    
    .. py:attribute:: sgl.KeyCode.i
        :type: KeyCode
        :value: KeyCode.i
    
    .. py:attribute:: sgl.KeyCode.j
        :type: KeyCode
        :value: KeyCode.j
    
    .. py:attribute:: sgl.KeyCode.k
        :type: KeyCode
        :value: KeyCode.k
    
    .. py:attribute:: sgl.KeyCode.l
        :type: KeyCode
        :value: KeyCode.l
    
    .. py:attribute:: sgl.KeyCode.m
        :type: KeyCode
        :value: KeyCode.m
    
    .. py:attribute:: sgl.KeyCode.n
        :type: KeyCode
        :value: KeyCode.n
    
    .. py:attribute:: sgl.KeyCode.o
        :type: KeyCode
        :value: KeyCode.o
    
    .. py:attribute:: sgl.KeyCode.p
        :type: KeyCode
        :value: KeyCode.p
    
    .. py:attribute:: sgl.KeyCode.q
        :type: KeyCode
        :value: KeyCode.q
    
    .. py:attribute:: sgl.KeyCode.r
        :type: KeyCode
        :value: KeyCode.r
    
    .. py:attribute:: sgl.KeyCode.s
        :type: KeyCode
        :value: KeyCode.s
    
    .. py:attribute:: sgl.KeyCode.t
        :type: KeyCode
        :value: KeyCode.t
    
    .. py:attribute:: sgl.KeyCode.u
        :type: KeyCode
        :value: KeyCode.u
    
    .. py:attribute:: sgl.KeyCode.v
        :type: KeyCode
        :value: KeyCode.v
    
    .. py:attribute:: sgl.KeyCode.w
        :type: KeyCode
        :value: KeyCode.w
    
    .. py:attribute:: sgl.KeyCode.x
        :type: KeyCode
        :value: KeyCode.x
    
    .. py:attribute:: sgl.KeyCode.y
        :type: KeyCode
        :value: KeyCode.y
    
    .. py:attribute:: sgl.KeyCode.z
        :type: KeyCode
        :value: KeyCode.z
    
    .. py:attribute:: sgl.KeyCode.left_bracket
        :type: KeyCode
        :value: KeyCode.left_bracket
    
    .. py:attribute:: sgl.KeyCode.backslash
        :type: KeyCode
        :value: KeyCode.backslash
    
    .. py:attribute:: sgl.KeyCode.right_bracket
        :type: KeyCode
        :value: KeyCode.right_bracket
    
    .. py:attribute:: sgl.KeyCode.grave_accent
        :type: KeyCode
        :value: KeyCode.grave_accent
    
    .. py:attribute:: sgl.KeyCode.escape
        :type: KeyCode
        :value: KeyCode.escape
    
    .. py:attribute:: sgl.KeyCode.tab
        :type: KeyCode
        :value: KeyCode.tab
    
    .. py:attribute:: sgl.KeyCode.enter
        :type: KeyCode
        :value: KeyCode.enter
    
    .. py:attribute:: sgl.KeyCode.backspace
        :type: KeyCode
        :value: KeyCode.backspace
    
    .. py:attribute:: sgl.KeyCode.insert
        :type: KeyCode
        :value: KeyCode.insert
    
    .. py:attribute:: sgl.KeyCode.delete
        :type: KeyCode
        :value: KeyCode.delete
    
    .. py:attribute:: sgl.KeyCode.right
        :type: KeyCode
        :value: KeyCode.right
    
    .. py:attribute:: sgl.KeyCode.left
        :type: KeyCode
        :value: KeyCode.left
    
    .. py:attribute:: sgl.KeyCode.down
        :type: KeyCode
        :value: KeyCode.down
    
    .. py:attribute:: sgl.KeyCode.up
        :type: KeyCode
        :value: KeyCode.up
    
    .. py:attribute:: sgl.KeyCode.page_up
        :type: KeyCode
        :value: KeyCode.page_up
    
    .. py:attribute:: sgl.KeyCode.page_down
        :type: KeyCode
        :value: KeyCode.page_down
    
    .. py:attribute:: sgl.KeyCode.home
        :type: KeyCode
        :value: KeyCode.home
    
    .. py:attribute:: sgl.KeyCode.end
        :type: KeyCode
        :value: KeyCode.end
    
    .. py:attribute:: sgl.KeyCode.caps_lock
        :type: KeyCode
        :value: KeyCode.caps_lock
    
    .. py:attribute:: sgl.KeyCode.scroll_lock
        :type: KeyCode
        :value: KeyCode.scroll_lock
    
    .. py:attribute:: sgl.KeyCode.num_lock
        :type: KeyCode
        :value: KeyCode.num_lock
    
    .. py:attribute:: sgl.KeyCode.print_screen
        :type: KeyCode
        :value: KeyCode.print_screen
    
    .. py:attribute:: sgl.KeyCode.pause
        :type: KeyCode
        :value: KeyCode.pause
    
    .. py:attribute:: sgl.KeyCode.f1
        :type: KeyCode
        :value: KeyCode.f1
    
    .. py:attribute:: sgl.KeyCode.f2
        :type: KeyCode
        :value: KeyCode.f2
    
    .. py:attribute:: sgl.KeyCode.f3
        :type: KeyCode
        :value: KeyCode.f3
    
    .. py:attribute:: sgl.KeyCode.f4
        :type: KeyCode
        :value: KeyCode.f4
    
    .. py:attribute:: sgl.KeyCode.f5
        :type: KeyCode
        :value: KeyCode.f5
    
    .. py:attribute:: sgl.KeyCode.f6
        :type: KeyCode
        :value: KeyCode.f6
    
    .. py:attribute:: sgl.KeyCode.f7
        :type: KeyCode
        :value: KeyCode.f7
    
    .. py:attribute:: sgl.KeyCode.f8
        :type: KeyCode
        :value: KeyCode.f8
    
    .. py:attribute:: sgl.KeyCode.f9
        :type: KeyCode
        :value: KeyCode.f9
    
    .. py:attribute:: sgl.KeyCode.f10
        :type: KeyCode
        :value: KeyCode.f10
    
    .. py:attribute:: sgl.KeyCode.f11
        :type: KeyCode
        :value: KeyCode.f11
    
    .. py:attribute:: sgl.KeyCode.f12
        :type: KeyCode
        :value: KeyCode.f12
    
    .. py:attribute:: sgl.KeyCode.keypad0
        :type: KeyCode
        :value: KeyCode.keypad0
    
    .. py:attribute:: sgl.KeyCode.keypad1
        :type: KeyCode
        :value: KeyCode.keypad1
    
    .. py:attribute:: sgl.KeyCode.keypad2
        :type: KeyCode
        :value: KeyCode.keypad2
    
    .. py:attribute:: sgl.KeyCode.keypad3
        :type: KeyCode
        :value: KeyCode.keypad3
    
    .. py:attribute:: sgl.KeyCode.keypad4
        :type: KeyCode
        :value: KeyCode.keypad4
    
    .. py:attribute:: sgl.KeyCode.keypad5
        :type: KeyCode
        :value: KeyCode.keypad5
    
    .. py:attribute:: sgl.KeyCode.keypad6
        :type: KeyCode
        :value: KeyCode.keypad6
    
    .. py:attribute:: sgl.KeyCode.keypad7
        :type: KeyCode
        :value: KeyCode.keypad7
    
    .. py:attribute:: sgl.KeyCode.keypad8
        :type: KeyCode
        :value: KeyCode.keypad8
    
    .. py:attribute:: sgl.KeyCode.keypad9
        :type: KeyCode
        :value: KeyCode.keypad9
    
    .. py:attribute:: sgl.KeyCode.keypad_del
        :type: KeyCode
        :value: KeyCode.keypad_del
    
    .. py:attribute:: sgl.KeyCode.keypad_divide
        :type: KeyCode
        :value: KeyCode.keypad_divide
    
    .. py:attribute:: sgl.KeyCode.keypad_multiply
        :type: KeyCode
        :value: KeyCode.keypad_multiply
    
    .. py:attribute:: sgl.KeyCode.keypad_subtract
        :type: KeyCode
        :value: KeyCode.keypad_subtract
    
    .. py:attribute:: sgl.KeyCode.keypad_add
        :type: KeyCode
        :value: KeyCode.keypad_add
    
    .. py:attribute:: sgl.KeyCode.keypad_enter
        :type: KeyCode
        :value: KeyCode.keypad_enter
    
    .. py:attribute:: sgl.KeyCode.keypad_equal
        :type: KeyCode
        :value: KeyCode.keypad_equal
    
    .. py:attribute:: sgl.KeyCode.left_shift
        :type: KeyCode
        :value: KeyCode.left_shift
    
    .. py:attribute:: sgl.KeyCode.left_control
        :type: KeyCode
        :value: KeyCode.left_control
    
    .. py:attribute:: sgl.KeyCode.left_alt
        :type: KeyCode
        :value: KeyCode.left_alt
    
    .. py:attribute:: sgl.KeyCode.left_super
        :type: KeyCode
        :value: KeyCode.left_super
    
    .. py:attribute:: sgl.KeyCode.right_shift
        :type: KeyCode
        :value: KeyCode.right_shift
    
    .. py:attribute:: sgl.KeyCode.right_control
        :type: KeyCode
        :value: KeyCode.right_control
    
    .. py:attribute:: sgl.KeyCode.right_alt
        :type: KeyCode
        :value: KeyCode.right_alt
    
    .. py:attribute:: sgl.KeyCode.right_super
        :type: KeyCode
        :value: KeyCode.right_super
    
    .. py:attribute:: sgl.KeyCode.menu
        :type: KeyCode
        :value: KeyCode.menu
    
    .. py:attribute:: sgl.KeyCode.unknown
        :type: KeyCode
        :value: KeyCode.unknown
    


----

.. py:class:: sgl.KeyboardEventType

    Base class: :py:class:`enum.Enum`
    
    Keyboard event types.
    
    .. py:attribute:: sgl.KeyboardEventType.key_press
        :type: KeyboardEventType
        :value: KeyboardEventType.key_press
    
    .. py:attribute:: sgl.KeyboardEventType.key_release
        :type: KeyboardEventType
        :value: KeyboardEventType.key_release
    
    .. py:attribute:: sgl.KeyboardEventType.key_repeat
        :type: KeyboardEventType
        :value: KeyboardEventType.key_repeat
    
    .. py:attribute:: sgl.KeyboardEventType.input
        :type: KeyboardEventType
        :value: KeyboardEventType.input
    


----

.. py:class:: sgl.KeyboardEvent

    Keyboard event.
    
    .. py:property:: type
        :type: sgl.KeyboardEventType
    
        The event type.
        
    .. py:property:: key
        :type: sgl.KeyCode
    
        The key that was pressed/released/repeated.
        
    .. py:property:: mods
        :type: sgl.KeyModifierFlags
    
        Keyboard modifier flags.
        
    .. py:property:: codepoint
        :type: int
    
        UTF-32 codepoint for input events.
        
    .. py:method:: is_key_press(self) -> bool
    
        Returns true if this event is a key press event.
        
    .. py:method:: is_key_release(self) -> bool
    
        Returns true if this event is a key release event.
        
    .. py:method:: is_key_repeat(self) -> bool
    
        Returns true if this event is a key repeat event.
        
    .. py:method:: is_input(self) -> bool
    
        Returns true if this event is an input event.
        
    .. py:method:: has_modifier(self, arg: sgl.KeyModifier, /) -> bool
    
        Returns true if the specified modifier is set.
        


----

.. py:class:: sgl.MouseEventType

    Base class: :py:class:`enum.Enum`
    
    Mouse event types.
    
    .. py:attribute:: sgl.MouseEventType.button_down
        :type: MouseEventType
        :value: MouseEventType.button_down
    
    .. py:attribute:: sgl.MouseEventType.button_up
        :type: MouseEventType
        :value: MouseEventType.button_up
    
    .. py:attribute:: sgl.MouseEventType.move
        :type: MouseEventType
        :value: MouseEventType.move
    
    .. py:attribute:: sgl.MouseEventType.scroll
        :type: MouseEventType
        :value: MouseEventType.scroll
    


----

.. py:class:: sgl.MouseEvent

    Mouse event.
    
    .. py:property:: type
        :type: sgl.MouseEventType
    
        The event type.
        
    .. py:property:: pos
        :type: sgl.math.float2
    
        The mouse position.
        
    .. py:property:: scroll
        :type: sgl.math.float2
    
        The scroll offset.
        
    .. py:property:: button
        :type: sgl.MouseButton
    
        The mouse button that was pressed/released.
        
    .. py:property:: mods
        :type: sgl.KeyModifierFlags
    
        Keyboard modifier flags.
        
    .. py:method:: is_button_down(self) -> bool
    
        Returns true if this event is a mouse button down event.
        
    .. py:method:: is_button_up(self) -> bool
    
        Returns true if this event is a mouse button up event.
        
    .. py:method:: is_move(self) -> bool
    
        Returns true if this event is a mouse move event.
        
    .. py:method:: is_scroll(self) -> bool
    
        Returns true if this event is a mouse scroll event.
        
    .. py:method:: has_modifier(self, arg: sgl.KeyModifier, /) -> bool
    
        Returns true if the specified modifier is set.
        


----

.. py:class:: sgl.GamepadEventType

    Base class: :py:class:`enum.Enum`
    
    Gamepad event types.
    
    .. py:attribute:: sgl.GamepadEventType.button_down
        :type: GamepadEventType
        :value: GamepadEventType.button_down
    
    .. py:attribute:: sgl.GamepadEventType.button_up
        :type: GamepadEventType
        :value: GamepadEventType.button_up
    
    .. py:attribute:: sgl.GamepadEventType.connect
        :type: GamepadEventType
        :value: GamepadEventType.connect
    
    .. py:attribute:: sgl.GamepadEventType.disconnect
        :type: GamepadEventType
        :value: GamepadEventType.disconnect
    


----

.. py:class:: sgl.GamepadButton

    Base class: :py:class:`enum.Enum`
    
    Gamepad buttons.
    
    .. py:attribute:: sgl.GamepadButton.a
        :type: GamepadButton
        :value: GamepadButton.a
    
    .. py:attribute:: sgl.GamepadButton.b
        :type: GamepadButton
        :value: GamepadButton.b
    
    .. py:attribute:: sgl.GamepadButton.x
        :type: GamepadButton
        :value: GamepadButton.x
    
    .. py:attribute:: sgl.GamepadButton.y
        :type: GamepadButton
        :value: GamepadButton.y
    
    .. py:attribute:: sgl.GamepadButton.left_bumper
        :type: GamepadButton
        :value: GamepadButton.left_bumper
    
    .. py:attribute:: sgl.GamepadButton.right_bumper
        :type: GamepadButton
        :value: GamepadButton.right_bumper
    
    .. py:attribute:: sgl.GamepadButton.back
        :type: GamepadButton
        :value: GamepadButton.back
    
    .. py:attribute:: sgl.GamepadButton.start
        :type: GamepadButton
        :value: GamepadButton.start
    
    .. py:attribute:: sgl.GamepadButton.guide
        :type: GamepadButton
        :value: GamepadButton.guide
    
    .. py:attribute:: sgl.GamepadButton.left_thumb
        :type: GamepadButton
        :value: GamepadButton.left_thumb
    
    .. py:attribute:: sgl.GamepadButton.right_thumb
        :type: GamepadButton
        :value: GamepadButton.right_thumb
    
    .. py:attribute:: sgl.GamepadButton.up
        :type: GamepadButton
        :value: GamepadButton.up
    
    .. py:attribute:: sgl.GamepadButton.right
        :type: GamepadButton
        :value: GamepadButton.right
    
    .. py:attribute:: sgl.GamepadButton.down
        :type: GamepadButton
        :value: GamepadButton.down
    
    .. py:attribute:: sgl.GamepadButton.left
        :type: GamepadButton
        :value: GamepadButton.left
    


----

.. py:class:: sgl.GamepadEvent

    Gamepad event.
    
    .. py:property:: type
        :type: sgl.GamepadEventType
    
        The event type.
        
    .. py:property:: button
        :type: sgl.GamepadButton
    
        The gamepad button that was pressed/released.
        
    .. py:method:: is_button_down(self) -> bool
    
        Returns true if this event is a gamepad button down event.
        
    .. py:method:: is_button_up(self) -> bool
    
        Returns true if this event is a gamepad button up event.
        
    .. py:method:: is_connect(self) -> bool
    
        Returns true if this event is a gamepad connect event.
        
    .. py:method:: is_disconnect(self) -> bool
    
        Returns true if this event is a gamepad disconnect event.
        


----

.. py:class:: sgl.GamepadState

    Gamepad state.
    
    .. py:property:: left_x
        :type: float
    
        X-axis of the left analog stick.
        
    .. py:property:: left_y
        :type: float
    
        Y-axis of the left analog stick.
        
    .. py:property:: right_x
        :type: float
    
        X-axis of the right analog stick.
        
    .. py:property:: right_y
        :type: float
    
        Y-axis of the right analog stick.
        
    .. py:property:: left_trigger
        :type: float
    
        Value of the left analog trigger.
        
    .. py:property:: right_trigger
        :type: float
    
        Value of the right analog trigger.
        
    .. py:property:: buttons
        :type: int
    
        Bitfield of gamepad buttons (see GamepadButton).
        
    .. py:method:: is_button_down(self, arg: sgl.GamepadButton, /) -> bool
    
        Returns true if the specified button is down.
        


----

Platform
--------

.. py:class:: sgl.platform.FileDialogFilter

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, name: str, pattern: str) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: tuple[str, str], /) -> None
        :no-index:
    
    .. py:property:: name
        :type: str
    
        Readable name (e.g. "JPEG").
        
    .. py:property:: pattern
        :type: str
    
        File extension pattern (e.g. "*.jpg" or "*.jpg,*.jpeg").
        


----

.. py:function:: sgl.platform.open_file_dialog(filters: collections.abc.Sequence[sgl.platform.FileDialogFilter] = []) -> pathlib.Path | None

    Show a file open dialog.
    
    Parameter ``filters``:
        List of file filters.
    
    Returns:
        The selected file path or nothing if the dialog was cancelled.
    


----

.. py:function:: sgl.platform.save_file_dialog(filters: collections.abc.Sequence[sgl.platform.FileDialogFilter] = []) -> pathlib.Path | None

    Show a file save dialog.
    
    Parameter ``filters``:
        List of file filters.
    
    Returns:
        The selected file path or nothing if the dialog was cancelled.
    


----

.. py:function:: sgl.platform.choose_folder_dialog() -> pathlib.Path | None

    Show a folder selection dialog.
    
    Returns:
        The selected folder path or nothing if the dialog was cancelled.
    


----

.. py:function:: sgl.platform.display_scale_factor() -> float

    The pixel scale factor of the primary display.
    


----

.. py:function:: sgl.platform.executable_path() -> pathlib.Path

    The full path to the current executable.
    


----

.. py:function:: sgl.platform.executable_directory() -> pathlib.Path

    The current executable directory.
    


----

.. py:function:: sgl.platform.executable_name() -> str

    The current executable name.
    


----

.. py:function:: sgl.platform.app_data_directory() -> pathlib.Path

    The application data directory.
    


----

.. py:function:: sgl.platform.home_directory() -> pathlib.Path

    The home directory.
    


----

.. py:function:: sgl.platform.project_directory() -> pathlib.Path

    The project source directory. Note that this is only valid during
    development.
    


----

.. py:function:: sgl.platform.runtime_directory() -> pathlib.Path

    The runtime directory. This is the path where the sgl runtime library
    (sgl.dll, libsgl.so or libsgl.dynlib) resides.
    


----

.. py:data:: sgl.platform.page_size
    :type: int
    :value: 4096



----

.. py:class:: sgl.platform.MemoryStats

    
    
    .. py:property:: rss
        :type: int
    
        Current resident/working set size in bytes.
        
    .. py:property:: peak_rss
        :type: int
    
        Peak resident/working set size in bytes.
        


----

.. py:function:: sgl.platform.memory_stats() -> sgl.platform.MemoryStats

    Get the current memory stats.
    


----

Threading
---------

.. py:function:: sgl.thread.wait_for_tasks() -> None

    Block until all scheduled tasks are completed.
    


----

Device
------

.. py:class:: sgl.AccelerationStructure

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: desc
        :type: sgl.AccelerationStructureDesc
    
    .. py:property:: handle
        :type: sgl.AccelerationStructureHandle
    


----

.. py:class:: sgl.AccelerationStructureBuildDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: inputs
        :type: list[sgl.AccelerationStructureBuildInputInstances | sgl.AccelerationStructureBuildInputTriangles | sgl.AccelerationStructureBuildInputProceduralPrimitives]
    
        List of build inputs. All inputs must be of the same type.
        
    .. py:property:: motion_options
        :type: sgl.AccelerationStructureBuildInputMotionOptions
    
    .. py:property:: mode
        :type: sgl.AccelerationStructureBuildMode
    
    .. py:property:: flags
        :type: sgl.AccelerationStructureBuildFlags
    


----

.. py:class:: sgl.AccelerationStructureBuildFlags

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.AccelerationStructureBuildFlags.none
        :type: AccelerationStructureBuildFlags
        :value: AccelerationStructureBuildFlags.none
    
    .. py:attribute:: sgl.AccelerationStructureBuildFlags.allow_update
        :type: AccelerationStructureBuildFlags
        :value: AccelerationStructureBuildFlags.allow_update
    
    .. py:attribute:: sgl.AccelerationStructureBuildFlags.allow_compaction
        :type: AccelerationStructureBuildFlags
        :value: AccelerationStructureBuildFlags.allow_compaction
    
    .. py:attribute:: sgl.AccelerationStructureBuildFlags.prefer_fast_trace
        :type: AccelerationStructureBuildFlags
        :value: AccelerationStructureBuildFlags.prefer_fast_trace
    
    .. py:attribute:: sgl.AccelerationStructureBuildFlags.prefer_fast_build
        :type: AccelerationStructureBuildFlags
        :value: AccelerationStructureBuildFlags.prefer_fast_build
    
    .. py:attribute:: sgl.AccelerationStructureBuildFlags.minimize_memory
        :type: AccelerationStructureBuildFlags
        :value: AccelerationStructureBuildFlags.minimize_memory
    


----

.. py:class:: sgl.AccelerationStructureBuildInputInstances

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: instance_buffer
        :type: sgl.BufferOffsetPair
    
    .. py:property:: instance_stride
        :type: int
    
    .. py:property:: instance_count
        :type: int
    


----

.. py:class:: sgl.AccelerationStructureBuildInputMotionOptions

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: key_count
        :type: int
    
    .. py:property:: time_start
        :type: float
    
    .. py:property:: time_end
        :type: float
    


----

.. py:class:: sgl.AccelerationStructureBuildInputProceduralPrimitives

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: aabb_buffers
        :type: list[sgl.BufferOffsetPair]
    
    .. py:property:: aabb_stride
        :type: int
    
    .. py:property:: primitive_count
        :type: int
    
    .. py:property:: flags
        :type: sgl.AccelerationStructureGeometryFlags
    


----

.. py:class:: sgl.AccelerationStructureBuildInputTriangles

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: vertex_buffers
        :type: list[sgl.BufferOffsetPair]
    
    .. py:property:: vertex_format
        :type: sgl.Format
    
    .. py:property:: vertex_count
        :type: int
    
    .. py:property:: vertex_stride
        :type: int
    
    .. py:property:: index_buffer
        :type: sgl.BufferOffsetPair
    
    .. py:property:: index_format
        :type: sgl.IndexFormat
    
    .. py:property:: index_count
        :type: int
    
    .. py:property:: pre_transform_buffer
        :type: sgl.BufferOffsetPair
    
    .. py:property:: flags
        :type: sgl.AccelerationStructureGeometryFlags
    


----

.. py:class:: sgl.AccelerationStructureBuildMode

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.AccelerationStructureBuildMode.build
        :type: AccelerationStructureBuildMode
        :value: AccelerationStructureBuildMode.build
    
    .. py:attribute:: sgl.AccelerationStructureBuildMode.update
        :type: AccelerationStructureBuildMode
        :value: AccelerationStructureBuildMode.update
    


----

.. py:class:: sgl.AccelerationStructureCopyMode

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.AccelerationStructureCopyMode.clone
        :type: AccelerationStructureCopyMode
        :value: AccelerationStructureCopyMode.clone
    
    .. py:attribute:: sgl.AccelerationStructureCopyMode.compact
        :type: AccelerationStructureCopyMode
        :value: AccelerationStructureCopyMode.compact
    


----

.. py:class:: sgl.AccelerationStructureDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: size
        :type: int
    
    .. py:property:: label
        :type: str
    


----

.. py:class:: sgl.AccelerationStructureGeometryFlags

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.AccelerationStructureGeometryFlags.none
        :type: AccelerationStructureGeometryFlags
        :value: AccelerationStructureGeometryFlags.none
    
    .. py:attribute:: sgl.AccelerationStructureGeometryFlags.opaque
        :type: AccelerationStructureGeometryFlags
        :value: AccelerationStructureGeometryFlags.opaque
    
    .. py:attribute:: sgl.AccelerationStructureGeometryFlags.no_duplicate_any_hit_invocation
        :type: AccelerationStructureGeometryFlags
        :value: AccelerationStructureGeometryFlags.no_duplicate_any_hit_invocation
    


----

.. py:class:: sgl.AccelerationStructureHandle

    N/A
    
    .. py:method:: __init__(self) -> None
    


----

.. py:class:: sgl.AccelerationStructureInstanceDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: transform
        :type: sgl.math.float3x4
    
    .. py:property:: instance_id
        :type: int
    
    .. py:property:: instance_mask
        :type: int
    
    .. py:property:: instance_contribution_to_hit_group_index
        :type: int
    
    .. py:property:: flags
        :type: sgl.AccelerationStructureInstanceFlags
    
    .. py:property:: acceleration_structure
        :type: sgl.AccelerationStructureHandle
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=uint8, shape=(64), writable=False]
    


----

.. py:class:: sgl.AccelerationStructureInstanceFlags

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.AccelerationStructureInstanceFlags.none
        :type: AccelerationStructureInstanceFlags
        :value: AccelerationStructureInstanceFlags.none
    
    .. py:attribute:: sgl.AccelerationStructureInstanceFlags.triangle_facing_cull_disable
        :type: AccelerationStructureInstanceFlags
        :value: AccelerationStructureInstanceFlags.triangle_facing_cull_disable
    
    .. py:attribute:: sgl.AccelerationStructureInstanceFlags.triangle_front_counter_clockwise
        :type: AccelerationStructureInstanceFlags
        :value: AccelerationStructureInstanceFlags.triangle_front_counter_clockwise
    
    .. py:attribute:: sgl.AccelerationStructureInstanceFlags.force_opaque
        :type: AccelerationStructureInstanceFlags
        :value: AccelerationStructureInstanceFlags.force_opaque
    
    .. py:attribute:: sgl.AccelerationStructureInstanceFlags.no_opaque
        :type: AccelerationStructureInstanceFlags
        :value: AccelerationStructureInstanceFlags.no_opaque
    


----

.. py:class:: sgl.AccelerationStructureInstanceList

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: size
        :type: int
    
    .. py:property:: instance_stride
        :type: int
    
    .. py:method:: resize(self, size: int) -> None
    
    .. py:method:: write(self, index: int, instance: sgl.AccelerationStructureInstanceDesc) -> None
    
    .. py:method:: write(self, index: int, instances: Sequence[sgl.AccelerationStructureInstanceDesc]) -> None
        :no-index:
    
    .. py:method:: buffer(self) -> sgl.Buffer
    
    .. py:method:: build_input_instances(self) -> sgl.AccelerationStructureBuildInputInstances
    


----

.. py:class:: sgl.AccelerationStructureQueryDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: query_type
        :type: sgl.QueryType
    
    .. py:property:: query_pool
        :type: sgl.QueryPool
    
    .. py:property:: first_query_index
        :type: int
    


----

.. py:class:: sgl.AccelerationStructureSizes

    
    
    .. py:property:: acceleration_structure_size
        :type: int
    
    .. py:property:: scratch_size
        :type: int
    
    .. py:property:: update_scratch_size
        :type: int
    


----

.. py:class:: sgl.AdapterInfo

    
    
    .. py:property:: name
        :type: str
    
        Descriptive name of the adapter.
        
    .. py:property:: vendor_id
        :type: int
    
        Unique identifier for the vendor (only available for D3D12 and
        Vulkan).
        
    .. py:property:: device_id
        :type: int
    
        Unique identifier for the physical device among devices from the
        vendor (only available for D3D12 and Vulkan).
        
    .. py:property:: luid
        :type: list[int]
    
        Logically unique identifier of the adapter.
        


----

.. py:class:: sgl.AspectBlendDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: src_factor
        :type: sgl.BlendFactor
    
    .. py:property:: dst_factor
        :type: sgl.BlendFactor
    
    .. py:property:: op
        :type: sgl.BlendOp
    


----

.. py:class:: sgl.BaseReflectionObject

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:property:: is_valid
        :type: bool
    


----

.. py:class:: sgl.BlendFactor

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.BlendFactor.zero
        :type: BlendFactor
        :value: BlendFactor.zero
    
    .. py:attribute:: sgl.BlendFactor.one
        :type: BlendFactor
        :value: BlendFactor.one
    
    .. py:attribute:: sgl.BlendFactor.src_color
        :type: BlendFactor
        :value: BlendFactor.src_color
    
    .. py:attribute:: sgl.BlendFactor.inv_src_color
        :type: BlendFactor
        :value: BlendFactor.inv_src_color
    
    .. py:attribute:: sgl.BlendFactor.src_alpha
        :type: BlendFactor
        :value: BlendFactor.src_alpha
    
    .. py:attribute:: sgl.BlendFactor.inv_src_alpha
        :type: BlendFactor
        :value: BlendFactor.inv_src_alpha
    
    .. py:attribute:: sgl.BlendFactor.dest_alpha
        :type: BlendFactor
        :value: BlendFactor.dest_alpha
    
    .. py:attribute:: sgl.BlendFactor.inv_dest_alpha
        :type: BlendFactor
        :value: BlendFactor.inv_dest_alpha
    
    .. py:attribute:: sgl.BlendFactor.dest_color
        :type: BlendFactor
        :value: BlendFactor.dest_color
    
    .. py:attribute:: sgl.BlendFactor.inv_dest_color
        :type: BlendFactor
        :value: BlendFactor.inv_dest_color
    
    .. py:attribute:: sgl.BlendFactor.src_alpha_saturate
        :type: BlendFactor
        :value: BlendFactor.src_alpha_saturate
    
    .. py:attribute:: sgl.BlendFactor.blend_color
        :type: BlendFactor
        :value: BlendFactor.blend_color
    
    .. py:attribute:: sgl.BlendFactor.inv_blend_color
        :type: BlendFactor
        :value: BlendFactor.inv_blend_color
    
    .. py:attribute:: sgl.BlendFactor.secondary_src_color
        :type: BlendFactor
        :value: BlendFactor.secondary_src_color
    
    .. py:attribute:: sgl.BlendFactor.inv_secondary_src_color
        :type: BlendFactor
        :value: BlendFactor.inv_secondary_src_color
    
    .. py:attribute:: sgl.BlendFactor.secondary_src_alpha
        :type: BlendFactor
        :value: BlendFactor.secondary_src_alpha
    
    .. py:attribute:: sgl.BlendFactor.inv_secondary_src_alpha
        :type: BlendFactor
        :value: BlendFactor.inv_secondary_src_alpha
    


----

.. py:class:: sgl.BlendOp

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.BlendOp.add
        :type: BlendOp
        :value: BlendOp.add
    
    .. py:attribute:: sgl.BlendOp.subtract
        :type: BlendOp
        :value: BlendOp.subtract
    
    .. py:attribute:: sgl.BlendOp.reverse_subtract
        :type: BlendOp
        :value: BlendOp.reverse_subtract
    
    .. py:attribute:: sgl.BlendOp.min
        :type: BlendOp
        :value: BlendOp.min
    
    .. py:attribute:: sgl.BlendOp.max
        :type: BlendOp
        :value: BlendOp.max
    


----

.. py:class:: sgl.Buffer

    Base class: :py:class:`sgl.Resource`
    
    
    
    .. py:property:: desc
        :type: sgl.BufferDesc
    
    .. py:property:: size
        :type: int
    
    .. py:property:: struct_size
        :type: int
    
    .. py:property:: device_address
        :type: int
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[]
    
    .. py:method:: copy_from_numpy(self, data: numpy.ndarray[]) -> None
    
    .. py:method:: to_torch(self, type: sgl.DataType = DataType.void, shape: collections.abc.Sequence[int] = [], strides: collections.abc.Sequence[int] = [], offset: int = 0) -> torch.Tensor[device='cuda']
    


----

.. py:class:: sgl.BufferCursor

    Base class: :py:class:`sgl.Object`
    
    Represents a list of elements in a block of memory, and provides
    simple interface to get a BufferElementCursor for each one. As this
    can be the owner of its data, it is a ref counted object that elements
    refer to.
    
    .. py:method:: __init__(self, element_layout: sgl.TypeLayoutReflection, size: int) -> None
    
    .. py:method:: __init__(self, element_layout: sgl.TypeLayoutReflection, buffer_resource: sgl.Buffer, load_before_write: bool = True) -> None
        :no-index:
    
    .. py:method:: __init__(self, element_layout: sgl.TypeLayoutReflection, buffer_resource: sgl.Buffer, size: int, offset: int, load_before_write: bool = True) -> None
        :no-index:
    
    .. py:property:: element_type_layout
        :type: sgl.TypeLayoutReflection
    
        Get type layout of an element of the cursor.
        
    .. py:property:: element_type
        :type: sgl.TypeReflection
    
        Get type of an element of the cursor.
        
    .. py:method:: find_element(self, index: int) -> sgl.BufferElementCursor
    
        Get element at a given index.
        
    .. py:property:: element_count
        :type: int
    
        Number of elements in the buffer.
        
    .. py:property:: element_size
        :type: int
    
        Size of element.
        
    .. py:property:: element_stride
        :type: int
    
        Stride of elements.
        
    .. py:property:: size
        :type: int
    
        Size of whole buffer.
        
    .. py:property:: is_loaded
        :type: bool
    
        Check if internal buffer exists.
        
    .. py:method:: load(self) -> None
    
        In case of GPU only buffers, loads all data from GPU.
        
    .. py:method:: apply(self) -> None
    
        In case of GPU only buffers, pushes all data to the GPU.
        
    .. py:property:: resource
        :type: sgl.Buffer
    
        Get the resource this cursor represents (if any).
        
    .. py:method:: to_numpy(self) -> numpy.ndarray[]
    
    .. py:method:: copy_from_numpy(self, data: numpy.ndarray[]) -> None
    


----

.. py:class:: sgl.BufferDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: size
        :type: int
    
        Buffer size in bytes.
        
    .. py:property:: struct_size
        :type: int
    
        Struct size in bytes.
        
    .. py:property:: format
        :type: sgl.Format
    
        Buffer format. Used when creating typed buffer views.
        
    .. py:property:: memory_type
        :type: sgl.MemoryType
    
        Memory type.
        
    .. py:property:: usage
        :type: sgl.BufferUsage
    
        Resource usage flags.
        
    .. py:property:: default_state
        :type: sgl.ResourceState
    
        Initial resource state.
        
    .. py:property:: label
        :type: str
    
        Debug label.
        


----

.. py:class:: sgl.BufferElementCursor

    Represents a single element of a given type in a block of memory, and
    provides read/write tools to access its members via reflection.
    
    .. py:method:: set_data(self, data: ndarray[device='cpu']) -> None
    
    .. py:method:: set_data(self, data: ndarray[device='cpu']) -> None
        :no-index:
    
    .. py:method:: is_valid(self) -> bool
    
        N/A
        
    .. py:method:: find_field(self, name: str) -> sgl.BufferElementCursor
    
        N/A
        
    .. py:method:: find_element(self, index: int) -> sgl.BufferElementCursor
    
        N/A
        
    .. py:method:: has_field(self, name: str) -> bool
    
        N/A
        
    .. py:method:: has_element(self, index: int) -> bool
    
        N/A
        
    .. py:method:: read(self) -> object
    
        N/A
        
    .. py:method:: write(self, val: object) -> None
    
        N/A
        


----

.. py:class:: sgl.BufferOffsetPair

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, buffer: sgl.Buffer) -> None
        :no-index:
    
    .. py:method:: __init__(self, buffer: sgl.Buffer, offset: int = 0) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: buffer
        :type: sgl.Buffer
    
    .. py:property:: offset
        :type: int
    


----

.. py:class:: sgl.BufferRange

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: offset
        :type: int
    
    .. py:property:: size
        :type: int
    


----

.. py:class:: sgl.BufferUsage

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.BufferUsage.none
        :type: BufferUsage
        :value: BufferUsage.none
    
    .. py:attribute:: sgl.BufferUsage.vertex_buffer
        :type: BufferUsage
        :value: BufferUsage.vertex_buffer
    
    .. py:attribute:: sgl.BufferUsage.index_buffer
        :type: BufferUsage
        :value: BufferUsage.index_buffer
    
    .. py:attribute:: sgl.BufferUsage.constant_buffer
        :type: BufferUsage
        :value: BufferUsage.constant_buffer
    
    .. py:attribute:: sgl.BufferUsage.shader_resource
        :type: BufferUsage
        :value: BufferUsage.shader_resource
    
    .. py:attribute:: sgl.BufferUsage.unordered_access
        :type: BufferUsage
        :value: BufferUsage.unordered_access
    
    .. py:attribute:: sgl.BufferUsage.indirect_argument
        :type: BufferUsage
        :value: BufferUsage.indirect_argument
    
    .. py:attribute:: sgl.BufferUsage.copy_source
        :type: BufferUsage
        :value: BufferUsage.copy_source
    
    .. py:attribute:: sgl.BufferUsage.copy_destination
        :type: BufferUsage
        :value: BufferUsage.copy_destination
    
    .. py:attribute:: sgl.BufferUsage.acceleration_structure
        :type: BufferUsage
        :value: BufferUsage.acceleration_structure
    
    .. py:attribute:: sgl.BufferUsage.acceleration_structure_build_input
        :type: BufferUsage
        :value: BufferUsage.acceleration_structure_build_input
    
    .. py:attribute:: sgl.BufferUsage.shader_table
        :type: BufferUsage
        :value: BufferUsage.shader_table
    
    .. py:attribute:: sgl.BufferUsage.shared
        :type: BufferUsage
        :value: BufferUsage.shared
    


----

.. py:class:: sgl.ColorTargetDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: format
        :type: sgl.Format
    
    .. py:property:: color
        :type: sgl.AspectBlendDesc
    
    .. py:property:: alpha
        :type: sgl.AspectBlendDesc
    
    .. py:property:: write_mask
        :type: sgl.RenderTargetWriteMask
    
    .. py:property:: enable_blend
        :type: bool
    
    .. py:property:: logic_op
        :type: sgl.LogicOp
    


----

.. py:class:: sgl.CommandBuffer

    Base class: :py:class:`sgl.DeviceResource`
    
    
    


----

.. py:class:: sgl.CommandEncoder

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:method:: begin_render_pass(self, desc: sgl.RenderPassDesc) -> sgl.RenderPassEncoder
    
    .. py:method:: begin_compute_pass(self) -> sgl.ComputePassEncoder
    
    .. py:method:: begin_ray_tracing_pass(self) -> sgl.RayTracingPassEncoder
    
    .. py:method:: copy_buffer(self, dst: sgl.Buffer, dst_offset: int, src: sgl.Buffer, src_offset: int, size: int) -> None
    
        Copy a buffer region.
        
        Parameter ``dst``:
            Destination buffer.
        
        Parameter ``dst_offset``:
            Destination offset in bytes.
        
        Parameter ``src``:
            Source buffer.
        
        Parameter ``src_offset``:
            Source offset in bytes.
        
        Parameter ``size``:
            Size in bytes.
        
    .. py:method:: copy_texture(self, dst: sgl.Texture, dst_subresource: int, dst_offset: sgl.math.uint3, src: sgl.Texture, src_subresource: int, src_offset: sgl.math.uint3, extent: sgl.math.uint3 = {4294967295, 4294967295, 4294967295}) -> None
    
        Copy a texture region.
        
        Parameter ``dst``:
            Destination texture.
        
        Parameter ``dst_subresource``:
            Destination subresource index.
        
        Parameter ``dst_offset``:
            Destination offset in texels.
        
        Parameter ``src``:
            Source texture.
        
        Parameter ``src_subresource``:
            Source subresource index.
        
        Parameter ``src_offset``:
            Source offset in texels.
        
        Parameter ``extent``:
            Size in texels (-1 for maximum possible size).
        
    .. py:method:: clear_buffer(self, buffer: sgl.Buffer, range: sgl.BufferRange = BufferRange(offset=0, size=18446744073709551615) -> None
    
    .. py:method:: clear_texture_float(self, texture: sgl.Texture, range: sgl.SubresourceRange = SubresourceRange(mip_level=0, mip_count=4294967295, base_array_layer=0, layer_count=4294967295, clear_value: sgl.math.float4 = {0, 0, 0, 0}) -> None
    
    .. py:method:: clear_texture_uint(self, texture: sgl.Texture, range: sgl.SubresourceRange = SubresourceRange(mip_level=0, mip_count=4294967295, base_array_layer=0, layer_count=4294967295, clear_value: sgl.math.uint4 = {0, 0, 0, 0}) -> None
    
    .. py:method:: clear_texture_sint(self, texture: sgl.Texture, range: sgl.SubresourceRange = SubresourceRange(mip_level=0, mip_count=4294967295, base_array_layer=0, layer_count=4294967295, clear_value: sgl.math.int4 = {0, 0, 0, 0}) -> None
    
    .. py:method:: clear_texture_depth_stencil(self, texture: sgl.Texture, range: sgl.SubresourceRange = SubresourceRange(mip_level=0, mip_count=4294967295, base_array_layer=0, layer_count=4294967295, clear_depth: bool = True, depth_value: float = 0.0, clear_stencil: bool = True, stencil_value: int = 0) -> None
    
    .. py:method:: blit(self, dst: sgl.TextureView, src: sgl.TextureView, filter: sgl.TextureFilteringMode = TextureFilteringMode.linear) -> None
    
        Blit a texture view.
        
        Blits the full extent of the source texture to the destination
        texture.
        
        Parameter ``dst``:
            View of the destination texture.
        
        Parameter ``src``:
            View of the source texture.
        
        Parameter ``filter``:
            Filtering mode to use.
        
    .. py:method:: blit(self, dst: sgl.Texture, src: sgl.Texture, filter: sgl.TextureFilteringMode = TextureFilteringMode.linear) -> None
        :no-index:
    
        Blit a texture.
        
        Blits the full extent of the source texture to the destination
        texture.
        
        Parameter ``dst``:
            Destination texture.
        
        Parameter ``src``:
            Source texture.
        
        Parameter ``filter``:
            Filtering mode to use.
        
    .. py:method:: resolve_query(self, query_pool: sgl.QueryPool, index: int, count: int, buffer: sgl.Buffer, offset: int) -> None
    
    .. py:method:: build_acceleration_structure(self, desc: sgl.AccelerationStructureBuildDesc, dst: sgl.AccelerationStructure, src: sgl.AccelerationStructure | None, scratch_buffer: sgl.BufferOffsetPair, queries: Sequence[sgl.AccelerationStructureQueryDesc] = []) -> None
    
    .. py:method:: copy_acceleration_structure(self, src: sgl.AccelerationStructure, dst: sgl.AccelerationStructure, mode: sgl.AccelerationStructureCopyMode) -> None
    
    .. py:method:: query_acceleration_structure_properties(self, acceleration_structures: Sequence[sgl.AccelerationStructure], queries: Sequence[sgl.AccelerationStructureQueryDesc]) -> None
    
    .. py:method:: serialize_acceleration_structure(self, dst: sgl.BufferOffsetPair, src: sgl.AccelerationStructure) -> None
    
    .. py:method:: deserialize_acceleration_structure(self, dst: sgl.AccelerationStructure, src: sgl.BufferOffsetPair) -> None
    
    .. py:method:: set_buffer_state(self, buffer: sgl.Buffer, state: sgl.ResourceState) -> None
    
        Transition resource state of a buffer and add a barrier if state has
        changed.
        
        Parameter ``buffer``:
            Buffer
        
        Parameter ``state``:
            New state
        
    .. py:method:: set_texture_state(self, texture: sgl.Texture, state: sgl.ResourceState) -> None
    
        Transition resource state of a texture and add a barrier if state has
        changed.
        
        Parameter ``texture``:
            Texture
        
        Parameter ``state``:
            New state
        
    .. py:method:: set_texture_state(self, texture: sgl.Texture, range: sgl.SubresourceRange, state: sgl.ResourceState) -> None
        :no-index:
    
    .. py:method:: push_debug_group(self, name: str, color: sgl.math.float3) -> None
    
        Push a debug group.
        
    .. py:method:: pop_debug_group(self) -> None
    
        Pop a debug group.
        
    .. py:method:: insert_debug_marker(self, name: str, color: sgl.math.float3) -> None
    
        Insert a debug marker.
        
        Parameter ``name``:
            Name of the marker.
        
        Parameter ``color``:
            Color of the marker.
        
    .. py:method:: write_timestamp(self, query_pool: sgl.QueryPool, index: int) -> None
    
        Write a timestamp.
        
        Parameter ``query_pool``:
            Query pool.
        
        Parameter ``index``:
            Index of the query.
        
    .. py:method:: finish(self) -> sgl.CommandBuffer
    


----

.. py:class:: sgl.CommandQueueType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.CommandQueueType.graphics
        :type: CommandQueueType
        :value: CommandQueueType.graphics
    


----

.. py:class:: sgl.ComparisonFunc

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.ComparisonFunc.never
        :type: ComparisonFunc
        :value: ComparisonFunc.never
    
    .. py:attribute:: sgl.ComparisonFunc.less
        :type: ComparisonFunc
        :value: ComparisonFunc.less
    
    .. py:attribute:: sgl.ComparisonFunc.equal
        :type: ComparisonFunc
        :value: ComparisonFunc.equal
    
    .. py:attribute:: sgl.ComparisonFunc.less_equal
        :type: ComparisonFunc
        :value: ComparisonFunc.less_equal
    
    .. py:attribute:: sgl.ComparisonFunc.greater
        :type: ComparisonFunc
        :value: ComparisonFunc.greater
    
    .. py:attribute:: sgl.ComparisonFunc.not_equal
        :type: ComparisonFunc
        :value: ComparisonFunc.not_equal
    
    .. py:attribute:: sgl.ComparisonFunc.greater_equal
        :type: ComparisonFunc
        :value: ComparisonFunc.greater_equal
    
    .. py:attribute:: sgl.ComparisonFunc.always
        :type: ComparisonFunc
        :value: ComparisonFunc.always
    


----

.. py:class:: sgl.ComputeKernel

    Base class: :py:class:`sgl.Kernel`
    
    
    
    .. py:property:: pipeline
        :type: sgl.ComputePipeline
    
    .. py:method:: dispatch(self, thread_count: sgl.math.uint3, vars: dict = {}, command_encoder: sgl.CommandEncoder | None = None, **kwargs) -> None
    


----

.. py:class:: sgl.ComputeKernelDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: program
        :type: sgl.ShaderProgram
    


----

.. py:class:: sgl.ComputePassEncoder

    Base class: :py:class:`sgl.PassEncoder`
    
    
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.ComputePipeline) -> sgl.ShaderObject
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.ComputePipeline, root_object: sgl.ShaderObject) -> None
        :no-index:
    
    .. py:method:: dispatch(self, thread_count: sgl.math.uint3) -> None
    
    .. py:method:: dispatch_compute(self, thread_group_count: sgl.math.uint3) -> None
    
    .. py:method:: dispatch_compute_indirect(self, arg_buffer: sgl.BufferOffsetPair) -> None
    


----

.. py:class:: sgl.ComputePipeline

    Base class: :py:class:`sgl.Pipeline`
    
    Compute pipeline.
    
    .. py:property:: thread_group_size
        :type: sgl.math.uint3
    
        Thread group size. Used to determine the number of thread groups to
        dispatch.
        


----

.. py:class:: sgl.ComputePipelineDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: program
        :type: sgl.ShaderProgram
    


----

.. py:class:: sgl.CoopVecMatrixDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, rows: int, cols: int, element_type: sgl.DataType, layout: sgl.CoopVecMatrixLayout, size: int, offset: int) -> None
        :no-index:
    
    .. py:property:: rows
        :type: int
    
    .. py:property:: cols
        :type: int
    
    .. py:property:: element_type
        :type: sgl.DataType
    
    .. py:property:: layout
        :type: sgl.CoopVecMatrixLayout
    
    .. py:property:: size
        :type: int
    
    .. py:property:: offset
        :type: int
    


----

.. py:class:: sgl.CoopVecMatrixLayout

    Base class: :py:class:`enum.Enum`
    
    
    
    .. py:attribute:: sgl.CoopVecMatrixLayout.row_major
        :type: CoopVecMatrixLayout
        :value: CoopVecMatrixLayout.row_major
    
    .. py:attribute:: sgl.CoopVecMatrixLayout.column_major
        :type: CoopVecMatrixLayout
        :value: CoopVecMatrixLayout.column_major
    
    .. py:attribute:: sgl.CoopVecMatrixLayout.inferencing_optimal
        :type: CoopVecMatrixLayout
        :value: CoopVecMatrixLayout.inferencing_optimal
    
    .. py:attribute:: sgl.CoopVecMatrixLayout.training_optimal
        :type: CoopVecMatrixLayout
        :value: CoopVecMatrixLayout.training_optimal
    


----

.. py:class:: sgl.CullMode

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.CullMode.none
        :type: CullMode
        :value: CullMode.none
    
    .. py:attribute:: sgl.CullMode.front
        :type: CullMode
        :value: CullMode.front
    
    .. py:attribute:: sgl.CullMode.back
        :type: CullMode
        :value: CullMode.back
    


----

.. py:class:: sgl.DeclReflection

    Base class: :py:class:`sgl.BaseReflectionObject`
    
    
    
    .. py:class:: sgl.DeclReflection.Kind
    
        Base class: :py:class:`enum.Enum`
        
        Different kinds of decl slang can return.
        
        .. py:attribute:: sgl.DeclReflection.Kind.unsupported
            :type: Kind
            :value: Kind.unsupported
        
        .. py:attribute:: sgl.DeclReflection.Kind.struct
            :type: Kind
            :value: Kind.struct
        
        .. py:attribute:: sgl.DeclReflection.Kind.func
            :type: Kind
            :value: Kind.func
        
        .. py:attribute:: sgl.DeclReflection.Kind.module
            :type: Kind
            :value: Kind.module
        
        .. py:attribute:: sgl.DeclReflection.Kind.generic
            :type: Kind
            :value: Kind.generic
        
        .. py:attribute:: sgl.DeclReflection.Kind.variable
            :type: Kind
            :value: Kind.variable
        
    .. py:property:: kind
        :type: sgl.DeclReflection.Kind
    
        Decl kind (struct/function/module/generic/variable).
        
    .. py:property:: children
        :type: sgl.DeclReflectionChildList
    
        List of children of this cursor.
        
    .. py:property:: child_count
        :type: int
    
        Get number of children.
        
    .. py:property:: name
        :type: str
    
    .. py:method:: children_of_kind(self, kind: sgl.DeclReflection.Kind) -> sgl.DeclReflectionIndexedChildList
    
        List of children of this cursor of a specific kind.
        
    .. py:method:: as_type(self) -> sgl.TypeReflection
    
        Get type corresponding to this decl ref.
        
    .. py:method:: as_variable(self) -> sgl.VariableReflection
    
        Get variable corresponding to this decl ref.
        
    .. py:method:: as_function(self) -> sgl.FunctionReflection
    
        Get function corresponding to this decl ref.
        
    .. py:method:: find_children_of_kind(self, kind: sgl.DeclReflection.Kind, child_name: str) -> sgl.DeclReflectionIndexedChildList
    
        Finds all children of a specific kind with a given name. Note: Only
        supported for types, functions and variables.
        
    .. py:method:: find_first_child_of_kind(self, kind: sgl.DeclReflection.Kind, child_name: str) -> sgl.DeclReflection
    
        Finds the first child of a specific kind with a given name. Note: Only
        supported for types, functions and variables.
        


----

.. py:class:: sgl.DeclReflectionChildList

    DeclReflection lazy child list evaluation.
    


----

.. py:class:: sgl.DeclReflectionIndexedChildList

    DeclReflection lazy search result evaluation.
    


----

.. py:class:: sgl.DepthStencilDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: format
        :type: sgl.Format
    
    .. py:property:: depth_test_enable
        :type: bool
    
    .. py:property:: depth_write_enable
        :type: bool
    
    .. py:property:: depth_func
        :type: sgl.ComparisonFunc
    
    .. py:property:: stencil_enable
        :type: bool
    
    .. py:property:: stencil_read_mask
        :type: int
    
    .. py:property:: stencil_write_mask
        :type: int
    
    .. py:property:: front_face
        :type: sgl.DepthStencilOpDesc
    
    .. py:property:: back_face
        :type: sgl.DepthStencilOpDesc
    


----

.. py:class:: sgl.DepthStencilOpDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: stencil_fail_op
        :type: sgl.StencilOp
    
    .. py:property:: stencil_depth_fail_op
        :type: sgl.StencilOp
    
    .. py:property:: stencil_pass_op
        :type: sgl.StencilOp
    
    .. py:property:: stencil_func
        :type: sgl.ComparisonFunc
    


----

.. py:class:: sgl.Device

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:method:: __init__(self, type: sgl.DeviceType = DeviceType.automatic, enable_debug_layers: bool = False, enable_cuda_interop: bool = False, enable_print: bool = False, enable_hot_reload: bool = True, adapter_luid: collections.abc.Sequence[int] | None = None, compiler_options: sgl.SlangCompilerOptions | None = None, shader_cache_path: str | os.PathLike | None = None) -> None
    
    .. py:method:: __init__(self, desc: sgl.DeviceDesc) -> None
        :no-index:
    
    .. py:property:: desc
        :type: sgl.DeviceDesc
    
    .. py:property:: info
        :type: sgl.DeviceInfo
    
        Device information.
        
    .. py:property:: shader_cache_stats
        :type: sgl.ShaderCacheStats
    
        Shader cache statistics.
        
    .. py:property:: supported_shader_model
        :type: sgl.ShaderModel
    
        The highest shader model supported by the device.
        
    .. py:property:: features
        :type: list[str]
    
        List of features supported by the device.
        
    .. py:property:: supports_cuda_interop
        :type: bool
    
        True if the device supports CUDA interoperability.
        
    .. py:method:: get_format_support(self, format: sgl.Format) -> sgl.FormatSupport
    
        Returns the supported resource states for a given format.
        
    .. py:property:: slang_session
        :type: sgl.SlangSession
    
        Default slang session.
        
    .. py:method:: close(self) -> None
    
        Close the device.
        
        This function should be called before the device is released. It waits
        for all pending work to be completed and releases internal resources,
        removing all cyclic references that might prevent the device from
        being destroyed. After closing the device, no new resources must be
        created and no new work must be submitted.
        
        \note The Python extension will automatically close all open devices
        when the interpreter is terminated through an `atexit` handler. If a
        device is to be destroyed at runtime, it must be closed explicitly.
        
    .. py:method:: create_surface(self, window: sgl.Window) -> sgl.Surface
    
        Create a new surface.
        
        Parameter ``window``:
            Window to create the surface for.
        
        Returns:
            New surface object.
        
    .. py:method:: create_surface(self, window_handle: sgl.WindowHandle) -> sgl.Surface
        :no-index:
    
        Create a new surface.
        
        Parameter ``window_handle``:
            Native window handle to create the surface for.
        
        Returns:
            New surface object.
        
    .. py:method:: create_buffer(self, size: int = 0, element_count: int = 0, struct_size: int = 0, struct_type: object | None = None, format: sgl.Format = Format.undefined, memory_type: sgl.MemoryType = MemoryType.device_local, usage: sgl.BufferUsage = BufferUsage.none, label: str = '', data: numpy.ndarray[] | None = None) -> sgl.Buffer
    
        Create a new buffer.
        
        Parameter ``size``:
            Buffer size in bytes.
        
        Parameter ``element_count``:
            Buffer size in number of struct elements. Can be used instead of
            ``size``.
        
        Parameter ``struct_size``:
            Struct size in bytes.
        
        Parameter ``struct_type``:
            Struct type. Can be used instead of ``struct_size`` to specify the
            size of the struct.
        
        Parameter ``format``:
            Buffer format. Used when creating typed buffer views.
        
        Parameter ``initial_state``:
            Initial resource state.
        
        Parameter ``usage``:
            Resource usage flags.
        
        Parameter ``memory_type``:
            Memory type.
        
        Parameter ``label``:
            Debug label.
        
        Parameter ``data``:
            Initial data to upload to the buffer.
        
        Parameter ``data_size``:
            Size of the initial data in bytes.
        
        Returns:
            New buffer object.
        
    .. py:method:: create_buffer(self, desc: sgl.BufferDesc) -> sgl.Buffer
        :no-index:
    
    .. py:method:: create_texture(self, type: sgl.TextureType = TextureType.texture_2d, format: sgl.Format = Format.undefined, width: int = 1, height: int = 1, depth: int = 1, array_length: int = 1, mip_count: int = 0, sample_count: int = 1, sample_quality: int = 0, memory_type: sgl.MemoryType = MemoryType.device_local, usage: sgl.TextureUsage = TextureUsage.none, label: str = '', data: numpy.ndarray[] | None = None) -> sgl.Texture
    
        Create a new texture.
        
        Parameter ``type``:
            Texture type.
        
        Parameter ``format``:
            Texture format.
        
        Parameter ``width``:
            Width in pixels.
        
        Parameter ``height``:
            Height in pixels.
        
        Parameter ``depth``:
            Depth in pixels.
        
        Parameter ``array_length``:
            Array length.
        
        Parameter ``mip_count``:
            Mip level count. Number of mip levels (0 for auto-generated mips).
        
        Parameter ``sample_count``:
            Number of samples for multisampled textures.
        
        Parameter ``quality``:
            Quality level for multisampled textures.
        
        Parameter ``usage``:
            Resource usage.
        
        Parameter ``memory_type``:
            Memory type.
        
        Parameter ``label``:
            Debug label.
        
        Parameter ``data``:
            Initial data.
        
        Returns:
            New texture object.
        
    .. py:method:: create_texture(self, desc: sgl.TextureDesc) -> sgl.Texture
        :no-index:
    
    .. py:method:: create_sampler(self, min_filter: sgl.TextureFilteringMode = TextureFilteringMode.linear, mag_filter: sgl.TextureFilteringMode = TextureFilteringMode.linear, mip_filter: sgl.TextureFilteringMode = TextureFilteringMode.linear, reduction_op: sgl.TextureReductionOp = TextureReductionOp.average, address_u: sgl.TextureAddressingMode = TextureAddressingMode.wrap, address_v: sgl.TextureAddressingMode = TextureAddressingMode.wrap, address_w: sgl.TextureAddressingMode = TextureAddressingMode.wrap, mip_lod_bias: float = 0.0, max_anisotropy: int = 1, comparison_func: sgl.ComparisonFunc = ComparisonFunc.never, border_color: sgl.math.float4 = {1, 1, 1, 1}, min_lod: float = -1000.0, max_lod: float = 1000.0) -> sgl.Sampler
    
        Create a new sampler.
        
        Parameter ``min_filter``:
            Minification filter.
        
        Parameter ``mag_filter``:
            Magnification filter.
        
        Parameter ``mip_filter``:
            Mip-map filter.
        
        Parameter ``reduction_op``:
            Reduction operation.
        
        Parameter ``address_u``:
            Texture addressing mode for the U coordinate.
        
        Parameter ``address_v``:
            Texture addressing mode for the V coordinate.
        
        Parameter ``address_w``:
            Texture addressing mode for the W coordinate.
        
        Parameter ``mip_lod_bias``:
            Mip-map LOD bias.
        
        Parameter ``max_anisotropy``:
            Maximum anisotropy.
        
        Parameter ``comparison_func``:
            Comparison function.
        
        Parameter ``border_color``:
            Border color.
        
        Parameter ``min_lod``:
            Minimum LOD level.
        
        Parameter ``max_lod``:
            Maximum LOD level.
        
        Parameter ``label``:
            Debug label.
        
        Returns:
            New sampler object.
        
    .. py:method:: create_sampler(self, desc: sgl.SamplerDesc) -> sgl.Sampler
        :no-index:
    
    .. py:method:: create_fence(self, initial_value: int = 0, shared: bool = False) -> sgl.Fence
    
        Create a new fence.
        
        Parameter ``initial_value``:
            Initial fence value.
        
        Parameter ``shared``:
            Create a shared fence.
        
        Returns:
            New fence object.
        
    .. py:method:: create_fence(self, desc: sgl.FenceDesc) -> sgl.Fence
        :no-index:
    
    .. py:method:: create_query_pool(self, type: sgl.QueryType, count: int) -> sgl.QueryPool
    
        Create a new query pool.
        
        Parameter ``type``:
            Query type.
        
        Parameter ``count``:
            Number of queries in the pool.
        
        Returns:
            New query pool object.
        
    .. py:method:: create_input_layout(self, input_elements: collections.abc.Sequence[sgl.InputElementDesc], vertex_streams: collections.abc.Sequence[sgl.VertexStreamDesc]) -> sgl.InputLayout
    
        Create a new input layout.
        
        Parameter ``input_elements``:
            List of input elements (see InputElementDesc for details).
        
        Parameter ``vertex_streams``:
            List of vertex streams (see VertexStreamDesc for details).
        
        Returns:
            New input layout object.
        
    .. py:method:: create_input_layout(self, desc: sgl.InputLayoutDesc) -> sgl.InputLayout
        :no-index:
    
    .. py:method:: create_command_encoder(self, queue: sgl.CommandQueueType = CommandQueueType.graphics) -> sgl.CommandEncoder
    
    .. py:method:: submit_command_buffer(self, command_buffer: sgl.CommandBuffer, queue: sgl.CommandQueueType = CommandQueueType.graphics) -> int
    
        Submit a command buffer to the device.
        
        The returned submission ID can be used to wait for the command buffer
        to complete.
        
        Parameter ``command_buffer``:
            Command buffer to submit.
        
        Parameter ``queue``:
            Command queue to submit to.
        
        Returns:
            Submission ID.
        
    .. py:method:: is_command_buffer_complete(self, id: int) -> bool
    
        Check if a command buffer is complete.
        
        Parameter ``id``:
            Submission ID.
        
        Returns:
            True if the command buffer is complete.
        
    .. py:method:: wait_command_buffer(self, id: int) -> None
    
        Wait for a command buffer to complete.
        
        Parameter ``id``:
            Submission ID.
        
    .. py:method:: wait_for_idle(self, queue: sgl.CommandQueueType = CommandQueueType.graphics) -> None
    
        Wait for the command queue to be idle.
        
        Parameter ``queue``:
            Command queue to wait for.
        
    .. py:method:: sync_to_cuda(self, cuda_stream: int = 0) -> None
    
        Synchronize CUDA -> device.
        
        This signals a shared CUDA semaphore from the CUDA stream and then
        waits for the signal on the command queue.
        
        Parameter ``cuda_stream``:
            CUDA stream
        
    .. py:method:: sync_to_device(self, cuda_stream: int = 0) -> None
    
        Synchronize device -> CUDA.
        
        This waits for a shared CUDA semaphore on the CUDA stream, making sure
        all commands on the device have completed.
        
        Parameter ``cuda_stream``:
            CUDA stream
        
    .. py:method:: get_acceleration_structure_sizes(self, desc: sgl.AccelerationStructureBuildDesc) -> sgl.AccelerationStructureSizes
    
        Query the device for buffer sizes required for acceleration structure
        builds.
        
        Parameter ``desc``:
            Acceleration structure build description.
        
        Returns:
            Acceleration structure sizes.
        
    .. py:method:: create_acceleration_structure(self, size: int = 0, label: str = '') -> sgl.AccelerationStructure
    
    .. py:method:: create_acceleration_structure(self, desc: sgl.AccelerationStructureDesc) -> sgl.AccelerationStructure
        :no-index:
    
    .. py:method:: create_acceleration_structure_instance_list(self, size: int) -> sgl.AccelerationStructureInstanceList
    
    .. py:method:: create_shader_table(self, program: sgl.ShaderProgram, ray_gen_entry_points: collections.abc.Sequence[str] = [], miss_entry_points: collections.abc.Sequence[str] = [], hit_group_names: collections.abc.Sequence[str] = [], callable_entry_points: collections.abc.Sequence[str] = []) -> sgl.ShaderTable
    
    .. py:method:: create_shader_table(self, desc: sgl.ShaderTableDesc) -> sgl.ShaderTable
        :no-index:
    
    .. py:method:: create_slang_session(self, compiler_options: sgl.SlangCompilerOptions | None = None, add_default_include_paths: bool = True, cache_path: str | os.PathLike | None = None) -> sgl.SlangSession
    
        Create a new slang session.
        
        Parameter ``compiler_options``:
            Compiler options (see SlangCompilerOptions for details).
        
        Returns:
            New slang session object.
        
    .. py:method:: reload_all_programs(self) -> None
    
    .. py:method:: load_module(self, module_name: str) -> sgl.SlangModule
    
    .. py:method:: load_module_from_source(self, module_name: str, source: str, path: str | os.PathLike | None = None) -> sgl.SlangModule
    
    .. py:method:: link_program(self, modules: collections.abc.Sequence[sgl.SlangModule], entry_points: collections.abc.Sequence[sgl.SlangEntryPoint], link_options: sgl.SlangLinkOptions | None = None) -> sgl.ShaderProgram
    
    .. py:method:: load_program(self, module_name: str, entry_point_names: collections.abc.Sequence[str], additional_source: str | None = None, link_options: sgl.SlangLinkOptions | None = None) -> sgl.ShaderProgram
    
    .. py:method:: create_root_shader_object(self, shader_program: sgl.ShaderProgram) -> sgl.ShaderObject
    
    .. py:method:: create_shader_object(self, type_layout: sgl.TypeLayoutReflection) -> sgl.ShaderObject
    
    .. py:method:: create_shader_object(self, cursor: sgl.ReflectionCursor) -> sgl.ShaderObject
        :no-index:
    
    .. py:method:: create_compute_pipeline(self, program: sgl.ShaderProgram) -> sgl.ComputePipeline
    
    .. py:method:: create_compute_pipeline(self, desc: sgl.ComputePipelineDesc) -> sgl.ComputePipeline
        :no-index:
    
    .. py:method:: create_render_pipeline(self, program: sgl.ShaderProgram, input_layout: sgl.InputLayout | None, primitive_topology: sgl.PrimitiveTopology = PrimitiveTopology.triangle_list, targets: collections.abc.Sequence[sgl.ColorTargetDesc] = [], depth_stencil: sgl.DepthStencilDesc | None = None, rasterizer: sgl.RasterizerDesc | None = None, multisample: sgl.MultisampleDesc | None = None) -> sgl.RenderPipeline
    
    .. py:method:: create_render_pipeline(self, desc: sgl.RenderPipelineDesc) -> sgl.RenderPipeline
        :no-index:
    
    .. py:method:: create_ray_tracing_pipeline(self, program: sgl.ShaderProgram, hit_groups: collections.abc.Sequence[sgl.HitGroupDesc], max_recursion: int = 0, max_ray_payload_size: int = 0, max_attribute_size: int = 8, flags: sgl.RayTracingPipelineFlags = RayTracingPipelineFlags.none) -> sgl.RayTracingPipeline
    
    .. py:method:: create_ray_tracing_pipeline(self, desc: sgl.RayTracingPipelineDesc) -> sgl.RayTracingPipeline
        :no-index:
    
    .. py:method:: create_compute_kernel(self, program: sgl.ShaderProgram) -> sgl.ComputeKernel
    
    .. py:method:: create_compute_kernel(self, desc: sgl.ComputeKernelDesc) -> sgl.ComputeKernel
        :no-index:
    
    .. py:method:: flush_print(self) -> None
    
        Block and flush all shader side debug print output.
        
    .. py:method:: flush_print_to_string(self) -> str
    
        Block and flush all shader side debug print output to a string.
        
    .. py:method:: run_garbage_collection(self) -> None
    
        Execute garbage collection.
        
        This function should be called regularly to execute deferred releases
        (at least once a frame).
        
    .. py:method:: wait(self) -> None
    
        Wait for all device work to complete.
        
    .. py:method:: register_shader_hot_reload_callback(self, callback: collections.abc.Callable[[sgl.ShaderHotReloadEvent], None]) -> None
    
        Register a hot reload hook, called immediately after any module is
        reloaded.
        
    .. py:method:: register_device_close_callback(self, callback: collections.abc.Callable[[sgl.Device], None]) -> None
    
        Register a device close callback, called at start of device close.
        
    .. py:method:: coopvec_query_matrix_size(self, rows: int, cols: int, layout: sgl.CoopVecMatrixLayout, element_type: sgl.DataType) -> int
    
        N/A
        
    .. py:method:: coopvec_create_matrix_desc(self, rows: int, cols: int, layout: sgl.CoopVecMatrixLayout, element_type: sgl.DataType, offset: int = 0) -> sgl.CoopVecMatrixDesc
    
        N/A
        
    .. py:method:: coopvec_convert_matrix_host(self, src: ndarray[device='cpu'], dst: ndarray[device='cpu'], src_layout: sgl.CoopVecMatrixLayout | None = None, dst_layout: sgl.CoopVecMatrixLayout | None = None) -> int
    
        N/A
        
    .. py:method:: coopvec_convert_matrix_device(self, src: sgl.Buffer, src_desc: sgl.CoopVecMatrixDesc, dst: sgl.Buffer, dst_desc: sgl.CoopVecMatrixDesc, encoder: sgl.CommandEncoder | None = None) -> None
    
        N/A
        
    .. py:method:: coopvec_convert_matrix_device(self, src: sgl.Buffer, src_desc: collections.abc.Sequence[sgl.CoopVecMatrixDesc], dst: sgl.Buffer, dst_desc: collections.abc.Sequence[sgl.CoopVecMatrixDesc], encoder: sgl.CommandEncoder | None = None) -> None
        :no-index:
    
    .. py:method:: coopvec_align_matrix_offset(self, offset: int) -> int
    
        N/A
        
    .. py:method:: coopvec_align_vector_offset(self, offset: int) -> int
    
        N/A
        
    .. py:staticmethod:: enumerate_adapters(type: sgl.DeviceType = DeviceType.automatic) -> list[sgl.AdapterInfo]
    
        Enumerates all available adapters of a given device type.
        
    .. py:staticmethod:: report_live_objects() -> None
    
        Report live objects in the slang/gfx layer. This is useful for
        checking clean shutdown with all resources released properly.
        


----

.. py:class:: sgl.DeviceDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: type
        :type: sgl.DeviceType
    
        The type of the device.
        
    .. py:property:: enable_debug_layers
        :type: bool
    
        Enable debug layers.
        
    .. py:property:: enable_cuda_interop
        :type: bool
    
        Enable CUDA interoperability.
        
    .. py:property:: enable_print
        :type: bool
    
        Enable device side printing (adds performance overhead).
        
    .. py:property:: enable_hot_reload
        :type: bool
    
        Adapter LUID to select adapter on which the device will be created.
        
    .. py:property:: adapter_luid
        :type: list[int] | None
    
        Adapter LUID to select adapter on which the device will be created.
        
    .. py:property:: compiler_options
        :type: sgl.SlangCompilerOptions
    
        Compiler options (used for default slang session).
        
    .. py:property:: shader_cache_path
        :type: pathlib.Path | None
    
        Path to the shader cache directory (optional). If a relative path is
        used, the cache is stored in the application data directory.
        


----

.. py:class:: sgl.DeviceInfo

    
    
    .. py:property:: type
        :type: sgl.DeviceType
    
        The type of the device.
        
    .. py:property:: api_name
        :type: str
    
        The name of the graphics API being used by this device.
        
    .. py:property:: adapter_name
        :type: str
    
        The name of the graphics adapter.
        
    .. py:property:: timestamp_frequency
        :type: int
    
        The frequency of the timestamp counter. To resolve a timestamp to
        seconds, divide by this value.
        
    .. py:property:: limits
        :type: sgl.DeviceLimits
    
        Limits of the device.
        


----

.. py:class:: sgl.DeviceLimits

    
    
    .. py:property:: max_texture_dimension_1d
        :type: int
    
        Maximum dimension for 1D textures.
        
    .. py:property:: max_texture_dimension_2d
        :type: int
    
        Maximum dimensions for 2D textures.
        
    .. py:property:: max_texture_dimension_3d
        :type: int
    
        Maximum dimensions for 3D textures.
        
    .. py:property:: max_texture_dimension_cube
        :type: int
    
        Maximum dimensions for cube textures.
        
    .. py:property:: max_texture_array_layers
        :type: int
    
        Maximum number of texture layers.
        
    .. py:property:: max_vertex_input_elements
        :type: int
    
        Maximum number of vertex input elements in a graphics pipeline.
        
    .. py:property:: max_vertex_input_element_offset
        :type: int
    
        Maximum offset of a vertex input element in the vertex stream.
        
    .. py:property:: max_vertex_streams
        :type: int
    
        Maximum number of vertex streams in a graphics pipeline.
        
    .. py:property:: max_vertex_stream_stride
        :type: int
    
        Maximum stride of a vertex stream.
        
    .. py:property:: max_compute_threads_per_group
        :type: int
    
        Maximum number of threads per thread group.
        
    .. py:property:: max_compute_thread_group_size
        :type: sgl.math.uint3
    
        Maximum dimensions of a thread group.
        
    .. py:property:: max_compute_dispatch_thread_groups
        :type: sgl.math.uint3
    
        Maximum number of thread groups per dimension in a single dispatch.
        
    .. py:property:: max_viewports
        :type: int
    
        Maximum number of viewports per pipeline.
        
    .. py:property:: max_viewport_dimensions
        :type: sgl.math.uint2
    
        Maximum viewport dimensions.
        
    .. py:property:: max_framebuffer_dimensions
        :type: sgl.math.uint3
    
        Maximum framebuffer dimensions.
        
    .. py:property:: max_shader_visible_samplers
        :type: int
    
        Maximum samplers visible in a shader stage.
        


----

.. py:class:: sgl.DeviceResource

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:class:: sgl.DeviceResource.MemoryUsage
    
        
        
        .. py:property:: device
            :type: int
        
            The amount of memory in bytes used on the device.
            
        .. py:property:: host
            :type: int
        
            The amount of memory in bytes used on the host.
            
    .. py:property:: device
        :type: sgl.Device
    
    .. py:property:: memory_usage
        :type: sgl.DeviceResource.MemoryUsage
    
        The memory usage by this resource.
        


----

.. py:class:: sgl.DeviceType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.DeviceType.automatic
        :type: DeviceType
        :value: DeviceType.automatic
    
    .. py:attribute:: sgl.DeviceType.d3d12
        :type: DeviceType
        :value: DeviceType.d3d12
    
    .. py:attribute:: sgl.DeviceType.vulkan
        :type: DeviceType
        :value: DeviceType.vulkan
    
    .. py:attribute:: sgl.DeviceType.metal
        :type: DeviceType
        :value: DeviceType.metal
    
    .. py:attribute:: sgl.DeviceType.wgpu
        :type: DeviceType
        :value: DeviceType.wgpu
    
    .. py:attribute:: sgl.DeviceType.cpu
        :type: DeviceType
        :value: DeviceType.cpu
    
    .. py:attribute:: sgl.DeviceType.cuda
        :type: DeviceType
        :value: DeviceType.cuda
    


----

.. py:class:: sgl.DrawArguments

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: vertex_count
        :type: int
    
    .. py:property:: instance_count
        :type: int
    
    .. py:property:: start_vertex_location
        :type: int
    
    .. py:property:: start_instance_location
        :type: int
    
    .. py:property:: start_index_location
        :type: int
    


----

.. py:class:: sgl.EntryPointLayout

    Base class: :py:class:`sgl.BaseReflectionObject`
    
    
    
    .. py:property:: name
        :type: str
    
    .. py:property:: name_override
        :type: str
    
    .. py:property:: stage
        :type: sgl.ShaderStage
    
    .. py:property:: compute_thread_group_size
        :type: sgl.math.uint3
    
    .. py:property:: parameters
        :type: sgl.EntryPointLayoutParameterList
    


----

.. py:class:: sgl.EntryPointLayoutParameterList

    EntryPointLayout lazy parameter list evaluation.
    


----

.. py:class:: sgl.Fence

    Base class: :py:class:`sgl.DeviceResource`
    
    Fence.
    
    .. py:property:: desc
        :type: sgl.FenceDesc
    
    .. py:method:: signal(self, value: int = 18446744073709551615) -> int
    
        Signal the fence. This signals the fence from the host.
        
        Parameter ``value``:
            The value to signal. If ``AUTO``, the signaled value will be auto-
            incremented.
        
        Returns:
            The signaled value.
        
    .. py:method:: wait(self, value: int = 18446744073709551615, timeout_ns: int = 18446744073709551615) -> None
    
        Wait for the fence to be signaled on the host. Blocks the host until
        the fence reaches or exceeds the specified value.
        
        Parameter ``value``:
            The value to wait for. If ``AUTO``, wait for the last signaled
            value.
        
        Parameter ``timeout_ns``:
            The timeout in nanoseconds. If ``TIMEOUT_INFINITE``, the function
            will block indefinitely.
        
    .. py:property:: current_value
        :type: int
    
        Returns the currently signaled value on the device.
        
    .. py:property:: signaled_value
        :type: int
    
        Returns the last signaled value on the device.
        
    .. py:attribute:: sgl.Fence.AUTO
        :type: int
        :value: 18446744073709551615
    
    .. py:attribute:: sgl.Fence.TIMEOUT_INFINITE
        :type: int
        :value: 18446744073709551615
    


----

.. py:class:: sgl.FenceDesc

    Fence descriptor.
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: initial_value
        :type: int
    
        Initial fence value.
        
    .. py:property:: shared
        :type: bool
    
        Create a shared fence.
        


----

.. py:class:: sgl.FillMode

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.FillMode.solid
        :type: FillMode
        :value: FillMode.solid
    
    .. py:attribute:: sgl.FillMode.wireframe
        :type: FillMode
        :value: FillMode.wireframe
    


----

.. py:class:: sgl.Format

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.Format.undefined
        :type: Format
        :value: Format.undefined
    
    .. py:attribute:: sgl.Format.r8_uint
        :type: Format
        :value: Format.r8_uint
    
    .. py:attribute:: sgl.Format.r8_sint
        :type: Format
        :value: Format.r8_sint
    
    .. py:attribute:: sgl.Format.r8_unorm
        :type: Format
        :value: Format.r8_unorm
    
    .. py:attribute:: sgl.Format.r8_snorm
        :type: Format
        :value: Format.r8_snorm
    
    .. py:attribute:: sgl.Format.rg8_uint
        :type: Format
        :value: Format.rg8_uint
    
    .. py:attribute:: sgl.Format.rg8_sint
        :type: Format
        :value: Format.rg8_sint
    
    .. py:attribute:: sgl.Format.rg8_unorm
        :type: Format
        :value: Format.rg8_unorm
    
    .. py:attribute:: sgl.Format.rg8_snorm
        :type: Format
        :value: Format.rg8_snorm
    
    .. py:attribute:: sgl.Format.rgba8_uint
        :type: Format
        :value: Format.rgba8_uint
    
    .. py:attribute:: sgl.Format.rgba8_sint
        :type: Format
        :value: Format.rgba8_sint
    
    .. py:attribute:: sgl.Format.rgba8_unorm
        :type: Format
        :value: Format.rgba8_unorm
    
    .. py:attribute:: sgl.Format.rgba8_unorm_srgb
        :type: Format
        :value: Format.rgba8_unorm_srgb
    
    .. py:attribute:: sgl.Format.rgba8_snorm
        :type: Format
        :value: Format.rgba8_snorm
    
    .. py:attribute:: sgl.Format.bgra8_unorm
        :type: Format
        :value: Format.bgra8_unorm
    
    .. py:attribute:: sgl.Format.bgra8_unorm_srgb
        :type: Format
        :value: Format.bgra8_unorm_srgb
    
    .. py:attribute:: sgl.Format.bgrx8_unorm
        :type: Format
        :value: Format.bgrx8_unorm
    
    .. py:attribute:: sgl.Format.bgrx8_unorm_srgb
        :type: Format
        :value: Format.bgrx8_unorm_srgb
    
    .. py:attribute:: sgl.Format.r16_uint
        :type: Format
        :value: Format.r16_uint
    
    .. py:attribute:: sgl.Format.r16_sint
        :type: Format
        :value: Format.r16_sint
    
    .. py:attribute:: sgl.Format.r16_unorm
        :type: Format
        :value: Format.r16_unorm
    
    .. py:attribute:: sgl.Format.r16_snorm
        :type: Format
        :value: Format.r16_snorm
    
    .. py:attribute:: sgl.Format.r16_float
        :type: Format
        :value: Format.r16_float
    
    .. py:attribute:: sgl.Format.rg16_uint
        :type: Format
        :value: Format.rg16_uint
    
    .. py:attribute:: sgl.Format.rg16_sint
        :type: Format
        :value: Format.rg16_sint
    
    .. py:attribute:: sgl.Format.rg16_unorm
        :type: Format
        :value: Format.rg16_unorm
    
    .. py:attribute:: sgl.Format.rg16_snorm
        :type: Format
        :value: Format.rg16_snorm
    
    .. py:attribute:: sgl.Format.rg16_float
        :type: Format
        :value: Format.rg16_float
    
    .. py:attribute:: sgl.Format.rgba16_uint
        :type: Format
        :value: Format.rgba16_uint
    
    .. py:attribute:: sgl.Format.rgba16_sint
        :type: Format
        :value: Format.rgba16_sint
    
    .. py:attribute:: sgl.Format.rgba16_unorm
        :type: Format
        :value: Format.rgba16_unorm
    
    .. py:attribute:: sgl.Format.rgba16_snorm
        :type: Format
        :value: Format.rgba16_snorm
    
    .. py:attribute:: sgl.Format.rgba16_float
        :type: Format
        :value: Format.rgba16_float
    
    .. py:attribute:: sgl.Format.r32_uint
        :type: Format
        :value: Format.r32_uint
    
    .. py:attribute:: sgl.Format.r32_sint
        :type: Format
        :value: Format.r32_sint
    
    .. py:attribute:: sgl.Format.r32_float
        :type: Format
        :value: Format.r32_float
    
    .. py:attribute:: sgl.Format.rg32_uint
        :type: Format
        :value: Format.rg32_uint
    
    .. py:attribute:: sgl.Format.rg32_sint
        :type: Format
        :value: Format.rg32_sint
    
    .. py:attribute:: sgl.Format.rg32_float
        :type: Format
        :value: Format.rg32_float
    
    .. py:attribute:: sgl.Format.rgb32_uint
        :type: Format
        :value: Format.rgb32_uint
    
    .. py:attribute:: sgl.Format.rgb32_sint
        :type: Format
        :value: Format.rgb32_sint
    
    .. py:attribute:: sgl.Format.rgb32_float
        :type: Format
        :value: Format.rgb32_float
    
    .. py:attribute:: sgl.Format.rgba32_uint
        :type: Format
        :value: Format.rgba32_uint
    
    .. py:attribute:: sgl.Format.rgba32_sint
        :type: Format
        :value: Format.rgba32_sint
    
    .. py:attribute:: sgl.Format.rgba32_float
        :type: Format
        :value: Format.rgba32_float
    
    .. py:attribute:: sgl.Format.r64_uint
        :type: Format
        :value: Format.r64_uint
    
    .. py:attribute:: sgl.Format.r64_sint
        :type: Format
        :value: Format.r64_sint
    
    .. py:attribute:: sgl.Format.bgra4_unorm
        :type: Format
        :value: Format.bgra4_unorm
    
    .. py:attribute:: sgl.Format.b5g6r5_unorm
        :type: Format
        :value: Format.b5g6r5_unorm
    
    .. py:attribute:: sgl.Format.bgr5a1_unorm
        :type: Format
        :value: Format.bgr5a1_unorm
    
    .. py:attribute:: sgl.Format.rgb9e5_ufloat
        :type: Format
        :value: Format.rgb9e5_ufloat
    
    .. py:attribute:: sgl.Format.rgb10a2_uint
        :type: Format
        :value: Format.rgb10a2_uint
    
    .. py:attribute:: sgl.Format.rgb10a2_unorm
        :type: Format
        :value: Format.rgb10a2_unorm
    
    .. py:attribute:: sgl.Format.r11g11b10_float
        :type: Format
        :value: Format.r11g11b10_float
    
    .. py:attribute:: sgl.Format.d32_float
        :type: Format
        :value: Format.d32_float
    
    .. py:attribute:: sgl.Format.d16_unorm
        :type: Format
        :value: Format.d16_unorm
    
    .. py:attribute:: sgl.Format.d32_float_s8_uint
        :type: Format
        :value: Format.d32_float_s8_uint
    
    .. py:attribute:: sgl.Format.bc1_unorm
        :type: Format
        :value: Format.bc1_unorm
    
    .. py:attribute:: sgl.Format.bc1_unorm_srgb
        :type: Format
        :value: Format.bc1_unorm_srgb
    
    .. py:attribute:: sgl.Format.bc2_unorm
        :type: Format
        :value: Format.bc2_unorm
    
    .. py:attribute:: sgl.Format.bc2_unorm_srgb
        :type: Format
        :value: Format.bc2_unorm_srgb
    
    .. py:attribute:: sgl.Format.bc3_unorm
        :type: Format
        :value: Format.bc3_unorm
    
    .. py:attribute:: sgl.Format.bc3_unorm_srgb
        :type: Format
        :value: Format.bc3_unorm_srgb
    
    .. py:attribute:: sgl.Format.bc4_unorm
        :type: Format
        :value: Format.bc4_unorm
    
    .. py:attribute:: sgl.Format.bc4_snorm
        :type: Format
        :value: Format.bc4_snorm
    
    .. py:attribute:: sgl.Format.bc5_unorm
        :type: Format
        :value: Format.bc5_unorm
    
    .. py:attribute:: sgl.Format.bc5_snorm
        :type: Format
        :value: Format.bc5_snorm
    
    .. py:attribute:: sgl.Format.bc6h_ufloat
        :type: Format
        :value: Format.bc6h_ufloat
    
    .. py:attribute:: sgl.Format.bc6h_sfloat
        :type: Format
        :value: Format.bc6h_sfloat
    
    .. py:attribute:: sgl.Format.bc7_unorm
        :type: Format
        :value: Format.bc7_unorm
    
    .. py:attribute:: sgl.Format.bc7_unorm_srgb
        :type: Format
        :value: Format.bc7_unorm_srgb
    


----

.. py:class:: sgl.FormatChannels

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.FormatChannels.none
        :type: FormatChannels
        :value: FormatChannels.none
    
    .. py:attribute:: sgl.FormatChannels.r
        :type: FormatChannels
        :value: FormatChannels.r
    
    .. py:attribute:: sgl.FormatChannels.g
        :type: FormatChannels
        :value: FormatChannels.g
    
    .. py:attribute:: sgl.FormatChannels.b
        :type: FormatChannels
        :value: FormatChannels.b
    
    .. py:attribute:: sgl.FormatChannels.a
        :type: FormatChannels
        :value: FormatChannels.a
    
    .. py:attribute:: sgl.FormatChannels.rg
        :type: FormatChannels
        :value: FormatChannels.rg
    
    .. py:attribute:: sgl.FormatChannels.rgb
        :type: FormatChannels
        :value: FormatChannels.rgb
    
    .. py:attribute:: sgl.FormatChannels.rgba
        :type: FormatChannels
        :value: FormatChannels.rgba
    


----

.. py:class:: sgl.FormatInfo

    Resource format information.
    
    .. py:property:: format
        :type: sgl.Format
    
        Resource format.
        
    .. py:property:: name
        :type: str
    
        Format name.
        
    .. py:property:: bytes_per_block
        :type: int
    
        Number of bytes per block (compressed) or pixel (uncompressed).
        
    .. py:property:: channel_count
        :type: int
    
        Number of channels.
        
    .. py:property:: type
        :type: sgl.FormatType
    
        Format type (typeless, float, unorm, unorm_srgb, snorm, uint, sint).
        
    .. py:property:: is_depth
        :type: bool
    
        True if format has a depth component.
        
    .. py:property:: is_stencil
        :type: bool
    
        True if format has a stencil component.
        
    .. py:property:: is_compressed
        :type: bool
    
        True if format is compressed.
        
    .. py:property:: block_width
        :type: int
    
        Block width for compressed formats (1 for uncompressed formats).
        
    .. py:property:: block_height
        :type: int
    
        Block height for compressed formats (1 for uncompressed formats).
        
    .. py:property:: channel_bit_count
        :type: list[int]
    
        Number of bits per channel.
        
    .. py:property:: dxgi_format
        :type: int
    
        DXGI format.
        
    .. py:property:: vk_format
        :type: int
    
        Vulkan format.
        
    .. py:method:: is_depth_stencil(self) -> bool
    
        True if format has a depth or stencil component.
        
    .. py:method:: is_float_format(self) -> bool
    
        True if format is floating point.
        
    .. py:method:: is_integer_format(self) -> bool
    
        True if format is integer.
        
    .. py:method:: is_normalized_format(self) -> bool
    
        True if format is normalized.
        
    .. py:method:: is_srgb_format(self) -> bool
    
        True if format is sRGB.
        
    .. py:method:: get_channels(self) -> sgl.FormatChannels
    
        Get the channels for the format (only for color formats).
        
    .. py:method:: get_channel_bits(self, arg: sgl.FormatChannels, /) -> int
    
        Get the number of bits for the specified channels.
        
    .. py:method:: has_equal_channel_bits(self) -> bool
    
        Check if all channels have the same number of bits.
        


----

.. py:class:: sgl.FormatSupport

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.FormatSupport.none
        :type: FormatSupport
        :value: FormatSupport.none
    
    .. py:attribute:: sgl.FormatSupport.buffer
        :type: FormatSupport
        :value: FormatSupport.buffer
    
    .. py:attribute:: sgl.FormatSupport.index_buffer
        :type: FormatSupport
        :value: FormatSupport.index_buffer
    
    .. py:attribute:: sgl.FormatSupport.vertex_buffer
        :type: FormatSupport
        :value: FormatSupport.vertex_buffer
    
    .. py:attribute:: sgl.FormatSupport.texture
        :type: FormatSupport
        :value: FormatSupport.texture
    
    .. py:attribute:: sgl.FormatSupport.depth_stencil
        :type: FormatSupport
        :value: FormatSupport.depth_stencil
    
    .. py:attribute:: sgl.FormatSupport.render_target
        :type: FormatSupport
        :value: FormatSupport.render_target
    
    .. py:attribute:: sgl.FormatSupport.blendable
        :type: FormatSupport
        :value: FormatSupport.blendable
    
    .. py:attribute:: sgl.FormatSupport.shader_load
        :type: FormatSupport
        :value: FormatSupport.shader_load
    
    .. py:attribute:: sgl.FormatSupport.shader_sample
        :type: FormatSupport
        :value: FormatSupport.shader_sample
    
    .. py:attribute:: sgl.FormatSupport.shader_uav_load
        :type: FormatSupport
        :value: FormatSupport.shader_uav_load
    
    .. py:attribute:: sgl.FormatSupport.shader_uav_store
        :type: FormatSupport
        :value: FormatSupport.shader_uav_store
    
    .. py:attribute:: sgl.FormatSupport.shader_atomic
        :type: FormatSupport
        :value: FormatSupport.shader_atomic
    


----

.. py:class:: sgl.FormatType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.FormatType.unknown
        :type: FormatType
        :value: FormatType.unknown
    
    .. py:attribute:: sgl.FormatType.float
        :type: FormatType
        :value: FormatType.float
    
    .. py:attribute:: sgl.FormatType.unorm
        :type: FormatType
        :value: FormatType.unorm
    
    .. py:attribute:: sgl.FormatType.unorm_srgb
        :type: FormatType
        :value: FormatType.unorm_srgb
    
    .. py:attribute:: sgl.FormatType.snorm
        :type: FormatType
        :value: FormatType.snorm
    
    .. py:attribute:: sgl.FormatType.uint
        :type: FormatType
        :value: FormatType.uint
    
    .. py:attribute:: sgl.FormatType.sint
        :type: FormatType
        :value: FormatType.sint
    


----

.. py:class:: sgl.FrontFaceMode

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.FrontFaceMode.counter_clockwise
        :type: FrontFaceMode
        :value: FrontFaceMode.counter_clockwise
    
    .. py:attribute:: sgl.FrontFaceMode.clockwise
        :type: FrontFaceMode
        :value: FrontFaceMode.clockwise
    


----

.. py:class:: sgl.FunctionReflection

    Base class: :py:class:`sgl.BaseReflectionObject`
    
    
    
    .. py:property:: name
        :type: str
    
        Function name.
        
    .. py:property:: return_type
        :type: sgl.TypeReflection
    
        Function return type.
        
    .. py:property:: parameters
        :type: sgl.FunctionReflectionParameterList
    
        List of all function parameters.
        
    .. py:method:: has_modifier(self, modifier: sgl.ModifierID) -> bool
    
        Check if the function has a given modifier (e.g. 'differentiable').
        
    .. py:method:: specialize_with_arg_types(self, types: collections.abc.Sequence[sgl.TypeReflection]) -> sgl.FunctionReflection
    
        Specialize a generic or interface based function with a set of
        concrete argument types. Calling on a none-generic/interface function
        will simply validate all argument types can be implicitly converted to
        their respective parameter types. Where a function contains multiple
        overloads, specialize will identify the correct overload based on the
        arguments.
        
    .. py:property:: is_overloaded
        :type: bool
    
        Check whether this function object represents a group of overloaded
        functions, accessible via the overloads list.
        
    .. py:property:: overloads
        :type: sgl.FunctionReflectionOverloadList
    
        List of all overloads of this function.
        


----

.. py:class:: sgl.FunctionReflectionOverloadList

    FunctionReflection lazy overload list evaluation.
    


----

.. py:class:: sgl.FunctionReflectionParameterList

    FunctionReflection lazy parameter list evaluation.
    


----

.. py:function:: sgl.get_format_info(arg: sgl.Format, /) -> sgl.FormatInfo



----

.. py:class:: sgl.HitGroupDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:method:: __init__(self, hit_group_name: str, closest_hit_entry_point: str = '', any_hit_entry_point: str = '', intersection_entry_point: str = '') -> None
        :no-index:
    
    .. py:property:: hit_group_name
        :type: str
    
    .. py:property:: closest_hit_entry_point
        :type: str
    
    .. py:property:: any_hit_entry_point
        :type: str
    
    .. py:property:: intersection_entry_point
        :type: str
    


----

.. py:class:: sgl.IndexFormat

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.IndexFormat.uint16
        :type: IndexFormat
        :value: IndexFormat.uint16
    
    .. py:attribute:: sgl.IndexFormat.uint32
        :type: IndexFormat
        :value: IndexFormat.uint32
    


----

.. py:class:: sgl.InputElementDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: semantic_name
        :type: str
    
        The name of the corresponding parameter in shader code.
        
    .. py:property:: semantic_index
        :type: int
    
        The index of the corresponding parameter in shader code. Only needed
        if multiple parameters share a semantic name.
        
    .. py:property:: format
        :type: sgl.Format
    
        The format of the data being fetched for this element.
        
    .. py:property:: offset
        :type: int
    
        The offset in bytes of this element from the start of the
        corresponding chunk of vertex stream data.
        
    .. py:property:: buffer_slot_index
        :type: int
    
        The index of the vertex stream to fetch this element's data from.
        


----

.. py:class:: sgl.InputLayout

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: desc
        :type: sgl.InputLayoutDesc
    


----

.. py:class:: sgl.InputLayoutDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: input_elements
        :type: list[sgl.InputElementDesc]
    
    .. py:property:: vertex_streams
        :type: list[sgl.VertexStreamDesc]
    


----

.. py:class:: sgl.InputSlotClass

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.InputSlotClass.per_vertex
        :type: InputSlotClass
        :value: InputSlotClass.per_vertex
    
    .. py:attribute:: sgl.InputSlotClass.per_instance
        :type: InputSlotClass
        :value: InputSlotClass.per_instance
    


----

.. py:class:: sgl.Kernel

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: program
        :type: sgl.ShaderProgram
    
    .. py:property:: reflection
        :type: sgl.ReflectionCursor
    


----

.. py:class:: sgl.LoadOp

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.LoadOp.load
        :type: LoadOp
        :value: LoadOp.load
    
    .. py:attribute:: sgl.LoadOp.clear
        :type: LoadOp
        :value: LoadOp.clear
    
    .. py:attribute:: sgl.LoadOp.dont_care
        :type: LoadOp
        :value: LoadOp.dont_care
    


----

.. py:class:: sgl.LogicOp

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.LogicOp.no_op
        :type: LogicOp
        :value: LogicOp.no_op
    


----

.. py:class:: sgl.MemoryType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.MemoryType.device_local
        :type: MemoryType
        :value: MemoryType.device_local
    
    .. py:attribute:: sgl.MemoryType.upload
        :type: MemoryType
        :value: MemoryType.upload
    
    .. py:attribute:: sgl.MemoryType.read_back
        :type: MemoryType
        :value: MemoryType.read_back
    


----

.. py:class:: sgl.ModifierID

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.ModifierID.shared
        :type: ModifierID
        :value: ModifierID.shared
    
    .. py:attribute:: sgl.ModifierID.nodiff
        :type: ModifierID
        :value: ModifierID.nodiff
    
    .. py:attribute:: sgl.ModifierID.static
        :type: ModifierID
        :value: ModifierID.static
    
    .. py:attribute:: sgl.ModifierID.const
        :type: ModifierID
        :value: ModifierID.const
    
    .. py:attribute:: sgl.ModifierID.export
        :type: ModifierID
        :value: ModifierID.export
    
    .. py:attribute:: sgl.ModifierID.extern
        :type: ModifierID
        :value: ModifierID.extern
    
    .. py:attribute:: sgl.ModifierID.differentiable
        :type: ModifierID
        :value: ModifierID.differentiable
    
    .. py:attribute:: sgl.ModifierID.mutating
        :type: ModifierID
        :value: ModifierID.mutating
    
    .. py:attribute:: sgl.ModifierID.inn
        :type: ModifierID
        :value: ModifierID.inn
    
    .. py:attribute:: sgl.ModifierID.out
        :type: ModifierID
        :value: ModifierID.out
    
    .. py:attribute:: sgl.ModifierID.inout
        :type: ModifierID
        :value: ModifierID.inout
    


----

.. py:class:: sgl.MultisampleDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: sample_count
        :type: int
    
    .. py:property:: sample_mask
        :type: int
    
    .. py:property:: alpha_to_coverage_enable
        :type: bool
    
    .. py:property:: alpha_to_one_enable
        :type: bool
    


----

.. py:class:: sgl.PassEncoder

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:method:: end(self) -> None
    
    .. py:method:: push_debug_group(self, name: str, color: sgl.math.float3) -> None
    
        Push a debug group.
        
    .. py:method:: pop_debug_group(self) -> None
    
        Pop a debug group.
        
    .. py:method:: insert_debug_marker(self, name: str, color: sgl.math.float3) -> None
    
        Insert a debug marker.
        
        Parameter ``name``:
            Name of the marker.
        
        Parameter ``color``:
            Color of the marker.
        


----

.. py:class:: sgl.Pipeline

    Base class: :py:class:`sgl.DeviceResource`
    
    Pipeline base class.
    


----

.. py:class:: sgl.PrimitiveTopology

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.PrimitiveTopology.point_list
        :type: PrimitiveTopology
        :value: PrimitiveTopology.point_list
    
    .. py:attribute:: sgl.PrimitiveTopology.line_list
        :type: PrimitiveTopology
        :value: PrimitiveTopology.line_list
    
    .. py:attribute:: sgl.PrimitiveTopology.line_strip
        :type: PrimitiveTopology
        :value: PrimitiveTopology.line_strip
    
    .. py:attribute:: sgl.PrimitiveTopology.triangle_list
        :type: PrimitiveTopology
        :value: PrimitiveTopology.triangle_list
    
    .. py:attribute:: sgl.PrimitiveTopology.triangle_strip
        :type: PrimitiveTopology
        :value: PrimitiveTopology.triangle_strip
    
    .. py:attribute:: sgl.PrimitiveTopology.patch_list
        :type: PrimitiveTopology
        :value: PrimitiveTopology.patch_list
    


----

.. py:class:: sgl.ProgramLayout

    Base class: :py:class:`sgl.BaseReflectionObject`
    
    
    
    .. py:class:: sgl.ProgramLayout.HashedString
    
        
        
        .. py:property:: string
            :type: str
        
        .. py:property:: hash
            :type: int
        
    .. py:property:: globals_type_layout
        :type: sgl.TypeLayoutReflection
    
    .. py:property:: globals_variable_layout
        :type: sgl.VariableLayoutReflection
    
    .. py:property:: parameters
        :type: sgl.ProgramLayoutParameterList
    
    .. py:property:: entry_points
        :type: sgl.ProgramLayoutEntryPointList
    
    .. py:method:: find_type_by_name(self, name: str) -> sgl.TypeReflection
    
        Find a given type by name. Handles generic specilization if generic
        variable values are provided.
        
    .. py:method:: find_function_by_name(self, name: str) -> sgl.FunctionReflection
    
        Find a given function by name. Handles generic specilization if
        generic variable values are provided.
        
    .. py:method:: find_function_by_name_in_type(self, type: sgl.TypeReflection, name: str) -> sgl.FunctionReflection
    
        Find a given function in a type by name. Handles generic specilization
        if generic variable values are provided.
        
    .. py:method:: get_type_layout(self, type: sgl.TypeReflection) -> sgl.TypeLayoutReflection
    
        Get corresponding type layout from a given type.
        
    .. py:method:: is_sub_type(self, sub_type: sgl.TypeReflection, super_type: sgl.TypeReflection) -> bool
    
        Test whether a type is a sub type of another type. Handles both struct
        inheritance and interface implementation.
        
    .. py:property:: hashed_strings
        :type: list[sgl.ProgramLayout.HashedString]
    


----

.. py:class:: sgl.ProgramLayoutEntryPointList

    ProgramLayout lazy entry point list evaluation.
    


----

.. py:class:: sgl.ProgramLayoutParameterList

    ProgramLayout lazy parameter list evaluation.
    


----

.. py:class:: sgl.QueryPool

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: desc
        :type: sgl.QueryPoolDesc
    
    .. py:method:: reset(self) -> None
    
    .. py:method:: get_result(self, index: int) -> int
    
    .. py:method:: get_results(self, index: int, count: int) -> list[int]
    
    .. py:method:: get_timestamp_result(self, index: int) -> float
    
    .. py:method:: get_timestamp_results(self, index: int, count: int) -> list[float]
    


----

.. py:class:: sgl.QueryPoolDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: type
        :type: sgl.QueryType
    
        Query type.
        
    .. py:property:: count
        :type: int
    
        Number of queries in the pool.
        


----

.. py:class:: sgl.QueryType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.QueryType.timestamp
        :type: QueryType
        :value: QueryType.timestamp
    
    .. py:attribute:: sgl.QueryType.acceleration_structure_compacted_size
        :type: QueryType
        :value: QueryType.acceleration_structure_compacted_size
    
    .. py:attribute:: sgl.QueryType.acceleration_structure_serialized_size
        :type: QueryType
        :value: QueryType.acceleration_structure_serialized_size
    
    .. py:attribute:: sgl.QueryType.acceleration_structure_current_size
        :type: QueryType
        :value: QueryType.acceleration_structure_current_size
    


----

.. py:class:: sgl.RasterizerDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: fill_mode
        :type: sgl.FillMode
    
    .. py:property:: cull_mode
        :type: sgl.CullMode
    
    .. py:property:: front_face
        :type: sgl.FrontFaceMode
    
    .. py:property:: depth_bias
        :type: int
    
    .. py:property:: depth_bias_clamp
        :type: float
    
    .. py:property:: slope_scaled_depth_bias
        :type: float
    
    .. py:property:: depth_clip_enable
        :type: bool
    
    .. py:property:: scissor_enable
        :type: bool
    
    .. py:property:: multisample_enable
        :type: bool
    
    .. py:property:: antialiased_line_enable
        :type: bool
    
    .. py:property:: enable_conservative_rasterization
        :type: bool
    
    .. py:property:: forced_sample_count
        :type: int
    


----

.. py:class:: sgl.RayTracingPassEncoder

    Base class: :py:class:`sgl.PassEncoder`
    
    
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.RayTracingPipeline, shader_table: sgl.ShaderTable) -> sgl.ShaderObject
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.RayTracingPipeline, shader_table: sgl.ShaderTable, root_object: sgl.ShaderObject) -> None
        :no-index:
    
    .. py:method:: dispatch_rays(self, ray_gen_shader_index: int, dimensions: sgl.math.uint3) -> None
    


----

.. py:class:: sgl.RayTracingPipeline

    Base class: :py:class:`sgl.Pipeline`
    
    Ray tracing pipeline.
    


----

.. py:class:: sgl.RayTracingPipelineDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: program
        :type: sgl.ShaderProgram
    
    .. py:property:: hit_groups
        :type: list[sgl.HitGroupDesc]
    
    .. py:property:: max_recursion
        :type: int
    
    .. py:property:: max_ray_payload_size
        :type: int
    
    .. py:property:: max_attribute_size
        :type: int
    
    .. py:property:: flags
        :type: sgl.RayTracingPipelineFlags
    


----

.. py:class:: sgl.RayTracingPipelineFlags

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.RayTracingPipelineFlags.none
        :type: RayTracingPipelineFlags
        :value: RayTracingPipelineFlags.none
    
    .. py:attribute:: sgl.RayTracingPipelineFlags.skip_triangles
        :type: RayTracingPipelineFlags
        :value: RayTracingPipelineFlags.skip_triangles
    
    .. py:attribute:: sgl.RayTracingPipelineFlags.skip_procedurals
        :type: RayTracingPipelineFlags
        :value: RayTracingPipelineFlags.skip_procedurals
    


----

.. py:class:: sgl.ReflectionCursor

    
    
    .. py:method:: __init__(self, shader_program: sgl.ShaderProgram) -> None
    
    .. py:method:: is_valid(self) -> bool
    
    .. py:method:: find_field(self, name: str) -> sgl.ReflectionCursor
    
    .. py:method:: find_element(self, index: int) -> sgl.ReflectionCursor
    
    .. py:method:: has_field(self, name: str) -> bool
    
    .. py:method:: has_element(self, index: int) -> bool
    
    .. py:property:: type_layout
        :type: sgl.TypeLayoutReflection
    
    .. py:property:: type
        :type: sgl.TypeReflection
    


----

.. py:class:: sgl.RenderPassColorAttachment

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: view
        :type: sgl.TextureView
    
    .. py:property:: resolve_target
        :type: sgl.TextureView
    
    .. py:property:: load_op
        :type: sgl.LoadOp
    
    .. py:property:: store_op
        :type: sgl.StoreOp
    
    .. py:property:: clear_value
        :type: sgl.math.float4
    


----

.. py:class:: sgl.RenderPassDepthStencilAttachment

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: view
        :type: sgl.TextureView
    
    .. py:property:: depth_load_op
        :type: sgl.LoadOp
    
    .. py:property:: depth_store_op
        :type: sgl.StoreOp
    
    .. py:property:: depth_clear_value
        :type: float
    
    .. py:property:: depth_read_only
        :type: bool
    
    .. py:property:: stencil_load_op
        :type: sgl.LoadOp
    
    .. py:property:: stencil_store_op
        :type: sgl.StoreOp
    
    .. py:property:: stencil_clear_value
        :type: int
    
    .. py:property:: stencil_read_only
        :type: bool
    


----

.. py:class:: sgl.RenderPassDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: color_attachments
        :type: list[sgl.RenderPassColorAttachment]
    
    .. py:property:: depth_stencil_attachment
        :type: sgl.RenderPassDepthStencilAttachment | None
    


----

.. py:class:: sgl.RenderPassEncoder

    Base class: :py:class:`sgl.PassEncoder`
    
    
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.RenderPipeline) -> sgl.ShaderObject
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.RenderPipeline, root_object: sgl.ShaderObject) -> None
        :no-index:
    
    .. py:method:: set_render_state(self, state: sgl.RenderState) -> None
    
    .. py:method:: draw(self, args: sgl.DrawArguments) -> None
    
    .. py:method:: draw_indexed(self, args: sgl.DrawArguments) -> None
    
    .. py:method:: draw_indirect(self, max_draw_count: int, arg_buffer: sgl.BufferOffsetPair, count_buffer: sgl.BufferOffsetPair = <sgl.BufferOffsetPair object at 0x7f5998eefd50>) -> None
    
    .. py:method:: draw_indexed_indirect(self, max_draw_count: int, arg_buffer: sgl.BufferOffsetPair, count_buffer: sgl.BufferOffsetPair = <sgl.BufferOffsetPair object at 0x7f5998eefd80>) -> None
    
    .. py:method:: draw_mesh_tasks(self, dimensions: sgl.math.uint3) -> None
    


----

.. py:class:: sgl.RenderPipeline

    Base class: :py:class:`sgl.Pipeline`
    
    Render pipeline.
    


----

.. py:class:: sgl.RenderPipelineDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: program
        :type: sgl.ShaderProgram
    
    .. py:property:: input_layout
        :type: sgl.InputLayout
    
    .. py:property:: primitive_topology
        :type: sgl.PrimitiveTopology
    
    .. py:property:: targets
        :type: list[sgl.ColorTargetDesc]
    
    .. py:property:: depth_stencil
        :type: sgl.DepthStencilDesc
    
    .. py:property:: rasterizer
        :type: sgl.RasterizerDesc
    
    .. py:property:: multisample
        :type: sgl.MultisampleDesc
    


----

.. py:class:: sgl.RenderState

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: stencil_ref
        :type: int
    
    .. py:property:: viewports
        :type: list[sgl.Viewport]
    
    .. py:property:: scissor_rects
        :type: list[sgl.ScissorRect]
    
    .. py:property:: vertex_buffers
        :type: list[sgl.BufferOffsetPair]
    
    .. py:property:: index_buffer
        :type: sgl.BufferOffsetPair
    
    .. py:property:: index_format
        :type: sgl.IndexFormat
    


----

.. py:class:: sgl.RenderTargetWriteMask

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.RenderTargetWriteMask.enable_none
        :type: RenderTargetWriteMask
        :value: RenderTargetWriteMask.enable_none
    
    .. py:attribute:: sgl.RenderTargetWriteMask.enable_red
        :type: RenderTargetWriteMask
        :value: RenderTargetWriteMask.enable_red
    
    .. py:attribute:: sgl.RenderTargetWriteMask.enable_green
        :type: RenderTargetWriteMask
        :value: RenderTargetWriteMask.enable_green
    
    .. py:attribute:: sgl.RenderTargetWriteMask.enable_blue
        :type: RenderTargetWriteMask
        :value: RenderTargetWriteMask.enable_blue
    
    .. py:attribute:: sgl.RenderTargetWriteMask.enable_alpha
        :type: RenderTargetWriteMask
        :value: RenderTargetWriteMask.enable_alpha
    
    .. py:attribute:: sgl.RenderTargetWriteMask.enable_all
        :type: RenderTargetWriteMask
        :value: RenderTargetWriteMask.enable_all
    


----

.. py:class:: sgl.Resource

    Base class: :py:class:`sgl.DeviceResource`
    
    
    


----

.. py:class:: sgl.ResourceState

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.ResourceState.undefined
        :type: ResourceState
        :value: ResourceState.undefined
    
    .. py:attribute:: sgl.ResourceState.general
        :type: ResourceState
        :value: ResourceState.general
    
    .. py:attribute:: sgl.ResourceState.vertex_buffer
        :type: ResourceState
        :value: ResourceState.vertex_buffer
    
    .. py:attribute:: sgl.ResourceState.index_buffer
        :type: ResourceState
        :value: ResourceState.index_buffer
    
    .. py:attribute:: sgl.ResourceState.constant_buffer
        :type: ResourceState
        :value: ResourceState.constant_buffer
    
    .. py:attribute:: sgl.ResourceState.stream_output
        :type: ResourceState
        :value: ResourceState.stream_output
    
    .. py:attribute:: sgl.ResourceState.shader_resource
        :type: ResourceState
        :value: ResourceState.shader_resource
    
    .. py:attribute:: sgl.ResourceState.unordered_access
        :type: ResourceState
        :value: ResourceState.unordered_access
    
    .. py:attribute:: sgl.ResourceState.render_target
        :type: ResourceState
        :value: ResourceState.render_target
    
    .. py:attribute:: sgl.ResourceState.depth_read
        :type: ResourceState
        :value: ResourceState.depth_read
    
    .. py:attribute:: sgl.ResourceState.depth_write
        :type: ResourceState
        :value: ResourceState.depth_write
    
    .. py:attribute:: sgl.ResourceState.present
        :type: ResourceState
        :value: ResourceState.present
    
    .. py:attribute:: sgl.ResourceState.indirect_argument
        :type: ResourceState
        :value: ResourceState.indirect_argument
    
    .. py:attribute:: sgl.ResourceState.copy_source
        :type: ResourceState
        :value: ResourceState.copy_source
    
    .. py:attribute:: sgl.ResourceState.copy_destination
        :type: ResourceState
        :value: ResourceState.copy_destination
    
    .. py:attribute:: sgl.ResourceState.resolve_source
        :type: ResourceState
        :value: ResourceState.resolve_source
    
    .. py:attribute:: sgl.ResourceState.resolve_destination
        :type: ResourceState
        :value: ResourceState.resolve_destination
    
    .. py:attribute:: sgl.ResourceState.acceleration_structure
        :type: ResourceState
        :value: ResourceState.acceleration_structure
    
    .. py:attribute:: sgl.ResourceState.acceleration_structure_build_output
        :type: ResourceState
        :value: ResourceState.acceleration_structure_build_output
    


----

.. py:class:: sgl.Sampler

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: desc
        :type: sgl.SamplerDesc
    


----

.. py:class:: sgl.SamplerDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: min_filter
        :type: sgl.TextureFilteringMode
    
    .. py:property:: mag_filter
        :type: sgl.TextureFilteringMode
    
    .. py:property:: mip_filter
        :type: sgl.TextureFilteringMode
    
    .. py:property:: reduction_op
        :type: sgl.TextureReductionOp
    
    .. py:property:: address_u
        :type: sgl.TextureAddressingMode
    
    .. py:property:: address_v
        :type: sgl.TextureAddressingMode
    
    .. py:property:: address_w
        :type: sgl.TextureAddressingMode
    
    .. py:property:: mip_lod_bias
        :type: float
    
    .. py:property:: max_anisotropy
        :type: int
    
    .. py:property:: comparison_func
        :type: sgl.ComparisonFunc
    
    .. py:property:: border_color
        :type: sgl.math.float4
    
    .. py:property:: min_lod
        :type: float
    
    .. py:property:: max_lod
        :type: float
    


----

.. py:class:: sgl.ScissorRect

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:staticmethod:: from_size(width: int, height: int) -> sgl.ScissorRect
    
    .. py:property:: min_x
        :type: int
    
    .. py:property:: min_y
        :type: int
    
    .. py:property:: max_x
        :type: int
    
    .. py:property:: max_y
        :type: int
    


----

.. py:class:: sgl.ShaderCacheStats

    
    
    .. py:property:: entry_count
        :type: int
    
        Number of entries in the cache.
        
    .. py:property:: hit_count
        :type: int
    
        Number of hits in the cache.
        
    .. py:property:: miss_count
        :type: int
    
        Number of misses in the cache.
        


----

.. py:class:: sgl.ShaderCursor

    Cursor used for parsing and setting shader object fields. This class
    does *NOT* use the SGL reflection wrappers for accessing due to the
    performance implications of allocating/freeing them repeatedly. This
    is far faster, however does introduce a risk of mem access problems if
    the shader cursor is kept alive longer than the shader object it was
    created from.
    
    .. py:method:: __init__(self, shader_object: sgl.ShaderObject) -> None
    
    .. py:method:: dereference(self) -> sgl.ShaderCursor
    
    .. py:method:: find_entry_point(self, index: int) -> sgl.ShaderCursor
    
    .. py:method:: is_valid(self) -> bool
    
        N/A
        
    .. py:method:: find_field(self, name: str) -> sgl.ShaderCursor
    
        N/A
        
    .. py:method:: find_element(self, index: int) -> sgl.ShaderCursor
    
        N/A
        
    .. py:method:: has_field(self, name: str) -> bool
    
        N/A
        
    .. py:method:: has_element(self, index: int) -> bool
    
        N/A
        
    .. py:method:: set_data(self, data: ndarray[device='cpu']) -> None
    
    .. py:method:: write(self, val: object) -> None
    
        N/A
        


----

.. py:class:: sgl.ShaderHotReloadEvent

    Event data for hot reload hook.
    


----

.. py:class:: sgl.ShaderModel

    Base class: :py:class:`enum.IntEnum`
    
    .. py:attribute:: sgl.ShaderModel.unknown
        :type: ShaderModel
        :value: ShaderModel.unknown
    
    .. py:attribute:: sgl.ShaderModel.sm_6_0
        :type: ShaderModel
        :value: ShaderModel.sm_6_0
    
    .. py:attribute:: sgl.ShaderModel.sm_6_1
        :type: ShaderModel
        :value: ShaderModel.sm_6_1
    
    .. py:attribute:: sgl.ShaderModel.sm_6_2
        :type: ShaderModel
        :value: ShaderModel.sm_6_2
    
    .. py:attribute:: sgl.ShaderModel.sm_6_3
        :type: ShaderModel
        :value: ShaderModel.sm_6_3
    
    .. py:attribute:: sgl.ShaderModel.sm_6_4
        :type: ShaderModel
        :value: ShaderModel.sm_6_4
    
    .. py:attribute:: sgl.ShaderModel.sm_6_5
        :type: ShaderModel
        :value: ShaderModel.sm_6_5
    
    .. py:attribute:: sgl.ShaderModel.sm_6_6
        :type: ShaderModel
        :value: ShaderModel.sm_6_6
    
    .. py:attribute:: sgl.ShaderModel.sm_6_7
        :type: ShaderModel
        :value: ShaderModel.sm_6_7
    


----

.. py:class:: sgl.ShaderObject

    Base class: :py:class:`sgl.Object`
    
    
    


----

.. py:class:: sgl.ShaderOffset

    Represents the offset of a shader variable relative to its enclosing
    type/buffer/block.
    
    A `ShaderOffset` can be used to store the offset of a shader variable
    that might use ordinary/uniform data, resources like
    textures/buffers/samplers, or some combination.
    
    A `ShaderOffset` can also encode an invalid offset, to indicate that a
    particular shader variable is not present.
    
    .. py:property:: uniform_offset
        :type: int
    
    .. py:property:: binding_range_index
        :type: int
    
    .. py:property:: binding_array_index
        :type: int
    
    .. py:method:: is_valid(self) -> bool
    
        Check whether this offset is valid.
        


----

.. py:class:: sgl.ShaderProgram

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: layout
        :type: sgl.ProgramLayout
    
    .. py:property:: reflection
        :type: sgl.ReflectionCursor
    


----

.. py:class:: sgl.ShaderStage

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.ShaderStage.none
        :type: ShaderStage
        :value: ShaderStage.none
    
    .. py:attribute:: sgl.ShaderStage.vertex
        :type: ShaderStage
        :value: ShaderStage.vertex
    
    .. py:attribute:: sgl.ShaderStage.hull
        :type: ShaderStage
        :value: ShaderStage.hull
    
    .. py:attribute:: sgl.ShaderStage.domain
        :type: ShaderStage
        :value: ShaderStage.domain
    
    .. py:attribute:: sgl.ShaderStage.geometry
        :type: ShaderStage
        :value: ShaderStage.geometry
    
    .. py:attribute:: sgl.ShaderStage.fragment
        :type: ShaderStage
        :value: ShaderStage.fragment
    
    .. py:attribute:: sgl.ShaderStage.compute
        :type: ShaderStage
        :value: ShaderStage.compute
    
    .. py:attribute:: sgl.ShaderStage.ray_generation
        :type: ShaderStage
        :value: ShaderStage.ray_generation
    
    .. py:attribute:: sgl.ShaderStage.intersection
        :type: ShaderStage
        :value: ShaderStage.intersection
    
    .. py:attribute:: sgl.ShaderStage.any_hit
        :type: ShaderStage
        :value: ShaderStage.any_hit
    
    .. py:attribute:: sgl.ShaderStage.closest_hit
        :type: ShaderStage
        :value: ShaderStage.closest_hit
    
    .. py:attribute:: sgl.ShaderStage.miss
        :type: ShaderStage
        :value: ShaderStage.miss
    
    .. py:attribute:: sgl.ShaderStage.callable
        :type: ShaderStage
        :value: ShaderStage.callable
    
    .. py:attribute:: sgl.ShaderStage.mesh
        :type: ShaderStage
        :value: ShaderStage.mesh
    
    .. py:attribute:: sgl.ShaderStage.amplification
        :type: ShaderStage
        :value: ShaderStage.amplification
    


----

.. py:class:: sgl.ShaderTable

    Base class: :py:class:`sgl.DeviceResource`
    
    
    


----

.. py:class:: sgl.ShaderTableDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: program
        :type: sgl.ShaderProgram
    
    .. py:property:: ray_gen_entry_points
        :type: list[str]
    
    .. py:property:: miss_entry_points
        :type: list[str]
    
    .. py:property:: hit_group_names
        :type: list[str]
    
    .. py:property:: callable_entry_points
        :type: list[str]
    


----

.. py:class:: sgl.SlangCompileError

    Base class: :py:class:`builtins.Exception`
    


----

.. py:class:: sgl.SlangCompilerOptions

    Slang compiler options. Can be set when creating a Slang session.
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: include_paths
        :type: list[pathlib.Path]
    
        Specifies a list of include paths to be used when resolving
        module/include paths.
        
    .. py:property:: defines
        :type: dict[str, str]
    
        Specifies a list of preprocessor defines.
        
    .. py:property:: shader_model
        :type: sgl.ShaderModel
    
        Specifies the shader model to use. Defaults to latest available on the
        device.
        
    .. py:property:: matrix_layout
        :type: sgl.SlangMatrixLayout
    
        Specifies the matrix layout. Defaults to row-major.
        
    .. py:property:: enable_warnings
        :type: list[str]
    
        Specifies a list of warnings to enable (warning codes or names).
        
    .. py:property:: disable_warnings
        :type: list[str]
    
        Specifies a list of warnings to disable (warning codes or names).
        
    .. py:property:: warnings_as_errors
        :type: list[str]
    
        Specifies a list of warnings to be treated as errors (warning codes or
        names, or "all" to indicate all warnings).
        
    .. py:property:: report_downstream_time
        :type: bool
    
        Turn on/off downstream compilation time report.
        
    .. py:property:: report_perf_benchmark
        :type: bool
    
        Turn on/off reporting of time spend in different parts of the
        compiler.
        
    .. py:property:: skip_spirv_validation
        :type: bool
    
        Specifies whether or not to skip the validation step after emitting
        SPIRV.
        
    .. py:property:: floating_point_mode
        :type: sgl.SlangFloatingPointMode
    
        Specifies the floating point mode.
        
    .. py:property:: debug_info
        :type: sgl.SlangDebugInfoLevel
    
        Specifies the level of debug information to include in the generated
        code.
        
    .. py:property:: optimization
        :type: sgl.SlangOptimizationLevel
    
        Specifies the optimization level.
        
    .. py:property:: downstream_args
        :type: list[str]
    
        Specifies a list of additional arguments to be passed to the
        downstream compiler.
        
    .. py:property:: dump_intermediates
        :type: bool
    
        When set will dump the intermediate source output.
        
    .. py:property:: dump_intermediates_prefix
        :type: str
    
        The file name prefix for the intermediate source output.
        


----

.. py:class:: sgl.SlangDebugInfoLevel

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.SlangDebugInfoLevel.none
        :type: SlangDebugInfoLevel
        :value: SlangDebugInfoLevel.none
    
    .. py:attribute:: sgl.SlangDebugInfoLevel.minimal
        :type: SlangDebugInfoLevel
        :value: SlangDebugInfoLevel.minimal
    
    .. py:attribute:: sgl.SlangDebugInfoLevel.standard
        :type: SlangDebugInfoLevel
        :value: SlangDebugInfoLevel.standard
    
    .. py:attribute:: sgl.SlangDebugInfoLevel.maximal
        :type: SlangDebugInfoLevel
        :value: SlangDebugInfoLevel.maximal
    


----

.. py:class:: sgl.SlangEntryPoint

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:property:: name
        :type: str
    
    .. py:property:: stage
        :type: sgl.ShaderStage
    
    .. py:property:: layout
        :type: sgl.EntryPointLayout
    
    .. py:method:: rename(self, new_name: str) -> sgl.SlangEntryPoint
    
    .. py:method:: with_name(self, new_name: str) -> sgl.SlangEntryPoint
    
        Returns a copy of the entry point with a new name.
        


----

.. py:class:: sgl.SlangFloatingPointMode

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.SlangFloatingPointMode.default
        :type: SlangFloatingPointMode
        :value: SlangFloatingPointMode.default
    
    .. py:attribute:: sgl.SlangFloatingPointMode.fast
        :type: SlangFloatingPointMode
        :value: SlangFloatingPointMode.fast
    
    .. py:attribute:: sgl.SlangFloatingPointMode.precise
        :type: SlangFloatingPointMode
        :value: SlangFloatingPointMode.precise
    


----

.. py:class:: sgl.SlangLinkOptions

    Slang link options. These can optionally be set when linking a shader
    program.
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: floating_point_mode
        :type: sgl.SlangFloatingPointMode | None
    
        Specifies the floating point mode.
        
    .. py:property:: debug_info
        :type: sgl.SlangDebugInfoLevel | None
    
        Specifies the level of debug information to include in the generated
        code.
        
    .. py:property:: optimization
        :type: sgl.SlangOptimizationLevel | None
    
        Specifies the optimization level.
        
    .. py:property:: downstream_args
        :type: list[str] | None
    
        Specifies a list of additional arguments to be passed to the
        downstream compiler.
        
    .. py:property:: dump_intermediates
        :type: bool | None
    
        When set will dump the intermediate source output.
        
    .. py:property:: dump_intermediates_prefix
        :type: str | None
    
        The file name prefix for the intermediate source output.
        


----

.. py:class:: sgl.SlangMatrixLayout

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.SlangMatrixLayout.row_major
        :type: SlangMatrixLayout
        :value: SlangMatrixLayout.row_major
    
    .. py:attribute:: sgl.SlangMatrixLayout.column_major
        :type: SlangMatrixLayout
        :value: SlangMatrixLayout.column_major
    


----

.. py:class:: sgl.SlangModule

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:property:: session
        :type: sgl.SlangSession
    
        The session from which this module was built.
        
    .. py:property:: name
        :type: str
    
        Module name.
        
    .. py:property:: path
        :type: pathlib.Path
    
        Module source path. This can be empty if the module was generated from
        a string.
        
    .. py:property:: layout
        :type: sgl.ProgramLayout
    
    .. py:property:: entry_points
        :type: list[sgl.SlangEntryPoint]
    
        Build and return vector of all current entry points in the module.
        
    .. py:property:: module_decl
        :type: sgl.DeclReflection
    
        Get root decl ref for this module
        
    .. py:method:: entry_point(self, name: str, type_conformances: Sequence[sgl.TypeConformance] = []) -> sgl.SlangEntryPoint
    
        Get an entry point, optionally applying type conformances to it.
        


----

.. py:class:: sgl.SlangOptimizationLevel

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.SlangOptimizationLevel.none
        :type: SlangOptimizationLevel
        :value: SlangOptimizationLevel.none
    
    .. py:attribute:: sgl.SlangOptimizationLevel.default
        :type: SlangOptimizationLevel
        :value: SlangOptimizationLevel.default
    
    .. py:attribute:: sgl.SlangOptimizationLevel.high
        :type: SlangOptimizationLevel
        :value: SlangOptimizationLevel.high
    
    .. py:attribute:: sgl.SlangOptimizationLevel.maximal
        :type: SlangOptimizationLevel
        :value: SlangOptimizationLevel.maximal
    


----

.. py:class:: sgl.SlangSession

    Base class: :py:class:`sgl.Object`
    
    A slang session, used to load modules and link programs.
    
    .. py:property:: device
        :type: sgl.Device
    
    .. py:property:: desc
        :type: sgl.SlangSessionDesc
    
    .. py:method:: load_module(self, module_name: str) -> sgl.SlangModule
    
        Load a module by name.
        
    .. py:method:: load_module_from_source(self, module_name: str, source: str, path: str | os.PathLike | None = None) -> sgl.SlangModule
    
        Load a module from string source code.
        
    .. py:method:: link_program(self, modules: collections.abc.Sequence[sgl.SlangModule], entry_points: collections.abc.Sequence[sgl.SlangEntryPoint], link_options: sgl.SlangLinkOptions | None = None) -> sgl.ShaderProgram
    
        Link a program with a set of modules and entry points.
        
    .. py:method:: load_program(self, module_name: str, entry_point_names: collections.abc.Sequence[str], additional_source: str | None = None, link_options: sgl.SlangLinkOptions | None = None) -> sgl.ShaderProgram
    
        Load a program from a given module with a set of entry points.
        Internally this simply wraps link_program without requiring the user
        to explicitly load modules.
        
    .. py:method:: load_source(self, module_name: str) -> str
    
        Load the source code for a given module.
        


----

.. py:class:: sgl.SlangSessionDesc

    Descriptor for slang session initialization.
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: compiler_options
        :type: sgl.SlangCompilerOptions
    
    .. py:property:: add_default_include_paths
        :type: bool
    
    .. py:property:: cache_path
        :type: pathlib.Path | None
    


----

.. py:class:: sgl.StencilOp

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.StencilOp.keep
        :type: StencilOp
        :value: StencilOp.keep
    
    .. py:attribute:: sgl.StencilOp.zero
        :type: StencilOp
        :value: StencilOp.zero
    
    .. py:attribute:: sgl.StencilOp.replace
        :type: StencilOp
        :value: StencilOp.replace
    
    .. py:attribute:: sgl.StencilOp.increment_saturate
        :type: StencilOp
        :value: StencilOp.increment_saturate
    
    .. py:attribute:: sgl.StencilOp.decrement_saturate
        :type: StencilOp
        :value: StencilOp.decrement_saturate
    
    .. py:attribute:: sgl.StencilOp.invert
        :type: StencilOp
        :value: StencilOp.invert
    
    .. py:attribute:: sgl.StencilOp.increment_wrap
        :type: StencilOp
        :value: StencilOp.increment_wrap
    
    .. py:attribute:: sgl.StencilOp.decrement_wrap
        :type: StencilOp
        :value: StencilOp.decrement_wrap
    


----

.. py:class:: sgl.StoreOp

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.StoreOp.store
        :type: StoreOp
        :value: StoreOp.store
    
    .. py:attribute:: sgl.StoreOp.dont_care
        :type: StoreOp
        :value: StoreOp.dont_care
    


----

.. py:class:: sgl.SubresourceLayout

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: size
        :type: sgl.math.uint3
    
        Dimensions of the subresource (in texels).
        
    .. py:property:: col_pitch
        :type: int
    
        Stride in bytes between columns (i.e. blocks) of the subresource
        tensor.
        
    .. py:property:: row_pitch
        :type: int
    
        Stride in bytes between rows of the subresource tensor.
        
    .. py:property:: slice_pitch
        :type: int
    
        Stride in bytes between slices of the subresource tensor.
        
    .. py:property:: size_in_bytes
        :type: int
    
        Overall size required to fit the subresource data (typically size.z *
        slice_pitch).
        
    .. py:property:: block_width
        :type: int
    
        Block width in texels (1 for uncompressed formats).
        
    .. py:property:: block_height
        :type: int
    
        Block height in texels (1 for uncompressed formats).
        
    .. py:property:: row_count
        :type: int
    
        Number of rows. For uncompressed formats this matches size.y. For
        compressed formats this matches align_up(size.y, block_height) /
        block_height.
        


----

.. py:class:: sgl.SubresourceRange

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: mip_level
        :type: int
    
        Most detailed mip level.
        
    .. py:property:: mip_count
        :type: int
    
        Number of mip levels.
        
    .. py:property:: base_array_layer
        :type: int
    
        First array layer.
        
    .. py:property:: layer_count
        :type: int
    
        Number of array layers.
        


----

.. py:class:: sgl.Surface

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:property:: info
        :type: sgl.SurfaceInfo
    
        Returns the surface info.
        
    .. py:property:: config
        :type: sgl.SurfaceConfig
    
        Returns the surface config.
        
    .. py:method:: configure(self, width: int, height: int, format: sgl.Format = Format.undefined, usage: sgl.TextureUsage = TextureUsage.render_target, desired_image_count: int = 3, vsync: bool = True) -> None
    
        Configure the surface.
        
    .. py:method:: configure(self, config: sgl.SurfaceConfig) -> None
        :no-index:
    
    .. py:method:: acquire_next_image(self) -> sgl.Texture
    
        Acquries the next surface image.
        
    .. py:method:: present(self) -> None
    
        Present the previously acquire image.
        


----

.. py:class:: sgl.SurfaceConfig

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: format
        :type: sgl.Format
    
        Surface texture format.
        
    .. py:property:: usage
        :type: sgl.TextureUsage
    
        Surface texture usage.
        
    .. py:property:: width
        :type: int
    
        Surface texture width.
        
    .. py:property:: height
        :type: int
    
        Surface texture height.
        
    .. py:property:: desired_image_count
        :type: int
    
        Desired number of images.
        
    .. py:property:: vsync
        :type: bool
    
        Enable/disable vertical synchronization.
        


----

.. py:class:: sgl.SurfaceInfo

    
    
    .. py:property:: preferred_format
        :type: sgl.Format
    
        Preferred format for the surface.
        
    .. py:property:: supported_usage
        :type: sgl.TextureUsage
    
        Supported texture usages.
        
    .. py:property:: formats
        :type: list[sgl.Format]
    
        Supported texture formats.
        


----

.. py:class:: sgl.Texture

    Base class: :py:class:`sgl.Resource`
    
    
    
    .. py:property:: desc
        :type: sgl.TextureDesc
    
    .. py:property:: type
        :type: sgl.TextureType
    
    .. py:property:: format
        :type: sgl.Format
    
    .. py:property:: width
        :type: int
    
    .. py:property:: height
        :type: int
    
    .. py:property:: depth
        :type: int
    
    .. py:property:: array_length
        :type: int
    
    .. py:property:: mip_count
        :type: int
    
    .. py:property:: layer_count
        :type: int
    
    .. py:property:: subresource_count
        :type: int
    
    .. py:method:: get_mip_width(self, mip_level: int = 0) -> int
    
    .. py:method:: get_mip_height(self, mip_level: int = 0) -> int
    
    .. py:method:: get_mip_depth(self, mip_level: int = 0) -> int
    
    .. py:method:: get_mip_size(self, mip_level: int = 0) -> sgl.math.uint3
    
    .. py:method:: get_subresource_layout(self, mip_level: int, row_alignment: int = 4294967295) -> sgl.SubresourceLayout
    
        Get layout of a texture subresource. By default, the row alignment
        used is that required by the target for direct buffer upload/download.
        Pass in 1 for a completely packed layout.
        
    .. py:method:: create_view(self, desc: sgl.TextureViewDesc) -> sgl.TextureView
    
    .. py:method:: create_view(self, dict: dict) -> sgl.TextureView
        :no-index:
    
    .. py:method:: create_view(self, mip_level: int = 0, mip_count: int = 4294967295, base_array_layer: int = 0, layer_count: int = 4294967295, format: sgl.Format = Format.undefined) -> sgl.TextureView
        :no-index:
    
    .. py:method:: to_bitmap(self, layer: int = 0, mip_level: int = 0) -> sgl.Bitmap
    
    .. py:method:: to_numpy(self, layer: int = 0, mip_level: int = 0) -> numpy.ndarray[]
    
    .. py:method:: copy_from_numpy(self, data: numpy.ndarray[], layer: int = 0, mip_level: int = 0) -> None
    


----

.. py:class:: sgl.TextureAddressingMode

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.TextureAddressingMode.wrap
        :type: TextureAddressingMode
        :value: TextureAddressingMode.wrap
    
    .. py:attribute:: sgl.TextureAddressingMode.clamp_to_edge
        :type: TextureAddressingMode
        :value: TextureAddressingMode.clamp_to_edge
    
    .. py:attribute:: sgl.TextureAddressingMode.clamp_to_border
        :type: TextureAddressingMode
        :value: TextureAddressingMode.clamp_to_border
    
    .. py:attribute:: sgl.TextureAddressingMode.mirror_repeat
        :type: TextureAddressingMode
        :value: TextureAddressingMode.mirror_repeat
    
    .. py:attribute:: sgl.TextureAddressingMode.mirror_once
        :type: TextureAddressingMode
        :value: TextureAddressingMode.mirror_once
    


----

.. py:class:: sgl.TextureAspect

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.TextureAspect.all
        :type: TextureAspect
        :value: TextureAspect.all
    
    .. py:attribute:: sgl.TextureAspect.depth_only
        :type: TextureAspect
        :value: TextureAspect.depth_only
    
    .. py:attribute:: sgl.TextureAspect.stencil_only
        :type: TextureAspect
        :value: TextureAspect.stencil_only
    


----

.. py:class:: sgl.TextureDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: type
        :type: sgl.TextureType
    
        Texture type.
        
    .. py:property:: format
        :type: sgl.Format
    
        Texture format.
        
    .. py:property:: width
        :type: int
    
        Width in pixels.
        
    .. py:property:: height
        :type: int
    
        Height in pixels.
        
    .. py:property:: depth
        :type: int
    
        Depth in pixels.
        
    .. py:property:: array_length
        :type: int
    
        Array length.
        
    .. py:property:: mip_count
        :type: int
    
        Number of mip levels (0 for auto-generated mips).
        
    .. py:property:: sample_count
        :type: int
    
        Number of samples per pixel.
        
    .. py:property:: sample_quality
        :type: int
    
        Quality level for multisampled textures.
        
    .. py:property:: memory_type
        :type: sgl.MemoryType
    
    .. py:property:: usage
        :type: sgl.TextureUsage
    
    .. py:property:: default_state
        :type: sgl.ResourceState
    
    .. py:property:: label
        :type: str
    
        Debug label.
        


----

.. py:class:: sgl.TextureFilteringMode

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.TextureFilteringMode.point
        :type: TextureFilteringMode
        :value: TextureFilteringMode.point
    
    .. py:attribute:: sgl.TextureFilteringMode.linear
        :type: TextureFilteringMode
        :value: TextureFilteringMode.linear
    


----

.. py:class:: sgl.TextureReductionOp

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.TextureReductionOp.average
        :type: TextureReductionOp
        :value: TextureReductionOp.average
    
    .. py:attribute:: sgl.TextureReductionOp.comparison
        :type: TextureReductionOp
        :value: TextureReductionOp.comparison
    
    .. py:attribute:: sgl.TextureReductionOp.minimum
        :type: TextureReductionOp
        :value: TextureReductionOp.minimum
    
    .. py:attribute:: sgl.TextureReductionOp.maximum
        :type: TextureReductionOp
        :value: TextureReductionOp.maximum
    


----

.. py:class:: sgl.TextureType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.TextureType.texture_1d
        :type: TextureType
        :value: TextureType.texture_1d
    
    .. py:attribute:: sgl.TextureType.texture_1d_array
        :type: TextureType
        :value: TextureType.texture_1d_array
    
    .. py:attribute:: sgl.TextureType.texture_2d
        :type: TextureType
        :value: TextureType.texture_2d
    
    .. py:attribute:: sgl.TextureType.texture_2d_array
        :type: TextureType
        :value: TextureType.texture_2d_array
    
    .. py:attribute:: sgl.TextureType.texture_2d_ms
        :type: TextureType
        :value: TextureType.texture_2d_ms
    
    .. py:attribute:: sgl.TextureType.texture_2d_ms_array
        :type: TextureType
        :value: TextureType.texture_2d_ms_array
    
    .. py:attribute:: sgl.TextureType.texture_3d
        :type: TextureType
        :value: TextureType.texture_3d
    
    .. py:attribute:: sgl.TextureType.texture_cube
        :type: TextureType
        :value: TextureType.texture_cube
    
    .. py:attribute:: sgl.TextureType.texture_cube_array
        :type: TextureType
        :value: TextureType.texture_cube_array
    


----

.. py:class:: sgl.TextureUsage

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.TextureUsage.none
        :type: TextureUsage
        :value: TextureUsage.none
    
    .. py:attribute:: sgl.TextureUsage.shader_resource
        :type: TextureUsage
        :value: TextureUsage.shader_resource
    
    .. py:attribute:: sgl.TextureUsage.unordered_access
        :type: TextureUsage
        :value: TextureUsage.unordered_access
    
    .. py:attribute:: sgl.TextureUsage.render_target
        :type: TextureUsage
        :value: TextureUsage.render_target
    
    .. py:attribute:: sgl.TextureUsage.depth_stencil
        :type: TextureUsage
        :value: TextureUsage.depth_stencil
    
    .. py:attribute:: sgl.TextureUsage.present
        :type: TextureUsage
        :value: TextureUsage.present
    
    .. py:attribute:: sgl.TextureUsage.copy_source
        :type: TextureUsage
        :value: TextureUsage.copy_source
    
    .. py:attribute:: sgl.TextureUsage.copy_destination
        :type: TextureUsage
        :value: TextureUsage.copy_destination
    
    .. py:attribute:: sgl.TextureUsage.resolve_source
        :type: TextureUsage
        :value: TextureUsage.resolve_source
    
    .. py:attribute:: sgl.TextureUsage.resolve_destination
        :type: TextureUsage
        :value: TextureUsage.resolve_destination
    
    .. py:attribute:: sgl.TextureUsage.typeless
        :type: TextureUsage
        :value: TextureUsage.typeless
    
    .. py:attribute:: sgl.TextureUsage.shared
        :type: TextureUsage
        :value: TextureUsage.shared
    


----

.. py:class:: sgl.TextureView

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: texture
        :type: sgl.Texture
    
    .. py:property:: desc
        :type: sgl.TextureViewDesc
    
    .. py:property:: format
        :type: sgl.Format
    
    .. py:property:: aspect
        :type: sgl.TextureAspect
    
    .. py:property:: subresource_range
        :type: sgl.SubresourceRange
    
    .. py:property:: label
        :type: str
    


----

.. py:class:: sgl.TextureViewDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: format
        :type: sgl.Format
    
    .. py:property:: aspect
        :type: sgl.TextureAspect
    
    .. py:property:: subresource_range
        :type: sgl.SubresourceRange
    
    .. py:property:: label
        :type: str
    


----

.. py:class:: sgl.TypeConformance

    Type conformance entry. Type conformances are used to narrow the set
    of types supported by a slang interface. They can be specified on an
    entry point to omit generating code for types that do not conform.
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, interface_name: str, type_name: str, id: int = -1) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: tuple, /) -> None
        :no-index:
    
    .. py:property:: interface_name
        :type: str
    
        Name of the interface.
        
    .. py:property:: type_name
        :type: str
    
        Name of the concrete type.
        
    .. py:property:: id
        :type: int
    
        Unique id per type for an interface (optional).
        


----

.. py:class:: sgl.TypeLayoutReflection

    Base class: :py:class:`sgl.BaseReflectionObject`
    
    
    
    .. py:property:: kind
        :type: sgl.TypeReflection.Kind
    
    .. py:property:: name
        :type: str
    
    .. py:property:: size
        :type: int
    
    .. py:property:: stride
        :type: int
    
    .. py:property:: alignment
        :type: int
    
    .. py:property:: type
        :type: sgl.TypeReflection
    
    .. py:property:: fields
        :type: sgl.TypeLayoutReflectionFieldList
    
    .. py:property:: element_type_layout
        :type: sgl.TypeLayoutReflection
    
    .. py:method:: unwrap_array(self) -> sgl.TypeLayoutReflection
    


----

.. py:class:: sgl.TypeLayoutReflectionFieldList

    TypeLayoutReflection lazy field list evaluation.
    


----

.. py:class:: sgl.TypeReflection

    Base class: :py:class:`sgl.BaseReflectionObject`
    
    
    
    .. py:class:: sgl.TypeReflection.Kind
    
        Base class: :py:class:`enum.Enum`
        
        
        
        .. py:attribute:: sgl.TypeReflection.Kind.none
            :type: Kind
            :value: Kind.none
        
        .. py:attribute:: sgl.TypeReflection.Kind.struct
            :type: Kind
            :value: Kind.struct
        
        .. py:attribute:: sgl.TypeReflection.Kind.array
            :type: Kind
            :value: Kind.array
        
        .. py:attribute:: sgl.TypeReflection.Kind.matrix
            :type: Kind
            :value: Kind.matrix
        
        .. py:attribute:: sgl.TypeReflection.Kind.vector
            :type: Kind
            :value: Kind.vector
        
        .. py:attribute:: sgl.TypeReflection.Kind.scalar
            :type: Kind
            :value: Kind.scalar
        
        .. py:attribute:: sgl.TypeReflection.Kind.constant_buffer
            :type: Kind
            :value: Kind.constant_buffer
        
        .. py:attribute:: sgl.TypeReflection.Kind.resource
            :type: Kind
            :value: Kind.resource
        
        .. py:attribute:: sgl.TypeReflection.Kind.sampler_state
            :type: Kind
            :value: Kind.sampler_state
        
        .. py:attribute:: sgl.TypeReflection.Kind.texture_buffer
            :type: Kind
            :value: Kind.texture_buffer
        
        .. py:attribute:: sgl.TypeReflection.Kind.shader_storage_buffer
            :type: Kind
            :value: Kind.shader_storage_buffer
        
        .. py:attribute:: sgl.TypeReflection.Kind.parameter_block
            :type: Kind
            :value: Kind.parameter_block
        
        .. py:attribute:: sgl.TypeReflection.Kind.generic_type_parameter
            :type: Kind
            :value: Kind.generic_type_parameter
        
        .. py:attribute:: sgl.TypeReflection.Kind.interface
            :type: Kind
            :value: Kind.interface
        
        .. py:attribute:: sgl.TypeReflection.Kind.output_stream
            :type: Kind
            :value: Kind.output_stream
        
        .. py:attribute:: sgl.TypeReflection.Kind.specialized
            :type: Kind
            :value: Kind.specialized
        
        .. py:attribute:: sgl.TypeReflection.Kind.feedback
            :type: Kind
            :value: Kind.feedback
        
        .. py:attribute:: sgl.TypeReflection.Kind.pointer
            :type: Kind
            :value: Kind.pointer
        
    .. py:class:: sgl.TypeReflection.ScalarType
    
        Base class: :py:class:`enum.Enum`
        
        
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.none
            :type: ScalarType
            :value: ScalarType.none
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.void
            :type: ScalarType
            :value: ScalarType.void
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.bool
            :type: ScalarType
            :value: ScalarType.bool
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.int32
            :type: ScalarType
            :value: ScalarType.int32
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.uint32
            :type: ScalarType
            :value: ScalarType.uint32
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.int64
            :type: ScalarType
            :value: ScalarType.int64
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.uint64
            :type: ScalarType
            :value: ScalarType.uint64
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.float16
            :type: ScalarType
            :value: ScalarType.float16
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.float32
            :type: ScalarType
            :value: ScalarType.float32
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.float64
            :type: ScalarType
            :value: ScalarType.float64
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.int8
            :type: ScalarType
            :value: ScalarType.int8
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.uint8
            :type: ScalarType
            :value: ScalarType.uint8
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.int16
            :type: ScalarType
            :value: ScalarType.int16
        
        .. py:attribute:: sgl.TypeReflection.ScalarType.uint16
            :type: ScalarType
            :value: ScalarType.uint16
        
    .. py:class:: sgl.TypeReflection.ResourceShape
    
        Base class: :py:class:`enum.Enum`
        
        
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.none
            :type: ResourceShape
            :value: ResourceShape.none
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_1d
            :type: ResourceShape
            :value: ResourceShape.texture_1d
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_2d
            :type: ResourceShape
            :value: ResourceShape.texture_2d
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_3d
            :type: ResourceShape
            :value: ResourceShape.texture_3d
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_cube
            :type: ResourceShape
            :value: ResourceShape.texture_cube
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_buffer
            :type: ResourceShape
            :value: ResourceShape.texture_buffer
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.structured_buffer
            :type: ResourceShape
            :value: ResourceShape.structured_buffer
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.byte_address_buffer
            :type: ResourceShape
            :value: ResourceShape.byte_address_buffer
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.unknown
            :type: ResourceShape
            :value: ResourceShape.unknown
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.acceleration_structure
            :type: ResourceShape
            :value: ResourceShape.acceleration_structure
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_feedback_flag
            :type: ResourceShape
            :value: ResourceShape.texture_feedback_flag
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_array_flag
            :type: ResourceShape
            :value: ResourceShape.texture_array_flag
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_multisample_flag
            :type: ResourceShape
            :value: ResourceShape.texture_multisample_flag
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_1d_array
            :type: ResourceShape
            :value: ResourceShape.texture_1d_array
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_2d_array
            :type: ResourceShape
            :value: ResourceShape.texture_2d_array
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_cube_array
            :type: ResourceShape
            :value: ResourceShape.texture_cube_array
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_2d_multisample
            :type: ResourceShape
            :value: ResourceShape.texture_2d_multisample
        
        .. py:attribute:: sgl.TypeReflection.ResourceShape.texture_2d_multisample_array
            :type: ResourceShape
            :value: ResourceShape.texture_2d_multisample_array
        
    .. py:class:: sgl.TypeReflection.ResourceAccess
    
        Base class: :py:class:`enum.Enum`
        
        
        
        .. py:attribute:: sgl.TypeReflection.ResourceAccess.none
            :type: ResourceAccess
            :value: ResourceAccess.none
        
        .. py:attribute:: sgl.TypeReflection.ResourceAccess.read
            :type: ResourceAccess
            :value: ResourceAccess.read
        
        .. py:attribute:: sgl.TypeReflection.ResourceAccess.read_write
            :type: ResourceAccess
            :value: ResourceAccess.read_write
        
        .. py:attribute:: sgl.TypeReflection.ResourceAccess.raster_ordered
            :type: ResourceAccess
            :value: ResourceAccess.raster_ordered
        
        .. py:attribute:: sgl.TypeReflection.ResourceAccess.access_append
            :type: ResourceAccess
            :value: ResourceAccess.access_append
        
        .. py:attribute:: sgl.TypeReflection.ResourceAccess.access_consume
            :type: ResourceAccess
            :value: ResourceAccess.access_consume
        
        .. py:attribute:: sgl.TypeReflection.ResourceAccess.access_write
            :type: ResourceAccess
            :value: ResourceAccess.access_write
        
    .. py:class:: sgl.TypeReflection.ParameterCategory
    
        Base class: :py:class:`enum.Enum`
        
        
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.none
            :type: ParameterCategory
            :value: ParameterCategory.none
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.mixed
            :type: ParameterCategory
            :value: ParameterCategory.mixed
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.constant_buffer
            :type: ParameterCategory
            :value: ParameterCategory.constant_buffer
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.shader_resource
            :type: ParameterCategory
            :value: ParameterCategory.shader_resource
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.unordered_access
            :type: ParameterCategory
            :value: ParameterCategory.unordered_access
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.varying_input
            :type: ParameterCategory
            :value: ParameterCategory.varying_input
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.varying_output
            :type: ParameterCategory
            :value: ParameterCategory.varying_output
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.sampler_state
            :type: ParameterCategory
            :value: ParameterCategory.sampler_state
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.uniform
            :type: ParameterCategory
            :value: ParameterCategory.uniform
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.descriptor_table_slot
            :type: ParameterCategory
            :value: ParameterCategory.descriptor_table_slot
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.specialization_constant
            :type: ParameterCategory
            :value: ParameterCategory.specialization_constant
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.push_constant_buffer
            :type: ParameterCategory
            :value: ParameterCategory.push_constant_buffer
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.register_space
            :type: ParameterCategory
            :value: ParameterCategory.register_space
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.generic
            :type: ParameterCategory
            :value: ParameterCategory.generic
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.ray_payload
            :type: ParameterCategory
            :value: ParameterCategory.ray_payload
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.hit_attributes
            :type: ParameterCategory
            :value: ParameterCategory.hit_attributes
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.callable_payload
            :type: ParameterCategory
            :value: ParameterCategory.callable_payload
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.shader_record
            :type: ParameterCategory
            :value: ParameterCategory.shader_record
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.existential_type_param
            :type: ParameterCategory
            :value: ParameterCategory.existential_type_param
        
        .. py:attribute:: sgl.TypeReflection.ParameterCategory.existential_object_param
            :type: ParameterCategory
            :value: ParameterCategory.existential_object_param
        
    .. py:property:: kind
        :type: sgl.TypeReflection.Kind
    
    .. py:property:: name
        :type: str
    
    .. py:property:: full_name
        :type: str
    
    .. py:property:: fields
        :type: sgl.TypeReflectionFieldList
    
    .. py:property:: element_count
        :type: int
    
    .. py:property:: element_type
        :type: sgl.TypeReflection
    
    .. py:property:: row_count
        :type: int
    
    .. py:property:: col_count
        :type: int
    
    .. py:property:: scalar_type
        :type: sgl.TypeReflection.ScalarType
    
    .. py:property:: resource_result_type
        :type: sgl.TypeReflection
    
    .. py:property:: resource_shape
        :type: sgl.TypeReflection.ResourceShape
    
    .. py:property:: resource_access
        :type: sgl.TypeReflection.ResourceAccess
    
    .. py:method:: unwrap_array(self) -> sgl.TypeReflection
    


----

.. py:class:: sgl.TypeReflectionFieldList

    TypeReflection lazy field list evaluation.
    


----

.. py:class:: sgl.VariableLayoutReflection

    Base class: :py:class:`sgl.BaseReflectionObject`
    
    
    
    .. py:property:: name
        :type: str
    
    .. py:property:: variable
        :type: sgl.VariableReflection
    
    .. py:property:: type_layout
        :type: sgl.TypeLayoutReflection
    
    .. py:property:: offset
        :type: int
    


----

.. py:class:: sgl.VariableReflection

    Base class: :py:class:`sgl.BaseReflectionObject`
    
    .. py:property:: name
        :type: str
    
        Variable name.
        
    .. py:property:: type
        :type: sgl.TypeReflection
    
        Variable type reflection.
        
    .. py:method:: has_modifier(self, modifier: sgl.ModifierID) -> bool
    
        Check if variable has a given modifier (e.g. 'inout').
        


----

.. py:class:: sgl.VertexStreamDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: stride
        :type: int
    
        The stride in bytes for this vertex stream.
        
    .. py:property:: slot_class
        :type: sgl.InputSlotClass
    
        Whether the stream contains per-vertex or per-instance data.
        
    .. py:property:: instance_data_step_rate
        :type: int
    
        How many instances to draw per chunk of data.
        


----

.. py:class:: sgl.Viewport

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:staticmethod:: from_size(width: float, height: float) -> sgl.Viewport
    
    .. py:property:: x
        :type: float
    
    .. py:property:: y
        :type: float
    
    .. py:property:: width
        :type: float
    
    .. py:property:: height
        :type: float
    
    .. py:property:: min_depth
        :type: float
    
    .. py:property:: max_depth
        :type: float
    


----

.. py:class:: sgl.WindowHandle

    Native window handle.
    
    .. py:method:: __init__(self, xdisplay: int, xwindow: int) -> None
    


----

Application
-----------

.. py:class:: sgl.AppDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: device
        :type: sgl.Device
    
        Device to use for rendering. If not provided, a default device will be
        created.
        


----

.. py:class:: sgl.App

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:method:: __init__(self, arg: sgl.AppDesc, /) -> None
    
    .. py:method:: __init__(self, device: sgl.Device | None = None) -> None
        :no-index:
    
    .. py:property:: device
        :type: sgl.Device
    
    .. py:method:: run(self) -> None
    
    .. py:method:: run_frame(self) -> None
    
    .. py:method:: terminate(self) -> None
    


----

.. py:class:: sgl.AppWindowDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: width
        :type: int
    
        Width of the window in pixels.
        
    .. py:property:: height
        :type: int
    
        Height of the window in pixels.
        
    .. py:property:: title
        :type: str
    
        Title of the window.
        
    .. py:property:: mode
        :type: sgl.WindowMode
    
        Window mode.
        
    .. py:property:: resizable
        :type: bool
    
        Whether the window is resizable.
        
    .. py:property:: surface_format
        :type: sgl.Format
    
        Format of the swapchain images.
        
    .. py:property:: enable_vsync
        :type: bool
    
        Enable/disable vertical synchronization.
        


----

.. py:class:: sgl.AppWindow

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:method:: __init__(self, app: sgl.App, width: int = 1920, height: int = 1280, title: str = 'sgl', mode: sgl.WindowMode = WindowMode.normal, resizable: bool = True, surface_format: sgl.Format = Format.undefined, enable_vsync: bool = False) -> None
    
    .. py:class:: sgl.AppWindow.RenderContext
    
        
        
        .. py:property:: surface_texture
            :type: sgl.Texture
        
        .. py:property:: command_encoder
            :type: sgl.CommandEncoder
        
    .. py:property:: device
        :type: sgl.Device
    
    .. py:property:: screen
        :type: sgl.ui.Screen
    
    .. py:method:: render(self, render_context: sgl.AppWindow.RenderContext) -> None
    
    .. py:method:: on_resize(self, width: int, height: int) -> None
    
    .. py:method:: on_keyboard_event(self, event: sgl.KeyboardEvent) -> None
    
    .. py:method:: on_mouse_event(self, event: sgl.MouseEvent) -> None
    
    .. py:method:: on_gamepad_event(self, event: sgl.GamepadEvent) -> None
    
    .. py:method:: on_drop_files(self, files: Sequence[str]) -> None
    


----

Math
----

.. py:class:: sgl.math.float1

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[float]) -> None
        :no-index:
    
    .. py:property:: x
        :type: float
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.float2

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: float, y: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[float]) -> None
        :no-index:
    
    .. py:property:: x
        :type: float
    
    .. py:property:: y
        :type: float
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.float3

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: float, y: float, z: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.float2, z: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: float, yz: sgl.math.float2) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[float]) -> None
        :no-index:
    
    .. py:property:: x
        :type: float
    
    .. py:property:: y
        :type: float
    
    .. py:property:: z
        :type: float
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.float4

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: float, y: float, z: float, w: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.float2, zw: sgl.math.float2) -> None
        :no-index:
    
    .. py:method:: __init__(self, xyz: sgl.math.float3, w: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: float, yzw: sgl.math.float3) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[float]) -> None
        :no-index:
    
    .. py:property:: x
        :type: float
    
    .. py:property:: y
        :type: float
    
    .. py:property:: z
        :type: float
    
    .. py:property:: w
        :type: float
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.float1
    :canonical: sgl.math.float1
    
    Alias class: :py:class:`sgl.math.float1`
    


----

.. py:class:: sgl.float2
    :canonical: sgl.math.float2
    
    Alias class: :py:class:`sgl.math.float2`
    


----

.. py:class:: sgl.float3
    :canonical: sgl.math.float3
    
    Alias class: :py:class:`sgl.math.float3`
    


----

.. py:class:: sgl.float4
    :canonical: sgl.math.float4
    
    Alias class: :py:class:`sgl.math.float4`
    


----

.. py:class:: sgl.math.int1

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[int]) -> None
        :no-index:
    
    .. py:property:: x
        :type: int
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.int2

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, y: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[int]) -> None
        :no-index:
    
    .. py:property:: x
        :type: int
    
    .. py:property:: y
        :type: int
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.int3

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, y: int, z: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.int2, z: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, yz: sgl.math.int2) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[int]) -> None
        :no-index:
    
    .. py:property:: x
        :type: int
    
    .. py:property:: y
        :type: int
    
    .. py:property:: z
        :type: int
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.int4

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, y: int, z: int, w: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.int2, zw: sgl.math.int2) -> None
        :no-index:
    
    .. py:method:: __init__(self, xyz: sgl.math.int3, w: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, yzw: sgl.math.int3) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[int]) -> None
        :no-index:
    
    .. py:property:: x
        :type: int
    
    .. py:property:: y
        :type: int
    
    .. py:property:: z
        :type: int
    
    .. py:property:: w
        :type: int
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.int1
    :canonical: sgl.math.int1
    
    Alias class: :py:class:`sgl.math.int1`
    


----

.. py:class:: sgl.int2
    :canonical: sgl.math.int2
    
    Alias class: :py:class:`sgl.math.int2`
    


----

.. py:class:: sgl.int3
    :canonical: sgl.math.int3
    
    Alias class: :py:class:`sgl.math.int3`
    


----

.. py:class:: sgl.int4
    :canonical: sgl.math.int4
    
    Alias class: :py:class:`sgl.math.int4`
    


----

.. py:class:: sgl.math.uint1

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[int]) -> None
        :no-index:
    
    .. py:property:: x
        :type: int
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.uint2

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, y: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[int]) -> None
        :no-index:
    
    .. py:property:: x
        :type: int
    
    .. py:property:: y
        :type: int
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.uint3

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, y: int, z: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.uint2, z: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, yz: sgl.math.uint2) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[int]) -> None
        :no-index:
    
    .. py:property:: x
        :type: int
    
    .. py:property:: y
        :type: int
    
    .. py:property:: z
        :type: int
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.uint4

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, y: int, z: int, w: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.uint2, zw: sgl.math.uint2) -> None
        :no-index:
    
    .. py:method:: __init__(self, xyz: sgl.math.uint3, w: int) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: int, yzw: sgl.math.uint3) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[int]) -> None
        :no-index:
    
    .. py:property:: x
        :type: int
    
    .. py:property:: y
        :type: int
    
    .. py:property:: z
        :type: int
    
    .. py:property:: w
        :type: int
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.uint1
    :canonical: sgl.math.uint1
    
    Alias class: :py:class:`sgl.math.uint1`
    


----

.. py:class:: sgl.uint2
    :canonical: sgl.math.uint2
    
    Alias class: :py:class:`sgl.math.uint2`
    


----

.. py:class:: sgl.uint3
    :canonical: sgl.math.uint3
    
    Alias class: :py:class:`sgl.math.uint3`
    


----

.. py:class:: sgl.uint4
    :canonical: sgl.math.uint4
    
    Alias class: :py:class:`sgl.math.uint4`
    


----

.. py:class:: sgl.math.bool1

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: bool) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[bool]) -> None
        :no-index:
    
    .. py:property:: x
        :type: bool
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.bool2

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: bool) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: bool, y: bool) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[bool]) -> None
        :no-index:
    
    .. py:property:: x
        :type: bool
    
    .. py:property:: y
        :type: bool
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.bool3

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: bool) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: bool, y: bool, z: bool) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.bool2, z: bool) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: bool, yz: sgl.math.bool2) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[bool]) -> None
        :no-index:
    
    .. py:property:: x
        :type: bool
    
    .. py:property:: y
        :type: bool
    
    .. py:property:: z
        :type: bool
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.bool4

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: bool) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: bool, y: bool, z: bool, w: bool) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.bool2, zw: sgl.math.bool2) -> None
        :no-index:
    
    .. py:method:: __init__(self, xyz: sgl.math.bool3, w: bool) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: bool, yzw: sgl.math.bool3) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[bool]) -> None
        :no-index:
    
    .. py:property:: x
        :type: bool
    
    .. py:property:: y
        :type: bool
    
    .. py:property:: z
        :type: bool
    
    .. py:property:: w
        :type: bool
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.bool1
    :canonical: sgl.math.bool1
    
    Alias class: :py:class:`sgl.math.bool1`
    


----

.. py:class:: sgl.bool2
    :canonical: sgl.math.bool2
    
    Alias class: :py:class:`sgl.math.bool2`
    


----

.. py:class:: sgl.bool3
    :canonical: sgl.math.bool3
    
    Alias class: :py:class:`sgl.math.bool3`
    


----

.. py:class:: sgl.bool4
    :canonical: sgl.math.bool4
    
    Alias class: :py:class:`sgl.math.bool4`
    


----

.. py:class:: sgl.math.float16_t

    .. py:method:: __init__(self, value: float) -> None
    
    .. py:method:: __init__(self, value: float) -> None
        :no-index:
    


----

.. py:class:: sgl.float16_t
    :canonical: sgl.math.float16_t
    
    Alias class: :py:class:`sgl.math.float16_t`
    


----

.. py:class:: sgl.math.float16_t1

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: sgl.math.float16_t) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[sgl.math.float16_t]) -> None
        :no-index:
    
    .. py:property:: x
        :type: sgl.math.float16_t
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.float16_t2

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: sgl.math.float16_t) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: sgl.math.float16_t, y: sgl.math.float16_t) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[sgl.math.float16_t]) -> None
        :no-index:
    
    .. py:property:: x
        :type: sgl.math.float16_t
    
    .. py:property:: y
        :type: sgl.math.float16_t
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.float16_t3

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: sgl.math.float16_t) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: sgl.math.float16_t, y: sgl.math.float16_t, z: sgl.math.float16_t) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.float16_t2, z: sgl.math.float16_t) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: sgl.math.float16_t, yz: sgl.math.float16_t2) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[sgl.math.float16_t]) -> None
        :no-index:
    
    .. py:property:: x
        :type: sgl.math.float16_t
    
    .. py:property:: y
        :type: sgl.math.float16_t
    
    .. py:property:: z
        :type: sgl.math.float16_t
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.math.float16_t4

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, scalar: sgl.math.float16_t) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: sgl.math.float16_t, y: sgl.math.float16_t, z: sgl.math.float16_t, w: sgl.math.float16_t) -> None
        :no-index:
    
    .. py:method:: __init__(self, xy: sgl.math.float16_t2, zw: sgl.math.float16_t2) -> None
        :no-index:
    
    .. py:method:: __init__(self, xyz: sgl.math.float16_t3, w: sgl.math.float16_t) -> None
        :no-index:
    
    .. py:method:: __init__(self, x: sgl.math.float16_t, yzw: sgl.math.float16_t3) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[sgl.math.float16_t]) -> None
        :no-index:
    
    .. py:property:: x
        :type: sgl.math.float16_t
    
    .. py:property:: y
        :type: sgl.math.float16_t
    
    .. py:property:: z
        :type: sgl.math.float16_t
    
    .. py:property:: w
        :type: sgl.math.float16_t
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.float16_t1
    :canonical: sgl.math.float16_t1
    
    Alias class: :py:class:`sgl.math.float16_t1`
    


----

.. py:class:: sgl.float16_t2
    :canonical: sgl.math.float16_t2
    
    Alias class: :py:class:`sgl.math.float16_t2`
    


----

.. py:class:: sgl.float16_t3
    :canonical: sgl.math.float16_t3
    
    Alias class: :py:class:`sgl.math.float16_t3`
    


----

.. py:class:: sgl.float16_t4
    :canonical: sgl.math.float16_t4
    
    Alias class: :py:class:`sgl.math.float16_t4`
    


----

.. py:class:: sgl.math.float2x2

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: collections.abc.Sequence[float], /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: ndarray[dtype=float32, shape=(2, 2)], /) -> None
        :no-index:
    
    .. py:staticmethod:: zeros() -> sgl.math.float2x2
    
    .. py:staticmethod:: identity() -> sgl.math.float2x2
    
    .. py:method:: get_row(self, row: int) -> sgl.math.float2
    
    .. py:method:: set_row(self, row: int, value: sgl.math.float2) -> None
    
    .. py:method:: get_col(self, col: int) -> sgl.math.float2
    
    .. py:method:: set_col(self, col: int, value: sgl.math.float2) -> None
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, shape=(2, 2), writable=False]
    


----

.. py:class:: sgl.math.float3x3

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: sgl.math.float4x4, /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: sgl.math.float3x4, /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: collections.abc.Sequence[float], /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: ndarray[dtype=float32, shape=(3, 3)], /) -> None
        :no-index:
    
    .. py:staticmethod:: zeros() -> sgl.math.float3x3
    
    .. py:staticmethod:: identity() -> sgl.math.float3x3
    
    .. py:method:: get_row(self, row: int) -> sgl.math.float3
    
    .. py:method:: set_row(self, row: int, value: sgl.math.float3) -> None
    
    .. py:method:: get_col(self, col: int) -> sgl.math.float3
    
    .. py:method:: set_col(self, col: int, value: sgl.math.float3) -> None
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, shape=(3, 3), writable=False]
    


----

.. py:class:: sgl.math.float2x4

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: collections.abc.Sequence[float], /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: ndarray[dtype=float32, shape=(2, 4)], /) -> None
        :no-index:
    
    .. py:staticmethod:: zeros() -> sgl.math.float2x4
    
    .. py:staticmethod:: identity() -> sgl.math.float2x4
    
    .. py:method:: get_row(self, row: int) -> sgl.math.float4
    
    .. py:method:: set_row(self, row: int, value: sgl.math.float4) -> None
    
    .. py:method:: get_col(self, col: int) -> sgl.math.float2
    
    .. py:method:: set_col(self, col: int, value: sgl.math.float2) -> None
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, shape=(2, 4), writable=False]
    


----

.. py:class:: sgl.math.float3x4

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: sgl.math.float3x3, /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: sgl.math.float4x4, /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: collections.abc.Sequence[float], /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: ndarray[dtype=float32, shape=(3, 4)], /) -> None
        :no-index:
    
    .. py:staticmethod:: zeros() -> sgl.math.float3x4
    
    .. py:staticmethod:: identity() -> sgl.math.float3x4
    
    .. py:method:: get_row(self, row: int) -> sgl.math.float4
    
    .. py:method:: set_row(self, row: int, value: sgl.math.float4) -> None
    
    .. py:method:: get_col(self, col: int) -> sgl.math.float3
    
    .. py:method:: set_col(self, col: int, value: sgl.math.float3) -> None
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, shape=(3, 4), writable=False]
    


----

.. py:class:: sgl.math.float4x4

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: sgl.math.float3x3, /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: sgl.math.float3x4, /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: collections.abc.Sequence[float], /) -> None
        :no-index:
    
    .. py:method:: __init__(self, arg: ndarray[dtype=float32, shape=(4, 4)], /) -> None
        :no-index:
    
    .. py:staticmethod:: zeros() -> sgl.math.float4x4
    
    .. py:staticmethod:: identity() -> sgl.math.float4x4
    
    .. py:method:: get_row(self, row: int) -> sgl.math.float4
    
    .. py:method:: set_row(self, row: int, value: sgl.math.float4) -> None
    
    .. py:method:: get_col(self, col: int) -> sgl.math.float4
    
    .. py:method:: set_col(self, col: int, value: sgl.math.float4) -> None
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, shape=(4, 4), writable=False]
    


----

.. py:class:: sgl.float2x2
    :canonical: sgl.math.float2x2
    
    Alias class: :py:class:`sgl.math.float2x2`
    


----

.. py:class:: sgl.float3x3
    :canonical: sgl.math.float3x3
    
    Alias class: :py:class:`sgl.math.float3x3`
    


----

.. py:class:: sgl.float2x4
    :canonical: sgl.math.float2x4
    
    Alias class: :py:class:`sgl.math.float2x4`
    


----

.. py:class:: sgl.float3x4
    :canonical: sgl.math.float3x4
    
    Alias class: :py:class:`sgl.math.float3x4`
    


----

.. py:class:: sgl.float4x4
    :canonical: sgl.math.float4x4
    
    Alias class: :py:class:`sgl.math.float4x4`
    


----

.. py:class:: sgl.math.quatf

    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, x: float, y: float, z: float, w: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, xyz: sgl.math.float3, w: float) -> None
        :no-index:
    
    .. py:method:: __init__(self, a: collections.abc.Sequence[float]) -> None
        :no-index:
    
    .. py:staticmethod:: identity() -> sgl.math.quatf
    
    .. py:property:: x
        :type: float
    
    .. py:property:: y
        :type: float
    
    .. py:property:: z
        :type: float
    
    .. py:property:: w
        :type: float
    
    .. py:property:: shape
        :type: tuple
    
    .. py:property:: element_type
        :type: object
    


----

.. py:class:: sgl.quatf
    :canonical: sgl.math.quatf
    
    Alias class: :py:class:`sgl.math.quatf`
    


----

.. py:class:: sgl.math.Handedness

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.math.Handedness.right_handed
        :type: Handedness
        :value: Handedness.right_handed
    
    .. py:attribute:: sgl.math.Handedness.left_handed
        :type: Handedness
        :value: Handedness.left_handed
    


----

.. py:function:: sgl.math.isfinite(x: float) -> bool

.. py:function:: sgl.math.isfinite(x: float) -> bool
    :no-index:

.. py:function:: sgl.math.isfinite(x: sgl.math.float16_t) -> bool
    :no-index:

.. py:function:: sgl.math.isfinite(x: sgl.math.float1) -> sgl.math.bool1
    :no-index:

.. py:function:: sgl.math.isfinite(x: sgl.math.float2) -> sgl.math.bool2
    :no-index:

.. py:function:: sgl.math.isfinite(x: sgl.math.float3) -> sgl.math.bool3
    :no-index:

.. py:function:: sgl.math.isfinite(x: sgl.math.float4) -> sgl.math.bool4
    :no-index:

.. py:function:: sgl.math.isfinite(x: sgl.math.quatf) -> sgl.math.bool4
    :no-index:



----

.. py:function:: sgl.math.isinf(x: float) -> bool

.. py:function:: sgl.math.isinf(x: float) -> bool
    :no-index:

.. py:function:: sgl.math.isinf(x: sgl.math.float16_t) -> bool
    :no-index:

.. py:function:: sgl.math.isinf(x: sgl.math.float1) -> sgl.math.bool1
    :no-index:

.. py:function:: sgl.math.isinf(x: sgl.math.float2) -> sgl.math.bool2
    :no-index:

.. py:function:: sgl.math.isinf(x: sgl.math.float3) -> sgl.math.bool3
    :no-index:

.. py:function:: sgl.math.isinf(x: sgl.math.float4) -> sgl.math.bool4
    :no-index:

.. py:function:: sgl.math.isinf(x: sgl.math.quatf) -> sgl.math.bool4
    :no-index:



----

.. py:function:: sgl.math.isnan(x: float) -> bool

.. py:function:: sgl.math.isnan(x: float) -> bool
    :no-index:

.. py:function:: sgl.math.isnan(x: sgl.math.float16_t) -> bool
    :no-index:

.. py:function:: sgl.math.isnan(x: sgl.math.float1) -> sgl.math.bool1
    :no-index:

.. py:function:: sgl.math.isnan(x: sgl.math.float2) -> sgl.math.bool2
    :no-index:

.. py:function:: sgl.math.isnan(x: sgl.math.float3) -> sgl.math.bool3
    :no-index:

.. py:function:: sgl.math.isnan(x: sgl.math.float4) -> sgl.math.bool4
    :no-index:

.. py:function:: sgl.math.isnan(x: sgl.math.quatf) -> sgl.math.bool4
    :no-index:



----

.. py:function:: sgl.math.floor(x: float) -> float

.. py:function:: sgl.math.floor(x: float) -> float
    :no-index:

.. py:function:: sgl.math.floor(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.floor(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.floor(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.floor(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.ceil(x: float) -> float

.. py:function:: sgl.math.ceil(x: float) -> float
    :no-index:

.. py:function:: sgl.math.ceil(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.ceil(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.ceil(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.ceil(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.trunc(x: float) -> float

.. py:function:: sgl.math.trunc(x: float) -> float
    :no-index:

.. py:function:: sgl.math.trunc(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.trunc(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.trunc(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.trunc(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.round(x: float) -> float

.. py:function:: sgl.math.round(x: float) -> float
    :no-index:

.. py:function:: sgl.math.round(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.round(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.round(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.round(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.pow(x: float, y: float) -> float

.. py:function:: sgl.math.pow(x: float, y: float) -> float
    :no-index:

.. py:function:: sgl.math.pow(x: sgl.math.float1, y: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.pow(x: sgl.math.float2, y: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.pow(x: sgl.math.float3, y: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.pow(x: sgl.math.float4, y: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.sqrt(x: float) -> float

.. py:function:: sgl.math.sqrt(x: float) -> float
    :no-index:

.. py:function:: sgl.math.sqrt(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.sqrt(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.sqrt(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.sqrt(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.rsqrt(x: float) -> float

.. py:function:: sgl.math.rsqrt(x: float) -> float
    :no-index:

.. py:function:: sgl.math.rsqrt(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.rsqrt(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.rsqrt(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.rsqrt(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.exp(x: float) -> float

.. py:function:: sgl.math.exp(x: float) -> float
    :no-index:

.. py:function:: sgl.math.exp(x: sgl.math.float16_t) -> sgl.math.float16_t
    :no-index:

.. py:function:: sgl.math.exp(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.exp(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.exp(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.exp(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.exp2(x: float) -> float

.. py:function:: sgl.math.exp2(x: float) -> float
    :no-index:

.. py:function:: sgl.math.exp2(x: sgl.math.float16_t) -> sgl.math.float16_t
    :no-index:

.. py:function:: sgl.math.exp2(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.exp2(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.exp2(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.exp2(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.log(x: float) -> float

.. py:function:: sgl.math.log(x: float) -> float
    :no-index:

.. py:function:: sgl.math.log(x: sgl.math.float16_t) -> sgl.math.float16_t
    :no-index:

.. py:function:: sgl.math.log(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.log(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.log(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.log(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.log2(x: float) -> float

.. py:function:: sgl.math.log2(x: float) -> float
    :no-index:

.. py:function:: sgl.math.log2(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.log2(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.log2(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.log2(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.log10(x: float) -> float

.. py:function:: sgl.math.log10(x: float) -> float
    :no-index:

.. py:function:: sgl.math.log10(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.log10(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.log10(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.log10(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.radians(x: float) -> float

.. py:function:: sgl.math.radians(x: float) -> float
    :no-index:

.. py:function:: sgl.math.radians(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.radians(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.radians(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.radians(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.degrees(x: float) -> float

.. py:function:: sgl.math.degrees(x: float) -> float
    :no-index:

.. py:function:: sgl.math.degrees(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.degrees(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.degrees(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.degrees(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.sin(x: float) -> float

.. py:function:: sgl.math.sin(x: float) -> float
    :no-index:

.. py:function:: sgl.math.sin(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.sin(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.sin(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.sin(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.cos(x: float) -> float

.. py:function:: sgl.math.cos(x: float) -> float
    :no-index:

.. py:function:: sgl.math.cos(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.cos(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.cos(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.cos(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.tan(x: float) -> float

.. py:function:: sgl.math.tan(x: float) -> float
    :no-index:

.. py:function:: sgl.math.tan(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.tan(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.tan(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.tan(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.asin(x: float) -> float

.. py:function:: sgl.math.asin(x: float) -> float
    :no-index:

.. py:function:: sgl.math.asin(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.asin(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.asin(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.asin(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.acos(x: float) -> float

.. py:function:: sgl.math.acos(x: float) -> float
    :no-index:

.. py:function:: sgl.math.acos(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.acos(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.acos(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.acos(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.atan(x: float) -> float

.. py:function:: sgl.math.atan(x: float) -> float
    :no-index:

.. py:function:: sgl.math.atan(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.atan(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.atan(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.atan(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.atan2(y: float, x: float) -> float

.. py:function:: sgl.math.atan2(y: float, x: float) -> float
    :no-index:

.. py:function:: sgl.math.atan2(y: sgl.math.float1, x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.atan2(y: sgl.math.float2, x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.atan2(y: sgl.math.float3, x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.atan2(y: sgl.math.float4, x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.sinh(x: float) -> float

.. py:function:: sgl.math.sinh(x: float) -> float
    :no-index:

.. py:function:: sgl.math.sinh(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.sinh(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.sinh(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.sinh(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.cosh(x: float) -> float

.. py:function:: sgl.math.cosh(x: float) -> float
    :no-index:

.. py:function:: sgl.math.cosh(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.cosh(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.cosh(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.cosh(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.tanh(x: float) -> float

.. py:function:: sgl.math.tanh(x: float) -> float
    :no-index:

.. py:function:: sgl.math.tanh(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.tanh(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.tanh(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.tanh(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.fmod(x: float, y: float) -> float

.. py:function:: sgl.math.fmod(x: float, y: float) -> float
    :no-index:

.. py:function:: sgl.math.fmod(x: sgl.math.float1, y: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.fmod(x: sgl.math.float2, y: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.fmod(x: sgl.math.float3, y: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.fmod(x: sgl.math.float4, y: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.frac(x: float) -> float

.. py:function:: sgl.math.frac(x: float) -> float
    :no-index:

.. py:function:: sgl.math.frac(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.frac(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.frac(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.frac(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.lerp(x: float, y: float, s: float) -> float

.. py:function:: sgl.math.lerp(x: float, y: float, s: float) -> float
    :no-index:

.. py:function:: sgl.math.lerp(x: sgl.math.float1, y: sgl.math.float1, s: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.lerp(x: sgl.math.float1, y: sgl.math.float1, s: float) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.lerp(x: sgl.math.float2, y: sgl.math.float2, s: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.lerp(x: sgl.math.float2, y: sgl.math.float2, s: float) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.lerp(x: sgl.math.float3, y: sgl.math.float3, s: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.lerp(x: sgl.math.float3, y: sgl.math.float3, s: float) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.lerp(x: sgl.math.float4, y: sgl.math.float4, s: sgl.math.float4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.lerp(x: sgl.math.float4, y: sgl.math.float4, s: float) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.lerp(x: sgl.math.quatf, y: sgl.math.quatf, s: float) -> sgl.math.quatf
    :no-index:



----

.. py:function:: sgl.math.rcp(x: float) -> float

.. py:function:: sgl.math.rcp(x: float) -> float
    :no-index:

.. py:function:: sgl.math.rcp(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.rcp(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.rcp(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.rcp(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.saturate(x: float) -> float

.. py:function:: sgl.math.saturate(x: float) -> float
    :no-index:

.. py:function:: sgl.math.saturate(x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.saturate(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.saturate(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.saturate(x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.step(x: float, y: float) -> float

.. py:function:: sgl.math.step(x: float, y: float) -> float
    :no-index:

.. py:function:: sgl.math.step(x: sgl.math.float1, y: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.step(x: sgl.math.float2, y: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.step(x: sgl.math.float3, y: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.step(x: sgl.math.float4, y: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.smoothstep(min: float, max: float, x: float) -> float

.. py:function:: sgl.math.smoothstep(min: float, max: float, x: float) -> float
    :no-index:

.. py:function:: sgl.math.smoothstep(min: sgl.math.float1, max: sgl.math.float1, x: sgl.math.float1) -> sgl.math.float1
    :no-index:

.. py:function:: sgl.math.smoothstep(min: sgl.math.float2, max: sgl.math.float2, x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.smoothstep(min: sgl.math.float3, max: sgl.math.float3, x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.smoothstep(min: sgl.math.float4, max: sgl.math.float4, x: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.f16tof32(x: int) -> float

.. py:function:: sgl.math.f16tof32(x: sgl.math.uint2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.f16tof32(x: sgl.math.uint3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.f16tof32(x: sgl.math.uint4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.f32tof16(x: float) -> int

.. py:function:: sgl.math.f32tof16(x: sgl.math.float2) -> sgl.math.uint2
    :no-index:

.. py:function:: sgl.math.f32tof16(x: sgl.math.float3) -> sgl.math.uint3
    :no-index:

.. py:function:: sgl.math.f32tof16(x: sgl.math.float4) -> sgl.math.uint4
    :no-index:



----

.. py:function:: sgl.math.asfloat(x: int) -> float

.. py:function:: sgl.math.asfloat(x: int) -> float
    :no-index:



----

.. py:function:: sgl.math.asfloat16(x: int) -> sgl.math.float16_t



----

.. py:function:: sgl.math.asuint(x: float) -> int

.. py:function:: sgl.math.asuint(x: sgl.math.float2) -> sgl.math.uint2
    :no-index:

.. py:function:: sgl.math.asuint(x: sgl.math.float3) -> sgl.math.uint3
    :no-index:

.. py:function:: sgl.math.asuint(x: sgl.math.float4) -> sgl.math.uint4
    :no-index:



----

.. py:function:: sgl.math.asint(x: float) -> int



----

.. py:function:: sgl.math.asuint16(x: sgl.math.float16_t) -> int



----

.. py:function:: sgl.math.min(x: sgl.math.float1, y: sgl.math.float1) -> sgl.math.float1

.. py:function:: sgl.math.min(x: sgl.math.float2, y: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.float3, y: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.float4, y: sgl.math.float4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.uint1, y: sgl.math.uint1) -> sgl.math.uint1
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.uint2, y: sgl.math.uint2) -> sgl.math.uint2
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.uint3, y: sgl.math.uint3) -> sgl.math.uint3
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.uint4, y: sgl.math.uint4) -> sgl.math.uint4
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.int1, y: sgl.math.int1) -> sgl.math.int1
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.int2, y: sgl.math.int2) -> sgl.math.int2
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.int3, y: sgl.math.int3) -> sgl.math.int3
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.int4, y: sgl.math.int4) -> sgl.math.int4
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.bool1, y: sgl.math.bool1) -> sgl.math.bool1
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.bool2, y: sgl.math.bool2) -> sgl.math.bool2
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.bool3, y: sgl.math.bool3) -> sgl.math.bool3
    :no-index:

.. py:function:: sgl.math.min(x: sgl.math.bool4, y: sgl.math.bool4) -> sgl.math.bool4
    :no-index:



----

.. py:function:: sgl.math.max(x: sgl.math.float1, y: sgl.math.float1) -> sgl.math.float1

.. py:function:: sgl.math.max(x: sgl.math.float2, y: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.float3, y: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.float4, y: sgl.math.float4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.uint1, y: sgl.math.uint1) -> sgl.math.uint1
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.uint2, y: sgl.math.uint2) -> sgl.math.uint2
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.uint3, y: sgl.math.uint3) -> sgl.math.uint3
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.uint4, y: sgl.math.uint4) -> sgl.math.uint4
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.int1, y: sgl.math.int1) -> sgl.math.int1
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.int2, y: sgl.math.int2) -> sgl.math.int2
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.int3, y: sgl.math.int3) -> sgl.math.int3
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.int4, y: sgl.math.int4) -> sgl.math.int4
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.bool1, y: sgl.math.bool1) -> sgl.math.bool1
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.bool2, y: sgl.math.bool2) -> sgl.math.bool2
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.bool3, y: sgl.math.bool3) -> sgl.math.bool3
    :no-index:

.. py:function:: sgl.math.max(x: sgl.math.bool4, y: sgl.math.bool4) -> sgl.math.bool4
    :no-index:



----

.. py:function:: sgl.math.clamp(x: sgl.math.float1, min: sgl.math.float1, max: sgl.math.float1) -> sgl.math.float1

.. py:function:: sgl.math.clamp(x: sgl.math.float2, min: sgl.math.float2, max: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.float3, min: sgl.math.float3, max: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.float4, min: sgl.math.float4, max: sgl.math.float4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.uint1, min: sgl.math.uint1, max: sgl.math.uint1) -> sgl.math.uint1
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.uint2, min: sgl.math.uint2, max: sgl.math.uint2) -> sgl.math.uint2
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.uint3, min: sgl.math.uint3, max: sgl.math.uint3) -> sgl.math.uint3
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.uint4, min: sgl.math.uint4, max: sgl.math.uint4) -> sgl.math.uint4
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.int1, min: sgl.math.int1, max: sgl.math.int1) -> sgl.math.int1
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.int2, min: sgl.math.int2, max: sgl.math.int2) -> sgl.math.int2
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.int3, min: sgl.math.int3, max: sgl.math.int3) -> sgl.math.int3
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.int4, min: sgl.math.int4, max: sgl.math.int4) -> sgl.math.int4
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.bool1, min: sgl.math.bool1, max: sgl.math.bool1) -> sgl.math.bool1
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.bool2, min: sgl.math.bool2, max: sgl.math.bool2) -> sgl.math.bool2
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.bool3, min: sgl.math.bool3, max: sgl.math.bool3) -> sgl.math.bool3
    :no-index:

.. py:function:: sgl.math.clamp(x: sgl.math.bool4, min: sgl.math.bool4, max: sgl.math.bool4) -> sgl.math.bool4
    :no-index:



----

.. py:function:: sgl.math.abs(x: sgl.math.float1) -> sgl.math.float1

.. py:function:: sgl.math.abs(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.abs(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.abs(x: sgl.math.float4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.abs(x: sgl.math.int1) -> sgl.math.int1
    :no-index:

.. py:function:: sgl.math.abs(x: sgl.math.int2) -> sgl.math.int2
    :no-index:

.. py:function:: sgl.math.abs(x: sgl.math.int3) -> sgl.math.int3
    :no-index:

.. py:function:: sgl.math.abs(x: sgl.math.int4) -> sgl.math.int4
    :no-index:



----

.. py:function:: sgl.math.sign(x: sgl.math.float1) -> sgl.math.float1

.. py:function:: sgl.math.sign(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.sign(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.sign(x: sgl.math.float4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.sign(x: sgl.math.int1) -> sgl.math.int1
    :no-index:

.. py:function:: sgl.math.sign(x: sgl.math.int2) -> sgl.math.int2
    :no-index:

.. py:function:: sgl.math.sign(x: sgl.math.int3) -> sgl.math.int3
    :no-index:

.. py:function:: sgl.math.sign(x: sgl.math.int4) -> sgl.math.int4
    :no-index:



----

.. py:function:: sgl.math.dot(x: sgl.math.float1, y: sgl.math.float1) -> float

.. py:function:: sgl.math.dot(x: sgl.math.float2, y: sgl.math.float2) -> float
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.float3, y: sgl.math.float3) -> float
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.float4, y: sgl.math.float4) -> float
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.uint1, y: sgl.math.uint1) -> int
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.uint2, y: sgl.math.uint2) -> int
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.uint3, y: sgl.math.uint3) -> int
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.uint4, y: sgl.math.uint4) -> int
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.int1, y: sgl.math.int1) -> int
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.int2, y: sgl.math.int2) -> int
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.int3, y: sgl.math.int3) -> int
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.int4, y: sgl.math.int4) -> int
    :no-index:

.. py:function:: sgl.math.dot(x: sgl.math.quatf, y: sgl.math.quatf) -> float
    :no-index:



----

.. py:function:: sgl.math.length(x: sgl.math.float1) -> float

.. py:function:: sgl.math.length(x: sgl.math.float2) -> float
    :no-index:

.. py:function:: sgl.math.length(x: sgl.math.float3) -> float
    :no-index:

.. py:function:: sgl.math.length(x: sgl.math.float4) -> float
    :no-index:

.. py:function:: sgl.math.length(x: sgl.math.quatf) -> float
    :no-index:



----

.. py:function:: sgl.math.normalize(x: sgl.math.float1) -> sgl.math.float1

.. py:function:: sgl.math.normalize(x: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.normalize(x: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.normalize(x: sgl.math.float4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.normalize(x: sgl.math.quatf) -> sgl.math.quatf
    :no-index:



----

.. py:function:: sgl.math.reflect(i: sgl.math.float1, n: sgl.math.float1) -> sgl.math.float1

.. py:function:: sgl.math.reflect(i: sgl.math.float2, n: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.reflect(i: sgl.math.float3, n: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.reflect(i: sgl.math.float4, n: sgl.math.float4) -> sgl.math.float4
    :no-index:



----

.. py:function:: sgl.math.cross(x: sgl.math.float3, y: sgl.math.float3) -> sgl.math.float3

.. py:function:: sgl.math.cross(x: sgl.math.uint3, y: sgl.math.uint3) -> sgl.math.uint3
    :no-index:

.. py:function:: sgl.math.cross(x: sgl.math.int3, y: sgl.math.int3) -> sgl.math.int3
    :no-index:

.. py:function:: sgl.math.cross(x: sgl.math.quatf, y: sgl.math.quatf) -> sgl.math.quatf
    :no-index:



----

.. py:function:: sgl.math.any(x: sgl.math.bool1) -> bool

.. py:function:: sgl.math.any(x: sgl.math.bool2) -> bool
    :no-index:

.. py:function:: sgl.math.any(x: sgl.math.bool3) -> bool
    :no-index:

.. py:function:: sgl.math.any(x: sgl.math.bool4) -> bool
    :no-index:



----

.. py:function:: sgl.math.all(x: sgl.math.bool1) -> bool

.. py:function:: sgl.math.all(x: sgl.math.bool2) -> bool
    :no-index:

.. py:function:: sgl.math.all(x: sgl.math.bool3) -> bool
    :no-index:

.. py:function:: sgl.math.all(x: sgl.math.bool4) -> bool
    :no-index:



----

.. py:function:: sgl.math.none(x: sgl.math.bool1) -> bool

.. py:function:: sgl.math.none(x: sgl.math.bool2) -> bool
    :no-index:

.. py:function:: sgl.math.none(x: sgl.math.bool3) -> bool
    :no-index:

.. py:function:: sgl.math.none(x: sgl.math.bool4) -> bool
    :no-index:



----

.. py:function:: sgl.math.transpose(x: sgl.math.float2x2) -> sgl.math.float2x2

.. py:function:: sgl.math.transpose(x: sgl.math.float3x3) -> sgl.math.float3x3
    :no-index:

.. py:function:: sgl.math.transpose(x: sgl.math.float2x4) -> sgl::math::matrix<float, 4, 2>
    :no-index:

.. py:function:: sgl.math.transpose(x: sgl.math.float3x4) -> sgl::math::matrix<float, 4, 3>
    :no-index:

.. py:function:: sgl.math.transpose(x: sgl.math.float4x4) -> sgl.math.float4x4
    :no-index:



----

.. py:function:: sgl.math.determinant(x: sgl.math.float2x2) -> float

.. py:function:: sgl.math.determinant(x: sgl.math.float3x3) -> float
    :no-index:

.. py:function:: sgl.math.determinant(x: sgl.math.float4x4) -> float
    :no-index:



----

.. py:function:: sgl.math.inverse(x: sgl.math.float2x2) -> sgl.math.float2x2

.. py:function:: sgl.math.inverse(x: sgl.math.float3x3) -> sgl.math.float3x3
    :no-index:

.. py:function:: sgl.math.inverse(x: sgl.math.float4x4) -> sgl.math.float4x4
    :no-index:

.. py:function:: sgl.math.inverse(x: sgl.math.quatf) -> sgl.math.quatf
    :no-index:



----

.. py:function:: sgl.math.mul(x: sgl.math.float2x2, y: sgl.math.float2x2) -> sgl.math.float2x2

.. py:function:: sgl.math.mul(x: sgl.math.float2x2, y: sgl.math.float2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float2, y: sgl.math.float2x2) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float3x3, y: sgl.math.float3x3) -> sgl.math.float3x3
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float3x3, y: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float3, y: sgl.math.float3x3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float2x4, y: sgl::math::matrix<float, 4, 2>) -> sgl.math.float2x2
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float2x4, y: sgl.math.float4) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float2, y: sgl.math.float2x4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float3x4, y: sgl::math::matrix<float, 4, 3>) -> sgl.math.float3x3
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float3x4, y: sgl.math.float4) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float3, y: sgl.math.float3x4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float4x4, y: sgl.math.float4x4) -> sgl.math.float4x4
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float4x4, y: sgl.math.float4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float4, y: sgl.math.float4x4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.quatf, y: sgl.math.quatf) -> sgl.math.quatf
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.quatf, y: sgl.math.float3) -> sgl.math.float3
    :no-index:



----

.. py:function:: sgl.math.transform_point(m: sgl.math.float4x4, v: sgl.math.float3) -> sgl.math.float3



----

.. py:function:: sgl.math.transform_vector(m: sgl.math.float3x3, v: sgl.math.float3) -> sgl.math.float3

.. py:function:: sgl.math.transform_vector(m: sgl.math.float4x4, v: sgl.math.float3) -> sgl.math.float3
    :no-index:

.. py:function:: sgl.math.transform_vector(q: sgl.math.quatf, v: sgl.math.float3) -> sgl.math.float3
    :no-index:



----

.. py:function:: sgl.math.translate(m: sgl.math.float4x4, v: sgl.math.float3) -> sgl.math.float4x4



----

.. py:function:: sgl.math.rotate(m: sgl.math.float4x4, angle: float, axis: sgl.math.float3) -> sgl.math.float4x4



----

.. py:function:: sgl.math.scale(m: sgl.math.float4x4, v: sgl.math.float3) -> sgl.math.float4x4



----

.. py:function:: sgl.math.perspective(fovy: float, aspect: float, z_near: float, z_far: float) -> sgl.math.float4x4



----

.. py:function:: sgl.math.ortho(left: float, right: float, bottom: float, top: float, z_near: float, z_far: float) -> sgl.math.float4x4



----

.. py:function:: sgl.math.matrix_from_translation(v: sgl.math.float3) -> sgl.math.float4x4



----

.. py:function:: sgl.math.matrix_from_scaling(v: sgl.math.float3) -> sgl.math.float4x4



----

.. py:function:: sgl.math.matrix_from_rotation(angle: float, axis: sgl.math.float3) -> sgl.math.float4x4



----

.. py:function:: sgl.math.matrix_from_rotation_x(angle: float) -> sgl.math.float4x4



----

.. py:function:: sgl.math.matrix_from_rotation_y(angle: float) -> sgl.math.float4x4



----

.. py:function:: sgl.math.matrix_from_rotation_z(angle: float) -> sgl.math.float4x4



----

.. py:function:: sgl.math.matrix_from_rotation_xyz(angle_x: float, angle_y: float, angle_z: float) -> sgl.math.float4x4

.. py:function:: sgl.math.matrix_from_rotation_xyz(angles: sgl.math.float3) -> sgl.math.float4x4
    :no-index:



----

.. py:function:: sgl.math.matrix_from_look_at(eye: sgl.math.float3, center: sgl.math.float3, up: sgl.math.float3, handedness: sgl.math.Handedness = Handedness.right_handed) -> sgl.math.float4x4



----

.. py:function:: sgl.math.matrix_from_quat(q: sgl.math.quatf) -> sgl.math.float3x3



----

.. py:function:: sgl.math.conjugate(x: sgl.math.quatf) -> sgl.math.quatf



----

.. py:function:: sgl.math.slerp(x: sgl.math.quatf, y: sgl.math.quatf, s: float) -> sgl.math.quatf



----

.. py:function:: sgl.math.pitch(x: sgl.math.quatf) -> float



----

.. py:function:: sgl.math.yaw(x: sgl.math.quatf) -> float



----

.. py:function:: sgl.math.roll(x: sgl.math.quatf) -> float



----

.. py:function:: sgl.math.euler_angles(x: sgl.math.quatf) -> sgl.math.float3



----

.. py:function:: sgl.math.quat_from_angle_axis(angle: float, axis: sgl.math.float3) -> sgl.math.quatf



----

.. py:function:: sgl.math.quat_from_rotation_between_vectors(from_: sgl.math.float3, to: sgl.math.float3) -> sgl.math.quatf



----

.. py:function:: sgl.math.quat_from_euler_angles(angles: sgl.math.float3) -> sgl.math.quatf



----

.. py:function:: sgl.math.quat_from_matrix(m: sgl.math.float3x3) -> sgl.math.quatf



----

.. py:function:: sgl.math.quat_from_look_at(dir: sgl.math.float3, up: sgl.math.float3, handedness: sgl.math.Handedness = Handedness.right_handed) -> sgl.math.quatf



----

UI
--

.. py:class:: sgl.ui.Context

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:method:: __init__(self, device: sgl.Device) -> None
    
    .. py:method:: new_frame(self, width: int, height: int) -> None
    
    .. py:method:: render(self, texture_view: sgl.TextureView, command_encoder: sgl.CommandEncoder) -> None
    
    .. py:method:: render(self, texture: sgl.Texture, command_encoder: sgl.CommandEncoder) -> None
        :no-index:
    
    .. py:method:: handle_keyboard_event(self, event: sgl.KeyboardEvent) -> bool
    
    .. py:method:: handle_mouse_event(self, event: sgl.MouseEvent) -> bool
    
    .. py:method:: process_events(self) -> None
    
    .. py:property:: screen
        :type: sgl.ui.Screen
    


----

.. py:class:: sgl.ui.Widget

    Base class: :py:class:`sgl.Object`
    
    Base class for Python UI widgets. Widgets own their children.
    
    .. py:property:: parent
        :type: sgl.ui.Widget
    
    .. py:property:: children
        :type: list[sgl.ui.Widget]
    
    .. py:property:: visible
        :type: bool
    
    .. py:property:: enabled
        :type: bool
    
    .. py:method:: child_index(self, child: sgl.ui.Widget) -> int
    
    .. py:method:: add_child(self, child: sgl.ui.Widget) -> None
    
    .. py:method:: add_child_at(self, child: sgl.ui.Widget, index: int) -> None
    
    .. py:method:: remove_child(self, child: sgl.ui.Widget) -> None
    
    .. py:method:: remove_child_at(self, index: int) -> None
    
    .. py:method:: remove_all_children(self) -> None
    


----

.. py:class:: sgl.ui.Screen

    Base class: :py:class:`sgl.ui.Widget`
    
    This is the main widget that represents the screen. It is intended to
    be used as the parent for ``Window`` widgets.
    
    .. py:method:: dispatch_events(self) -> None
    


----

.. py:class:: sgl.ui.Window

    Base class: :py:class:`sgl.ui.Widget`
    
    
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, title: str = '', position: sgl.math.float2 = {10, 10}, size: sgl.math.float2 = {400, 400}) -> None
    
    .. py:method:: show(self) -> None
    
    .. py:method:: close(self) -> None
    
    .. py:property:: title
        :type: str
    
    .. py:property:: position
        :type: sgl.math.float2
    
    .. py:property:: size
        :type: sgl.math.float2
    


----

.. py:class:: sgl.ui.Group

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '') -> None
    
    .. py:property:: label
        :type: str
    


----

.. py:class:: sgl.ui.Text

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, text: str = '') -> None
    
    .. py:property:: text
        :type: str
    


----

.. py:class:: sgl.ui.ProgressBar

    Base class: :py:class:`sgl.ui.Widget`
    
    
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, fraction: float = 0.0) -> None
    
    .. py:property:: fraction
        :type: float
    


----

.. py:class:: sgl.ui.Button

    Base class: :py:class:`sgl.ui.Widget`
    
    
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', callback: collections.abc.Callable[[], None] | None = None) -> None
    
    .. py:property:: label
        :type: str
    
    .. py:property:: callback
        :type: collections.abc.Callable[[], None]
    


----

.. py:class:: sgl.ui.ValuePropertyBool

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: bool
    
    .. py:property:: callback
        :type: collections.abc.Callable[[bool], None]
    


----

.. py:class:: sgl.ui.ValuePropertyInt

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: int
    
    .. py:property:: callback
        :type: collections.abc.Callable[[int], None]
    


----

.. py:class:: sgl.ui.ValuePropertyInt2

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: sgl.math.int2
    
    .. py:property:: callback
        :type: collections.abc.Callable[[sgl.math.int2], None]
    


----

.. py:class:: sgl.ui.ValuePropertyInt3

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: sgl.math.int3
    
    .. py:property:: callback
        :type: collections.abc.Callable[[sgl.math.int3], None]
    


----

.. py:class:: sgl.ui.ValuePropertyInt4

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: sgl.math.int4
    
    .. py:property:: callback
        :type: collections.abc.Callable[[sgl.math.int4], None]
    


----

.. py:class:: sgl.ui.ValuePropertyFloat

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: float
    
    .. py:property:: callback
        :type: collections.abc.Callable[[float], None]
    


----

.. py:class:: sgl.ui.ValuePropertyFloat2

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: sgl.math.float2
    
    .. py:property:: callback
        :type: collections.abc.Callable[[sgl.math.float2], None]
    


----

.. py:class:: sgl.ui.ValuePropertyFloat3

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: sgl.math.float3
    
    .. py:property:: callback
        :type: collections.abc.Callable[[sgl.math.float3], None]
    


----

.. py:class:: sgl.ui.ValuePropertyFloat4

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: sgl.math.float4
    
    .. py:property:: callback
        :type: collections.abc.Callable[[sgl.math.float4], None]
    


----

.. py:class:: sgl.ui.ValuePropertyString

    Base class: :py:class:`sgl.ui.Widget`
    
    .. py:property:: label
        :type: str
    
    .. py:property:: value
        :type: str
    
    .. py:property:: callback
        :type: collections.abc.Callable[[str], None]
    


----

.. py:class:: sgl.ui.CheckBox

    Base class: :py:class:`sgl.ui.ValuePropertyBool`
    
    
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: bool = False, callback: collections.abc.Callable[[bool], None] | None = None) -> None
    


----

.. py:class:: sgl.ui.ComboBox

    Base class: :py:class:`sgl.ui.ValuePropertyInt`
    
    
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: int = 0, callback: collections.abc.Callable[[int], None] | None = None, items: collections.abc.Sequence[str] = []) -> None
    
    .. py:property:: items
        :type: list[str]
    


----

.. py:class:: sgl.ui.ListBox

    Base class: :py:class:`sgl.ui.ValuePropertyInt`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: int = 0, callback: collections.abc.Callable[[int], None] | None = None, items: collections.abc.Sequence[str] = [], height_in_items: int = -1) -> None
    
    .. py:property:: items
        :type: list[str]
    
    .. py:property:: height_in_items
        :type: int
    


----

.. py:class:: sgl.ui.SliderFlags

    Base class: :py:class:`enum.IntFlag`
    
    
    
    .. py:attribute:: sgl.ui.SliderFlags.none
        :type: SliderFlags
        :value: SliderFlags.none
    
    .. py:attribute:: sgl.ui.SliderFlags.always_clamp
        :type: SliderFlags
        :value: SliderFlags.always_clamp
    
    .. py:attribute:: sgl.ui.SliderFlags.logarithmic
        :type: SliderFlags
        :value: SliderFlags.logarithmic
    
    .. py:attribute:: sgl.ui.SliderFlags.no_round_to_format
        :type: SliderFlags
        :value: SliderFlags.no_round_to_format
    
    .. py:attribute:: sgl.ui.SliderFlags.no_input
        :type: SliderFlags
        :value: SliderFlags.no_input
    


----

.. py:class:: sgl.ui.DragFloat

    Base class: :py:class:`sgl.ui.ValuePropertyFloat`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: float = 0.0, callback: collections.abc.Callable[[float], None] | None = None, speed: float = 1.0, min: float = 0.0, max: float = 0.0, format: str = '%.3f', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: speed
        :type: float
    
    .. py:property:: min
        :type: float
    
    .. py:property:: max
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.DragFloat2

    Base class: :py:class:`sgl.ui.ValuePropertyFloat2`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.float2 = {0, 0}, callback: collections.abc.Callable[[sgl.math.float2], None] | None = None, speed: float = 1.0, min: float = 0.0, max: float = 0.0, format: str = '%.3f', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: speed
        :type: float
    
    .. py:property:: min
        :type: float
    
    .. py:property:: max
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.DragFloat3

    Base class: :py:class:`sgl.ui.ValuePropertyFloat3`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.float3 = {0, 0, 0}, callback: collections.abc.Callable[[sgl.math.float3], None] | None = None, speed: float = 1.0, min: float = 0.0, max: float = 0.0, format: str = '%.3f', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: speed
        :type: float
    
    .. py:property:: min
        :type: float
    
    .. py:property:: max
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.DragFloat4

    Base class: :py:class:`sgl.ui.ValuePropertyFloat4`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.float4 = {0, 0, 0, 0}, callback: collections.abc.Callable[[sgl.math.float4], None] | None = None, speed: float = 1.0, min: float = 0.0, max: float = 0.0, format: str = '%.3f', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: speed
        :type: float
    
    .. py:property:: min
        :type: float
    
    .. py:property:: max
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.DragInt

    Base class: :py:class:`sgl.ui.ValuePropertyInt`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: int = 0, callback: collections.abc.Callable[[int], None] | None = None, speed: float = 1.0, min: int = 0, max: int = 0, format: str = '%d', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: speed
        :type: int
    
    .. py:property:: min
        :type: int
    
    .. py:property:: max
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.DragInt2

    Base class: :py:class:`sgl.ui.ValuePropertyInt2`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.int2 = {0, 0}, callback: collections.abc.Callable[[sgl.math.int2], None] | None = None, speed: float = 1.0, min: int = 0, max: int = 0, format: str = '%d', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: speed
        :type: int
    
    .. py:property:: min
        :type: int
    
    .. py:property:: max
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.DragInt3

    Base class: :py:class:`sgl.ui.ValuePropertyInt3`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.int3 = {0, 0, 0}, callback: collections.abc.Callable[[sgl.math.int3], None] | None = None, speed: float = 1.0, min: int = 0, max: int = 0, format: str = '%d', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: speed
        :type: int
    
    .. py:property:: min
        :type: int
    
    .. py:property:: max
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.DragInt4

    Base class: :py:class:`sgl.ui.ValuePropertyInt4`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.int4 = {0, 0, 0, 0}, callback: collections.abc.Callable[[sgl.math.int4], None] | None = None, speed: float = 1.0, min: int = 0, max: int = 0, format: str = '%d', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: speed
        :type: int
    
    .. py:property:: min
        :type: int
    
    .. py:property:: max
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.SliderFloat

    Base class: :py:class:`sgl.ui.ValuePropertyFloat`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: float = 0.0, callback: collections.abc.Callable[[float], None] | None = None, min: float = 0.0, max: float = 0.0, format: str = '%.3f', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: min
        :type: float
    
    .. py:property:: max
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.SliderFloat2

    Base class: :py:class:`sgl.ui.ValuePropertyFloat2`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.float2 = {0, 0}, callback: collections.abc.Callable[[sgl.math.float2], None] | None = None, min: float = 0.0, max: float = 0.0, format: str = '%.3f', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: min
        :type: float
    
    .. py:property:: max
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.SliderFloat3

    Base class: :py:class:`sgl.ui.ValuePropertyFloat3`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.float3 = {0, 0, 0}, callback: collections.abc.Callable[[sgl.math.float3], None] | None = None, min: float = 0.0, max: float = 0.0, format: str = '%.3f', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: min
        :type: float
    
    .. py:property:: max
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.SliderFloat4

    Base class: :py:class:`sgl.ui.ValuePropertyFloat4`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.float4 = {0, 0, 0, 0}, callback: collections.abc.Callable[[sgl.math.float4], None] | None = None, min: float = 0.0, max: float = 0.0, format: str = '%.3f', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: min
        :type: float
    
    .. py:property:: max
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.SliderInt

    Base class: :py:class:`sgl.ui.ValuePropertyInt`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: int = 0, callback: collections.abc.Callable[[int], None] | None = None, min: int = 0, max: int = 0, format: str = '%d', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: min
        :type: int
    
    .. py:property:: max
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.SliderInt2

    Base class: :py:class:`sgl.ui.ValuePropertyInt2`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.int2 = {0, 0}, callback: collections.abc.Callable[[sgl.math.int2], None] | None = None, min: int = 0, max: int = 0, format: str = '%d', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: min
        :type: int
    
    .. py:property:: max
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.SliderInt3

    Base class: :py:class:`sgl.ui.ValuePropertyInt3`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.int3 = {0, 0, 0}, callback: collections.abc.Callable[[sgl.math.int3], None] | None = None, min: int = 0, max: int = 0, format: str = '%d', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: min
        :type: int
    
    .. py:property:: max
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.SliderInt4

    Base class: :py:class:`sgl.ui.ValuePropertyInt4`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.int4 = {0, 0, 0, 0}, callback: collections.abc.Callable[[sgl.math.int4], None] | None = None, min: int = 0, max: int = 0, format: str = '%d', flags: sgl.ui.SliderFlags = SliderFlags.none) -> None
    
    .. py:property:: min
        :type: int
    
    .. py:property:: max
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.SliderFlags
    


----

.. py:class:: sgl.ui.InputTextFlags

    Base class: :py:class:`enum.IntFlag`
    
    
    
    .. py:attribute:: sgl.ui.InputTextFlags.none
        :type: InputTextFlags
        :value: InputTextFlags.none
    
    .. py:attribute:: sgl.ui.InputTextFlags.chars_decimal
        :type: InputTextFlags
        :value: InputTextFlags.chars_decimal
    
    .. py:attribute:: sgl.ui.InputTextFlags.chars_hexadecimal
        :type: InputTextFlags
        :value: InputTextFlags.chars_hexadecimal
    
    .. py:attribute:: sgl.ui.InputTextFlags.chars_uppercase
        :type: InputTextFlags
        :value: InputTextFlags.chars_uppercase
    
    .. py:attribute:: sgl.ui.InputTextFlags.chars_no_blank
        :type: InputTextFlags
        :value: InputTextFlags.chars_no_blank
    
    .. py:attribute:: sgl.ui.InputTextFlags.auto_select_all
        :type: InputTextFlags
        :value: InputTextFlags.auto_select_all
    
    .. py:attribute:: sgl.ui.InputTextFlags.enter_returns_true
        :type: InputTextFlags
        :value: InputTextFlags.enter_returns_true
    
    .. py:attribute:: sgl.ui.InputTextFlags.callback_completion
        :type: InputTextFlags
        :value: InputTextFlags.callback_completion
    
    .. py:attribute:: sgl.ui.InputTextFlags.callback_history
        :type: InputTextFlags
        :value: InputTextFlags.callback_history
    
    .. py:attribute:: sgl.ui.InputTextFlags.callback_always
        :type: InputTextFlags
        :value: InputTextFlags.callback_always
    
    .. py:attribute:: sgl.ui.InputTextFlags.callback_char_filter
        :type: InputTextFlags
        :value: InputTextFlags.callback_char_filter
    
    .. py:attribute:: sgl.ui.InputTextFlags.allow_tab_input
        :type: InputTextFlags
        :value: InputTextFlags.allow_tab_input
    
    .. py:attribute:: sgl.ui.InputTextFlags.ctrl_enter_for_new_line
        :type: InputTextFlags
        :value: InputTextFlags.ctrl_enter_for_new_line
    
    .. py:attribute:: sgl.ui.InputTextFlags.no_horizontal_scroll
        :type: InputTextFlags
        :value: InputTextFlags.no_horizontal_scroll
    
    .. py:attribute:: sgl.ui.InputTextFlags.always_overwrite
        :type: InputTextFlags
        :value: InputTextFlags.always_overwrite
    
    .. py:attribute:: sgl.ui.InputTextFlags.read_only
        :type: InputTextFlags
        :value: InputTextFlags.read_only
    
    .. py:attribute:: sgl.ui.InputTextFlags.password
        :type: InputTextFlags
        :value: InputTextFlags.password
    
    .. py:attribute:: sgl.ui.InputTextFlags.no_undo_redo
        :type: InputTextFlags
        :value: InputTextFlags.no_undo_redo
    
    .. py:attribute:: sgl.ui.InputTextFlags.chars_scientific
        :type: InputTextFlags
        :value: InputTextFlags.chars_scientific
    
    .. py:attribute:: sgl.ui.InputTextFlags.escape_clears_all
        :type: InputTextFlags
        :value: InputTextFlags.escape_clears_all
    


----

.. py:class:: sgl.ui.InputFloat

    Base class: :py:class:`sgl.ui.ValuePropertyFloat`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: float = 0.0, callback: collections.abc.Callable[[float], None] | None = None, step: float = 1.0, step_fast: float = 100.0, format: str = '%.3f', flags: sgl.ui.InputTextFlags = InputTextFlags.none) -> None
    
    .. py:property:: step
        :type: float
    
    .. py:property:: step_fast
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.InputTextFlags
    


----

.. py:class:: sgl.ui.InputFloat2

    Base class: :py:class:`sgl.ui.ValuePropertyFloat2`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.float2 = {0, 0}, callback: collections.abc.Callable[[sgl.math.float2], None] | None = None, step: float = 1.0, step_fast: float = 100.0, format: str = '%.3f', flags: sgl.ui.InputTextFlags = InputTextFlags.none) -> None
    
    .. py:property:: step
        :type: float
    
    .. py:property:: step_fast
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.InputTextFlags
    


----

.. py:class:: sgl.ui.InputFloat3

    Base class: :py:class:`sgl.ui.ValuePropertyFloat3`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.float3 = {0, 0, 0}, callback: collections.abc.Callable[[sgl.math.float3], None] | None = None, step: float = 1.0, step_fast: float = 100.0, format: str = '%.3f', flags: sgl.ui.InputTextFlags = InputTextFlags.none) -> None
    
    .. py:property:: step
        :type: float
    
    .. py:property:: step_fast
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.InputTextFlags
    


----

.. py:class:: sgl.ui.InputFloat4

    Base class: :py:class:`sgl.ui.ValuePropertyFloat4`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.float4 = {0, 0, 0, 0}, callback: collections.abc.Callable[[sgl.math.float4], None] | None = None, step: float = 1.0, step_fast: float = 100.0, format: str = '%.3f', flags: sgl.ui.InputTextFlags = InputTextFlags.none) -> None
    
    .. py:property:: step
        :type: float
    
    .. py:property:: step_fast
        :type: float
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.InputTextFlags
    


----

.. py:class:: sgl.ui.InputInt

    Base class: :py:class:`sgl.ui.ValuePropertyInt`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: int = 0, callback: collections.abc.Callable[[int], None] | None = None, step: int = 1, step_fast: int = 100, format: str = '%d', flags: sgl.ui.InputTextFlags = InputTextFlags.none) -> None
    
    .. py:property:: step
        :type: int
    
    .. py:property:: step_fast
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.InputTextFlags
    


----

.. py:class:: sgl.ui.InputInt2

    Base class: :py:class:`sgl.ui.ValuePropertyInt2`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.int2 = {0, 0}, callback: collections.abc.Callable[[sgl.math.int2], None] | None = None, step: int = 1, step_fast: int = 100, format: str = '%d', flags: sgl.ui.InputTextFlags = InputTextFlags.none) -> None
    
    .. py:property:: step
        :type: int
    
    .. py:property:: step_fast
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.InputTextFlags
    


----

.. py:class:: sgl.ui.InputInt3

    Base class: :py:class:`sgl.ui.ValuePropertyInt3`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.int3 = {0, 0, 0}, callback: collections.abc.Callable[[sgl.math.int3], None] | None = None, step: int = 1, step_fast: int = 100, format: str = '%d', flags: sgl.ui.InputTextFlags = InputTextFlags.none) -> None
    
    .. py:property:: step
        :type: int
    
    .. py:property:: step_fast
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.InputTextFlags
    


----

.. py:class:: sgl.ui.InputInt4

    Base class: :py:class:`sgl.ui.ValuePropertyInt4`
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: sgl.math.int4 = {0, 0, 0, 0}, callback: collections.abc.Callable[[sgl.math.int4], None] | None = None, step: int = 1, step_fast: int = 100, format: str = '%d', flags: sgl.ui.InputTextFlags = InputTextFlags.none) -> None
    
    .. py:property:: step
        :type: int
    
    .. py:property:: step_fast
        :type: int
    
    .. py:property:: format
        :type: str
    
    .. py:property:: flags
        :type: sgl.ui.InputTextFlags
    


----

.. py:class:: sgl.ui.InputText

    Base class: :py:class:`sgl.ui.ValuePropertyString`
    
    
    
    .. py:method:: __init__(self, parent: sgl.ui.Widget | None, label: str = '', value: str = False, callback: collections.abc.Callable[[str], None] | None = None, multi_line: bool = False, flags: sgl.ui.InputTextFlags = InputTextFlags.none) -> None
    


----

Utilities
---------

.. py:class:: sgl.TextureLoader

    Base class: :py:class:`sgl.Object`
    
    Utility class for loading textures from bitmaps and image files.
    
    .. py:method:: __init__(self, device: sgl.Device) -> None
    
    .. py:class:: sgl.TextureLoader.Options
    
        
        
        .. py:method:: __init__(self) -> None
        
        .. py:method:: __init__(self, arg: dict, /) -> None
            :no-index:
        
        .. py:property:: load_as_normalized
            :type: bool
        
            Load 8/16-bit integer data as normalized resource format.
            
        .. py:property:: load_as_srgb
            :type: bool
        
            Use ``Format::rgba8_unorm_srgb`` format if bitmap is 8-bit RGBA with
            sRGB gamma.
            
        .. py:property:: extend_alpha
            :type: bool
        
            Extend RGB to RGBA if RGB texture format is not available.
            
        .. py:property:: allocate_mips
            :type: bool
        
            Allocate mip levels for the texture.
            
        .. py:property:: generate_mips
            :type: bool
        
            Generate mip levels for the texture.
            
        .. py:property:: usage
            :type: sgl.TextureUsage
        
    .. py:method:: load_texture(self, bitmap: sgl.Bitmap, options: sgl.TextureLoader.Options | None = None) -> sgl.Texture
    
        Load a texture from a bitmap.
        
        Parameter ``bitmap``:
            Bitmap to load.
        
        Parameter ``options``:
            Texture loading options.
        
        Returns:
            New texture object.
        
    .. py:method:: load_texture(self, path: str | os.PathLike, options: sgl.TextureLoader.Options | None = None) -> sgl.Texture
        :no-index:
    
        Load a texture from an image file.
        
        Parameter ``path``:
            Image file path.
        
        Parameter ``options``:
            Texture loading options.
        
        Returns:
            New texture object.
        
    .. py:method:: load_textures(self, bitmaps: Sequence[sgl.Bitmap], options: sgl.TextureLoader.Options | None = None) -> list[sgl.Texture]
    
        Load textures from a list of bitmaps.
        
        Parameter ``bitmaps``:
            Bitmaps to load.
        
        Parameter ``options``:
            Texture loading options.
        
        Returns:
            List of new of texture objects.
        
    .. py:method:: load_textures(self, paths: Sequence[str | os.PathLike], options: sgl.TextureLoader.Options | None = None) -> list[sgl.Texture]
        :no-index:
    
        Load textures from a list of image files.
        
        Parameter ``paths``:
            Image file paths.
        
        Parameter ``options``:
            Texture loading options.
        
        Returns:
            List of new texture objects.
        
    .. py:method:: load_texture_array(self, bitmaps: Sequence[sgl.Bitmap], options: sgl.TextureLoader.Options | None = None) -> sgl.Texture
    
        Load a texture array from a list of bitmaps.
        
        All bitmaps need to have the same format and dimensions.
        
        Parameter ``bitmaps``:
            Bitmaps to load.
        
        Parameter ``options``:
            Texture loading options.
        
        Returns:
            New texture array object.
        
    .. py:method:: load_texture_array(self, paths: Sequence[str | os.PathLike], options: sgl.TextureLoader.Options | None = None) -> sgl.Texture
        :no-index:
    
        Load a texture array from a list of image files.
        
        All images need to have the same format and dimensions.
        
        Parameter ``paths``:
            Image file paths.
        
        Parameter ``options``:
            Texture loading options.
        
        Returns:
            New texture array object.
        


----

.. py:function:: sgl.tev.show(bitmap: sgl.Bitmap, name: str = '', host: str = '127.0.0.1', port: int = 14158, max_retries: int = 3) -> bool

    Show a bitmap in the tev viewer (https://github.com/Tom94/tev).
    
    This will block until the image is sent over.
    
    Parameter ``bitmap``:
        Bitmap to show.
    
    Parameter ``name``:
        Name of the image in tev. If not specified, a unique name will be
        generated.
    
    Parameter ``host``:
        Host to connect to.
    
    Parameter ``port``:
        Port to connect to.
    
    Parameter ``max_retries``:
        Maximum number of retries.
    
    Returns:
        True if successful.
    
.. py:function:: sgl.tev.show(texture: sgl.Texture, name: str = '', host: str = '127.0.0.1', port: int = 14158, max_retries: int = 3) -> bool
    :no-index:

    Show texture in the tev viewer (https://github.com/Tom94/tev).
    
    This will block until the image is sent over.
    
    Parameter ``texture``:
        Texture to show.
    
    Parameter ``name``:
        Name of the image in tev. If not specified, a unique name will be
        generated.
    
    Parameter ``host``:
        Host to connect to.
    
    Parameter ``port``:
        Port to connect to.
    
    Parameter ``max_retries``:
        Maximum number of retries.
    
    Returns:
        True if successful.
    


----

.. py:function:: sgl.tev.show_async(bitmap: sgl.Bitmap, name: str = '', host: str = '127.0.0.1', port: int = 14158, max_retries: int = 3) -> None

    Show a bitmap in the tev viewer (https://github.com/Tom94/tev).
    
    This will return immediately and send the image asynchronously in the
    background.
    
    Parameter ``bitmap``:
        Bitmap to show.
    
    Parameter ``name``:
        Name of the image in tev. If not specified, a unique name will be
        generated.
    
    Parameter ``host``:
        Host to connect to.
    
    Parameter ``port``:
        Port to connect to.
    
    Parameter ``max_retries``:
        Maximum number of retries.
    
.. py:function:: sgl.tev.show_async(texture: sgl.Texture, name: str = '', host: str = '127.0.0.1', port: int = 14158, max_retries: int = 3) -> None
    :no-index:

    Show a texture in the tev viewer (https://github.com/Tom94/tev).
    
    This will return immediately and send the image asynchronously in the
    background.
    
    Parameter ``bitmap``:
        Texture to show.
    
    Parameter ``name``:
        Name of the image in tev. If not specified, a unique name will be
        generated.
    
    Parameter ``host``:
        Host to connect to.
    
    Parameter ``port``:
        Port to connect to.
    
    Parameter ``max_retries``:
        Maximum number of retries.
    


----

.. py:function:: sgl.renderdoc.is_available() -> bool

    Check if RenderDoc is available.
    
    This is typically the case when the application is running under the
    RenderDoc.
    
    Returns:
        True if RenderDoc is available.
    


----

.. py:function:: sgl.renderdoc.start_frame_capture(device: sgl.Device, window: sgl.Window | None = None) -> bool

    Start capturing a frame in RenderDoc.
    
    This function will start capturing a frame (or some partial
    compute/graphics workload) in RenderDoc.
    
    To end the frame capture, call ``end_frame_capture``().
    
    Parameter ``device``:
        The device to capture the frame for.
    
    Parameter ``window``:
        The window to capture the frame for (optional).
    
    Returns:
        True if the frame capture was started successfully.
    


----

.. py:function:: sgl.renderdoc.end_frame_capture() -> bool

    End capturing a frame in RenderDoc.
    
    This function will end capturing a frame (or some partial
    compute/graphics workload) in RenderDoc.
    
    Returns:
        True if the frame capture was ended successfully.
    


----

.. py:function:: sgl.renderdoc.is_frame_capturing() -> bool

    Check if a frame is currently being captured in RenderDoc.
    
    Returns:
        True if a frame is currently being captured.
    


----

SlangPy
-------

.. py:class:: sgl.slangpy.AccessType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.slangpy.AccessType.none
        :type: AccessType
        :value: AccessType.none
    
    .. py:attribute:: sgl.slangpy.AccessType.read
        :type: AccessType
        :value: AccessType.read
    
    .. py:attribute:: sgl.slangpy.AccessType.write
        :type: AccessType
        :value: AccessType.write
    
    .. py:attribute:: sgl.slangpy.AccessType.readwrite
        :type: AccessType
        :value: AccessType.readwrite
    


----

.. py:class:: sgl.slangpy.CallMode

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.slangpy.CallMode.prim
        :type: CallMode
        :value: CallMode.prim
    
    .. py:attribute:: sgl.slangpy.CallMode.bwds
        :type: CallMode
        :value: CallMode.bwds
    
    .. py:attribute:: sgl.slangpy.CallMode.fwds
        :type: CallMode
        :value: CallMode.fwds
    


----

.. py:function:: sgl.slangpy.unpack_args(*args) -> list

    N/A
    


----

.. py:function:: sgl.slangpy.unpack_kwargs(**kwargs) -> dict

    N/A
    


----

.. py:function:: sgl.slangpy.unpack_arg(arg: object) -> object

    N/A
    


----

.. py:function:: sgl.slangpy.pack_arg(arg: object, unpacked_arg: object) -> None

    N/A
    


----

.. py:function:: sgl.slangpy.get_value_signature(o: object) -> str

    N/A
    


----

.. py:class:: sgl.slangpy.SignatureBuilder

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        
    .. py:method:: add(self, value: str) -> None
    
        N/A
        
    .. py:property:: str
        :type: str
    
        N/A
        
    .. py:property:: bytes
        :type: bytes
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeObject

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        
    .. py:property:: slangpy_signature
        :type: str
    
    .. py:method:: read_signature(self, builder: sgl.slangpy.SignatureBuilder) -> None
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeSlangType

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        
    .. py:property:: type_reflection
        :type: sgl.TypeReflection
    
        N/A
        
    .. py:property:: shape
        :type: sgl.slangpy.Shape
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeMarshall

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        
    .. py:property:: concrete_shape
        :type: sgl.slangpy.Shape
    
        N/A
        
    .. py:property:: match_call_shape
        :type: bool
    
        N/A
        
    .. py:method:: get_shape(self, value: object) -> sgl.slangpy.Shape
    
        N/A
        
    .. py:property:: slang_type
        :type: sgl.slangpy.NativeSlangType
    
        N/A
        
    .. py:method:: write_shader_cursor_pre_dispatch(self, context: sgl.slangpy.CallContext, binding: sgl.slangpy.NativeBoundVariableRuntime, cursor: sgl.ShaderCursor, value: object, read_back: list) -> None
    
        N/A
        
    .. py:method:: create_calldata(self, arg0: sgl.slangpy.CallContext, arg1: sgl.slangpy.NativeBoundVariableRuntime, arg2: object, /) -> object
    
        N/A
        
    .. py:method:: read_calldata(self, arg0: sgl.slangpy.CallContext, arg1: sgl.slangpy.NativeBoundVariableRuntime, arg2: object, arg3: object, /) -> None
    
        N/A
        
    .. py:method:: create_output(self, arg0: sgl.slangpy.CallContext, arg1: sgl.slangpy.NativeBoundVariableRuntime, /) -> object
    
        N/A
        
    .. py:method:: read_output(self, arg0: sgl.slangpy.CallContext, arg1: sgl.slangpy.NativeBoundVariableRuntime, arg2: object, /) -> object
    
        N/A
        
    .. py:property:: has_derivative
        :type: bool
    
        N/A
        
    .. py:property:: is_writable
        :type: bool
    
        N/A
        
    .. py:method:: gen_calldata(self, cgb: object, context: object, binding: object) -> None
    
        N/A
        
    .. py:method:: reduce_type(self, context: object, dimensions: int) -> sgl.slangpy.NativeSlangType
    
        N/A
        
    .. py:method:: resolve_type(self, context: object, bound_type: sgl.slangpy.NativeSlangType) -> sgl.slangpy.NativeSlangType
    
        N/A
        
    .. py:method:: resolve_dimensionality(self, context: object, binding: object, vector_target_type: sgl.slangpy.NativeSlangType) -> int
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeBoundVariableRuntime

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        
    .. py:property:: access
        :type: tuple[sgl.slangpy.AccessType, sgl.slangpy.AccessType]
    
        N/A
        
    .. py:property:: transform
        :type: sgl.slangpy.Shape
    
        N/A
        
    .. py:property:: python_type
        :type: sgl.slangpy.NativeMarshall
    
        N/A
        
    .. py:property:: vector_type
        :type: sgl.slangpy.NativeSlangType
    
        N/A
        
    .. py:property:: shape
        :type: sgl.slangpy.Shape
    
        N/A
        
    .. py:property:: variable_name
        :type: str
    
        N/A
        
    .. py:property:: children
        :type: dict[str, sgl.slangpy.NativeBoundVariableRuntime] | None
    
        N/A
        
    .. py:method:: populate_call_shape(self, arg0: collections.abc.Sequence[int], arg1: object, arg2: sgl.slangpy.NativeCallData, /) -> None
    
        N/A
        
    .. py:method:: read_call_data_post_dispatch(self, arg0: sgl.slangpy.CallContext, arg1: dict, arg2: object, /) -> None
    
        N/A
        
    .. py:method:: write_raw_dispatch_data(self, arg0: dict, arg1: object, /) -> None
    
        N/A
        
    .. py:method:: read_output(self, arg0: sgl.slangpy.CallContext, arg1: object, /) -> object
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeBoundCallRuntime

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        
    .. py:property:: args
        :type: list[sgl.slangpy.NativeBoundVariableRuntime]
    
        N/A
        
    .. py:property:: kwargs
        :type: dict[str, sgl.slangpy.NativeBoundVariableRuntime]
    
        N/A
        
    .. py:method:: find_kwarg(self, arg: str, /) -> sgl.slangpy.NativeBoundVariableRuntime
    
        N/A
        
    .. py:method:: calculate_call_shape(self, arg0: int, arg1: list, arg2: dict, arg3: sgl.slangpy.NativeCallData, /) -> sgl.slangpy.Shape
    
        N/A
        
    .. py:method:: read_call_data_post_dispatch(self, arg0: sgl.slangpy.CallContext, arg1: dict, arg2: list, arg3: dict, /) -> None
    
        N/A
        
    .. py:method:: write_raw_dispatch_data(self, arg0: dict, arg1: dict, /) -> None
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeCallRuntimeOptions

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        
    .. py:property:: uniforms
        :type: list
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeCallData

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        
    .. py:property:: device
        :type: sgl.Device
    
        N/A
        
    .. py:property:: kernel
        :type: sgl.ComputeKernel
    
        N/A
        
    .. py:property:: call_dimensionality
        :type: int
    
        N/A
        
    .. py:property:: runtime
        :type: sgl.slangpy.NativeBoundCallRuntime
    
        N/A
        
    .. py:property:: call_mode
        :type: sgl.slangpy.CallMode
    
        N/A
        
    .. py:property:: last_call_shape
        :type: sgl.slangpy.Shape
    
        N/A
        
    .. py:method:: call(self, opts: sgl.slangpy.NativeCallRuntimeOptions, *args, **kwargs) -> object
    
        N/A
        
    .. py:method:: append_to(self, opts: sgl.slangpy.NativeCallRuntimeOptions, command_buffer: sgl.CommandEncoder, *args, **kwargs) -> object
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeCallDataCache

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        
    .. py:method:: get_value_signature(self, builder: sgl.slangpy.SignatureBuilder, o: object) -> None
    
        N/A
        
    .. py:method:: get_args_signature(self, builder: sgl.slangpy.SignatureBuilder, *args, **kwargs) -> None
    
        N/A
        
    .. py:method:: find_call_data(self, signature: str) -> sgl.slangpy.NativeCallData
    
        N/A
        
    .. py:method:: add_call_data(self, signature: str, call_data: sgl.slangpy.NativeCallData) -> None
    
        N/A
        
    .. py:method:: lookup_value_signature(self, o: object) -> str | None
    
        N/A
        


----

.. py:class:: sgl.slangpy.Shape

    .. py:method:: __init__(self, *args) -> None
    
        N/A
        
    .. py:property:: valid
        :type: bool
    
        N/A
        
    .. py:property:: concrete
        :type: bool
    
        N/A
        
    .. py:method:: as_tuple(self) -> tuple
    
        N/A
        
    .. py:method:: as_list(self) -> list[int]
    
        N/A
        


----

.. py:class:: sgl.slangpy.CallContext

    Base class: :py:class:`sgl.Object`
    
    .. py:method:: __init__(self, device: sgl.Device, call_shape: sgl.slangpy.Shape, call_mode: sgl.slangpy.CallMode) -> None
    
        N/A
        
    .. py:property:: device
        :type: sgl.Device
    
        N/A
        
    .. py:property:: call_shape
        :type: sgl.slangpy.Shape
    
        N/A
        
    .. py:property:: call_mode
        :type: sgl.slangpy.CallMode
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeNDBufferDesc

    .. py:method:: __init__(self) -> None
    
    .. py:property:: dtype
        :type: sgl.slangpy.NativeSlangType
    
    .. py:property:: element_layout
        :type: sgl.TypeLayoutReflection
    
    .. py:property:: shape
        :type: sgl.slangpy.Shape
    
    .. py:property:: strides
        :type: sgl.slangpy.Shape
    
    .. py:property:: usage
        :type: sgl.BufferUsage
    
    .. py:property:: memory_type
        :type: sgl.MemoryType
    


----

.. py:class:: sgl.slangpy.NativeNDBuffer

    Base class: :py:class:`sgl.slangpy.NativeObject`
    
    .. py:method:: __init__(self, arg0: sgl.Device, arg1: sgl.slangpy.NativeNDBufferDesc, /) -> None
    
    .. py:property:: device
        :type: sgl.Device
    
    .. py:property:: dtype
        :type: sgl.slangpy.NativeSlangType
    
    .. py:property:: shape
        :type: sgl.slangpy.Shape
    
    .. py:property:: strides
        :type: sgl.slangpy.Shape
    
    .. py:property:: element_count
        :type: int
    
    .. py:property:: usage
        :type: sgl.BufferUsage
    
    .. py:property:: memory_type
        :type: sgl.MemoryType
    
    .. py:property:: storage
        :type: sgl.Buffer
    
    .. py:method:: cursor(self, start: int | None = None, count: int | None = None) -> sgl.BufferCursor
    
    .. py:method:: uniforms(self) -> dict
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[]
    
        N/A
        
    .. py:method:: copy_from_numpy(self, data: numpy.ndarray[]) -> None
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeNDBufferMarshall

    Base class: :py:class:`sgl.slangpy.NativeMarshall`
    
    .. py:method:: __init__(self, dims: int, writable: bool, slang_type: sgl.slangpy.NativeSlangType, slang_element_type: sgl.slangpy.NativeSlangType, element_layout: sgl.TypeLayoutReflection) -> None
    
        N/A
        
    .. py:property:: dims
        :type: int
    
    .. py:property:: writable
        :type: bool
    
    .. py:property:: slang_element_type
        :type: sgl.slangpy.NativeSlangType
    


----

.. py:class:: sgl.slangpy.NativeNumpyMarshall

    Base class: :py:class:`sgl.slangpy.NativeNDBufferMarshall`
    
    .. py:method:: __init__(self, dims: int, slang_type: sgl.slangpy.NativeSlangType, slang_element_type: sgl.slangpy.NativeSlangType, element_layout: sgl.TypeLayoutReflection, numpydtype: object) -> None
    
        N/A
        
    .. py:property:: dtype
        :type: dlpack::dtype
    


----

.. py:class:: sgl.slangpy.FunctionNodeType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.slangpy.FunctionNodeType.unknown
        :type: FunctionNodeType
        :value: FunctionNodeType.unknown
    
    .. py:attribute:: sgl.slangpy.FunctionNodeType.uniforms
        :type: FunctionNodeType
        :value: FunctionNodeType.uniforms
    
    .. py:attribute:: sgl.slangpy.FunctionNodeType.kernelgen
        :type: FunctionNodeType
        :value: FunctionNodeType.kernelgen
    
    .. py:attribute:: sgl.slangpy.FunctionNodeType.this
        :type: FunctionNodeType
        :value: FunctionNodeType.this
    


----

.. py:class:: sgl.slangpy.NativeFunctionNode

    Base class: :py:class:`sgl.slangpy.NativeObject`
    
    .. py:method:: __init__(self, parent: sgl.slangpy.NativeFunctionNode | None, type: sgl.slangpy.FunctionNodeType, data: object | None) -> None
    
        N/A
        
    .. py:method:: generate_call_data(self, *args, **kwargs) -> sgl.slangpy.NativeCallData
    
        N/A
        
    .. py:method:: read_signature(self, builder: sgl.slangpy.SignatureBuilder) -> None
    
        N/A
        
    .. py:method:: gather_runtime_options(self, options: sgl.slangpy.NativeCallRuntimeOptions) -> None
    
        N/A
        


----

.. py:function:: sgl.slangpy.get_texture_shape(texture: sgl.Texture, mip: int = 0) -> sgl.slangpy.Shape

    N/A
    


----

.. py:class:: sgl.slangpy.NativeBufferMarshall

    Base class: :py:class:`sgl.slangpy.NativeMarshall`
    
    .. py:method:: __init__(self, slang_type: sgl.slangpy.NativeSlangType, usage: sgl.BufferUsage) -> None
    
        N/A
        
    .. py:method:: write_shader_cursor_pre_dispatch(self, context: sgl.slangpy.CallContext, binding: sgl.slangpy.NativeBoundVariableRuntime, cursor: sgl.ShaderCursor, value: object, read_back: list) -> None
    
        N/A
        
    .. py:method:: get_shape(self, value: object) -> sgl.slangpy.Shape
    
        N/A
        
    .. py:property:: usage
        :type: sgl.BufferUsage
    
    .. py:property:: slang_type
        :type: sgl.slangpy.NativeSlangType
    


----

.. py:class:: sgl.slangpy.NativeTextureMarshall

    Base class: :py:class:`sgl.slangpy.NativeMarshall`
    
    .. py:method:: __init__(self, slang_type: sgl.slangpy.NativeSlangType, element_type: sgl.slangpy.NativeSlangType, resource_shape: sgl.TypeReflection.ResourceShape, format: sgl.Format, usage: sgl.TextureUsage, dims: int) -> None
    
        N/A
        
    .. py:method:: write_shader_cursor_pre_dispatch(self, context: sgl.slangpy.CallContext, binding: sgl.slangpy.NativeBoundVariableRuntime, cursor: sgl.ShaderCursor, value: object, read_back: list) -> None
    
        N/A
        
    .. py:method:: get_shape(self, value: object) -> sgl.slangpy.Shape
    
        N/A
        
    .. py:method:: get_texture_shape(self, texture: sgl.Texture, mip: int) -> sgl.slangpy.Shape
    
        N/A
        
    .. py:property:: resource_shape
        :type: sgl.TypeReflection.ResourceShape
    
        N/A
        
    .. py:property:: usage
        :type: sgl.TextureUsage
    
        N/A
        
    .. py:property:: texture_dims
        :type: int
    
        N/A
        
    .. py:property:: slang_element_type
        :type: sgl.slangpy.NativeSlangType
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeTensorDesc

    .. py:method:: __init__(self) -> None
    
    .. py:property:: dtype
        :type: sgl.slangpy.NativeSlangType
    
    .. py:property:: element_layout
        :type: sgl.TypeLayoutReflection
    
    .. py:property:: shape
        :type: sgl.slangpy.Shape
    
    .. py:property:: strides
        :type: sgl.slangpy.Shape
    
    .. py:property:: offset
        :type: int
    


----

.. py:class:: sgl.slangpy.NativeTensor

    Base class: :py:class:`sgl.slangpy.NativeObject`
    
    .. py:method:: __init__(self, desc: sgl.slangpy.NativeTensorDesc, storage: sgl.Buffer, grad_in: sgl.slangpy.NativeTensor | None, grad_out: sgl.slangpy.NativeTensor | None) -> None
    
    .. py:property:: device
        :type: sgl.Device
    
    .. py:property:: dtype
        :type: sgl.slangpy.NativeSlangType
    
    .. py:property:: shape
        :type: sgl.slangpy.Shape
    
    .. py:property:: strides
        :type: sgl.slangpy.Shape
    
    .. py:property:: offset
        :type: int
    
    .. py:property:: element_count
        :type: int
    
    .. py:property:: usage
        :type: sgl.BufferUsage
    
    .. py:property:: memory_type
        :type: sgl.MemoryType
    
    .. py:property:: storage
        :type: sgl.Buffer
    
    .. py:property:: grad_in
        :type: sgl.slangpy.NativeTensor
    
    .. py:property:: grad_out
        :type: sgl.slangpy.NativeTensor
    
    .. py:property:: grad
        :type: sgl.slangpy.NativeTensor
    
    .. py:method:: broadcast_to(self, shape: sgl.slangpy.Shape) -> sgl.slangpy.NativeTensor
    
    .. py:method:: clear(self, cmd: sgl.CommandEncoder | None = None) -> None
    
    .. py:method:: with_grads(self, grad_in: sgl.slangpy.NativeTensor | None = None, grad_out: sgl.slangpy.NativeTensor | None = None, zero: bool = False) -> sgl.slangpy.NativeTensor
    
    .. py:method:: cursor(self, start: int | None = None, count: int | None = None) -> sgl.BufferCursor
    
    .. py:method:: uniforms(self) -> dict
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[]
    
        N/A
        


----

.. py:class:: sgl.slangpy.NativeTensorMarshall

    Base class: :py:class:`sgl.slangpy.NativeMarshall`
    
    .. py:method:: __init__(self, dims: int, writable: bool, slang_type: sgl.slangpy.NativeSlangType, slang_element_type: sgl.slangpy.NativeSlangType, element_layout: sgl.TypeLayoutReflection, d_in: sgl.slangpy.NativeTensorMarshall | None, d_out: sgl.slangpy.NativeTensorMarshall | None) -> None
    
        N/A
        
    .. py:property:: dims
        :type: int
    
    .. py:property:: writable
        :type: bool
    
    .. py:property:: slang_element_type
        :type: sgl.slangpy.NativeSlangType
    
    .. py:property:: d_in
        :type: sgl.slangpy.NativeTensorMarshall
    
    .. py:property:: d_out
        :type: sgl.slangpy.NativeTensorMarshall
    


----

.. py:class:: sgl.slangpy.NativeValueMarshall

    Base class: :py:class:`sgl.slangpy.NativeMarshall`
    
    .. py:method:: __init__(self) -> None
    
        N/A
        


----

Miscellaneous
-------------



----

