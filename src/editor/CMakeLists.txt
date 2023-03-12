# -----------------------------------------------------------------------------
# editor
# -----------------------------------------------------------------------------

add_executable(editor)
target_sources(editor PRIVATE editor.cpp)
target_link_libraries(editor PRIVATE kali)
set_target_properties(editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${KALI_RUNTIME_OUTPUT_DIRECTORY}
)
