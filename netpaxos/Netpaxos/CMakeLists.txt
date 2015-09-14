set (NETPAXOS_SRCS netpaxos_time.c
	netpaxos_client.c netpaxos_learner.c
)
add_library(netpaxos ${NETPAXOS_SRCS})
target_link_libraries (netpaxos event)