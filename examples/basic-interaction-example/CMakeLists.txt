add_example_executable(CubeChaosNodeEditor
    basic-interaction-example.cpp
)

# Optional – make the .exe name pretty
set_target_properties(CubeChaosNodeEditor PROPERTIES
    OUTPUT_NAME "CubeChaos Node Editor"
)
