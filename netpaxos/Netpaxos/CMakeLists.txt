set (NETPAXOS_SRCS netpaxos_time.c
	netpaxosclient.c netpaxoslearner.c
)
add_library(netpaxos ${NETPAXOS_SRCS})
target_link_libraries (netpaxos event)