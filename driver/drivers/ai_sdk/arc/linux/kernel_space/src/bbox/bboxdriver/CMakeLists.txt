set(product_list "ascend910B" "ascend920esl" "ascend310B" "as31xm1" "ascend310Besl" "ascend310Bemu" "ascend310Brc" "ascend310Brcesl" "ascend310Brcemu" "ascend610" "ascend610Liteesl" "ascend610Lite" "bs9sx1a")
if (${PRODUCT} IN_LIST product_list)
add_device_ko(LOCAL_MODULE mntn_bbox
    KO_SRC_FOLDER "${CMAKE_CURRENT_LIST_DIR}"
    MAKE_ARGS ${MAKE_ARGS}
)
endif ()
add_subdirectory(agent)
