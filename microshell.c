#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#ifdef TEST_SH
# define TEST		1
#else
# define TEST		0
#endif

typedef struct 		s_inp
{
	char			**args;
	int				start;
	int				end;
	int				index;
	struct s_inp	*prev;
	struct s_inp	*next;
}					t_inp;

int	ft_strlen(char *str) //1//
{
	int	i = 0;

	if (!str)
		return (0);
	while (str[i])
		i++;
	return (i);
}

int	ft_arrlen(char **arr) //2//
{
	int	i = 0;

	if (!arr)
		return (0);
	while (arr[i])
		i++;
	return (i);
}

void	ft_error(char *s1, char *s2) //3//
{
	if (s1)
		write(2, s1, ft_strlen(s1));
	if (s2)
		write(2, s2, ft_strlen(s2));
	write(2, "\n", 1);
	exit(1);
}

char	**ft_add_str_to_arr(char **arr, char *str) //4//
{
	int		len = ft_arrlen(arr);
	int		i = -1;
	char	**new_arr;

	new_arr = (char **)malloc(sizeof(char *) * (len + 2));
	if (!new_arr)
		ft_error("error: fatal", NULL);
	while (++i < len)
		new_arr[i] = arr[i];
	new_arr[i] = str;
	new_arr[i + 1] = NULL;
	free(arr);
	return (new_arr);
}

int	ft_node_cnt(char **av) //5//
{
	int	cnt = 0;
	int	i = 1;

	if (ft_arrlen(av) < 2)
		return (0);
	while (av[i])
	{
		if (!strcmp(av[i], ";") || !strcmp(av[i], "|"))
		{
			cnt++;
			i++;
		}
		else
		{
			while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
				i++;
			cnt++;
		}
	}
	return (cnt);
}

int	ft_pipe_cnt(char **av) //6//
{
	int	cnt = 0;
	int	i = 0;

	if (ft_arrlen(av) < 2)
		return (0);
	while (av[++i])
		if (!strcmp(av[i], "|"))
			cnt++;
	return (cnt);
}

int	ft_proc_cnt(char **av) //7//
{
	int	cnt = 0;
	int	i = 1;

	if (ft_arrlen(av) < 2)
		return (0);
	while (av[i])
	{
		if (!strcmp(av[i], ";") || !strcmp(av[i], "|"))
			i++;
		else
		{
			while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
				i++;
			cnt++;
		}
	}
	return (cnt);
}

t_inp	*ft_newnode(void) //8//
{
	t_inp	*node;

	node = (t_inp *)malloc(sizeof(t_inp));
	if (!node)
		ft_error("error: fatal", NULL);
	node->args = (char **)malloc(sizeof(char *));
	if (!(node->args))
		ft_error("error: fatal", NULL);
	node->args[0] = NULL;
	node->start = -1;
	node->end = -1;
	node->index = -1;
	node->prev = NULL;
	node->next = NULL;
	return (node);
}

t_inp	*ft_lastnode(t_inp *head) //9//
{
	t_inp *tmp = head;

	while (tmp)
	{
		if (!(tmp->next))
			return (tmp);
		tmp = tmp->next;
	}
	return (NULL);
}

void	ft_addnode(t_inp **head, t_inp *new) //10//
{
	t_inp	*last_node;

	if (!(*head))
	{
		*head = new;
		return ;
	}
	last_node = ft_lastnode(*head);
	last_node->next = new;
	new->prev = last_node;
}

t_inp	*ft_create_list(int node_cnt) //11//
{
	t_inp	*head = NULL;
	t_inp	*node;

	while (node_cnt--)
	{
		node = ft_newnode();
		ft_addnode(&head, node);
	}
	return (head);
}

void	ft_fill_list(t_inp *inp, char **av) //12//
{
	t_inp	*tmp = inp;
	int		i = 1;

	while (av[i])
	{
		if (!strcmp(av[i], ";") || !strcmp(av[i], "|"))
		{
			tmp->args = ft_add_str_to_arr(tmp->args, av[i]);
			i++;
		}
		else
		{
			while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
			{
				tmp->args = ft_add_str_to_arr(tmp->args, av[i]);
				i++;
			}
		}
		tmp = tmp->next;
	}
}

void	ft_set_flags(t_inp *inp) //13//
{
	t_inp	*tmp = inp;

	while (tmp)
	{
		if ((!tmp->prev || !strcmp(tmp->prev->args[0], ";")) && (tmp->next && !strcmp(tmp->next->args[0], "|")))
			tmp->start = 1;
		if ((tmp->prev && !strcmp(tmp->prev->args[0], "|")) && (!tmp->next || !strcmp(tmp->next->args[0], ";")))
			tmp->end = 1;
		tmp = tmp->next;
	}
}

void	ft_set_indexes(t_inp *inp) //14//
{
	t_inp	*tmp = inp;
	int		i = 0;

	while (tmp)
	{
		if (tmp->start == 1 || tmp->end == 1)
		{
			tmp->index = i;
			i++;
		}
		else if (tmp->prev && tmp->next && !strcmp(tmp->prev->args[0], "|") && !strcmp(tmp->next->args[0], "|"))
		{
			tmp->index = i;
			i++;
		}
		tmp = tmp->next;
	}	
}

int	*ft_create_pipes(int pipe_cnt) //15//
{
	int	*fds = NULL;
	int	i = -1;

	if (pipe_cnt < 1)
		return (NULL);
	fds = (int *)malloc(sizeof(int) * (pipe_cnt * 2));
	if (!fds)
		ft_error("error: fatal", NULL);
	while (++i < pipe_cnt)
		if (pipe(&fds[2 * i]) == -1)
			ft_error("error: fatal", NULL);
	return (fds);
}

void	ft_close_pipes(int *fds, int pipe_cnt) //16//
{
	int	i = -1;

	if (pipe_cnt < 1)
		return ;
	while (++i < (pipe_cnt * 2))
		close(fds[i]);
	if (fds)
		free(fds);
}

void	ft_create_proc(t_inp *node, int *fds, int pipe_cnt) //17//
{
	pid_t	pid = fork();

	if (pid == -1)
		ft_error("error: fatal", NULL);
	if (pid == 0)
	{
		if (node->index >= 0)
		{
			if (node->start == 1)
				dup2(fds[2 * node->index + 1], 1);
			else if (node->end == 1)
				dup2(fds[2 * node->index - 2], 0);
			else
			{
				dup2(fds[2 * node->index + 1], 1);
				dup2(fds[2 * node->index - 2], 0);
			}
		}
		ft_close_pipes(fds, pipe_cnt);
		if (execve(node->args[0], node->args, NULL) == -1)
			ft_error("error: cannot execute ", node->args[0]);
	}
	else if (node->index < 0)
		waitpid(pid, NULL, 0);
}

void	ft_call_proc(t_inp *inp, int *fds, int pipe_cnt) //18//
{
	t_inp	*tmp = inp;

	while (tmp)
	{
		if (!strcmp(tmp->args[0], ";") || !strcmp(tmp->args[0], "|"))
		{
			tmp = tmp->next;
			continue ;
		}
		if (!strcmp(tmp->args[0], "cd"))
		{
			if (ft_arrlen(tmp->args) != 2)
				ft_error("error: cd: bad arguments", NULL);
			else if (chdir(tmp->args[1]) == -1)
				ft_error("error: cd: cannot change directory to ", tmp->args[1]);
		}
		else
			ft_create_proc(tmp, fds, pipe_cnt);
		tmp = tmp->next;
	}
}

void	ft_wait_for_children(int proc_cnt) //19//
{
	int	i = -1;

	while (++i < proc_cnt)
		waitpid(0, NULL, 0);
}

int	main(int ac, char **av) //20//
{
	int		node_cnt = ft_node_cnt(av);
	int		pipe_cnt = ft_pipe_cnt(av);
	int		proc_cnt = ft_proc_cnt(av);
	int		*fds = NULL;
	t_inp	*inp = NULL;

	if (ac < 2)
		return (0);
	inp = ft_create_list(node_cnt);
	ft_fill_list(inp, av);
	ft_set_flags(inp);
	ft_set_indexes(inp);
	fds = ft_create_pipes(pipe_cnt);
	ft_call_proc(inp, fds, pipe_cnt);
	ft_close_pipes(fds, pipe_cnt);
	ft_wait_for_children(proc_cnt);
	if (TEST)
		while (1);
	return (0);
}

