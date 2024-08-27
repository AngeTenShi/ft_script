/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   output.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anggonza <anggonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 11:30:37 by anggonza          #+#    #+#             */
/*   Updated: 2024/08/27 11:37:29 by anggonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_script.h"

void	output_term_info(int output_fd)
{
	struct winsize	ws;
	char			*cols_str;
	char			*lines_str;

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1)
		handle_error("ioctl TIOCGWINSZ");
	lines_str = ft_itoa(ws.ws_row);
	cols_str = ft_itoa(ws.ws_col);
	write(output_fd, "\" COLUMNS=", 10);
	write(output_fd, cols_str, ft_strlen(cols_str));
	write(output_fd, " LINES=", 7);
	write(output_fd, lines_str, ft_strlen(lines_str));
	free(cols_str);
	free(lines_str);
	write(output_fd, "]\n", 2);
}

void	output_helper(char *f, struct termios term, int fd, int output_fd)
{
	time_t			t;

	ft_putstr_fd("Script started, output file is ", 1);
	ft_putstr_fd(f, 1);
	ft_putstr_fd("\n", 1);
	tty_set_raw(&term);
	write(output_fd, "Script started on ", 18);
	t = time(NULL);
	write(output_fd, ctime(&t), ft_strlen(ctime(&t)) - 1);
	write(output_fd, " [TERM=\"", 8);
	write(output_fd, getenv("TERM"), ft_strlen(getenv("TERM")));
	write(output_fd, "\" TTY=", 6);
	write(output_fd, ttyname(fd), ft_strlen(ttyname(fd)));
	close(fd);
	output_term_info(output_fd);
}

int	check_fds(int master_fd, int output_fd, fd_set *in_fds)
{
	ssize_t	num_read;
	char	buffer[256];

	if (FD_ISSET(STDIN_FILENO, in_fds))
	{
		num_read = read(STDIN_FILENO, buffer, sizeof(buffer));
		if (num_read <= 0)
			return (0);
		if (write(master_fd, buffer, num_read) != num_read)
			handle_error("partial/failed write (master_fd)");
	}
	if (FD_ISSET(master_fd, in_fds))
	{
		num_read = read(master_fd, buffer, sizeof(buffer));
		if (num_read <= 0)
			return (0);
		if (write(STDOUT_FILENO, buffer, num_read) != num_read)
			handle_error("partial/failed write (STDOUT_FILENO)");
		if (write(output_fd, buffer, num_read) != num_read)
			handle_error("partial/failed write (output_fd)");
	}
	return (1);
}

void	fd_loop(int master_fd, int output_fd)
{
	fd_set	in_fds;

	while (1)
	{
		FD_ZERO(&in_fds);
		FD_SET(STDIN_FILENO, &in_fds);
		FD_SET(master_fd, &in_fds);
		if (select(master_fd + 1, &in_fds, NULL, NULL, NULL) == -1)
			handle_error("select");
		if (!check_fds(master_fd, output_fd, &in_fds))
			break ;
	}
}
