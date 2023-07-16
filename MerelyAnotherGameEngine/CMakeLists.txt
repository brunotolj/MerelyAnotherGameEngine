
add_executable(MerelyAnotherGameEngine
	"MerelyAnotherGameEngine.cpp"
    "MV/MV_Device.cpp"
    "MV/MV_Model.cpp"
    "MV/MV_Object.cpp"
    "MV/MV_Pipeline.cpp"
    "MV/MV_Renderer.cpp"
    "MV/MV_Swapchain.cpp"
    "MV/MV_TestRenderSystem.cpp"
    "MV/MV_Window.cpp"
)

set (SHADERS
    "SimpleShader.frag"
    "SimpleShader.vert"
)

function(add_shader SHADER)
    find_program(GLSLC glslc)

    set(current-shader-path ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER})
    set(current-output-path ${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv)

    get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
    file(MAKE_DIRECTORY ${current-output-dir})

    add_custom_command(
           OUTPUT ${current-output-path}
           COMMAND ${GLSLC} -o ${current-output-path} ${current-shader-path}
           DEPENDS ${current-shader-path}
           IMPLICIT_DEPENDS CXX ${current-shader-path}
           VERBATIM)

    set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
    target_sources(MerelyAnotherGameEngine PRIVATE ${current-output-path})
endfunction(add_shader)

foreach(SHADER IN ITEMS ${SHADERS})
    add_shader(${SHADER})
endforeach()

find_package(Vulkan REQUIRED)

target_include_directories(MerelyAnotherGameEngine PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${Vulkan_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/ThirdParty/glfw-3.3.8/include
    ${PROJECT_SOURCE_DIR}/ThirdParty/glm-master
)

target_link_libraries(MerelyAnotherGameEngine PRIVATE
    ${Vulkan_LIBRARY}
    ${PROJECT_SOURCE_DIR}/ThirdParty/glfw-3.3.8/glfw3.lib
)

set_property(TARGET MerelyAnotherGameEngine PROPERTY CXX_STANDARD 20)