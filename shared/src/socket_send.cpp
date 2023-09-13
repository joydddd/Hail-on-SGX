#include "socket_send.h"
#include <iostream>
#include <curl/curl.h>
#include "errno.h"

int make_server_sockaddr(struct sockaddr_in *addr, int port) {
	// Step (1): specify socket family.
	// This is an internet socket.
	addr->sin_family = AF_INET;

	// Step (2): specify socket address (hostname).
	// The socket will be a server, so it will only be listening.
	// Let the OS map it to the correct address.
	addr->sin_addr.s_addr = INADDR_ANY;

	// Step (3): Set the port value.
	// If port is 0, the OS will choose the port for us.
	// Use htons to convert from local byte order to network byte order.
	addr->sin_port = htons(port);

	return 0;
}

int make_client_sockaddr(struct sockaddr_in *addr, const char *hostname, int port) {
	// Step (1): specify socket family.
	// This is an internet socket.
	addr->sin_family = AF_INET;

	// Step (2): specify socket address (hostname).
	// The socket will be a client, so call this unix helper function
	// to convert a hostname string to a useable `hostent` struct.
	struct hostent *host = gethostbyname(hostname);
	if (host == NULL) {
		fprintf(stderr, "%s: unknown host\n", hostname);
		return -1;
	}
	memcpy(&(addr->sin_addr), host->h_addr, host->h_length);

	// Step (3): Set the port value.
	// Use htons to convert from local byte order to network byte order.
	addr->sin_port = htons(port);

	return 0;
}

int get_port_number(int sockfd) {
 	struct sockaddr_in addr;
	socklen_t length = sizeof(addr);
	if (getsockname(sockfd, (sockaddr *) &addr, &length) == -1) {
		perror("Error getting port of socket");
		return -1;
	}
	// Use ntohs to convert from network byte order to host byte order.
	return ntohs(addr.sin_port);
 }

int send_message(const char *hostname, int port, const char *message, const int message_length, int sock) {
	if (message_length > MAX_MESSAGE_SIZE) {
		throw std::runtime_error("Message exceeds maximum length: " + std::to_string(message_length));
		return -1;
	}
	// Connect to remote server
	if (sock == -1) {
		struct addrinfo hints = {}, *addrs;
		char port_str[16] = {};

		hints.ai_family = AF_INET; 
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		sprintf(port_str, "%d", port);

		if (getaddrinfo(hostname, port_str, &hints, &addrs) != 0) {
			throw std::runtime_error("Failed to get addr info");
		}

		for (struct addrinfo *addr = addrs; addr != NULL; addr = addr->ai_next) {
			sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
			if (sock == -1)
				break;
			
			if (!addr->ai_addr) {
				std::cout << "addr ai was null, go to next?" << std::endl;
				sock = -1;
				continue;
			}
			if (!addr->ai_addrlen) {
				std::cout << "addr ai len was 0, go to next?" << std::endl;
				sock = -1;
				continue;
			}
			std::cout << "b4 connect" << std::endl;
			if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0) {
				break;
			}

			close(sock);
			sock = -1;
		}
		std::cout << "after connect" << std::endl;
		freeaddrinfo(addrs);
		std::cout << "after freeaddrinfo" << std::endl;
		if (sock == -1) {
			char buffer[ 256 ];
			char * errorMsg = strerror_r( errno, buffer, 256 ); // GNU-specific version, Linux default
			printf("Error %s\n", errorMsg); //return value has to be used since buffer might not be modified
			throw std::runtime_error("Failed to connect\n");
		}
	}
	std::cout << "about to send" << std::endl;
	// Send message to remote server
	if (send(sock, message, message_length, 0) == -1) {
		throw std::runtime_error("Hostname: " + std::string(hostname) + " error sending on stream socket");
	}
	std::cout << "after send" << std::endl;

	return sock;
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, std::string *s) 
{
  s->append(static_cast<char *>(ptr), size*nmemb);
  return size*nmemb;
}

// Find a better solution, but this works for now
std::string get_hostname_str() {
	CURL *curl = curl_easy_init();
	std::string readBuffer;

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://ipv4.icanhazip.com");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);

		readBuffer.pop_back();

		return readBuffer;
  	}
	return "Failed to find IP\n";
}