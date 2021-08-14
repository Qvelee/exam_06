/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nelisabe <nelisabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/07/27 10:15:05 by nelisabe          #+#    #+#             */
/*   Updated: 2021/08/14 09:24:23 by nelisabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <errno.h>
# include <string.h>
# include <strings.h>
# include <unistd.h>
# include <netdb.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h>
# include <sys/select.h>
# include <stdio.h>

// int extract_message(char **buf, char **msg)
// {
// 	char	*newbuf;
// 	int	i;

// 	*msg = 0;
// 	if (*buf == 0)
// 		return (0);
// 	i = 0;
// 	while ((*buf)[i])
// 	{
// 		if ((*buf)[i] == '\n')
// 		{
// 			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
// 			if (newbuf == 0)
// 				return (-1);
// 			strcpy(newbuf, *buf + i + 1);
// 			*msg = *buf;
// 			(*msg)[i + 1] = 0;
// 			*buf = newbuf;
// 			return (1);
// 		}
// 		i++;
// 	}
// 	return (0);
// }

// char	*str_join(char *buf, char *add)
// {
// 	char	*newbuf;
// 	int		len;

// 	if (buf == 0)
// 		len = 0;
// 	else
// 		len = strlen(buf);
// 	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
// 	if (newbuf == 0)
// 		return (0);
// 	newbuf[0] = 0;
// 	if (buf != 0)
// 		strcat(newbuf, buf);
// 	free(buf);
// 	strcat(newbuf, add);
// 	return (newbuf);
// }

int		error(void)
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	return 1;
}

void	accept_new_client(int server_socket, int *clients,\
	fd_set *read_fds, fd_set *write_fds)
{
	int		i = -1;
	int		new_client;

	if (!FD_ISSET(server_socket, read_fds))
		return;
	new_client = accept(server_socket, NULL, NULL);
	while (clients[++i] = -1)
		if (clients[i] != -2 && FD_ISSET(clients[i], write_fds))
		{
			char	message[60];

			sprintf(message, "server: client %d just arrived\n", i);
			send(clients[i], message, strlen(message), 0);
		}
	clients[i] = new_client;
}

int		init_sockets_fds(int server_socket, int *clients,
	fd_set *read_fds, fd_set *write_fds)
{
	int		i = -1;
	int		max_fd = server_socket;

	FD_SET(server_socket, read_fds);
	while (clients[++i] != -1)
	{
		FD_SET(clients[i], read_fds);
		FD_SET(clients[i], write_fds);
		if (clients[i] > max_fd)
			max_fd = clients[i];
	}
	return max_fd;
}

void	send_to_all(int	*clients, char *message, )
{
	
}

int		connetion(int socket_ID, int *clients)
{
	fd_set	read_fds;
	fd_set	write_fds;
	int		i;
	int		max_fd;

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	max_fd = init_sockets_fds(socket_ID, clients, &read_fds, &write_fds);
	if (select(max_fd + 1, &read_fds, &write_fds, NULL, NULL) == -1)
		return -1;
	accept_new_client(socket_ID, clients, &read_fds, &write_fds);
	i = -1;
	while (clients[++i] != -1)
	{
		if (clients[i] != -2 && FD_ISSET(clients[i], &read_fds))
		{
			int		bytes;
			char	buffer[65536];

			bytes = recv(clients[i], buffer, 65536, 0);
			if (bytes > 0)
			{
				buffer[bytes] = '\0';
				
			}
			else
			{
				//close
			}
		}
	}
	return 0;
}

int		setup(int port)
{
	int				socket_Id;
	struct sockaddr_in	socket_parameters_in;

	bzero(&socket_parameters_in, sizeof(socket_parameters_in));
	if ((socket_Id = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	socket_parameters_in.sin_family = AF_INET;
	socket_parameters_in.sin_port = htons(port);
	socket_parameters_in.sin_addr.s_addr = (1 << 24) | 127;
	if (bind(socket_Id, (struct sockaddr*)(&socket_parameters_in),\
		sizeof(socket_parameters_in)) == -1)
	{
		close(socket_Id);
		return -1;
	}
	if (listen(socket_Id, 5) == -1)
	{
		close(socket_Id);
		return -1;
	}
	return socket_Id;
}

int		main(int argc, char **argv)
{
	int		socket_ID;
	int		port;
	int		clients[65536];

	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n",\
			strlen("Wrong number of arguments\n"));
		return (1);
	}
	port = atoi(argv[1]);
	if ((socket_ID = setup(port)) == -1)
		return error();
	for (int i = 0; i < 65536; i++)
		clients[i] = -1;
	while (1)
		if (connetion(socket_ID, clients) == -1)
		{
			int		i = -1;

			close(socket_ID);
			while (clients[++i] != -1)
				if (clients[i] != -2)
					close(clients[i]);
			return error();
		}
	return 0;
}
