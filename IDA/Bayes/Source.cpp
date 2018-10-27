#include <fstream>
#include <windows.h>

#define N 2// amount of classes

typedef struct	word_list
{
	char *str;
	int amount;
	int amount_in_clss[N];
	double probability[N];
	struct word_list *next;
}				wlist;

typedef struct 	t_classes
{
	int	total_items_amount;
	int	items_amount_in_clss[N];
	double probability_of_clss[N];
}				s_classes;

typedef struct	res_list
{
	char *file_name;
	double probabiliti_of_class[N];
	struct res_list *next;
}				rlist;

WIN32_FIND_DATA fd;
s_classes g_classes;
/*
char *src[N + 1] = {
	"my_case\\autosport\\*",
	"my_case\\hockey\\*",
	"my_case\\basketball\\*",
	"my_case\\real_case\\*"
};*/
char *src[N + 1] = {
	"protocol example\\not_spam\\*",
	"protocol example\\spam\\*",
	"protocol example\\real_case\\*"
};

void	print_words(wlist *t)
{
	int i;
	while (t)
	{
		printf("%s\t\t", t->str);
		i = -1;
		while (++i < N)
			printf("(%i{%lf}) ", t->amount_in_clss[i], t->probability[i]);
		printf("[%i]", t->amount);
		printf("\n");
		t = t->next;
	}
	i = -1;
	while (++i < N)
		printf("## %i (%lf)\n", g_classes.items_amount_in_clss[i], g_classes.probability_of_clss[i]);
	printf("@@@@@ %i\n", g_classes.total_items_amount);
}

void	add_word(char *str, wlist **tt, int clss)
{
	if (!tt)
		return ;
	wlist *f = *tt;
	wlist *t;
	while (f)
	{
		if (!strcmp(str, f->str))
		{
			f->amount++;
			f->amount_in_clss[clss]++;
			free(str);
			return ;
		}
		if (f->next)
			f = f->next;
		else
			break ;
	}
	if (!(t = (wlist*)malloc(sizeof(wlist))))
		return ;
	t->str = str;
	int i = -1;
	t->amount = 1;
	while (++i < N)
		t->amount_in_clss[i] = 0;
	t->amount_in_clss[clss]++;
	t->next = NULL;
	if (*tt)
		f->next = t;
	else
		*tt = t;
}

void	read_from_study_file(wlist **t, int clss)
{
	int c = 0;
	int i, j, k;

	char *w;
	char buf[1024];
	char tc[MAX_PATH];

	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(src[clss], &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return ;
	while (1)
	{
		if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0)
		{
			c++;
			strcpy_s(tc, src[clss]);
			tc[strlen(tc) - 1] = 0;
			strcat_s(tc, fd.cFileName);
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
						if (strlen(w) > 2)
							add_word(w, t, clss);
						else
							free(w);
						i--;
					}
				}
			}
			f.close();
		}
		if (FindNextFile(hFind, &fd) == 0)
			break;
	}
	g_classes.items_amount_in_clss[clss] = c;
	g_classes.total_items_amount += c;
}

void	classes_probability_calc(void)
{
	int i;

	i = -1;
	while (++i < N)
		g_classes.probability_of_clss[i] = (double)g_classes.items_amount_in_clss[i] / (double)g_classes.total_items_amount;
}

void	words_probability_calc(wlist *t)
{
	int i;

	while (t)
	{
		i = -1;
		while (++i < N)
			t->probability[i] = ((double)t->amount_in_clss[i] + g_classes.probability_of_clss[i]) / ((double)t->amount + (double)1.0);
		t = t->next;
	}
}

double	probability_of_word(wlist *t, char *str, int clss)
{
	while (t)
	{
		if (!(strcmp(str, t->str)))
			return (t->probability[clss]);
		t = t->next;
	}
	return (1.0);
}

void	read_from_real_file(wlist *t)
{
	int i, j, k;

	//setlocale(LC_ALL, "Russian");

	char *w;
	char buf[1024];
	double res[N];

	char tc[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(src[N], &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;
	while (1)
	{
		if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0)
		{
			i = -1;
			while (++i < N)
				res[i] = g_classes.probability_of_clss[i];
			printf("%s\t", fd.cFileName);
			strcpy_s(tc, src[N]);
			tc[strlen(tc) - 1] = 0;
			strcat_s(tc, fd.cFileName);
			//setlocale(0, "");
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
						//printf("@@@@@@@@@@ %s\n", w);
						k = -1;
						if (strlen(w) > 2)
						{
							while (++k < N)
								res[k] *= probability_of_word(t, w, k);
						}
						free(w);
						//add_word(w, t, clss);
						i--;
					}
				}
			}
			f.close();
			k = -1;
			double tmp = 0;
			int tmp_clss = 0;
			while (++k < N)
			{
				if (res[k] > tmp)
				{
					tmp = res[k];
					tmp_clss = k;
				}
				//printf("(%.10le) ", res[k]);
			}
			printf("   class [%i]\n", tmp_clss);
		}
		if (FindNextFile(hFind, &fd) == 0)
			break;
	}
}

int		main(void)
{
	setlocale(0, "");
	wlist *word = NULL;
	rlist *res = NULL;
	char buf[2048];
	buf[0] = 0;
	
	int i = -1;

	g_classes.total_items_amount = 0;

	while (++i < N)
		read_from_study_file(&word, i);
	classes_probability_calc();
	words_probability_calc(word);

	read_from_real_file(word);
	
	getchar();
	return (0);
}
