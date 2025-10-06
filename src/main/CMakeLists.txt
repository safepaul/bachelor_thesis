idf_component_register(SRCS "app.c" "source/mcinit.c" "data/tasks.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS "")
