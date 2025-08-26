#include "../../include/server.hpp"

void check_clients_timeout(int epoll_fd, Parsing_class &server) {
	time_t current_time = time(NULL);
	const int timeout_seconds = 3;

	for (std::map<int, ClientData>::iterator it = server.getClients().begin(); it != server.getClients().end(); ) {
		double elapsed_time = difftime(current_time, it->second.last_activity);
		if (elapsed_time > timeout_seconds) {
			it->second.write_buff = "HTTP/1.1 408 Request Timeout\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
			it->second.keep_alive = false;

			struct epoll_event ev;
			ev.events = EPOLLOUT | EPOLLET;
			ev.data.fd = it->first;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, it->first, &ev);

			++it;
		} else {
			++it;
		}
	}
}
