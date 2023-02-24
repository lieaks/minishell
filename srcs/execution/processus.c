/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   processus.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dly <dly@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/21 19:17:06 by dly               #+#    #+#             */
/*   Updated: 2023/02/24 21:21:33 by dly              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/minishell.h"

static void	child(t_proc *tmp, t_proc *proc, t_list *env)
{
	if (proc->next_pipeline == PIPE)
		assign_pipe_cmd(proc);
	if (isbuiltin(proc->path) && (!tmp->next || tmp->next_pipeline == AND
			|| tmp->next_pipeline == OR))
	{
		proc->exit_code = exec_builtin(proc, env);
		g_exit_code = proc->exit_code;
		return ;
	}
	proc->pid = fork();
	if (!proc->pid)
	{
		if (double_dup2(proc->fd_in, proc->fd_out) == -1)
			exit(1);
		if (isbuiltin(proc->path))
		{
			proc->exit_code = exec_builtin(proc, env);
			exit(proc->exit_code);
		}
		close_pipe(tmp);
		if (!access(proc->path, 0))
			execve(proc->path, ft_lst_to_tab(proc->args), ft_lst_to_tab(env));
		exit(-1);
	}
}

static void	wait_loop(t_proc *tmp, t_proc *proc, t_list *env)
{
	close_pipe(tmp);
	while (tmp)
	{
		if (tmp->type == COMMAND && tmp->path)
			waitpid(tmp->pid, &tmp->exit_code, 0);
		g_exit_code = tmp->exit_code;
		if (tmp->next_pipeline == AND || tmp->next_pipeline == OR)
			break ;
		tmp = tmp->next;
	}
	if (proc)
		recursive_and_or(proc, env, 1);
}

int	cmd_not_found(t_proc *proc)
{
	if (!proc->path && proc->type == COMMAND)
	{
		proc->exit_code = 127;
		g_exit_code = 127;
		return (1);
	}
	return (0);
}

int	process(t_proc *proc, t_list *env)
{
	t_proc	*tmp;

	tmp = proc;
	while (proc)
	{
		parse_line_to_proc(proc->line, proc, env);
		if (!cmd_not_found(proc) && (proc->path || proc->type == SUBSHELL))
		{
			if (proc->type == COMMAND && proc->fd_in != -1)
				child(tmp, proc, env);
			if (proc->type == SUBSHELL)
			{
				assign_pipe_subshell(proc->procs, proc, env);
				process(proc->procs, env);
				if (proc->next_pipeline == AND || proc->next_pipeline == OR)
					return (recursive_and_or(proc, env, 0), 0);
			}
		}
		if (proc->next_pipeline == AND || proc->next_pipeline == OR)
			break ;
		proc = proc->next;
	}
	wait_loop(tmp, proc, env);
	return (0);
}
