#include "ft_script.h"
#include <stdlib.h>
void handle_error(char *msg) {
    ft_putstr_fd(msg, 2);
}

void restore_terminal(struct termios *orig_termios) {
    if (tcsetattr(0, TCSANOW, orig_termios) == -1) {
        handle_error("tcsetattr");
    }
}

int ttySetRaw(struct termios *prevTermios)
{
    struct termios t;
    if (tcgetattr(0, &t) == -1)
        return -1;
    if (prevTermios != NULL)
    *prevTermios = t;
    t.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
    t.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR |
    INPCK | ISTRIP | IXON | PARMRK);
    t.c_oflag &= ~OPOST;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSAFLUSH, &t) == -1)
        return -1;
    return 0;
}

// The Linux Programming Interface book page 1380 Pseudo Terminals

int main(int argc, char **argv) {
    int master_fd, slave_fd;
    pid_t child_pid;
    char buffer[256];
    fd_set inFds; 
    struct termios orig_termios;
    char *slave_name;

    if (argc > 2) {
        const char *usage = "Usage: ft_script outputfile\n";
        write(2, usage, ft_strlen(usage));
        exit(EXIT_FAILURE);
    }
    int output_fd = open((argc > 1) ? argv[1] : "typescript", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1) {
        handle_error("open output file");
    }
    if (tcgetattr(0, &orig_termios) == -1)
        handle_error("tcgetattr");
    master_fd = open("/dev/ptmx", O_RDWR);
    if (master_fd == -1) {
        handle_error("posix_openpt");
    }

    if (grantpt(master_fd) == -1 || unlockpt(master_fd) == -1) {
        handle_error("grantpt/unlockpt");
    }

    slave_name = ptsname(master_fd);
    if (!slave_name) {
        handle_error("ptsname");
    }

    slave_fd = open(slave_name, O_RDWR);
    if (slave_fd == -1) {
        handle_error("open slave pty");
    }

    child_pid = fork();
    if (child_pid == -1) {
        handle_error("fork");
    }

    if (child_pid == 0)
    { 
        if (setsid() == -1) {
            handle_error("setsid");
        }
        close(master_fd);
        if (ioctl(slave_fd, TIOCSCTTY, NULL) == -1) // means that the slave_fd becomes the controlling terminal of the calling process
            handle_error("ioctl TIOCSCTTY");
        if (tcsetattr(slave_fd, TCSANOW, &orig_termios) == -1) // giving the attributes of the terminal to the slave_fd
            handle_error("tcsetattr");
        dup2(slave_fd, 0);
        dup2(slave_fd, 1);
        dup2(slave_fd, 2);
        if (slave_fd > 2)
            close(slave_fd);
        char *shell = getenv("SHELL");
        if (!shell)
            shell = "/bin/bash";
        execlp(shell, shell, (char *)NULL);
        exit(1);
    } 
    else
    {
        ttySetRaw(&orig_termios);
        close(slave_fd);
        for (;;)
        {
            FD_ZERO(&inFds);
            FD_SET(STDIN_FILENO, &inFds);
            FD_SET(master_fd, &inFds);

            if (select(master_fd + 1, &inFds, NULL, NULL, NULL) == -1)
                handle_error("select");
            if (FD_ISSET(STDIN_FILENO, &inFds))
            {
                ssize_t numRead = read(STDIN_FILENO, buffer, sizeof(buffer));
                if (numRead <= 0)
                    break;
                if (write(master_fd, buffer, numRead) != numRead)
                    handle_error("partial/failed write (master_fd)");
            }
            if (FD_ISSET(master_fd, &inFds))
            {
                ssize_t numRead = read(master_fd, buffer, sizeof(buffer));
                if (numRead <= 0)
                    break;
                if (write(STDOUT_FILENO, buffer, numRead) != numRead)
                    handle_error("partial/failed write (STDOUT_FILENO)");
                if (write(output_fd, buffer, numRead) != numRead)
                    handle_error("partial/failed write (output_fd)");
            }
        }
        close(master_fd);
        restore_terminal(&orig_termios);
        int status;
        waitpid(child_pid, &status, 0);
        write(output_fd, "Script done on ", 12);
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S%z", tm);
        write(output_fd, timestamp, ft_strlen(timestamp));
        write(output_fd, " [COMMAND_EXIT_CODE=", 20);
        char *command_exit_code_str;
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

    return 0;
}