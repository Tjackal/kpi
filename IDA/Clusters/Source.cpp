#include <fstream>
#include <windows.h>

#define N 2
#define F(x1, x0, y1, y0) sqrt(pow((x1) - (x0), 2) + pow((y1) - (y0), 2))
int g_words_amount = 0;

typedef struct	word_list
{
	char *str;
	int amount;
	double x;
	double y;
	struct word_list *next;
	struct word_list *left;
	struct word_list *right;
}				wlist;

WIN32_FIND_DATA fd;
char *src = "protocol\\*";

void	add_word(char *str, wlist **tt, int x, int y)
{
	if (!tt)
		return;
	wlist *f = *tt;
	wlist *t;
	while (f)
	{
		if (!strcmp(str, f->str))
		{
			f->amount++;
			f->x += x;
			f->y += y;
			return ;
		}
		if (f->next)
			f = f->next;
		else
			break;
	}
	if (!(t = (wlist*)malloc(sizeof(wlist))))
		return;
	t->str = str;
	t->amount = 1;
	t->x = x;
	t->y = y;
	t->next = NULL;
	t->left = NULL;
	t->right = NULL;
	if (*tt)
		f->next = t;
	else
		*tt = t;
}

void	del_word(wlist **tt)
{
	wlist *t;
	if (!tt || !(*tt))
		return;
	t = (*tt)->next;
	free(*tt);
	*tt = t;
}

void	print_words(wlist *t)
{
	while (t)
	{
		printf("%s   amount = %i, x = %lf, y = %lf\n", t->str, t->amount, t->x, t->y);
		t = t->next;
	}
}

void	calculate_amount_of_words_in_sentences(char *src, int **sentences_stats)
{
	int sentences = 0;
	char buf[1024];
	std::ifstream f(src);
	while (f.getline(buf, 1024))
	{
		int i = -1;
		while (buf[++i])
		{
			if ((buf[i] == '.' || buf[i] == '?' || buf[i] == '!') && (buf[i + 1] != '.' && buf[i + 1] != '?' && buf[i + 1] != '!'))
				sentences++;
		}
		if (buf[i - 1] != '.' && buf[i - 1] != '?' && buf[i - 1] != '!')
			sentences++;
	}
	int words;
	if (!sentences)
	{
		words = 0;
		f.close();
		std::ifstream f(src);
		while (f >> buf)
		{
			int i = -1;
			while (buf[++i])
			{
				if ((buf[i] >= 'a' && buf[i] <= 'z') || (buf[i] >= 'A' && buf[i] <= 'Z'))
				{
					words++;
					break;
				}
			}
		}
		if (words)
		{
			*sentences_stats = (int*)malloc(sizeof(int) * 2);
			(*sentences_stats)[1] = 0;
			(*sentences_stats)[0] = words;
		}
		return;
	}
	if (sentences)
	{
		int j = -1;
		*sentences_stats = (int*)malloc(sizeof(int) * (sentences + 1));
		while (++j <= sentences)
			(*sentences_stats)[j] = 0;
		words = 0;
		j = 0;
		f.close();
		std::ifstream f(src);
		while (f >> buf)
		{
			int i = -1;
			while (buf[++i])
			{
				if ((buf[i] >= 'a' && buf[i] <= 'z') || (buf[i] >= 'A' && buf[i] <= 'Z'))
				{
					(*sentences_stats)[j]++;
					while (buf[++i])
					{
						if (buf[i] == '.' || buf[i] == '?' || buf[i] == '!')
						{
							j++;
							break;
						}
					}
					break;
				}
			}
		}
	}
	f.close();
}

void	exclude_singles(wlist **t)
{
	wlist *a;
	if (!t || !(*t))
		return ;
	/*while (*t && (*t)->amount == 1)
	{
		a = (*t)->next;
		free((*t)->str);
		free(*t);
		*t = a;
	}*/
	wlist *b = *t;
	while (*t && (*t)->next)
	{
		a = (*t)->next;
		if (a->amount == 1)
		{
			(*t)->next = a->next;
			free(a->str);
			free(a);
		}
		else
			*t = (*t)->next;
	}
	*t = b;
}

void	calculate_x_y(wlist *t)
{
	while (t)
	{
		g_words_amount++;
		t->x /= t->amount;
		t->y /= t->amount;
		t = t->next;
	}
}

void	read_from_study_file(wlist **t)
{
	int c = 0;
	int i, j, k;
	int *sentences_stats = NULL;

	char *w;
	char buf[1024];
	char tc[MAX_PATH];

	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(src, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;
	int wn;
	int ii;
	while (1)
	{
		if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0)
		{
			c++;
			strcpy_s(tc, src);
			tc[strlen(tc) - 1] = 0;
			strcat_s(tc, fd.cFileName);

			ii = 0;
			wn = 0;
			calculate_amount_of_words_in_sentences(tc, &sentences_stats);
			printf("%s\n", fd.cFileName);
			std::ifstream f(tc);
			while (f.getline(buf, 1024))
			{
				i = -1;
				while (buf[++i])
				{
					j = 0;
					while ((buf[i + j] >= 'a' && buf[i + j] <= 'z') || (buf[i + j] >= 'A' && buf[i + j] <= 'Z')
						|| (buf[i + j] >= 'à' && buf[i + j] <= 'ÿ') || (buf[i + j] >= 'À' && buf[i + j] <= 'ß'))
						j++;
					if (j)
					{
						w = (char*)malloc(sizeof(char) * (j + 1));
						w[j] = 0;
						k = 0;
						while (k < j)
						{
							if ((buf[i] >= 'A' && buf[i] <= 'Z') || (buf[i] >= 'À' && buf[i] <= 'ß'))
								w[k++] = buf[i++] + 32;
							else
								w[k++] = buf[i++];
						}
						wn++;
						printf("%s  pos = %i  sentnum = %i  words in sentnc = %i\n", w, wn, ii, sentences_stats[ii]);
						if (strlen(w) > 2)
							add_word(w, t, sentences_stats[ii], wn);
						else
							free(w);
						if (wn == sentences_stats[ii])
						{
							wn = 0;
							ii++;
						}
						i--;
					}
				}
			}
			f.close();
			free(sentences_stats);
			printf("\n");
		}
		if (FindNextFile(hFind, &fd) == 0)
			break;
	}
}

void	make_node(wlist **t, wlist *l, wlist *r)
{
	wlist *tt = *t;
	wlist *ttt = NULL;
	
	ttt = (wlist*)malloc(sizeof(wlist));
	ttt->left = l;
	ttt->right = r;
	ttt->x = (l->x + r->x) / 2;
	ttt->y = (l->y + r->y) / 2;
	ttt->str = NULL;
	ttt->next = *t;
	*t = ttt;
	while (*t)
	{
		if ((*t)->next == r)
			(*t)->next = r->next;
		else if ((*t)->next == l)
			(*t)->next = l->next;
		else
			*t = (*t)->next;
	}
	*t = ttt;
}

void	clusters(wlist **t)
{
	double res;
	double value;
	wlist *tt;
	wlist *ttt = NULL;
	wlist *f = NULL;
	wlist *s = NULL;
	
	while (g_words_amount-- > N)
	{
		tt = *t;
		value = 2000000.0;
		while (tt)
		{
			ttt = tt->next;
			while (ttt)
			{
				res = F((tt->x), (ttt->x), (tt->y), (ttt->y));
				printf("( %s  +  %s )  res =   %.2lf\n", tt->str, ttt->str, res);
				if (res < value)
				{
					f = tt;
					s = ttt;
					value = res;
				}
				ttt = ttt->next;
			}
			tt = tt->next;
		}
		make_node(t, f, s);
	}
}

void	prin_tree(wlist *t)
{
	if (!t)
		return ;
	printf("(");
	prin_tree(t->right);
	if (t->str)
		printf("%s", t->str);
	prin_tree(t->left);
	printf(")");
}

void	prin_trees(wlist*t)
{
	while (t)
	{
		prin_tree(t);
		printf("\n");
		t = t->next;
	}
}

int		main(void)
{
	wlist *test = NULL;
	wlist **t = &test;

	read_from_study_file(&test);

	exclude_singles(&test);
	calculate_x_y(test);
	printf("\n\n");

	print_words(test);
	printf("\n\n");
	clusters(&test);
	
	printf("\n");
	prin_trees(test);
	printf("\n");

	printf("\n");
	system("pause");
	return (0);
}