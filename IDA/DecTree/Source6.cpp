#include <fstream>

char learn_src[_MAX_PATH] = "data\\contact-lenses.arff";
char test_src[_MAX_PATH] = "data\\test.arff";

int attributes_amount;
int instances_amount;
int test_instances_amount;

typedef struct		s_attr
{
	char *name;
	int vars_amount;
	char **var_name;
}					attr;

typedef struct		s_rtree
{
	int attr_of_node;
	int answer;
	int *attributes_available_for_node;
	int n_of_attributes_available_for_node;
	int *instances_available_for_node;
	int n_of_instances_available_for_node;
	s_rtree *leaf;
}					rtree;

attr *attribute;
int **instance;
int **test_instance;

//////////////////////////////////////////////////Reading From .arff////////////////////////////////////////////////

void	*e_malloc(unsigned int s)
{
	void *t;

	if (!(t = malloc(s)))
	{
		printf("memory allocation error!\nprogram will be terminated\n\n");
		system("pause");
		exit(1);
	}
	return (t);
}

int		my_strcmp(const char *s1, const char *s2)
{
	if ((!(*s1) && *s2) || (*s1 && !(*s2)))
		return (0);
	while (*s1 && *s2)
	{
		if (*s1++ != *s2++)
			return (0);
	}
	return (1);
}

void	count_attributes()
{
	attributes_amount = 0;
	char buf[2048];
	std::ifstream f(learn_src);
	while (f.getline(buf, 2048))
	{
		if (my_strcmp(buf, "@attribute") || my_strcmp(buf, "@ATTRIBUTE"))
			attributes_amount++;
	}
	//name_of_var = (char***)e_malloc(sizeof(char**) * attributes_amount);
	f.close();
}

int		count_variants_amount(char *s)
{
	int n = 0;
	while (*s)
	{
		if (*s == '{')
		{
			while (*s != '}')
			{
				if (*s == ',')
					n++;
				s++;
			}
			break;
		}
		s++;
	}
	return (n + 1);
}

char	*get_attribute_name(char *s)
{
	int l = 0;
	while (*s && *s != ' ')
		s++;
	while (*s && *s == ' ')
		s++;
	while (*s && *s != ' ' && *s != '\t')
	{
		l++;
		s++;
	}
	char *t = (char*)e_malloc(sizeof(char) * (l + 1));
	t[l] = '\0';
	while (--l >= 0)
		t[l] = *(--s);
	return (t);
}

void	get_vars(int i, char *s)
{
	attribute[i].vars_amount = count_variants_amount(s);
	//name_of_var[i] = (char**)e_malloc(sizeof(char*) * attribute[i].vars_amount);
	attribute[i].var_name = (char**)e_malloc(sizeof(char*) * attribute[i].vars_amount);
	int j = 0;
	int v = -1;
	while (s[j])
	{
		if (s[j] == '{')
		{
			while (++v < attribute[i].vars_amount)
			{
				int vl = 0;
				while (s[++j] && s[j] != ',' && s[j] != '}')
				{
					if (s[j] != ' ')
						vl++;
				}
				//printf("%i  ", vl);
				attribute[i].var_name[v] = (char*)e_malloc(sizeof(char) * (vl + 1));
				attribute[i].var_name[v][vl] = '\0';
				int k = -1;
				while (++k < vl)
				{
					attribute[i].var_name[v][k] = s[j - vl + k];
				}
				//printf("%s    ", attribute[i].var_name[v]);
			}
			//printf("\n");
		}
		j++;
	}
}

void	get_attributes()
{
	count_attributes();
	attribute = (attr*)e_malloc(sizeof(attr) * attributes_amount);
	int i = 0;
	int n = -1;
	char buf[2048];
	std::ifstream f(learn_src);
	while (f.getline(buf, 2048))
	{
		if (my_strcmp(buf, "@attribute") || my_strcmp(buf, "@ATTRIBUTE"))
		{
			attribute[i].name = get_attribute_name(buf);
			get_vars(i, buf);
			int j = -1;
			i++;
		}
	}
	f.close();
}

void	print_attr()
{
	int i = -1;
	while (++i < attributes_amount)
	{
		printf("%s { ", attribute[i].name);
		int j = -1;
		while (++j < attribute[i].vars_amount)
		{
			//printf("%s(%i) ", attribute[i].var_name[j], attribute[i].var_id[j]);
		}
		printf("}\n");
	}
}



int		count_instances(char *src)
{
	int instances_amount = 0;
	char buf[2048];
	std::ifstream f(src);
	while (f.getline(buf, 2048))
	{
		if (my_strcmp(buf, "@data") || my_strcmp(buf, "@DATA"))
		{
			while (f.getline(buf, 2048))
			{
				if (*buf != ' ' && *buf != '\t' && *buf != '%' && *buf != '@' && *buf != '\0')
					instances_amount++;
			}
			break;
		}
	}
	//printf("%i instances\n", instances_amount);
	f.close();
	return (instances_amount);
}

int		var_cmp(char *s, char *attr)
{
	int k = -1;
	while (s[++k] && attr[k] && s[k] != ',' && s[k] != '}' && s[k] != '\n')
	{
		if (s[k] != attr[k])
			return (0);
	}
	if (!attr[k] && (s[k] != ',' || s[k] != '}' || s[k] != '\n'))
		return (1);
	return (0);
}

int		detect_var(int j, char *s)
{
	int i = -1;
	while (++i < attribute[j].vars_amount)
	{
		if (var_cmp(s, attribute[j].var_name[i]))
			return (i);
		else
			continue;
	}
}

int		get_instances(char *src, int ***t_inst)
{
	int inst_amount = count_instances(src);
	*t_inst = (int**)e_malloc(sizeof(int*) * inst_amount);
	int **inst = *t_inst;
	int i = -1;
	char buf[2048];
	std::ifstream f(src);
	while (f.getline(buf, 2048))
	{
		if (my_strcmp(buf, "@data") || my_strcmp(buf, "@DATA"))
		{
			while (f.getline(buf, 2048))
			{
				int k = 0;
				if (*buf != ' ' && *buf != '\t' && *buf != '%' && *buf != '@' && *buf != '\0')
				{
					inst[++i] = (int*)e_malloc(sizeof(int) * attributes_amount);
					int j = -1;
					while (++j < attributes_amount)
					{
						inst[i][j] = detect_var(j, &(buf[k]));
						while (buf[k] && buf[k] != ',')
							k++;
						k++;
					}
				}
			}
			break;
		}
	}
	f.close();
	return inst_amount;
}

void	print_instnc(int **inst,int  inst_am)
{
	int i = -1;
	while (++i < inst_am)
	{
		int j = -1;
		while (++j < attributes_amount)
			printf("%i  ", inst[i][j]);
		printf("\n");
	}
}

/////////////////////////////////////////////////////////////////////Tree////////////////////////////////////////////////////////////////////

#define LOG2(n) log10(n) / log10(2)

double	cnt_ent(int *inst, int inst_am)
{
	int n = attribute[attributes_amount - 1].vars_amount;
	double *var_prob = (double*)e_malloc(sizeof(double) * n);
	for (size_t i = 0; i < n; i++)
		var_prob[i] = 0.0;
	for (size_t i = 0; i < inst_am; i++)
		var_prob[instance[inst[i]][attributes_amount - 1]] += 1.0;
	double res = 0.0;
	for (size_t i = 0; i < n; i++)
	{
		double t = var_prob[i] / (double)inst_am;
		if (t)
			res += -t * LOG2(t);
	}
	free(var_prob);
	return res;
}

double	cnt_ent_by_attr(int *inst, int inst_am, int attrN)
{
	int n = attribute[attrN].vars_amount;
	double *tmp = (double*)e_malloc(sizeof(double) * n);
	double **tmpp = (double**)e_malloc(sizeof(double*) * n);
	for (size_t i = 0; i < n; i++)
	{
		tmp[i] = 0.0;
		tmpp[i] = (double*)e_malloc(sizeof(double) * attribute[attributes_amount - 1].vars_amount);
		for (size_t j = 0; j < attribute[attributes_amount - 1].vars_amount; j++)
			tmpp[i][j] = 0.0;
	}
	for (size_t i = 0; i < inst_am; i++)
	{
		tmpp[instance[inst[i]][attrN]][instance[inst[i]][attributes_amount - 1]] += 1.0;
		tmp[instance[inst[i]][attrN]] += 1.0;
	}
	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < attribute[attributes_amount - 1].vars_amount; j++)
			tmpp[i][j] /= tmp[i];
	double res = 0.0;
	for (size_t i = 0; i < n; i++)
	{
		double t = 0.0;
		for (size_t j = 0; j < attribute[attributes_amount - 1].vars_amount; j++)
		{
			double tt = tmpp[i][j];
			if (tt)
				t += -tt * (LOG2(tt));
		}
		res += tmp[i] / (double)inst_am * t;
	}
	return res;
}

int		get_i_of_attribute_to_divide_by(rtree *nod)
{
	double ent = cnt_ent(nod->instances_available_for_node, nod->n_of_instances_available_for_node);
	if (ent == 0.0)
		return -1;
	double max_gain = 0.0;
	int i_of_attribute_to_divide_by;
	for (size_t i = 0; i < nod->n_of_attributes_available_for_node - 1; i++)
	{
		double t = cnt_ent_by_attr(nod->instances_available_for_node, nod->n_of_instances_available_for_node, nod->attributes_available_for_node[i]);
		if (ent - t > max_gain)
		{
			max_gain = ent - t;
			i_of_attribute_to_divide_by = i;
		}
	}
	if (max_gain == 0.0)
		return -1;
	return nod->attributes_available_for_node[i_of_attribute_to_divide_by];
}

void	gen_new_attr_arr(rtree *o_nod, rtree *n_nod, int attrN)
{
	n_nod->n_of_attributes_available_for_node = o_nod->n_of_attributes_available_for_node - 1;
	n_nod->attributes_available_for_node = (int*)e_malloc(sizeof(int) * n_nod->n_of_attributes_available_for_node);
	for (size_t i = 0, j = 0; i < o_nod->n_of_attributes_available_for_node; i++)
	{
		if (o_nod->attributes_available_for_node[i] != attrN)
			n_nod->attributes_available_for_node[j++] = o_nod->attributes_available_for_node[i];
	}
}

void	gen_new_inst_arr(rtree *o_nod, rtree *n_nod, int attrN, int i_of_var)
{
	int n = 0;
	for (size_t i = 0; i < o_nod->n_of_instances_available_for_node; i++)
	{
		if (instance[o_nod->instances_available_for_node[i]][attrN] == i_of_var)
			n++;
	}
	n_nod->n_of_instances_available_for_node = n;
	n_nod->instances_available_for_node = (int*)e_malloc(sizeof(int) * n);
	for (size_t i = 0, j = 0; i < o_nod->n_of_instances_available_for_node; i++)
	{
		if (instance[o_nod->instances_available_for_node[i]][attrN] == i_of_var)
			n_nod->instances_available_for_node[j++] = o_nod->instances_available_for_node[i];
	}
}

int		pick_answer(rtree *nod)
{
	int *n = (int*)e_malloc(sizeof(int) * attribute[attributes_amount -1].vars_amount);
	for (size_t i = 0; i < attribute[attributes_amount - 1].vars_amount; i++)
		n[i] = 0;
	for (size_t i = 0; i < nod->n_of_instances_available_for_node; i++)
		n[instance[nod->instances_available_for_node[i]][attributes_amount - 1]]++;
	int max = 0;
	int i_of_max = 0;
	for (size_t i = 0; i < attribute[attributes_amount - 1].vars_amount; i++)
	{
		if (n[i] > max)
		{
			max = n[i];
			i_of_max = i;
		}
	}
	return i_of_max;
}

void	rt(rtree *nod)
{
	nod->attr_of_node = -1;
	nod->answer = -1;
	nod->leaf = NULL;
	int i_of_attribute_to_divide_by = get_i_of_attribute_to_divide_by(nod);
	if (i_of_attribute_to_divide_by == -1)
	{
		nod->answer = pick_answer(nod);
		return;
	}
	nod->attr_of_node = i_of_attribute_to_divide_by;
	nod->leaf = (rtree*)e_malloc(sizeof(rtree) * attribute[i_of_attribute_to_divide_by].vars_amount);
	for (size_t i = 0; i < attribute[i_of_attribute_to_divide_by].vars_amount; i++)
	{
		gen_new_attr_arr(nod, &(nod->leaf[i]), i_of_attribute_to_divide_by);
		gen_new_inst_arr(nod, &(nod->leaf[i]), i_of_attribute_to_divide_by, i);
		rt(&(nod->leaf[i]));
	}
}

void	show_tree(rtree *t, int deep)
{
	if (t->answer != -1)
		printf(" : %s", attribute[attributes_amount - 1].var_name[t->answer]);
	else if (t->leaf)
	{
		for (size_t i = 0; i < attribute[t->attr_of_node].vars_amount; i++)
		{
			printf("\n");
			for (size_t i = 0; i < deep; i++)
				printf("|\t");
			printf("%s = %s", attribute[t->attr_of_node].name, attribute[t->attr_of_node].var_name[i]);
			show_tree(&(t->leaf[i]), deep + 1);
		}
	}
}

bool	check_instance(rtree *nod, int i)
{
	while (nod)
	{
		if (nod->answer != -1)
		{
			if (test_instance[i][attributes_amount - 1] == nod->answer)
				return true;
			return false;
		}
		nod = &(nod->leaf[test_instance[i][nod->attr_of_node]]);
	}
	return false;
}

void	test(rtree *t)
{
	bool *instance_determined_correctly = (bool*)e_malloc(sizeof(bool) * test_instances_amount);
	for (size_t i = 0; i < test_instances_amount; i++)
	{
		instance_determined_correctly[i] = check_instance(t,i);
	}
	int corr_amount = 0;
	for (size_t i = 0; i < test_instances_amount; i++)
	{
		if (instance_determined_correctly[i])
			corr_amount++;
	}
	printf("\n\n\nDetermination success level:  %lf%%\n", (double)corr_amount / test_instances_amount * 100);
}

int		main(void)
{
	get_attributes();
	instances_amount = get_instances(learn_src,&instance);
	test_instances_amount = get_instances(test_src, &test_instance);

	rtree *res = (rtree*)e_malloc(sizeof(rtree));
	res->attr_of_node = -1;
	res->attributes_available_for_node = (int*)e_malloc(sizeof(int) * attributes_amount);
	for (size_t i = 0; i < attributes_amount; i++)
		res->attributes_available_for_node[i] = i;
	res->n_of_attributes_available_for_node = attributes_amount;
	res->instances_available_for_node = (int*)e_malloc(sizeof(int) * instances_amount);
	for (size_t i = 0; i < instances_amount; i++)
		res->instances_available_for_node[i] = i;
	res->n_of_instances_available_for_node = instances_amount;
	res->leaf = NULL;
	
	rt(res);

	//printf("\n\n");
	show_tree(res,0);

	test(res);

	printf("\n\n\n");
	system("pause");
	return (0);
}