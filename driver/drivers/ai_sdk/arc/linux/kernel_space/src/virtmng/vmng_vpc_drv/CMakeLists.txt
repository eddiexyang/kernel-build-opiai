set(drv_vpc_host_depends drv_seclib_host drv_vascend_stub)

set(HOST_MAKE_ARGS
	${MAKE_ARGS}
	SERVER_TYPE=host
)

add_host_ko(LOCAL_MODULE drv_vpc_host
			KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
			MAKE_ARGS ${HOST_MAKE_ARGS}
			TARGETE_DPENDS "${drv_vpc_host_depends}")


set(drv_vpc_agent_depends drv_seclib_host)
set(AGENT_MAKE_ARGS
	${MAKE_ARGS}
	SERVER_TYPE=agent
)

add_host_ko(LOCAL_MODULE drv_vpc_agent
			KO_SRC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}
			MAKE_ARGS ${AGENT_MAKE_ARGS}
			TARGETE_DPENDS "${drv_vpc_agent_depends}")