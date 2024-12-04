Core
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
    :value: "0.3.0"



----

.. py:data:: sgl.SGL_VERSION_MAJOR
    :type: int
    :value: 0



----

.. py:data:: sgl.SGL_VERSION_MINOR
    :type: int
    :value: 3



----

.. py:data:: sgl.SGL_VERSION_PATCH
    :type: int
    :value: 0



----

.. py:data:: sgl.SGL_GIT_VERSION
    :type: str
    :value: "commit: c993bb7 / branch: main (local changes)"



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
    :value: 65536



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
    
    
    
    .. py:method:: kind(self) -> sgl.AccelerationStructureKind
    
    .. py:property:: device_address
        :type: int
    


----

.. py:class:: sgl.AccelerationStructureBuildDesc

    
    


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
    
    .. py:attribute:: sgl.AccelerationStructureBuildFlags.perform_update
        :type: AccelerationStructureBuildFlags
        :value: AccelerationStructureBuildFlags.perform_update
    


----

.. py:class:: sgl.AccelerationStructureBuildInputs

    Base class: :py:class:`sgl.AccelerationStructureBuildInputsBase`
    
    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: kind
        :type: sgl.AccelerationStructureKind
    
    .. py:property:: flags
        :type: sgl.AccelerationStructureBuildFlags
    
    .. py:property:: desc_count
        :type: int
    
    .. py:property:: instance_descs
        :type: int
    
        Array of `RayTracingInstanceDesc` values in device memory. Used when
        `kind` is `top_level`.
        
    .. py:property:: geometry_descs
        :type: list[sgl.RayTracingGeometryDesc]
    
        Array of `RayTracingGeometryDesc` values. Used when `kind` is
        `bottom_level`.
        


----

.. py:class:: sgl.AccelerationStructureBuildInputsBase



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

.. py:class:: sgl.AccelerationStructureKind

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.AccelerationStructureKind.top_level
        :type: AccelerationStructureKind
        :value: AccelerationStructureKind.top_level
    
    .. py:attribute:: sgl.AccelerationStructureKind.bottom_level
        :type: AccelerationStructureKind
        :value: AccelerationStructureKind.bottom_level
    


----

.. py:class:: sgl.AccelerationStructurePrebuildInfo

    .. py:property:: result_data_max_size
        :type: int
    
    .. py:property:: scratch_data_size
        :type: int
    
    .. py:property:: update_scratch_data_size
        :type: int
    


----

.. py:class:: sgl.AccelerationStructureQueryDesc

    
    


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
    
    
    


----

.. py:class:: sgl.BlendDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: targets
        :type: list[sgl.TargetBlendDesc]
    
    .. py:property:: alpha_to_coverage_enable
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
    
    .. py:method:: get_srv(self, offset: int = 0, size: int = 18446744073709551615) -> sgl.ResourceView
    
        Get a shader resource view for a range of the buffer.
        
    .. py:method:: get_uav(self, offset: int = 0, size: int = 18446744073709551615) -> sgl.ResourceView
    
        Get a unordered access view for a range of the buffer.
        
    .. py:method:: to_numpy(self) -> numpy.ndarray[]
    
    .. py:method:: from_numpy(self, data: numpy.ndarray[]) -> None
    


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
        
    .. py:property:: initial_state
        :type: sgl.ResourceState
    
        Initial resource state.
        
    .. py:property:: usage
        :type: sgl.ResourceUsage
    
        Resource usage flags.
        
    .. py:property:: memory_type
        :type: sgl.MemoryType
    
        Memory type.
        
    .. py:property:: debug_name
        :type: str
    
        Resource debug name.
        


----

.. py:class:: sgl.CommandBuffer

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:method:: open(self) -> None
    
        Open the command buffer for recording.
        
        No-op if command buffer is already open.
        
        \note Due to current limitations, only a single command buffer can be
        open at any given time.
        
    .. py:method:: close(self) -> None
    
        Close the command buffer.
        
        No-op if command buffer is already closed.
        
    .. py:method:: submit(self, queue: sgl.CommandQueueType = CommandQueueType.graphics) -> int
    
        Submit the command buffer to the device.
        
        The returned submission ID can be used to wait for the command buffer
        to complete.
        
        Parameter ``queue``:
            Command queue to submit to.
        
        Returns:
            Submission ID.
        
    .. py:method:: write_timestamp(self, query_pool: sgl.QueryPool, index: int) -> None
    
        Write a timestamp.
        
        Parameter ``query_pool``:
            Query pool.
        
        Parameter ``index``:
            Index of the query.
        
    .. py:method:: resolve_query(self, query_pool: sgl.QueryPool, index: int, count: int, buffer: sgl.Buffer, offset: int) -> None
    
        Resolve a list of queries and write the results to a buffer.
        
        Parameter ``query_pool``:
            Query pool.
        
        Parameter ``index``:
            Index of the first query.
        
        Parameter ``count``:
            Number of queries to resolve.
        
        Parameter ``buffer``:
            Destination buffer.
        
        Parameter ``offset``:
            Offset into the destination buffer.
        
    .. py:method:: set_resource_state(self, resource: sgl.Resource, new_state: sgl.ResourceState) -> bool
    
        Transition resource state of a resource and add a barrier if state has
        changed.
        
        Parameter ``resource``:
            Resource
        
        Parameter ``new_state``:
            New state
        
        Returns:
            True if barrier was recorded (i.e. state has changed).
        
    .. py:method:: set_resource_state(self, resource_view: sgl.ResourceView, new_state: sgl.ResourceState) -> bool
        :no-index:
    
        Transition resource state of a resource and add a barrier if state has
        changed. For buffer views, this will set the resource state of the
        entire buffer. For texture views, this will set the resource state of
        all its sub-resources.
        
        Parameter ``resource_view``:
            Resource view
        
        Parameter ``new_state``:
            New state
        
        Returns:
            True if barrier was recorded (i.e. state has changed).
        
    .. py:method:: set_buffer_state(self, buffer: sgl.Buffer, new_state: sgl.ResourceState) -> bool
    
        Transition resource state of a buffer and add a barrier if state has
        changed.
        
        Parameter ``buffer``:
            Buffer
        
        Parameter ``new_state``:
            New state
        
        Returns:
            True if barrier was recorded (i.e. state has changed).
        
    .. py:method:: set_texture_state(self, texture: sgl.Texture, new_state: sgl.ResourceState) -> bool
    
        Transition resource state of a texture and add a barrier if state has
        changed.
        
        Parameter ``texture``:
            Texture
        
        Parameter ``new_state``:
            New state
        
        Returns:
            True if barrier was recorded (i.e. state has changed).
        
    .. py:method:: uav_barrier(self, resource: sgl.Resource) -> None
    
        Insert a UAV barrier
        
    .. py:method:: clear_resource_view(self, resource_view: sgl.ResourceView, clear_value: sgl.math.float4) -> None
    
    .. py:method:: clear_resource_view(self, resource_view: sgl.ResourceView, clear_value: sgl.math.uint4) -> None
        :no-index:
    
    .. py:method:: clear_resource_view(self, resource_view: sgl.ResourceView, depth_value: float, stencil_value: int, clear_depth: bool, clear_stencil: bool) -> None
        :no-index:
    
    .. py:method:: clear_texture(self, texture: sgl.Texture, clear_value: sgl.math.float4) -> None
    
    .. py:method:: clear_texture(self, texture: sgl.Texture, clear_value: sgl.math.uint4) -> None
        :no-index:
    
    .. py:method:: copy_resource(self, dst: sgl.Resource, src: sgl.Resource) -> None
    
        Copy an entire resource.
        
        Parameter ``dst``:
            Destination resource.
        
        Parameter ``src``:
            Source resource.
        
    .. py:method:: copy_buffer_region(self, dst: sgl.Buffer, dst_offset: int, src: sgl.Buffer, src_offset: int, size: int) -> None
    
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
        
    .. py:method:: copy_texture_region(self, dst: sgl.Texture, dst_subresource: int, dst_offset: sgl.math.uint3, src: sgl.Texture, src_subresource: int, src_offset: sgl.math.uint3, extent: sgl.math.uint3 = {4294967295, 4294967295, 4294967295}) -> None
    
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
        
    .. py:method:: blit(self, dst: sgl.ResourceView, src: sgl.ResourceView, filter: sgl.TextureFilteringMode = TextureFilteringMode.linear) -> None
    
        Blit a SRV to an RTV.
        
        Blits the full extent of the source texture to the destination
        texture.
        
        Parameter ``dst``:
            RTV of the destination texture.
        
        Parameter ``src``:
            SRV of the source texture.
        
        Parameter ``filter``:
            Filtering mode to use.
        
    .. py:method:: blit(self, dst: sgl.Texture, src: sgl.Texture, filter: sgl.TextureFilteringMode = TextureFilteringMode.linear) -> None
        :no-index:
    
        Blit a texture to another texture.
        
        Blits the full extent of the source texture to the destination
        texture.
        
        Parameter ``dst``:
            Destination texture.
        
        Parameter ``src``:
            Source texture.
        
        Parameter ``filter``:
            Filtering mode to use.
        
    .. py:method:: encode_compute_commands(self) -> sgl.ComputeCommandEncoder
    
        Start encoding compute commands.
        
        The returned ``ComputeCommandEncoder`` is used to bind compute
        pipelines and issue dispatches. The encoding is ended when the
        ``ComputeCommandEncoder`` is destroyed.
        
    .. py:method:: encode_render_commands(self, arg: sgl.Framebuffer, /) -> sgl.RenderCommandEncoder
    
        Start encoding render commands.
        
        The returned ``RenderCommandEncoder`` is used to bind graphics
        pipelines and issue dispatches. The encoding is ended when the
        ``RenderCommandEncoder`` is destroyed.
        
    .. py:method:: encode_ray_tracing_commands(self) -> sgl.RayTracingCommandEncoder
    
        Start encoding ray tracing commands.
        
        The returned ``RayTracingCommandEncoder`` is used to bind ray tracing
        pipelines and issue dispatches. It also serves for building and
        managing acceleration structures. The encoding is ended when the
        ``RayTracingCommandEncoder`` is destroyed.
        


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

.. py:class:: sgl.ComputeCommandEncoder

    
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.ComputePipeline) -> sgl.TransientShaderObject
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.ComputePipeline, shader_object: sgl.ShaderObject) -> None
        :no-index:
    
    .. py:method:: dispatch(self, thread_count: sgl.math.uint3) -> None
    
    .. py:method:: dispatch_thread_groups(self, thread_group_count: sgl.math.uint3) -> None
    


----

.. py:class:: sgl.ComputeKernel

    Base class: :py:class:`sgl.Kernel`
    
    
    
    .. py:property:: pipeline
        :type: sgl.ComputePipeline
    
    .. py:method:: dispatch(self, thread_count: sgl.math.uint3, vars: dict = {}, command_buffer: sgl.CommandBuffer | None = None, **kwargs) -> None
    


----

.. py:class:: sgl.ComputeKernelDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: program
        :type: sgl.ShaderProgram
    


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
    
    .. py:property:: stencil_ref
        :type: int
    


----

.. py:class:: sgl.DepthStencilOpDesc

    
    
    .. py:method:: __init__(self) -> None
    
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
        
    .. py:method:: get_format_supported_resource_states(self, format: sgl.Format) -> set[sgl.ResourceState]
    
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
        
    .. py:method:: create_swapchain(self, window: sgl.Window, format: sgl.Format = Format.bgra8_unorm_srgb, width: int = 0, height: int = 0, image_count: int = 3, enable_vsync: bool = False) -> sgl.Swapchain
    
        Create a new swapchain.
        
        Parameter ``format``:
            Format of the swapchain images.
        
        Parameter ``width``:
            Width of the swapchain images in pixels.
        
        Parameter ``height``:
            Height of the swapchain images in pixels.
        
        Parameter ``image_count``:
            Number of swapchain images.
        
        Parameter ``enable_vsync``:
            Enable/disable vertical synchronization.
        
        Parameter ``window``:
            Window to create the swapchain for.
        
        Returns:
            New swapchain object.
        
    .. py:method:: create_swapchain(self, window_handle: sgl.WindowHandle, format: sgl.Format = Format.bgra8_unorm_srgb, width: int = 0, height: int = 0, image_count: int = 3, enable_vsync: bool = False) -> sgl.Swapchain
        :no-index:
    
        Create a new swapchain.
        
        Parameter ``format``:
            Format of the swapchain images.
        
        Parameter ``width``:
            Width of the swapchain images in pixels.
        
        Parameter ``height``:
            Height of the swapchain images in pixels.
        
        Parameter ``image_count``:
            Number of swapchain images.
        
        Parameter ``enable_vsync``:
            Enable/disable vertical synchronization.
        
        Parameter ``window_handle``:
            Native window handle to create the swapchain for.
        
        Returns:
            New swapchain object.
        
    .. py:method:: create_swapchain(self, desc: sgl.SwapchainDesc, window: sgl.Window) -> sgl.Swapchain
        :no-index:
    
        Create a new swapchain.
        
        Parameter ``format``:
            Format of the swapchain images.
        
        Parameter ``width``:
            Width of the swapchain images in pixels.
        
        Parameter ``height``:
            Height of the swapchain images in pixels.
        
        Parameter ``image_count``:
            Number of swapchain images.
        
        Parameter ``enable_vsync``:
            Enable/disable vertical synchronization.
        
        Parameter ``window``:
            Window to create the swapchain for.
        
        Returns:
            New swapchain object.
        
    .. py:method:: create_buffer(self, size: int = 0, element_count: int = 0, struct_size: int = 0, struct_type: object | None = None, format: sgl.Format = Format.unknown, usage: sgl.ResourceUsage = ResourceUsage.none, memory_type: sgl.MemoryType = MemoryType.device_local, debug_name: str = '', data: numpy.ndarray[] | None = None) -> sgl.Buffer
    
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
        
        Parameter ``debug_name``:
            Resource debug name.
        
        Parameter ``data``:
            Initial data to upload to the buffer.
        
        Parameter ``data_size``:
            Size of the initial data in bytes.
        
        Returns:
            New buffer object.
        
    .. py:method:: create_buffer(self, desc: sgl.BufferDesc) -> sgl.Buffer
        :no-index:
    
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
        
        Parameter ``debug_name``:
            Resource debug name.
        
        Parameter ``data``:
            Initial data to upload to the buffer.
        
        Parameter ``data_size``:
            Size of the initial data in bytes.
        
        Returns:
            New buffer object.
        
    .. py:method:: create_texture(self, type: sgl.ResourceType = ResourceType.unknown, format: sgl.Format = Format.unknown, width: int = 0, height: int = 0, depth: int = 0, array_size: int = 1, mip_count: int = 0, sample_count: int = 1, quality: int = 0, usage: sgl.ResourceUsage = ResourceUsage.none, memory_type: sgl.MemoryType = MemoryType.device_local, debug_name: str = '', data: numpy.ndarray[] | None = None) -> sgl.Texture
    
        Create a new texture.
        
        Parameter ``type``:
            Resource type (optional). Type is inferred from width, height,
            depth if not specified.
        
        Parameter ``format``:
            Texture format.
        
        Parameter ``width``:
            Width in pixels.
        
        Parameter ``height``:
            Height in pixels.
        
        Parameter ``depth``:
            Depth in pixels.
        
        Parameter ``array_size``:
            Number of array slices (1 for non-array textures).
        
        Parameter ``mip_count``:
            Number of mip levels (0 for auto-generated mips).
        
        Parameter ``sample_count``:
            Number of samples per pixel (1 for non-multisampled textures).
        
        Parameter ``quality``:
            Quality level for multisampled textures.
        
        Parameter ``usage``:
            Resource usage.
        
        Parameter ``memory_type``:
            Memory type.
        
        Parameter ``debug_name``:
            Debug name.
        
        Parameter ``data``:
            Initial data.
        
        Returns:
            New texture object.
        
    .. py:method:: create_texture(self, desc: sgl.TextureDesc) -> sgl.Texture
        :no-index:
    
        Create a new texture.
        
        Parameter ``type``:
            Resource type (optional). Type is inferred from width, height,
            depth if not specified.
        
        Parameter ``format``:
            Texture format.
        
        Parameter ``width``:
            Width in pixels.
        
        Parameter ``height``:
            Height in pixels.
        
        Parameter ``depth``:
            Depth in pixels.
        
        Parameter ``array_size``:
            Number of array slices (1 for non-array textures).
        
        Parameter ``mip_count``:
            Number of mip levels (0 for auto-generated mips).
        
        Parameter ``sample_count``:
            Number of samples per pixel (1 for non-multisampled textures).
        
        Parameter ``quality``:
            Quality level for multisampled textures.
        
        Parameter ``usage``:
            Resource usage.
        
        Parameter ``memory_type``:
            Memory type.
        
        Parameter ``debug_name``:
            Debug name.
        
        Parameter ``data``:
            Initial data.
        
        Returns:
            New texture object.
        
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
        
        Returns:
            New sampler object.
        
    .. py:method:: create_sampler(self, desc: sgl.SamplerDesc) -> sgl.Sampler
        :no-index:
    
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
        
        Returns:
            New sampler object.
        
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
    
        Create a new fence.
        
        Parameter ``initial_value``:
            Initial fence value.
        
        Parameter ``shared``:
            Create a shared fence.
        
        Returns:
            New fence object.
        
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
    
        Create a new input layout.
        
        Parameter ``input_elements``:
            List of input elements (see InputElementDesc for details).
        
        Parameter ``vertex_streams``:
            List of vertex streams (see VertexStreamDesc for details).
        
        Returns:
            New input layout object.
        
    .. py:method:: create_framebuffer(self, render_targets: collections.abc.Sequence[sgl.ResourceView], depth_stencil: sgl.ResourceView | None = None, layout: sgl.FramebufferLayout | None = None) -> sgl.Framebuffer
    
        Create a new framebuffer.
        
        Parameter ``render_target``:
            List of render targets (see FramebufferAttachmentDesc for
            details).
        
        Parameter ``depth_stencil``:
            Optional depth-stencil attachment (see FramebufferAttachmentDesc
            for details).
        
        Returns:
            New framebuffer object.
        
    .. py:method:: create_framebuffer(self, desc: sgl.FramebufferDesc) -> sgl.Framebuffer
        :no-index:
    
        Create a new framebuffer.
        
        Parameter ``render_target``:
            List of render targets (see FramebufferAttachmentDesc for
            details).
        
        Parameter ``depth_stencil``:
            Optional depth-stencil attachment (see FramebufferAttachmentDesc
            for details).
        
        Returns:
            New framebuffer object.
        
    .. py:method:: create_command_buffer(self) -> sgl.CommandBuffer
    
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
        
    .. py:method:: get_acceleration_structure_prebuild_info(self, build_inputs: sgl.AccelerationStructureBuildInputsBase) -> sgl.AccelerationStructurePrebuildInfo
    
    .. py:method:: create_acceleration_structure(self, kind: sgl.AccelerationStructureKind, buffer: sgl.Buffer, offset: int = 0, size: int = 0) -> sgl.AccelerationStructure
    
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
    
    .. py:method:: create_mutable_shader_object(self, shader_program: sgl.ShaderProgram) -> sgl.MutableShaderObject
    
    .. py:method:: create_mutable_shader_object(self, type_layout: sgl.TypeLayoutReflection) -> sgl.MutableShaderObject
        :no-index:
    
    .. py:method:: create_mutable_shader_object(self, cursor: sgl.ReflectionCursor) -> sgl.MutableShaderObject
        :no-index:
    
    .. py:method:: create_compute_pipeline(self, program: sgl.ShaderProgram) -> sgl.ComputePipeline
    
    .. py:method:: create_compute_pipeline(self, desc: sgl.ComputePipelineDesc) -> sgl.ComputePipeline
        :no-index:
    
    .. py:method:: create_graphics_pipeline(self, program: sgl.ShaderProgram, input_layout: sgl.InputLayout | None, framebuffer_layout: sgl.FramebufferLayout, primitive_type: sgl.PrimitiveType = PrimitiveType.triangle, depth_stencil: sgl.DepthStencilDesc | None = None, rasterizer: sgl.RasterizerDesc | None = None, blend: sgl.BlendDesc | None = None) -> sgl.GraphicsPipeline
    
    .. py:method:: create_graphics_pipeline(self, desc: sgl.GraphicsPipelineDesc) -> sgl.GraphicsPipeline
        :no-index:
    
    .. py:method:: create_ray_tracing_pipeline(self, program: sgl.ShaderProgram, hit_groups: collections.abc.Sequence[sgl.HitGroupDesc], max_recursion: int = 0, max_ray_payload_size: int = 0, max_attribute_size: int = 8, flags: sgl.RayTracingPipelineFlags = RayTracingPipelineFlags.none) -> sgl.RayTracingPipeline
    
    .. py:method:: create_ray_tracing_pipeline(self, desc: sgl.RayTracingPipelineDesc) -> sgl.RayTracingPipeline
        :no-index:
    
    .. py:method:: create_compute_kernel(self, program: sgl.ShaderProgram) -> sgl.ComputeKernel
    
    .. py:method:: create_compute_kernel(self, desc: sgl.ComputeKernelDesc) -> sgl.ComputeKernel
        :no-index:
    
    .. py:method:: create_memory_heap(self, memory_type: sgl.MemoryType, usage: sgl.ResourceUsage, page_size: int = 4194304, retain_large_pages: bool = False, debug_name: str = '') -> sgl.MemoryHeap
    
    .. py:method:: create_memory_heap(self, desc: sgl.MemoryHeapDesc) -> sgl.MemoryHeap
        :no-index:
    
    .. py:property:: upload_heap
        :type: sgl.MemoryHeap
    
    .. py:property:: read_back_heap
        :type: sgl.MemoryHeap
    
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
    
    .. py:attribute:: sgl.DeviceType.cpu
        :type: DeviceType
        :value: DeviceType.cpu
    
    .. py:attribute:: sgl.DeviceType.cuda
        :type: DeviceType
        :value: DeviceType.cuda
    


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
    
    .. py:attribute:: sgl.Format.unknown
        :type: Format
        :value: Format.unknown
    
    .. py:attribute:: sgl.Format.rgba32_typeless
        :type: Format
        :value: Format.rgba32_typeless
    
    .. py:attribute:: sgl.Format.rgb32_typeless
        :type: Format
        :value: Format.rgb32_typeless
    
    .. py:attribute:: sgl.Format.rg32_typeless
        :type: Format
        :value: Format.rg32_typeless
    
    .. py:attribute:: sgl.Format.r32_typeless
        :type: Format
        :value: Format.r32_typeless
    
    .. py:attribute:: sgl.Format.rgba16_typeless
        :type: Format
        :value: Format.rgba16_typeless
    
    .. py:attribute:: sgl.Format.rg16_typeless
        :type: Format
        :value: Format.rg16_typeless
    
    .. py:attribute:: sgl.Format.r16_typeless
        :type: Format
        :value: Format.r16_typeless
    
    .. py:attribute:: sgl.Format.rgba8_typeless
        :type: Format
        :value: Format.rgba8_typeless
    
    .. py:attribute:: sgl.Format.rg8_typeless
        :type: Format
        :value: Format.rg8_typeless
    
    .. py:attribute:: sgl.Format.r8_typeless
        :type: Format
        :value: Format.r8_typeless
    
    .. py:attribute:: sgl.Format.bgra8_typeless
        :type: Format
        :value: Format.bgra8_typeless
    
    .. py:attribute:: sgl.Format.rgba32_float
        :type: Format
        :value: Format.rgba32_float
    
    .. py:attribute:: sgl.Format.rgb32_float
        :type: Format
        :value: Format.rgb32_float
    
    .. py:attribute:: sgl.Format.rg32_float
        :type: Format
        :value: Format.rg32_float
    
    .. py:attribute:: sgl.Format.r32_float
        :type: Format
        :value: Format.r32_float
    
    .. py:attribute:: sgl.Format.rgba16_float
        :type: Format
        :value: Format.rgba16_float
    
    .. py:attribute:: sgl.Format.rg16_float
        :type: Format
        :value: Format.rg16_float
    
    .. py:attribute:: sgl.Format.r16_float
        :type: Format
        :value: Format.r16_float
    
    .. py:attribute:: sgl.Format.rgba32_uint
        :type: Format
        :value: Format.rgba32_uint
    
    .. py:attribute:: sgl.Format.rgb32_uint
        :type: Format
        :value: Format.rgb32_uint
    
    .. py:attribute:: sgl.Format.rg32_uint
        :type: Format
        :value: Format.rg32_uint
    
    .. py:attribute:: sgl.Format.r32_uint
        :type: Format
        :value: Format.r32_uint
    
    .. py:attribute:: sgl.Format.rgba16_uint
        :type: Format
        :value: Format.rgba16_uint
    
    .. py:attribute:: sgl.Format.rg16_uint
        :type: Format
        :value: Format.rg16_uint
    
    .. py:attribute:: sgl.Format.r16_uint
        :type: Format
        :value: Format.r16_uint
    
    .. py:attribute:: sgl.Format.rgba8_uint
        :type: Format
        :value: Format.rgba8_uint
    
    .. py:attribute:: sgl.Format.rg8_uint
        :type: Format
        :value: Format.rg8_uint
    
    .. py:attribute:: sgl.Format.r8_uint
        :type: Format
        :value: Format.r8_uint
    
    .. py:attribute:: sgl.Format.rgba32_sint
        :type: Format
        :value: Format.rgba32_sint
    
    .. py:attribute:: sgl.Format.rgb32_sint
        :type: Format
        :value: Format.rgb32_sint
    
    .. py:attribute:: sgl.Format.rg32_sint
        :type: Format
        :value: Format.rg32_sint
    
    .. py:attribute:: sgl.Format.r32_sint
        :type: Format
        :value: Format.r32_sint
    
    .. py:attribute:: sgl.Format.rgba16_sint
        :type: Format
        :value: Format.rgba16_sint
    
    .. py:attribute:: sgl.Format.rg16_sint
        :type: Format
        :value: Format.rg16_sint
    
    .. py:attribute:: sgl.Format.r16_sint
        :type: Format
        :value: Format.r16_sint
    
    .. py:attribute:: sgl.Format.rgba8_sint
        :type: Format
        :value: Format.rgba8_sint
    
    .. py:attribute:: sgl.Format.rg8_sint
        :type: Format
        :value: Format.rg8_sint
    
    .. py:attribute:: sgl.Format.r8_sint
        :type: Format
        :value: Format.r8_sint
    
    .. py:attribute:: sgl.Format.rgba16_unorm
        :type: Format
        :value: Format.rgba16_unorm
    
    .. py:attribute:: sgl.Format.rg16_unorm
        :type: Format
        :value: Format.rg16_unorm
    
    .. py:attribute:: sgl.Format.r16_unorm
        :type: Format
        :value: Format.r16_unorm
    
    .. py:attribute:: sgl.Format.rgba8_unorm
        :type: Format
        :value: Format.rgba8_unorm
    
    .. py:attribute:: sgl.Format.rgba8_unorm_srgb
        :type: Format
        :value: Format.rgba8_unorm_srgb
    
    .. py:attribute:: sgl.Format.rg8_unorm
        :type: Format
        :value: Format.rg8_unorm
    
    .. py:attribute:: sgl.Format.r8_unorm
        :type: Format
        :value: Format.r8_unorm
    
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
    
    .. py:attribute:: sgl.Format.rgba16_snorm
        :type: Format
        :value: Format.rgba16_snorm
    
    .. py:attribute:: sgl.Format.rg16_snorm
        :type: Format
        :value: Format.rg16_snorm
    
    .. py:attribute:: sgl.Format.r16_snorm
        :type: Format
        :value: Format.r16_snorm
    
    .. py:attribute:: sgl.Format.rgba8_snorm
        :type: Format
        :value: Format.rgba8_snorm
    
    .. py:attribute:: sgl.Format.rg8_snorm
        :type: Format
        :value: Format.rg8_snorm
    
    .. py:attribute:: sgl.Format.r8_snorm
        :type: Format
        :value: Format.r8_snorm
    
    .. py:attribute:: sgl.Format.d32_float
        :type: Format
        :value: Format.d32_float
    
    .. py:attribute:: sgl.Format.d16_unorm
        :type: Format
        :value: Format.d16_unorm
    
    .. py:attribute:: sgl.Format.d32_float_s8_uint
        :type: Format
        :value: Format.d32_float_s8_uint
    
    .. py:attribute:: sgl.Format.r32_float_x32_typeless
        :type: Format
        :value: Format.r32_float_x32_typeless
    
    .. py:attribute:: sgl.Format.bgra4_unorm
        :type: Format
        :value: Format.bgra4_unorm
    
    .. py:attribute:: sgl.Format.b5g6r5_unorm
        :type: Format
        :value: Format.b5g6r5_unorm
    
    .. py:attribute:: sgl.Format.b5g5r5a1_unorm
        :type: Format
        :value: Format.b5g5r5a1_unorm
    
    .. py:attribute:: sgl.Format.r9g9b9e5_sharedexp
        :type: Format
        :value: Format.r9g9b9e5_sharedexp
    
    .. py:attribute:: sgl.Format.r10g10b10a2_typeless
        :type: Format
        :value: Format.r10g10b10a2_typeless
    
    .. py:attribute:: sgl.Format.r10g10b10a2_unorm
        :type: Format
        :value: Format.r10g10b10a2_unorm
    
    .. py:attribute:: sgl.Format.r10g10b10a2_uint
        :type: Format
        :value: Format.r10g10b10a2_uint
    
    .. py:attribute:: sgl.Format.r11g11b10_float
        :type: Format
        :value: Format.r11g11b10_float
    
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
    
    .. py:attribute:: sgl.Format.bc6h_uf16
        :type: Format
        :value: Format.bc6h_uf16
    
    .. py:attribute:: sgl.Format.bc6h_sf16
        :type: Format
        :value: Format.bc6h_sf16
    
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
        
    .. py:method:: is_typeless_format(self) -> bool
    
        True if format is typeless.
        
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

.. py:class:: sgl.FormatType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.FormatType.unknown
        :type: FormatType
        :value: FormatType.unknown
    
    .. py:attribute:: sgl.FormatType.typeless
        :type: FormatType
        :value: FormatType.typeless
    
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

.. py:class:: sgl.Framebuffer

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: desc
        :type: sgl.FramebufferDesc
    
    .. py:property:: layout
        :type: sgl.FramebufferLayout
    


----

.. py:class:: sgl.FramebufferDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: render_targets
        :type: list[sgl.ResourceView]
    
        List of render targets.
        
    .. py:property:: depth_stencil
        :type: sgl.ResourceView
    
        Depth-stencil target (optional).
        
    .. py:property:: layout
        :type: sgl.FramebufferLayout
    
        Framebuffer layout (optional). If not provided, framebuffer layout is
        determined from the render targets.
        


----

.. py:class:: sgl.FramebufferLayout

    Base class: :py:class:`sgl.DeviceResource`
    
    
    
    .. py:property:: desc
        :type: sgl.FramebufferLayoutDesc
    


----

.. py:class:: sgl.FramebufferLayoutDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: render_targets
        :type: list[sgl.FramebufferLayoutTargetDesc]
    
    .. py:property:: depth_stencil
        :type: sgl.FramebufferLayoutTargetDesc | None
    


----

.. py:class:: sgl.FramebufferLayoutTargetDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: format
        :type: sgl.Format
    
    .. py:property:: sample_count
        :type: int
    


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
    
    .. py:property:: return_type
        :type: sgl.TypeReflection
    
    .. py:property:: parameters
        :type: sgl.FunctionReflectionParameterList
    
    .. py:method:: has_modifier(self, modifier: sgl.ModifierID) -> bool
    
        Check if variable has a given modifier (e.g. 'inout').
        


----

.. py:class:: sgl.FunctionReflectionParameterList

    FunctionReflection lazy parameter list evaluation.
    


----

.. py:function:: sgl.get_format_info(arg: sgl.Format, /) -> sgl.FormatInfo



----

.. py:class:: sgl.GraphicsPipeline

    Base class: :py:class:`sgl.Pipeline`
    
    Graphics pipeline.
    


----

.. py:class:: sgl.GraphicsPipelineDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: program
        :type: sgl.ShaderProgram
    
    .. py:property:: input_layout
        :type: sgl.InputLayout
    
    .. py:property:: framebuffer_layout
        :type: sgl.FramebufferLayout
    
    .. py:property:: primitive_type
        :type: sgl.PrimitiveType
    
    .. py:property:: depth_stencil
        :type: sgl.DepthStencilDesc
    
    .. py:property:: rasterizer
        :type: sgl.RasterizerDesc
    
    .. py:property:: blend
        :type: sgl.BlendDesc
    


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

.. py:class:: sgl.LogicOp

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.LogicOp.no_op
        :type: LogicOp
        :value: LogicOp.no_op
    


----

.. py:class:: sgl.MemoryHeap

    Base class: :py:class:`sgl.DeviceResource`
    
    A memory heap is used to allocate temporary host-visible memory.
    
    A memory heap is a collection of memory pages. Each page has a buffer
    of size ``page_size``. When allocating memory, the heap tries to add
    the allocation to the current page. If the allocation does not fit, a
    new page is allocated. For allocations larger than the configured page
    size, a new large page is allocated.
    
    The memory heap is tied to a fence. Each allocation records the
    currently signaled fence value when it is created. On release, the
    allocation is put on a deferred release queue. Only if the fence value
    of the memory heap is greater than the fence value of the allocation,
    the allocation is actually freed. This ensures that memory is not
    freed while still in use by the device.
    
    Allocations are returned as unique pointers. When the pointer is
    destroyed, the allocation is released. This ensures that the memory is
    freed when it is no longer used.
    
    .. py:class:: sgl.MemoryHeap.Allocation
    
        
        
        .. py:property:: buffer
            :type: sgl.Buffer
        
            The buffer this allocation belongs to.
            
        .. py:property:: size
            :type: int
        
            The size of the allocation.
            
        .. py:property:: offset
            :type: int
        
            The offset of the allocation within the buffer.
            
        .. py:property:: device_address
            :type: int
        
            The device address of the allocation.
            
    .. py:class:: sgl.MemoryHeap.Stats
    
        
        
        .. py:property:: total_size
            :type: int
        
            The total size of the heap.
            
        .. py:property:: used_size
            :type: int
        
            The used size of the heap.
            
        .. py:property:: page_count
            :type: int
        
            The number of pages in the heap.
            
        .. py:property:: large_page_count
            :type: int
        
            The number of large pages in the heap.
            
    .. py:method:: allocate(self, size: int, alignment: int = 1) -> sgl.MemoryHeap.Allocation
    
        Allocate memory from this heap.
        
        Parameter ``size``:
            The number of bytes to allocate.
        
        Parameter ``alignment``:
            The alignment of the allocation.
        
        Returns:
            Returns a unique pointer to the allocation.
        
    .. py:property:: stats
        :type: sgl.MemoryHeap.Stats
    
        Statistics of the heap.
        


----

.. py:class:: sgl.MemoryHeapDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: memory_type
        :type: sgl.MemoryType
    
        The memory type of the heap.
        
    .. py:property:: usage
        :type: sgl.ResourceUsage
    
        The resource usage of the heap.
        
    .. py:property:: page_size
        :type: int
    
        The size of a page in bytes.
        
    .. py:property:: retain_large_pages
        :type: bool
    
        True to retain large pages, false to release them after use.
        
    .. py:property:: debug_name
        :type: str
    
        The debug name of the heap.
        


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

.. py:class:: sgl.MutableShaderObject

    Base class: :py:class:`sgl.ShaderObject`
    
    
    


----

.. py:class:: sgl.Pipeline

    Base class: :py:class:`sgl.DeviceResource`
    
    Pipeline base class.
    


----

.. py:class:: sgl.PrimitiveTopology

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.PrimitiveTopology.triangle_list
        :type: PrimitiveTopology
        :value: PrimitiveTopology.triangle_list
    
    .. py:attribute:: sgl.PrimitiveTopology.triangle_strip
        :type: PrimitiveTopology
        :value: PrimitiveTopology.triangle_strip
    
    .. py:attribute:: sgl.PrimitiveTopology.point_list
        :type: PrimitiveTopology
        :value: PrimitiveTopology.point_list
    
    .. py:attribute:: sgl.PrimitiveTopology.line_list
        :type: PrimitiveTopology
        :value: PrimitiveTopology.line_list
    
    .. py:attribute:: sgl.PrimitiveTopology.line_strip
        :type: PrimitiveTopology
        :value: PrimitiveTopology.line_strip
    


----

.. py:class:: sgl.PrimitiveType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.PrimitiveType.point
        :type: PrimitiveType
        :value: PrimitiveType.point
    
    .. py:attribute:: sgl.PrimitiveType.line
        :type: PrimitiveType
        :value: PrimitiveType.line
    
    .. py:attribute:: sgl.PrimitiveType.triangle
        :type: PrimitiveType
        :value: PrimitiveType.triangle
    
    .. py:attribute:: sgl.PrimitiveType.patch
        :type: PrimitiveType
        :value: PrimitiveType.patch
    


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

.. py:class:: sgl.RayTracingAABB

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: min
        :type: sgl.math.float3
    
    .. py:property:: max
        :type: sgl.math.float3
    


----

.. py:class:: sgl.RayTracingAABBsDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: count
        :type: int
    
        Number of AABBs.
        
    .. py:property:: data
        :type: int
    
        Pointer to an array of `RayTracingAABB` values in device memory.
        
    .. py:property:: stride
        :type: int
    
        Stride in bytes of the AABB values array.
        


----

.. py:class:: sgl.RayTracingCommandEncoder

    
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.RayTracingPipeline) -> sgl.TransientShaderObject
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.RayTracingPipeline, shader_object: sgl.ShaderObject) -> None
        :no-index:
    
    .. py:method:: dispatch_rays(self, ray_gen_shader_index: int, shader_table: sgl.ShaderTable, dimensions: sgl.math.uint3) -> None
    
    .. py:method:: build_acceleration_structure(self, inputs: sgl.AccelerationStructureBuildInputsBase, dst: sgl.AccelerationStructure, scratch_data: int, src: sgl.AccelerationStructure | None = None) -> None
    
    .. py:method:: copy_acceleration_structure(self, src: sgl.AccelerationStructure, dst: sgl.AccelerationStructure, mode: sgl.AccelerationStructureCopyMode) -> None
    


----

.. py:class:: sgl.RayTracingGeometryDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: type
        :type: sgl.RayTracingGeometryType
    
    .. py:property:: flags
        :type: sgl.RayTracingGeometryFlags
    
    .. py:property:: triangles
        :type: sgl.RayTracingTrianglesDesc
    
    .. py:property:: aabbs
        :type: sgl.RayTracingAABBsDesc
    


----

.. py:class:: sgl.RayTracingGeometryFlags

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.RayTracingGeometryFlags.none
        :type: RayTracingGeometryFlags
        :value: RayTracingGeometryFlags.none
    
    .. py:attribute:: sgl.RayTracingGeometryFlags.opaque
        :type: RayTracingGeometryFlags
        :value: RayTracingGeometryFlags.opaque
    
    .. py:attribute:: sgl.RayTracingGeometryFlags.no_duplicate_any_hit_invocation
        :type: RayTracingGeometryFlags
        :value: RayTracingGeometryFlags.no_duplicate_any_hit_invocation
    


----

.. py:class:: sgl.RayTracingGeometryType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.RayTracingGeometryType.triangles
        :type: RayTracingGeometryType
        :value: RayTracingGeometryType.triangles
    
    .. py:attribute:: sgl.RayTracingGeometryType.procedural_primitives
        :type: RayTracingGeometryType
        :value: RayTracingGeometryType.procedural_primitives
    


----

.. py:class:: sgl.RayTracingInstanceDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: transform
        :type: sgl.math.float3x4
    
    .. py:property:: instance_id
        :type: int
    
    .. py:property:: instance_mask
        :type: int
    
    .. py:property:: instance_contribution_to_hit_group_index
        :type: int
    
    .. py:property:: flags
        :type: sgl.RayTracingInstanceFlags
    
    .. py:property:: acceleration_structure
        :type: int
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=uint8, writable=False, shape=(64)]
    


----

.. py:class:: sgl.RayTracingInstanceFlags

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.RayTracingInstanceFlags.none
        :type: RayTracingInstanceFlags
        :value: RayTracingInstanceFlags.none
    
    .. py:attribute:: sgl.RayTracingInstanceFlags.triangle_facing_cull_disable
        :type: RayTracingInstanceFlags
        :value: RayTracingInstanceFlags.triangle_facing_cull_disable
    
    .. py:attribute:: sgl.RayTracingInstanceFlags.triangle_front_counter_clockwise
        :type: RayTracingInstanceFlags
        :value: RayTracingInstanceFlags.triangle_front_counter_clockwise
    
    .. py:attribute:: sgl.RayTracingInstanceFlags.force_opaque
        :type: RayTracingInstanceFlags
        :value: RayTracingInstanceFlags.force_opaque
    
    .. py:attribute:: sgl.RayTracingInstanceFlags.no_opaque
        :type: RayTracingInstanceFlags
        :value: RayTracingInstanceFlags.no_opaque
    


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

.. py:class:: sgl.RayTracingTrianglesDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:property:: transform3x4
        :type: int
    
    .. py:property:: index_format
        :type: sgl.Format
    
    .. py:property:: vertex_format
        :type: sgl.Format
    
    .. py:property:: index_count
        :type: int
    
    .. py:property:: vertex_count
        :type: int
    
    .. py:property:: index_data
        :type: int
    
    .. py:property:: vertex_data
        :type: int
    
    .. py:property:: vertex_stride
        :type: int
    


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

.. py:class:: sgl.RenderCommandEncoder

    
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.GraphicsPipeline) -> sgl.TransientShaderObject
    
    .. py:method:: bind_pipeline(self, pipeline: sgl.GraphicsPipeline, shader_object: sgl.ShaderObject) -> None
        :no-index:
    
    .. py:method:: set_viewports(self, viewports: Sequence[sgl.Viewport]) -> None
    
    .. py:method:: set_scissor_rects(self, scissor_rects: Sequence[sgl.ScissorRect]) -> None
    
    .. py:method:: set_viewport_and_scissor_rect(self, viewport: sgl.Viewport) -> None
    
    .. py:method:: set_primitive_topology(self, topology: sgl.PrimitiveTopology) -> None
    
    .. py:method:: set_stencil_reference(self, reference_value: int) -> None
    
    .. py:method:: set_vertex_buffer(self, slot: int, buffer: sgl.Buffer, offset: int = 0) -> None
    
    .. py:method:: set_index_buffer(self, buffer: sgl.Buffer, index_format: sgl.Format, offset: int = 0) -> None
    
    .. py:method:: draw(self, vertex_count: int, start_vertex: int = 0) -> None
    
    .. py:method:: draw_indexed(self, index_count: int, start_index: int = 0, base_vertex: int = 0) -> None
    
    .. py:method:: draw_instanced(self, vertex_count: int, instance_count: int, start_vertex: int = 0, start_instance: int = 0) -> None
    
    .. py:method:: draw_indexed_instanced(self, index_count: int, instance_count: int, start_index: int = 0, base_vertex: int = 0, start_instance: int = 0) -> None
    
    .. py:method:: draw_indirect(self, max_draw_count: int, arg_buffer: sgl.Buffer, arg_offset: int, count_buffer: sgl.Buffer | None = None, count_offset: int = 0) -> None
    
    .. py:method:: draw_indexed_indirect(self, max_draw_count: int, arg_buffer: sgl.Buffer, arg_offset: int, count_buffer: sgl.Buffer | None = None, count_offset: int = 0) -> None
    


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
    
    
    
    .. py:property:: type
        :type: sgl.ResourceType
    
    .. py:property:: format
        :type: sgl.Format
    


----

.. py:class:: sgl.ResourceState

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.ResourceState.undefined
        :type: ResourceState
        :value: ResourceState.undefined
    
    .. py:attribute:: sgl.ResourceState.general
        :type: ResourceState
        :value: ResourceState.general
    
    .. py:attribute:: sgl.ResourceState.pre_initialized
        :type: ResourceState
        :value: ResourceState.pre_initialized
    
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
    
    .. py:attribute:: sgl.ResourceState.pixel_shader_resource
        :type: ResourceState
        :value: ResourceState.pixel_shader_resource
    
    .. py:attribute:: sgl.ResourceState.non_pixel_shader_resource
        :type: ResourceState
        :value: ResourceState.non_pixel_shader_resource
    


----

.. py:class:: sgl.ResourceType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.ResourceType.unknown
        :type: ResourceType
        :value: ResourceType.unknown
    
    .. py:attribute:: sgl.ResourceType.buffer
        :type: ResourceType
        :value: ResourceType.buffer
    
    .. py:attribute:: sgl.ResourceType.texture_1d
        :type: ResourceType
        :value: ResourceType.texture_1d
    
    .. py:attribute:: sgl.ResourceType.texture_2d
        :type: ResourceType
        :value: ResourceType.texture_2d
    
    .. py:attribute:: sgl.ResourceType.texture_3d
        :type: ResourceType
        :value: ResourceType.texture_3d
    
    .. py:attribute:: sgl.ResourceType.texture_cube
        :type: ResourceType
        :value: ResourceType.texture_cube
    


----

.. py:class:: sgl.ResourceUsage

    Base class: :py:class:`enum.IntFlag`
    
    .. py:attribute:: sgl.ResourceUsage.none
        :type: ResourceUsage
        :value: ResourceUsage.none
    
    .. py:attribute:: sgl.ResourceUsage.vertex
        :type: ResourceUsage
        :value: ResourceUsage.vertex
    
    .. py:attribute:: sgl.ResourceUsage.index
        :type: ResourceUsage
        :value: ResourceUsage.index
    
    .. py:attribute:: sgl.ResourceUsage.constant
        :type: ResourceUsage
        :value: ResourceUsage.constant
    
    .. py:attribute:: sgl.ResourceUsage.stream_output
        :type: ResourceUsage
        :value: ResourceUsage.stream_output
    
    .. py:attribute:: sgl.ResourceUsage.shader_resource
        :type: ResourceUsage
        :value: ResourceUsage.shader_resource
    
    .. py:attribute:: sgl.ResourceUsage.unordered_access
        :type: ResourceUsage
        :value: ResourceUsage.unordered_access
    
    .. py:attribute:: sgl.ResourceUsage.render_target
        :type: ResourceUsage
        :value: ResourceUsage.render_target
    
    .. py:attribute:: sgl.ResourceUsage.depth_stencil
        :type: ResourceUsage
        :value: ResourceUsage.depth_stencil
    
    .. py:attribute:: sgl.ResourceUsage.indirect_arg
        :type: ResourceUsage
        :value: ResourceUsage.indirect_arg
    
    .. py:attribute:: sgl.ResourceUsage.shared
        :type: ResourceUsage
        :value: ResourceUsage.shared
    
    .. py:attribute:: sgl.ResourceUsage.acceleration_structure
        :type: ResourceUsage
        :value: ResourceUsage.acceleration_structure
    


----

.. py:class:: sgl.ResourceView

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:property:: type
        :type: sgl.ResourceViewType
    
    .. py:property:: resource
        :type: sgl.Resource
    


----

.. py:class:: sgl.ResourceViewType

    Base class: :py:class:`enum.Enum`
    
    .. py:attribute:: sgl.ResourceViewType.unknown
        :type: ResourceViewType
        :value: ResourceViewType.unknown
    
    .. py:attribute:: sgl.ResourceViewType.render_target
        :type: ResourceViewType
        :value: ResourceViewType.render_target
    
    .. py:attribute:: sgl.ResourceViewType.depth_stencil
        :type: ResourceViewType
        :value: ResourceViewType.depth_stencil
    
    .. py:attribute:: sgl.ResourceViewType.shader_resource
        :type: ResourceViewType
        :value: ResourceViewType.shader_resource
    
    .. py:attribute:: sgl.ResourceViewType.unordered_access
        :type: ResourceViewType
        :value: ResourceViewType.unordered_access
    


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

    
    
    .. py:method:: __init__(self, shader_object: sgl.ShaderObject) -> None
    
    .. py:property:: type_layout
        :type: sgl.TypeLayoutReflection
    
    .. py:property:: type
        :type: sgl.TypeReflection
    
    .. py:property:: offset
        :type: sgl.ShaderOffset
    
    .. py:method:: is_valid(self) -> bool
    
    .. py:method:: dereference(self) -> sgl.ShaderCursor
    
    .. py:method:: find_field(self, name: str) -> sgl.ShaderCursor
    
    .. py:method:: find_element(self, index: int) -> sgl.ShaderCursor
    
    .. py:method:: find_entry_point(self, index: int) -> sgl.ShaderCursor
    
    .. py:method:: has_field(self, name: str) -> bool
    
    .. py:method:: has_element(self, index: int) -> bool
    
    .. py:method:: set_object(self, object: sgl.MutableShaderObject) -> None
    
    .. py:method:: set_resource(self, resource_view: sgl.ResourceView) -> None
    
    .. py:method:: set_buffer(self, buffer: sgl.Buffer) -> None
    
    .. py:method:: set_texture(self, texture: sgl.Texture) -> None
    
    .. py:method:: set_sampler(self, sampler: sgl.Sampler) -> None
    
    .. py:method:: set_acceleration_structure(self, acceleration_structure: sgl.AccelerationStructure) -> None
    
    .. py:method:: set_data(self, data: ndarray[device='cpu']) -> None
    


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

.. py:class:: sgl.SubresourceLayout

    
    
    .. py:property:: row_pitch
        :type: int
    
        Size of a single row in bytes (unaligned).
        
    .. py:property:: row_pitch_aligned
        :type: int
    
        Size of a single row in bytes (aligned to device texture alignment).
        
    .. py:property:: row_count
        :type: int
    
        Number of rows.
        
    .. py:property:: depth
        :type: int
    
        Number of depth slices.
        
    .. py:property:: total_size
        :type: int
    
        Get the total size of the subresource in bytes (unaligned).
        
    .. py:property:: total_size_aligned
        :type: int
    
        Get the total size of the subresource in bytes (aligned to device
        texture alignment).
        


----

.. py:class:: sgl.Swapchain

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:property:: desc
        :type: sgl.SwapchainDesc
    
        Returns the swapchain description.
        
    .. py:property:: images
        :type: list[sgl.Texture]
    
        Returns the back buffer images.
        
    .. py:method:: get_image(self, index: int) -> sgl.Texture
    
        Returns the back buffer image at position `index`.
        
    .. py:method:: present(self) -> None
    
        Present the next image in the swapchain.
        
    .. py:method:: acquire_next_image(self) -> int
    
        Returns the index of next back buffer image that will be presented in
        the next `present` call. Returns -1 if no image is available and the
        caller should skip the frame.
        
    .. py:method:: resize(self, width: int, height: int) -> None
    
        Resizes the back buffers of this swapchain. All render target views
        and framebuffers referencing the back buffer images must be freed
        before calling this method.
        
    .. py:method:: is_occluded(self) -> bool
    
        Returns true if the window is occluded.
        
    .. py:property:: fullscreen_mode
        :type: bool
    


----

.. py:class:: sgl.SwapchainDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: format
        :type: sgl.Format
    
        Format of the swapchain images.
        
    .. py:property:: width
        :type: int
    
        Width of the swapchain images in pixels.
        
    .. py:property:: height
        :type: int
    
        Height of the swapchain images in pixels.
        
    .. py:property:: image_count
        :type: int
    
        Number of swapchain images.
        
    .. py:property:: enable_vsync
        :type: bool
    
        Enable/disable vertical synchronization.
        


----

.. py:class:: sgl.TargetBlendDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: color
        :type: sgl.AspectBlendDesc
    
    .. py:property:: alpha
        :type: sgl.AspectBlendDesc
    
    .. py:property:: enable_blend
        :type: bool
    
    .. py:property:: logic_op
        :type: sgl.LogicOp
    
    .. py:property:: write_mask
        :type: sgl.RenderTargetWriteMask
    


----

.. py:class:: sgl.Texture

    Base class: :py:class:`sgl.Resource`
    
    
    
    .. py:property:: desc
        :type: sgl.TextureDesc
    
    .. py:property:: width
        :type: int
    
    .. py:property:: height
        :type: int
    
    .. py:property:: depth
        :type: int
    
    .. py:property:: array_size
        :type: int
    
    .. py:property:: mip_count
        :type: int
    
    .. py:property:: subresource_count
        :type: int
    
    .. py:method:: get_subresource_index(self, mip_level: int, array_slice: int = 0) -> int
    
    .. py:method:: get_subresource_array_slice(self, subresource: int) -> int
    
    .. py:method:: get_subresource_mip_level(self, subresource: int) -> int
    
    .. py:method:: get_mip_width(self, mip_level: int = 0) -> int
    
    .. py:method:: get_mip_height(self, mip_level: int = 0) -> int
    
    .. py:method:: get_mip_depth(self, mip_level: int = 0) -> int
    
    .. py:method:: get_mip_dimensions(self, mip_level: int = 0) -> sgl.math.uint3
    
    .. py:method:: get_subresource_layout(self, subresource: int) -> sgl.SubresourceLayout
    
    .. py:method:: get_srv(self, mip_level: int = 0, mip_count: int = 4294967295, base_array_layer: int = 0, layer_count: int = 4294967295) -> sgl.ResourceView
    
        Get a shader resource view for a subresource range of the texture.
        
    .. py:method:: get_uav(self, mip_level: int = 0, base_array_layer: int = 0, layer_count: int = 4294967295) -> sgl.ResourceView
    
        Get a unordered access view for a subresource range of the texture.
        \note Only a single mip level can be bound.
        
    .. py:method:: get_dsv(self, mip_level: int = 0, base_array_layer: int = 0, layer_count: int = 4294967295) -> sgl.ResourceView
    
        Get a depth stencil view for a subresource range of the texture. \note
        Only a single mip level can be bound.
        
    .. py:method:: get_rtv(self, mip_level: int = 0, base_array_layer: int = 0, layer_count: int = 4294967295) -> sgl.ResourceView
    
        Get a render target view for a subresource range of the texture. \note
        Only a single mip level can be bound.
        
    .. py:method:: to_bitmap(self, mip_level: int = 0, array_slice: int = 0) -> sgl.Bitmap
    
    .. py:method:: to_numpy(self, mip_level: int = 0, array_slice: int = 0) -> numpy.ndarray[]
    
    .. py:method:: from_numpy(self, data: numpy.ndarray[], mip_level: int = 0, array_slice: int = 0) -> None
    


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

.. py:class:: sgl.TextureDesc

    
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, arg: dict, /) -> None
        :no-index:
    
    .. py:property:: type
        :type: sgl.ResourceType
    
        Resource type (optional). Type is inferred from width, height, depth
        if not specified.
        
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
        
    .. py:property:: array_size
        :type: int
    
        Number of array slices (1 for non-array textures).
        
    .. py:property:: mip_count
        :type: int
    
        Number of mip levels (0 for auto-generated mips).
        
    .. py:property:: sample_count
        :type: int
    
        Number of samples per pixel (1 for non-multisampled textures).
        
    .. py:property:: quality
        :type: int
    
        Quality level for multisampled textures.
        
    .. py:property:: initial_state
        :type: sgl.ResourceState
    
    .. py:property:: usage
        :type: sgl.ResourceUsage
    
    .. py:property:: memory_type
        :type: sgl.MemoryType
    
    .. py:property:: debug_name
        :type: str
    


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

.. py:class:: sgl.TransientShaderObject

    Base class: :py:class:`sgl.ShaderObject`
    
    
    


----

.. py:class:: sgl.TypeConformance

    Type conformance entry. Type conformances are used to narrow the set
    of types supported by a slang interface. They can be specified on an
    entry point to omit generating code for types that do not conform.
    
    .. py:method:: __init__(self) -> None
    
    .. py:method:: __init__(self, type_name: str, interface_name: str, id: int = -1) -> None
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
    
    .. py:method:: __init__(self, hwnd: int) -> None
    


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
        
    .. py:property:: swapchain_format
        :type: sgl.Format
    
        Format of the swapchain images.
        
    .. py:property:: enable_vsync
        :type: bool
    
        Enable/disable vertical synchronization.
        


----

.. py:class:: sgl.AppWindow

    Base class: :py:class:`sgl.Object`
    
    
    
    .. py:method:: __init__(self, app: sgl.App, width: int = 1920, height: int = 1280, title: str = 'sgl', mode: sgl.WindowMode = WindowMode.normal, resizable: bool = True, swapchain_format: sgl.Format = Format.bgra8_unorm_srgb, enable_vsync: bool = False) -> None
    
    .. py:class:: sgl.AppWindow.RenderContext
    
        
        
        .. py:property:: swapchain_image
            :type: sgl.Texture
        
        .. py:property:: framebuffer
            :type: sgl.Framebuffer
        
        .. py:property:: command_buffer
            :type: sgl.CommandBuffer
        
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
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, writable=False, shape=(2, 2)]
    


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
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, writable=False, shape=(3, 3)]
    


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
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, writable=False, shape=(2, 4)]
    


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
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, writable=False, shape=(3, 4)]
    


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
    
    .. py:method:: to_numpy(self) -> numpy.ndarray[dtype=float32, writable=False, shape=(4, 4)]
    


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

.. py:function:: sgl.math.transpose(x: sgl.math.float2x4) -> sgl::math::matrix<float,4,2>
    :no-index:

.. py:function:: sgl.math.transpose(x: sgl.math.float3x4) -> sgl::math::matrix<float,4,3>
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

.. py:function:: sgl.math.mul(x: sgl.math.float2x4, y: sgl::math::matrix<float,4,2>) -> sgl.math.float2x2
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float2x4, y: sgl.math.float4) -> sgl.math.float2
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float2, y: sgl.math.float2x4) -> sgl.math.float4
    :no-index:

.. py:function:: sgl.math.mul(x: sgl.math.float3x4, y: sgl::math::matrix<float,4,3>) -> sgl.math.float3x3
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
    
    .. py:method:: render(self, framebuffer: sgl.Framebuffer, command_buffer: sgl.CommandBuffer) -> None
    
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
            :type: sgl.ResourceUsage
        
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

Miscellaneous
-------------



----

