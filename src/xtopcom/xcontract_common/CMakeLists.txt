add_compile_options(-Wpedantic)

aux_source_directory(./src src_dir)
add_library(xcontract_common STATIC ${src_dir})

add_dependencies(xcontract_common xdata xcommon xbasic xxbase)

target_link_libraries(xcontract_common PRIVATE xdata xcommon xbasic xxbase pthread)
