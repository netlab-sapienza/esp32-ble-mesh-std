set(COMPONENT_REQUIRES)
set(COMPONENT_PRIV_REQUIRES)

set(COMPONENT_SRCS "main.c")

set(EXTRA_COMPONENT_DIRS ../components/init)
set(EXTRA_COMPONENT_DIRS ../components/client)
set(EXTRA_COMPONENT_DIRS ../components/server)

set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()