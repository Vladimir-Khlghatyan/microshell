#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

typedef struct		s_inp
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

void ft_error(char *s1, char *s2, int exit_flag) //3//
{
	if (s1)
		write(2, s1, ft_strlen(s1));
	if (s2)
		write(2, s2, ft_strlen(s2));
	write(2, "\n", 1);
	if (exit_flag)
		exit(1);
}

char	**ft_add_str_to_arr(char **arr, char *str) //4//
{
	int		i = -1;
	char	**new_arr;

	if (!arr || !str)
		return (arr);
	new_arr = (char **)malloc(sizeof(char *) *(ft_arrlen(arr) + 2));
	if (!new_arr)
		ft_error("error: fatal", NULL, 1);
	while (arr[++i])
		new_arr[i] = arr[i];
	new_arr[i] = str;
	new_arr[i + 1] = NULL;
	free(arr);
	return (new_arr);
}

t_inp	*ft_newnode(void) //5//
{
	t_inp	*node;

	node = (t_inp *)malloc(sizeof(t_inp));
	if (!node)
		ft_error("error: fatal", NULL, 1);
	node->args = (char **)malloc(sizeof(char *));
	if (!node->args)
		ft_error("error: fatal", NULL, 1);
	node->args[0] = NULL;
	node->start = -1;
	node->end = -1;
	node->index = -1;
	node->prev = NULL;
	node->next = NULL;
	return (node);
}

void	ft_addnode(t_inp **head, t_inp *new) //6//
{
	t_inp	*last_node = *head;

	if (!*head)
		*head = new;
	else
	{
		while (last_node->next)
			last_node = last_node->next;
		last_node->next = new;
		new->prev = last_node;
	}
}

t_inp	*ft_create_list(char **av) //7//
{
	t_inp	*inp = NULL;
	t_inp	*node;
	int		i = 1;

	while (av[i])
	{
		node = ft_newnode();
		if (!strcmp(av[i], ";") || !strcmp(av[i], "|"))
		{
			node->args = ft_add_str_to_arr(node->args, av[i]);
			i++;
		}
		else
		{
			while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
			{
				node->args = ft_add_str_to_arr(node->args, av[i]);
				i++;
			}
		}
		ft_addnode(&inp, node);
	}
	return (inp);
}

void	ft_set_flags(t_inp *inp) //8//
{
	t_inp	*tmp = inp;
	int		i = 0;

	while (tmp)
	{
		if ((!tmp->prev || !strcmp(tmp->prev->args[0], ";")) && (tmp->next && !strcmp(tmp->next->args[0], "|")))
		{
			tmp->start = 1;
			tmp->index = i++;
		}
		if ((tmp->prev && !strcmp(tmp->prev->args[0], "|")) && (!tmp->next || !strcmp(tmp->next->args[0], ";")))
		{
			tmp->end = 1;
			tmp->index = i;
			i = 0;
		}
		if (tmp->prev && tmp->next && !strcmp(tmp->prev->args[0], "|") && !strcmp(tmp->next->args[0], "|"))
			tmp->index = i++;
		tmp = tmp->next;
	}
}

int	*ft_create_pipes(t_inp *node) //9//
{
	int		*fds = NULL;
	t_inp	*tmp = node;
	int		pipe_cnt = 0;
	int		i = -1;

	if (node->start != 1)
		return (NULL);
	while (tmp->end != 1)
	{
		pipe_cnt++;
		tmp = tmp->prev;
	}
	fds = (int *)malloc(sizeof(int) * (pipe_cnt * 2 + 1));
	if (!fds)
		ft_error("error: fatal", NULL, 1);
	while (++i < pipe_cnt)
		if (pipe(&fds[2 * i]) == -1)
			ft_error("error: fatal", NULL, 1);
	fds[2 * i] = -1;
	return (fds);
}

void	ft_close_pipes(int	*fds) //10//
{
	int	i = -1;

	if (!fds)
		return ;
	while (fds[++i])
		if (close(fds[i]) == -1)
			ft_error("error: fatal", NULL, 1);
	free(fds);
}

void	ft_wait_for_children(t_inp *node) //11//
{
	t_inp	*tmp = node;

	if (node->end != 1)
		return ;
	while (tmp && tmp->index >= 0)
	{
		waitpid(0, NULL, 0);
		tmp = tmp->prev;
	}
}

void	ft_create_proc(t_inp *node, int *fds) //12//
{
	pid_t	pid = fork();

	if (pid == -1)
		ft_error("error: fatal", NULL, 1);
	if (pid == 0)
	{
		if (node->index >= 0)
		{
			if (node->start == 1)
			{
				if (dup2(fds[2 * node->index + 1], 1) == -1)
					ft_error("error: fatal", NULL, 1);
			}
			else if (node->end == 1)
			{
				if (dup2(fds[2 * node->index - 2], 0) == -1)
					ft_error("error: fatal", NULL, 1);
			}
			else
			{
				if (dup2(fds[2 * node->index - 2], 0) == -1)
					ft_error("error: fatal", NULL, 1);
				if (dup2(fds[2 * node->index + 1], 1) == -1)
					ft_error("error: fatal", NULL, 1);
			}
		}
		ft_close_pipes(fds);
		if (execve(node->args[0], node->args, NULL) == -1)
			ft_error("error: cannot execute ", node->args[0], 1);
	}
	else if (node->index < 0)
		waitpid(pid, NULL, 0);
	else if (node->end == 1)
	{
		ft_close_pipes(fds);
		ft_wait_for_children(node);
	}
}

void	ft_call_proc(t_inp *inp) //13//
{
	int		*fds = NULL;
	t_inp	*tmp = inp;

	while (tmp)
	{
		if (!strcmp(tmp->args[0], ";") || !strcmp(tmp->args[0], "|"))
			tmp = tmp->next;
		else if (!strcmp(tmp->args[0], "cd"))
		{
			if (ft_arrlen(tmp->args) != 2)
				ft_error("error: cd: bad arguments", NULL, 0);
			if (chdir(tmp->args[1]) == -1)
				ft_error("error: cd: cannot change directory to ", tmp->args[1], 0);
			tmp = tmp->next;
		}
		else
		{
			if (tmp->start == 1)
				fds = ft_create_pipes(tmp);
			ft_create_proc(tmp, fds);
			if (tmp->end == 1)
				fds = NULL;
			tmp = tmp->next;
		}
	}
}

void	ft_free_list(t_inp *inp) //14//
{
	t_inp	*tmp;

	while (inp)
	{
		free(inp->args);
		tmp = inp->next;
		free(inp);
		inp = tmp;
	}
}

int	main(int ac, char **av) //15//
{
	if (ac < 2)
		return (0);
	t_inp	*inp = ft_create_list(av);
	ft_set_flags(inp);
	ft_call_proc(inp);
	ft_free_list(inp);
	return (0);
}
