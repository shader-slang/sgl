### 0.1

- rename project
- add support for loading EXR files
- add support for texture init data
- add more type checking to `ShaderCursor`
- add rdoc strings
- add support for passing entrypoint arguments
- add a simple windowed application base class


### Misc

- expose framebuffer layout so we can use the same pipeline for different framebuffer objects that share the same layout
- describe `Bitmap` pixel format with a `Struct`
- add support for XYZ images
- check performance for loading images through memory mapped streams
- implement `StructConverter` JIT for arm64
- implement `weak_ref`
