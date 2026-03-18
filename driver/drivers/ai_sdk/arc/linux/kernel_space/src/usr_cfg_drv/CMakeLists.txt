if( ${PRODUCT} STREQUAL ascend610)
    set(drv_user_cfg_depends )
elseif ( (${PRODUCT} STREQUAL ascend610Liteesl) OR (${PRODUCT} STREQUAL ascend610Lite))
    set(drv_user_cfg_depends )
elseif ( ${PRODUCT} STREQUAL ascend310p)
    set(drv_user_cfg_depends hsm_firmware_update hsm_flash hsm_heartbeat)
elseif ( ${PRODUCT} STREQUAL ascend910B)
    set(drv_upgrade_depends )
elseif ( ${PRODUCT} STREQUAL ascend920esl)
    set(drv_upgrade_depends )
elseif ( ${PRODUCT} STREQUAL ascend920emu)
    set(drv_upgrade_depends )
elseif ( ${PRODUCT} STREQUAL helper310p)
    set(drv_user_cfg_depends )
elseif ( ${PRODUCT} STREQUAL ascend310B)
    set(drv_user_cfg_depends )
elseif ( ${PRODUCT} STREQUAL ascend310Besl)
    set(drv_user_cfg_depends hsm_firmware_update hsm_flash hsm_heartbeat)
elseif ( ${PRODUCT} STREQUAL ascend310Bemu)
    set(drv_user_cfg_depends hsm_firmware_update hsm_flash hsm_heartbeat)
elseif ( ${PRODUCT} STREQUAL ascend310Brc)
    set(drv_user_cfg_depends hsm_firmware_update hsm_flash hsm_heartbeat)
elseif ( ${PRODUCT} STREQUAL as31xm1)
    set(drv_user_cfg_depends hsm_firmware_update hsm_flash hsm_heartbeat)
elseif ( ${PRODUCT} STREQUAL ascend310Brcesl)
    set(drv_user_cfg_depends hsm_firmware_update hsm_flash hsm_heartbeat)
elseif ( ${PRODUCT} STREQUAL ascend310Brcemu)
    set(drv_user_cfg_depends )
else ()
    set(drv_user_cfg_depends drv_nor_flash tee_drv)
endif ()

list(APPEND MAKE_ARGS
    KBUILD_MODPOST_WARN=1
)

add_device_ko(LOCAL_MODULE drv_user_cfg
    KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
    MAKE_ARGS ${MAKE_ARGS}
    TARGETE_DPENDS "${drv_user_cfg_depends}")

#add_device_ko_target(drv_user_cfg ${CMAKE_CURRENT_SOURCE_DIR})

# if((NOT ${PRODUCT} STREQUAL ascend610) AND (NOT ${PRODUCT} STREQUAL ascend610))
# add_dependencies(drv_user_cfg drv_nor_flash tee_drv)
# endif ()
