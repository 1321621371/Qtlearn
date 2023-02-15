#ifndef _EVFACTORY_PCBA_NETWORK_H_
#define _EVFACTORY_PCBA_NETWORK_H_

enum ETH_LINK_STATE {
	ETH_LINK_DOWN,
	ETH_LINK_UP,
	ETH_LINK_UNKNOWN,
};

int get_eth_link_state(const char *ethName);
int get_eth_link_speed(const char *ethName);
int reset_eth_link(const char *ethName);

#endif
