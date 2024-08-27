/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anggonza <anggonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 09:46:20 by anggonza          #+#    #+#             */
/*   Updated: 2024/08/27 11:21:11 by anggonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_script.h"

void	handle_error(char *msg)
{
	ft_putstr_fd(msg, 2);
}

void	restore_terminal(struct termios *orig_termios)
{
	if (tcsetattr(0, TCSANOW, orig_termios) == -1)
	{
		handle_error("tcsetattr");
	}
}

int	tty_set_raw(struct termios *prevTermios)
{
	struct termios	t;

	if (tcgetattr(0, &t) == -1)
		return (-1);
	if (prevTermios != NULL)
		*prevTermios = t;
	t.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
	t.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR
			| INPCK | ISTRIP | IXON | PARMRK);
	t.c_oflag &= ~OPOST;
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSAFLUSH, &t) == -1)
		return (-1);
	return (0);
}
