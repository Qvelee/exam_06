# include <stdlib.h>
# include <unistd.h>
# include <sys/select.h>
# include <sys/socket.h>
# include <stdio.h>
# include <string.h>
# include <netdb.h>

int		server_socket;
int		max_fd;
fd_set	read_fds;
fd_set	write_fds;
fd_set	clients_fds;
typedef struct	clients_s
{
	int		fd;
	char	*message;
}				clients_t;
clients_t	clients[60000];

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char	*str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void	error_exit(char *message)
{
	write(2, message, strlen(message));
	write(2, "\n", 1);
	if (server_socket != -1)
		close(server_socket);
	exit(1);
}

int		setup(int	port)
{
	struct sockaddr_in	_sockaddr_in;

	_sockaddr_in.sin_family = AF_INET;
	_sockaddr_in.sin_port = htons(port);
	_sockaddr_in.sin_addr.s_addr = (1 << 24) | 127;
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return 1;
	socklen_t	len = sizeof(_sockaddr_in);
	if (bind(server_socket, (struct sockaddr*)&_sockaddr_in, len) == -1)
		return -1;
	if (listen(server_socket, 120) == -1)
		return -1;
	return 0;
}

void	add_message_to_clients(char *message, int sender)
{
	char	buffer[60];

	sprintf(buffer, message, sender);
	for (int i = 0; clients[i].fd != -2; ++i)
		if (clients[i].fd != -1 && i != sender)
			clients[i].message = str_join(clients[i].message, buffer);
}

void	add_message_to_clients_no_p(char *message, int sender)
{
	for (int i = 0; clients[i].fd != -2; ++i)
		if (clients[i].fd != -1 && i != sender)
			clients[i].message = str_join(clients[i].message, message);
}

void	accept_new_client()
{
	int		i;

	for (i = 0; clients[i].fd != -2; ++i);
	if ((clients[i].fd = accept(server_socket, NULL, NULL)) == -1)
	{
		clients[i].fd = -2;
		return;
	}
	if (clients[i].fd > max_fd)
		max_fd = clients[i].fd;
	FD_SET(clients[i].fd, &clients_fds);
	add_message_to_clients("server: client %d just arrived\n", i);
}

void	read_from_client(int client)
{
	if (!FD_ISSET(clients[client].fd, &read_fds))
		return;
	int		status;
	char	*buffer = calloc(42, 4096);
	status = recv(clients[client].fd, buffer, 64535, 0);
	if (status == 0)
	{
		FD_CLR(clients[client].fd, &clients_fds);
		close(clients[client].fd);
		clients[client].fd = -1;
		free(clients[client].message);
		clients[client].message = NULL;
		add_message_to_clients("server: client %d just left\n", client);
	}
	else if (status > 0)
	{
		char	*message;

		while (extract_message(&buffer, &message))
		{
			add_message_to_clients("client %d: ", client);
			add_message_to_clients_no_p(message, client);
			free(message);
		}
	}
	free(buffer);
}

void	send_message_to_client(int client)
{
	if (!FD_ISSET(clients[client].fd, &write_fds))
		return;
	if (clients[client].message != NULL)
	{
		send(clients[client].fd, clients[client].message, strlen(clients[client].message), 0);
		free(clients[client].message);
		clients[client].message = NULL;
	}
}

int		connection()
{
	if (select(max_fd + 1, &read_fds, &write_fds, NULL, NULL) == -1)
		return -1;
	if (FD_ISSET(server_socket, &read_fds))
		accept_new_client();
	for (int i = 0; clients[i].fd != -2; ++i)
		if (clients[i].fd != -1)
			read_from_client(i);
	for (int i = 0; clients[i].fd != -2; ++i)
		if (clients[i].fd != -1)
			send_message_to_client(i);
	return 0;
}

int		main(int argc, char **argv)
{
	server_socket = -1;
	if (argc != 2)
		error_exit("Wrong number of arguments");
	if (setup(atoi(argv[1])))
		error_exit("Fatal error");
	int		status;

	FD_ZERO(&clients_fds);
	FD_SET(server_socket, &clients_fds);
	max_fd = server_socket;
	for (int i = 0; i < 60000; ++i)
	{
		clients[i].fd = -2;
		clients[i].message = NULL;
	}
	while (1)
	{
		read_fds = write_fds = clients_fds;
		status = connection();
		if (status == -1)
		{
			for (int i = 0; i < 60000; ++i)
			{
				if (clients[i].fd > 0)
					close(clients[i].fd);
				if (clients[i].message != NULL)
					free(clients[i].message);
			}
			error_exit("Fatal error");
		}
	}
	return 0;
}
