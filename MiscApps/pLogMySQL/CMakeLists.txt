PROJECT(pLogMySQL)

#what files are needed?
SET(PLOGMYSQL_SRCS MOOSLogMySQL.cpp MOOSLogMySQL.h pLogMySQLMain.cpp)

SET(MYSQL_INCLUDES /usr/include/mysql /usr/include/mysql++)
INCLUDE_DIRECTORIES( ${MYSQL_INCLUDES} )

# Add executable called pLogMySQL
ADD_EXECUTABLE( pLogMySQL ${PLOGMYSQL_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(pLogMySQL ${MOOSLIBS} ${PLATFORM_LIBS} mysqlpp)
