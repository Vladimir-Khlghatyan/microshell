/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vkhlghat <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/09 14:33:20 by vkhlghat          #+#    #+#             */
/*   Updated: 2022/10/09 14:33:21 by vkhlghat         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

typedef struct s_inp
{
	char			**args;
	int				start;
	int				end;
	int				index;
	struct s_inp	*prev;
	struct s_inp	*next;
}					t_inp;

//=============================================================================
// 1
//=============================================================================
int	ft_strlen(char *str)
{
	int	i = 0;

	if (!str)
		return (0);
	while (str[i])
		i++;
	return (i);
}

//=============================================================================
// 2
//=============================================================================
int	ft_arrlen(char **arr)
{
	int	i = 0;

	if (!arr)
		return (0);
	while (arr[i])
		i++;
	return (i);
}

//=============================================================================
// 3
//=============================================================================
void	ft_error(char *s1, char *s2, int exit_flag)
{
	if (s1)
		write(2, s1, ft_strlen(s1));
	if (s2)
		write(2, s2, ft_strlen(s2));
	write(2, "\n", 1);
	if (exit_flag)
		exit(1);
}

//=============================================================================
// 4
//=============================================================================
char	**ft_add_str_to_arr(char **arr, char *str)
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

//=============================================================================
// 5
//=============================================================================
t_inp	*ft_newnode(void)
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

//=============================================================================
// 6
//=============================================================================
void	ft_addnode(t_inp **head, t_inp *new)
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

//=============================================================================
// 7
//=============================================================================
t_inp	*ft_create_list(char **av)
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

//=============================================================================
// 8
//=============================================================================
void	ft_set_flags(t_inp *inp)
{
	t_inp	*tmp = inp;
	int		i = 0;

	while (tmp)
	{
		if ((!tmp->prev || !strcmp(tmp->prev->args[0], ";")) && \
			(tmp->next && !strcmp(tmp->next->args[0], "|")))
		{
			tmp->start = 1;
			tmp->index = i++;
		}
		if ((tmp->prev && !strcmp(tmp->prev->args[0], "|")) && \
			(!tmp->next || !strcmp(tmp->next->args[0], ";")))
		{
			tmp->end = 1;
			tmp->index = i;
			i = 0;
		}
		if (tmp->prev && tmp->next && !strcmp(tmp->prev->args[0], "|") && \
			!strcmp(tmp->next->args[0], "|"))
			tmp->index = i++;
		tmp = tmp->next;
	}
}

//=============================================================================
// 9
//=============================================================================
void	ft_close_pipe(int	*fds)
{
	if (close(fds[0]) == -1)
		ft_error("error: fatal", NULL, 1);
	if (close(fds[1]) == -1)
		ft_error("error: fatal", NULL, 1);
}

//=============================================================================
// 10
//=============================================================================
void	ft_wait_for_children(t_inp *node)
{
	t_inp	*tmp = node;

	if (node->end != 1)
		return ;
	while (tmp->start != 1)
	{
		waitpid(0, NULL, 0);
		tmp = tmp->prev;
	}
	waitpid(0, NULL, 0);
}

//=============================================================================
// 11
//=============================================================================
void	ft_create_proc(t_inp *node, int *fds_front, int	*fds_back)
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
				if (dup2(fds_back[1], 1) == -1)
					ft_error("error: fatal", NULL, 1);
				ft_close_pipe(fds_back);
			}
			else if (node->end == 1)
			{
				if (dup2(fds_front[0], 0) == -1)
					ft_error("error: fatal", NULL, 1);
				ft_close_pipe(fds_front);
			}
			else
			{
				if (dup2(fds_front[0], 0) == -1)
					ft_error("error: fatal", NULL, 1);
				if (dup2(fds_back[1], 1) == -1)
					ft_error("error: fatal", NULL, 1);
				ft_close_pipe(fds_front);
				ft_close_pipe(fds_back);
			}
		}
		if (execve(node->args[0], node->args, NULL) == -1)
			ft_error("error: cannot execute ", node->args[0], 1);
	}
	else if (node->index < 0)
		waitpid(pid, NULL, 0);
	else if (node->start != 1 && node->end != 1)
		ft_close_pipe(fds_front);
	else if (node->end == 1)
	{
		ft_close_pipe(fds_front);
		ft_wait_for_children(node);
	}
}

//=============================================================================
// 12
//=============================================================================
void	ft_call_proc(t_inp *inp)
{
	int		fds_front[2];
	int		fds_back[2];
	t_inp	*tmp = inp;

	while (tmp)
	{
		if (!strcmp(tmp->args[0], ";") || !strcmp(tmp->args[0], "|"))
			tmp = tmp->next;
		else if (!strcmp(tmp->args[0], "cd"))
		{
			if (ft_arrlen(tmp->args) != 2)
				ft_error("error: cd: bad arguments", NULL, 0);
			else if (chdir(tmp->args[1]) == -1)
				ft_error("error: cd: cannot change directory to ", \
					tmp->args[1], 0);
			tmp = tmp->next;
		}
		else
		{
			if (tmp->start == 1)
			{
				if (pipe(fds_back) == -1)
					ft_error("error: fatal", NULL, 1);
			}
			else if (tmp->start != 1 && tmp->index >= 0)
			{
				if (pipe(fds_front) == -1)
					ft_error("error: fatal", NULL, 1);
				if (dup2(fds_back[0], fds_front[0]) == -1)
					ft_error("error: fatal", NULL, 1);
				ft_close_pipe(fds_back);
				if (tmp->end != 1)
				{
					if (pipe(fds_back) == -1)
						ft_error("error: fatal", NULL, 1);
				}
			}
			ft_create_proc(tmp, fds_front, fds_back);
			tmp = tmp->next;
		}
	}
}

//=============================================================================
// 13
//=============================================================================
void	ft_free_list(t_inp *inp)
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

//=============================================================================
// 14
//=============================================================================
int	main(int ac, char **av)
{
	t_inp	*inp;

	if (ac < 2)
		return (0);
	inp = ft_create_list(av);
	ft_set_flags(inp);
	ft_call_proc(inp);
	ft_free_list(inp);
	return (0);
}

//=============================================================================