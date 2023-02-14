/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_redirections.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mirsella <mirsella@protonmail.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/10 15:38:38 by mirsella          #+#    #+#             */
/*   Updated: 2023/02/12 17:59:17 by mirsella         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/minishell.h"
#include <string.h>

static int	ismeta(char c)
{
	return (c == '>' || c == '<' || c == '|' || c == '&');
}

char	*get_word_expand(t_data *data, char *line, int *ret)
{
	int		stop;
	char	*tmp;

	if (*(line + ft_skip_spaces(line)) == 0)
		return (*ret = 1,
			(char [2]){print_syntax_error("empty redirect", 0), 0});
	stop = 0;
	while (line[stop] && !ismeta(line[stop]) && !ft_isspace(line[stop]))
	{
		if (line[stop] == '*' || line[stop] == '(')
			return (*ret = 1, (char [2]){print_syntax_error(
					"ambiguous redirect ", line[stop]), 0});
		if (line[stop] == '\'' || line[stop] == '"')
			stop += skip_quotes(line);
		else if (line[stop] == '(')
			stop += skip_parenthesis(line);
		else
			stop++;
	}
	tmp = ft_substr(line, 0, stop);
	line = expand_everything(data->env, tmp);
	free(tmp);
	if (!line)
		return (*ret = 1, perror("malloc"), NULL);
	return (*ret = 0, line);
}

int	remove_redirections(char *line)
{
	int	i;

	i = 0;
	i += ft_skip_spaces(line);
	while (line[i] == '>' || line[i] == '<')
		line[i++] = ' ';
	i += ft_skip_spaces(line + i);
	while (line[i] && !ismeta(line[i]) && !ft_isspace(line[i]))
	{
		if (line[i] == '\'' || line[i] == '"')
		{
			ft_memset(line + i, ' ', skip_quotes(line + i));
			i += skip_quotes(line + i);
		}
		else if (line[i] == '(')
		{
			ft_memset(line + i, ' ', skip_parenthesis(line + i));
			i += skip_parenthesis(line + i);
		}
		else
			line[i++] = ' ';
	}
	return (i);
}

int	parse_redirections(t_data *data, char *line, t_proc *proc)
{
	int	ret;

	ret = 0;
	while (*line)
	{
		line += ft_skip_spaces(line);
		if (!*line)
			break ;
		if (*line == '>')
		{
			ret = output_redirection(data, line + 1, proc);
			line += remove_redirections(line);
		}
		// still need to parse input redirections here
		else
			line++;
		if (ret)
			return (ret);
	}
	return (0);
}