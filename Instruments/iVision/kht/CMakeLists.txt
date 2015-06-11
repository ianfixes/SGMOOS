SET(KHT_SRCS 
buffer_2d.cpp buffer_2d.h
eigen.cpp eigen.h
kht.cpp kht.h
linking.cpp linking.h
peak_detection.cpp peak_detection.h
subdivision.cpp subdivision.h
types.h
voting.cpp voting.h
)

ADD_LIBRARY(KHT ${KHT_SRCS})
