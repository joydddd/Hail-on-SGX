#include "socket_send.h"
#include <iostream>
#include <curl/curl.h>

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
		throw std::runtime_error("Message exceeds maximum length: " + std::to_string(message_length) + "\t" + message);
		return -1;
	}
    int sockfd = sock == -1 ? socket(AF_INET, SOCK_STREAM, 0) : sock;

	// (3) Connect to remote server
	if (sock == -1) {
		// (2) Create a sockaddr_in to specify remote host and port
		struct sockaddr_in* addr = (sockaddr_in*)malloc(sizeof(sockaddr_in));
		if (make_client_sockaddr(addr, hostname, port) == -1) {
			return -1;
		}

		if (connect(sockfd, (sockaddr *) addr, sizeof(sockaddr_in)) == -1) {
			perror("Error connecting stream socket");
			return -1;
		}
		free(addr);
	}

	// (4) Send message to remote server
	if (send(sockfd, message, message_length, 0) == -1) {
		perror("Error sending on stream socket");
		return -1;
	}
	

	return sockfd;
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
    return "127.0.0.1";

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