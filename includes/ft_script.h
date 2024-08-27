/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_script.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anggonza <anggonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 11:25:05 by anggonza          #+#    #+#             */
/*   Updated: 2024/08/27 11:36:37 by anggonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_SCRIPT_H
# define FT_SCRIPT_H


# define _GNU_SOURCE
# include "libft.h"
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/ioctl.h>
# include <sys/wait.h>
# include <termios.h>
# include <stdio.h>
# include <time.h>
# include <stdlib.h>

void	handle_error(char *msg);
void	restore_terminal(struct termios *orig_termios);
int		tty_set_raw(struct termios *prevTermios);
void	fd_loop(int master_fd, int output_fd);
void	output_term_info(int output_fd);
void	output_helper(char *f, struct termios term, int fd, int output_fd);

#endif
