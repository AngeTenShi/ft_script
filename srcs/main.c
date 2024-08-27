/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anggonza <anggonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 09:45:01 by anggonza          #+#    #+#             */
/*   Updated: 2024/08/27 11:42:55 by anggonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_script.h"

// The Linux Programming Interface book page 1380 Pseudo Terminals

char	*g_filename = "typescript";

void	manage_status(int status, int output_fd)
{
	char	*command_exit_code_str;

	if (WIFEXITED(status))
		command_exit_code_str = ft_itoa(WEXITSTATUS(status));
	else
		command_exit_code_str = ft_strdup("UNKNOWN");
	write(output_fd, command_exit_code_str, ft_strlen(command_exit_code_str));
	write(output_fd, "]\n", 2);
	free(command_exit_code_str);
	close(output_fd);
	ft_putstr_fd("Script done\n", 1);
}

void	do_parent(int s_fd, int m_fd, struct termios term, pid_t pid)
{
	int			status;
	time_t		t;
	int			output_fd;

	if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)
		handle_error("tcsetattr");
	output_fd = open(g_filename, O_WRONLY | O_CREAT | O_APPEND,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (output_fd == -1)
		handle_error("open output file");
	output_helper(g_filename, term, s_fd, output_fd);
	fd_loop(m_fd, output_fd);
	close(m_fd);
	restore_terminal(&term);
	waitpid(pid, &status, 0);
	write(output_fd, "Script done on ", 12);
	t = time(NULL);
	write(output_fd, ctime(&t), ft_strlen(ctime(&t)) - 1);
	write(output_fd, " [COMMAND_EXIT_CODE=", 20);
	manage_status(status, output_fd);
}

void	do_child(int master_fd, int slave_fd, struct termios orig_termios)
{
	if (setsid() == -1)
		handle_error("setsid");
	close(master_fd);
	if (ioctl(slave_fd, TIOCSCTTY, NULL) == -1)
		handle_error("ioctl TIOCSCTTY");
	if (tcsetattr(slave_fd, TCSANOW, &orig_termios) == -1)
		handle_error("tcsetattr");
	dup2(slave_fd, 0);
	dup2(slave_fd, 1);
	dup2(slave_fd, 2);
	if (slave_fd > 2)
		close(slave_fd);
	if (!getenv("SHELL"))
		execlp("/bin/sh", "/bin/sh", (char *) NULL);
	else
		execlp(getenv("SHELL"), getenv("SHELL"), (char *) NULL);
	exit(1);
}

int	do_fork(int master_fd, struct termios orig_termios)
{
	pid_t	child_pid;
	int		slave_fd;

	slave_fd = open(ptsname(master_fd), O_RDWR);
	if (slave_fd == -1)
		handle_error("open slave pty");
	child_pid = fork();
	if (child_pid == -1)
		handle_error("fork");
	if (child_pid == 0)
		do_child(master_fd, slave_fd, orig_termios);
	else
	{
		do_parent(slave_fd, master_fd, orig_termios, child_pid);
		close(slave_fd);
	}
	return (0);
}

int	main(int argc, char **argv)
{
	int				master_fd;
	struct termios	orig_termios;

	if (argc > 2)
	{
		write(2, "Usage: ft_script outputfile\n", 28);
		exit(EXIT_FAILURE);
	}
	if (argc == 2)
		g_filename = argv[1];
	if (tcgetattr(0, &orig_termios) == -1)
		handle_error("tcgetattr");
	master_fd = open("/dev/ptmx", O_RDWR);
	if (master_fd == -1)
		handle_error("posix_openpt");
	if (grantpt(master_fd) == -1 || unlockpt(master_fd) == -1
		|| !ptsname(master_fd))
		handle_error("grantpt/unlockpt || ptsname");
	return (do_fork(master_fd, orig_termios));
}
